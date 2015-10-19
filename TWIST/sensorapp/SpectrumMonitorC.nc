#include "spectrummonitor.h"
#include <stdio.h>
#include "message.h"
#include "app_profile.h"

//#define SPECTRUM_MONITOR_DEBUG

generic module SpectrumMonitorC(dwell_mode_t DWELL_MODE) {
  uses {
    interface Boot;
    interface Leds;
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
    interface Packet as SerialPacket;
  }
}
implementation {

  message_t *m_currentSweepMsgPtr;
  norace cb_sweep_data_msg_t *m_currentSweep;
  norace int8_t m_last_sweep_rssi[NUM_FREQUENCIES];
  uint16_t fvector[] = FREQUENCY_VECTOR;
  norace uint16_t findex;
  norace uint32_t m_seqno;
  norace bool m_overflow2;
  norace bool m_ctrlChannelListen;

  task void startRxControlTask();
  task void sendDataTask();
  void setNextSweepDataMsg();
  inline int8_t readRssiFast();

# define WDTCONFIG ( WDTCNTCL | WDTSSEL  )

  event void Boot.booted()
  {
    uint16_t i;
    WDTCTL = WDTPW | WDTCONFIG ;
    call Leds.set(0xFF);
    for (i=0; i<32767; i++)
      nop();
    call Leds.set(0);
    setNextSweepDataMsg();
    findex = m_seqno = 0;
    call SerialControl.start();
  }

  event void SerialControl.startDone(error_t err)
  {
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
    WDTCTL = WDTPW | WDTCONFIG ; // kick the watchdog
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

        call CC2420Power.rfOff();
        call CC2420Power.setFrequency(fvector[findex]);
        call CC2420Power.rxOn();
        start = call Alarm.getNow();
        while (call Alarm.getNow() < start+15)
          nop();

        rssi = readRssiFast() - 45;
        while (call Alarm.getNow() < start + DWELL_TIME)
        {
          tmp = readRssiFast()-45;
          switch (DWELL_MODE)
          {
            case MODE_MAX: if (tmp > rssi) rssi = tmp; break;
            case MODE_MIN: if (tmp < rssi) rssi = tmp; break;
          }
        }
/*        printf("%d ",rssi);*/
        m_currentSweep->rssi[findex] = rssi;
        m_last_sweep_rssi[findex] = rssi;


        if (findex == NUM_FREQUENCIES-1) {
          // finished a sweep: send result over serial and, for a while, tune into control channel
/*          printf("\n");*/
          post sendDataTask();

        } else {
          findex = findex + 1;
/*          call CC2420Power.rfOff();*/
/*          call CC2420Power.flushRxFifo();*/
/*          call CC2420Power.setFrequency(fvector[findex++]);*/
/*          call CC2420Power.rxOn();*/
        }
      }

        call Alarm.startAt(call Alarm.getNow(), SAMPLING_PERIOD);
    }
  }

  async event bool CC2420Rx.receive(uint8_t *data, uint16_t time, bool isTimeValid, int8_t rssi)
  {
    return FALSE;
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
      } else
        WDTCTL = WDTPW | WDTCONFIG ; // kick the watchdog
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

  async event void CC2420Tx.loadTXFIFODone(uint8_t *data, error_t error ) {
    call SpiResource.immediateRequest();
    call CC2420Tx.send();
  }

  async event void CC2420Tx.sendDone(uint8_t *data, uint16_t time, error_t error) {
    call CC2420Power.flushRxFifo();
    call CC2420Power.rxOn();
    if (error == SUCCESS)
      call Leds.led1Toggle();
    call Alarm.startAt(call Alarm.getNow(), SAMPLING_PERIOD); // continue with noise sampling
  }

  event void CC2420Tx.cancelDone(error_t error) {}
  async event void CC2420Tx.transmissionStarted ( uint8_t *data ) {}
  async event void CC2420Tx.transmittedSFD ( uint32_t time, uint8_t *data ) {}

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

