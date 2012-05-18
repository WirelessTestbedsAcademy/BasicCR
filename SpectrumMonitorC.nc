#include "spectrummonitor.h"
#include <stdio.h>
#include <UserButton.h>
#include "message.h"

module SpectrumMonitorC {
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
  norace uint32_t tlast;
  norace uint32_t m_seqno;
  norace bool m_overflow2;

  norace bool m_isChannelMaskMsgReady;
  norace uint8_t m_channelMaskMpdu[sizeof(header_154_t) + 1 + sizeof(cb_channelmask_msg_t) + 2]; // extra 1 for AM ID, extra 2 for MAC CRC

  message_t m_repoQueryMsg;
  cb_repo_query_msg_t *m_repoQuery;

  task void startRxControlTask();
  task void sendDataTask();
  void setNextSweepDataMsg();
  inline int8_t readRssiFast();
  task void sendRepoQueryTask();

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
    call UserButton.enable();
    call Leds.led2On();
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
    call Leds.led2Off();

    tlast = call Alarm.getNow();
    call Alarm.startAt(tlast, SAMPLING_PERIOD);
  }

  async event void Alarm.fired()
  {
    uint32_t now = call Alarm.getNow();

    atomic {

      if (findex == NUM_FREQUENCIES)
        m_overflow2 = TRUE;
      else {

        if (now>32767 && now > tlast + SAMPLING_PERIOD + 1)
          m_currentSweep->errorcode |= OVERFLOW1_ERROR;

        if (m_overflow2) {
          m_overflow2 = FALSE;
          m_currentSweep->errorcode |= OVERFLOW2_ERROR;
        }

        m_currentSweep->rssi[findex] = readRssiFast() - 45;

        call CC2420Power.rfOff();
        call CC2420Power.setFrequency(fvector[findex++]);
        call CC2420Power.rxOn();

        if (findex == NUM_FREQUENCIES)
          post sendDataTask();
      }

      tlast += SAMPLING_PERIOD;

      if (m_isChannelMaskMsgReady) { // send a packet now!
        call CC2420Power.rfOff();
        call SpiResource.release(); 
        if (call CC2420Tx.loadTXFIFO(m_channelMaskMpdu) != SUCCESS) {
          call Leds.led0On();
          call SpiResource.immediateRequest();
          call CC2420Power.rxOn();
        }
      } else
        call Alarm.startAt(tlast, SAMPLING_PERIOD);
    }
  }

  task void sendDataTask()
  {
    atomic {
      if (call SendSweepData.send(AM_BROADCAST_ADDR, m_currentSweepMsgPtr, sizeof(cb_sweep_data_msg_t)) != SUCCESS) {
        if (call Queue.enqueue(m_currentSweepMsgPtr) != SUCCESS)
          call Leds.led0On();
      } else 
        call Leds.led2Toggle();
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
    header_154_t *header = (header_154_t*) &m_channelMaskMpdu;
    uint8_t *amid = (uint8_t*) (((uint8_t*) header));
    cb_channelmask_msg_t *channelmask_msg = (cb_channelmask_msg_t*) (((uint8_t*) header)+1);

    if (len != sizeof(cb_channelmask_msg_t) || m_isChannelMaskMsgReady) {
      call Leds.led0On();
      return bufPtr;
    }
    m_isChannelMaskMsgReady = TRUE;

    header->length = sizeof(m_channelMaskMpdu) - 1;
    header->fcf = FRAME_TYPE_DATA | (DEST_MODE_SHORT << 8);
    header->dsn = 0;
    header->destpan = BROADCAST_ADDRESS;
    header->dest = BROADCAST_ADDRESS;

    *amid = AM_CB_CHANNELMASK_MSG;
    memcpy(channelmask_msg, payload, len);

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
    call Alarm.startAt(tlast, SAMPLING_PERIOD); // continue with noise sampling
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
       m_repoQuery->forwarder = TOS_NODE_ID;   // TWIST node ID
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
    call UserButton.enable();
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

