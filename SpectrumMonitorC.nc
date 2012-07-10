#include "spectrummonitor.h"
#include <stdio.h>
#include <UserButton.h>
#include "message.h"

generic module SpectrumMonitorC(dwell_mode_t DWELL_MODE) {
  uses {
    interface Boot;
    interface Leds;
    interface Notify<button_state_t> as UserButton;
    interface Queue<message_t*>;
    interface Pool<message_t>;
    interface Alarm<T32khz,uint32_t>;

    /* radio */
    interface Resource as SpiResource;
    interface CC2420Power;
    interface StdControl as TxControl;
    interface AsyncSplitControl as RxControl;
    interface CC2420Tx;
    interface CC2420Rx;      

    /* serial */
    interface SplitControl as SerialControl;
    interface AMSend as SendSweepData;
    interface AMSend as SendRepoQuery;
    interface Receive as ReceiveChannelMask;
    interface Packet as SerialPacket;
  }
}
implementation {

  message_t *m_currentSweepMsgPtr;
  norace cb_sweep_data_msg_t *m_currentSweep;
  uint16_t fvector[] = FREQUENCY_VECTOR;
  norace uint16_t findex;
  norace uint32_t m_seqno;
  norace bool m_overflow2;
  norace bool m_ctrlChannelListen;

  norace bool m_isChannelMaskMsgReady;
  norace uint8_t m_channelMaskMpdu[sizeof(header_154_t) - 1 + sizeof(cb_channelmask_msg_t) + 2]; // extra 2 for MAC CRC

  message_t m_repoQueryMsg;
  cb_repo_query_msg_t *m_repoQuery;

  task void startRxControlTask();
  task void sendDataTask();
  void setNextSweepDataMsg();
  inline int8_t readRssiFast();
  task void sendRepoQueryTask();
  task void forwardRepoQueryMsgTask();

  event void Boot.booted() 
  {
    setNextSweepDataMsg();
    findex = m_seqno = 0;
    m_repoQuery = (cb_repo_query_msg_t*) call SerialPacket.getPayload(&m_repoQueryMsg, sizeof(cb_repo_query_msg_t));
    if (m_repoQuery == NULL)
      call Leds.led0On();
    call SerialControl.start();
  }

  event void SerialControl.startDone(error_t err) 
  {
/*    call UserButton.enable();*/
    call SpiResource.request();
  }

  event void SpiResource.granted() 
  {
    call CC2420Power.startVReg(); 
  }

  async event void CC2420Power.startVRegDone()
  {
    call CC2420Power.startOscillator();
  }

  async event void CC2420Power.startOscillatorDone()
  {
    call CC2420Power.setFrequency(fvector[findex]);
    post startRxControlTask();
  }

  task void startRxControlTask()
  {
    error_t error;
    call TxControl.start();
    call SpiResource.release(); // need to be available for RxControl... 
    error = call RxControl.start();
    if (error != SUCCESS)
      call Leds.led0On();
  }

  async event void RxControl.startDone(error_t error)
  {
    // RadioRx component is ready for Rx in unbuffered mode!
    if (error != SUCCESS)
      call Leds.led0On();
    if (call SpiResource.immediateRequest() != SUCCESS)
      call Leds.led0On();

    call CC2420Power.flushRxFifo();
    call CC2420Power.rxOn();

    call Alarm.start(SAMPLING_PERIOD);
  }

  async event void Alarm.fired()
  {
    uint32_t start = call Alarm.getNow();
    int8_t rssi,tmp;

    atomic {
      
      if (m_ctrlChannelListen) {
        if (call SpiResource.immediateRequest() != SUCCESS) {
          call Alarm.start(32); // spin
          return;
        }
        m_ctrlChannelListen = FALSE;
        call CC2420Power.rfOff();
        call CC2420Power.flushRxFifo();
        call CC2420Power.setFrequency(fvector[findex]);
        call CC2420Power.rxOn();       
        call Alarm.start(32); // some time for the radio to get ready for RSSI sampling
        return;
      }

      if (findex == NUM_FREQUENCIES)
        m_overflow2 = TRUE;
      else {

        if (start>32767 && start > call Alarm.getNow() + SAMPLING_PERIOD + 1)
          m_currentSweep->errorcode |= OVERFLOW1_ERROR;

        if (m_overflow2) {
          m_overflow2 = FALSE;
          m_currentSweep->errorcode |= OVERFLOW2_ERROR;
        }

        rssi = readRssiFast();
        while (call Alarm.getNow() < start + DWELL_TIME)
        {
          tmp = readRssiFast();
          switch (DWELL_MODE)
          {
            case MODE_MAX: if (tmp > rssi) rssi = tmp; break;
            case MODE_MIN: if (tmp < rssi) rssi = tmp; break;
          }
        }
        m_currentSweep->rssi[findex] = rssi - 45;

        if (findex == NUM_FREQUENCIES-1) {
          // finished a sweep: send result over serial and, for a while, tune into control channel
          post sendDataTask();
          call CC2420Power.rfOff();
          call CC2420Power.setFrequency(CONTROL_CHANNEL_FREQUENCY);
          call CC2420Power.rxOn();
          call SpiResource.release();
          m_ctrlChannelListen = TRUE;
          call Alarm.start(CONTROL_CHANNEL_LISTEN_TIME);
          return;
        } else {
          call CC2420Power.rfOff();
          call CC2420Power.flushRxFifo();
          call CC2420Power.setFrequency(fvector[findex++]);
          call CC2420Power.rxOn();
        }
      }

      if (m_isChannelMaskMsgReady) { // send a packet now!
        call CC2420Power.rfOff();
        call CC2420Power.setFrequency(CONTROL_CHANNEL_FREQUENCY);
        call SpiResource.release(); 
        if (call CC2420Tx.loadTXFIFO(m_channelMaskMpdu) != SUCCESS) {
          call Leds.led0On();
          call SpiResource.immediateRequest();
          call CC2420Power.rxOn();
        }
      } else
        call Alarm.startAt(call Alarm.getNow(), SAMPLING_PERIOD);
    }
  }

  norace cb_repo_query_msg_t m_repoQueryPayload;
  async event bool CC2420Rx.receive(uint8_t *data, uint16_t time, bool isTimeValid, int8_t rssi)
  {
    // reveived a message over the radio
    uint16_t len = data[0];
    call Leds.led2Toggle();
    if (len == 13 + sizeof(cb_repo_query_msg_t)) { // 13 is the MAC overhead for our frames
      memcpy(&m_repoQueryPayload, &data[12], sizeof(cb_repo_query_msg_t));
      m_repoQueryPayload.forwarderRSSI = rssi;
      post forwardRepoQueryMsgTask();
    }
    return FALSE;
  }

  task void forwardRepoQueryMsgTask()
  {
    // fake a cb_repo_query_msg_t
    m_repoQuery->forwarderID = m_repoQueryPayload.forwarderID;   // TWIST node ID
    m_repoQuery->forwarderRSSI = m_repoQueryPayload.forwarderRSSI;   // TWIST node ID
    m_repoQuery->srcID = m_repoQueryPayload.srcID;      // BAN src ID
    m_repoQuery->srcPANID = m_repoQueryPayload.srcPANID;   // BAN src PAN ID
    m_repoQuery->mode = m_repoQueryPayload.mode;        // type of request: MODE_CHANNEL_MASK etc.
    if (call SendRepoQuery.send(AM_BROADCAST_ADDR, &m_repoQueryMsg, sizeof(cb_repo_query_msg_t)) != SUCCESS) {
      post forwardRepoQueryMsgTask();
      call Leds.led0Toggle();
    }
  }

  task void sendDataTask()
  {
    atomic {
      if (m_currentSweepMsgPtr != NULL) {
        cb_sweep_data_msg_t *s = call SerialPacket.getPayload(m_currentSweepMsgPtr, sizeof(cb_sweep_data_msg_t));
        if (s != NULL)
          s->src = TOS_NODE_ID;
      }
      if (call SendSweepData.send(AM_BROADCAST_ADDR, m_currentSweepMsgPtr, sizeof(cb_sweep_data_msg_t)) != SUCCESS) {
        if (call Queue.enqueue(m_currentSweepMsgPtr) != SUCCESS)
          call Leds.led0On();
      }
      setNextSweepDataMsg();
      findex = 0;
    }
  }

  event void SendSweepData.sendDone(message_t* bufPtr, error_t error) {
    if (error != SUCCESS) 
      call Leds.led0On();
    call Pool.put(bufPtr);
    if (!call Queue.empty())
      if (call SendSweepData.send(AM_BROADCAST_ADDR, call Queue.dequeue(), sizeof(cb_sweep_data_msg_t)) != SUCCESS) 
        call Leds.led0On();
  }

  event message_t* ReceiveChannelMask.receive(message_t* bufPtr, 
        void* payload, uint8_t len) 
  {
    // received channel mask reply over serial line
    header_154_t *header = (header_154_t*) &m_channelMaskMpdu;
    uint8_t *msdu = (uint8_t *) header + sizeof(header_154_t);

    if (len != sizeof(cb_channelmask_msg_t) || m_isChannelMaskMsgReady) {
      call Leds.led0On();
      return bufPtr;
    }
    m_isChannelMaskMsgReady = TRUE;

    header->length = sizeof(m_channelMaskMpdu);
    header->fcf = FRAME_TYPE_DATA | (DEST_MODE_SHORT << 8);
    header->dsn = 0;
    header->destpan = BROADCAST_ADDRESS;
    header->dest = BROADCAST_ADDRESS;

    memcpy(msdu, payload, len);

    // message will be sent from Alarm eventhandler above, 
    // which notices that m_isChannelMaskMsgReady is TRUE

    return bufPtr;
  }

  async event void CC2420Tx.loadTXFIFODone(uint8_t *data, error_t error ) {
    call SpiResource.immediateRequest();
    call CC2420Tx.send();
  }

  async event void CC2420Tx.sendDone(uint8_t *data, uint16_t time, error_t error) {
    m_isChannelMaskMsgReady = FALSE;
    call CC2420Power.flushRxFifo();
    call CC2420Power.rxOn(); 
    if (error == SUCCESS)
      call Leds.led1Toggle();
    call Alarm.startAt(call Alarm.getNow(), SAMPLING_PERIOD); // continue with noise sampling
  }

  event void CC2420Tx.cancelDone(error_t error) {}
  async event void CC2420Tx.transmissionStarted ( uint8_t *data ) {}
  async event void CC2420Tx.transmittedSFD ( uint32_t time, uint8_t *data ) {}

  /* DEBUGGING: if you press the user button then a cb_repo_query_msg 
   * will be created and sent over the serial line. */

  event void UserButton.notify(button_state_t val)
  {
     if (val == BUTTON_PRESSED) {
       // fake a cb_repo_query_msg_t
       call UserButton.disable();
       m_repoQuery->forwarderID = TOS_NODE_ID;   // TWIST node ID
       m_repoQuery->srcID = 17;      // BAN src ID
       m_repoQuery->srcPANID = 1234;   // BAN src PAN ID
       m_repoQuery->mode = 1;        // type of request: MODE_CHANNEL_MASK etc.
       if (call SendRepoQuery.send(AM_BROADCAST_ADDR, &m_repoQueryMsg, sizeof(cb_repo_query_msg_t)) != SUCCESS)
         post sendRepoQueryTask();
     }
  }

  task void sendRepoQueryTask()
  {
    if (call SendRepoQuery.send(AM_BROADCAST_ADDR, &m_repoQueryMsg, sizeof(cb_repo_query_msg_t)) != SUCCESS)
      post sendRepoQueryTask();
/*    call UserButton.enable();*/
  }

  event void SendRepoQuery.sendDone(message_t* bufPtr, error_t error) {}

  inline int8_t readRssiFast()
  {
    // for a register read: after writing the address byte, two bytes are
    // read (we provide the clock by sending two dummy bytes)
    // this now takes only 13 us !
    int8_t rssi;
    //return call CC2420Power.rssiFast(); 
    P4OUT &= ~0x04;      // clear CSN, CS low
    // write address 0x53  (0x40 for register read, 0x13 for RSSI register address)
    U0TXBUF = 0x53;
    // wait until data has moved from UxTXBUF to the TX shift register 
    // and UxTXBUF is ready for new data. It doesnot indicate RX/TX completion.
    while (!(IFG1 & UTXIFG0))  
      ;
    U0TXBUF = 0;
    while (!(IFG1 & UTXIFG0))  
      ;
    U0TXBUF = 0;
    while (!(IFG1 & UTXIFG0))  
      ;
    while (!(U0TCTL & TXEPT))
      ;
    rssi = U0RXBUF;
    P4OUT |= 0x04;      // CS high
    return rssi; 
  }

  void setNextSweepDataMsg()
  {
    m_currentSweepMsgPtr = call Pool.get();
    if (m_currentSweepMsgPtr == NULL) {
      call Leds.led0On();
      return;
    }
    m_currentSweep = call SerialPacket.getPayload(m_currentSweepMsgPtr, sizeof(m_currentSweep));
    if (m_currentSweep == NULL) {
      call Leds.led0On();
      return;
    }
    m_currentSweep->seqno = m_seqno++;
  }

  async event void RxControl.stopDone(error_t error) { call Leds.led0On(); }
  event void SerialControl.stopDone(error_t err) {}
}

