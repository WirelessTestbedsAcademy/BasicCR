/*
 * Copyright (c) 2005-2006 Arch Rock Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the Arch Rock Corporation nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * ARCHED ROCK OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE
 */

/**
 * @author Jonathan Hui <jhui@archrock.com>
 * @author David Moss
 * @author Jung Il Choi Initial SACK implementation
 * @author Jan Hauer <hauer@tkn.tu-berlin.de> (adapted interfaces)
 * @version $Revision: 1.1 $ $Date: 2007/07/04 00:37:16 $
 */

#include "CC2420.h"
#include "crc.h"
#include "message.h"

module CC2420TransmitP {

  provides interface Init;
  provides interface StdControl;
/*    interface CC2420Transmit;*/
  provides interface CC2420Tx;
/*  provides interface RadioBackoff;*/
/*  provides interface RadioTimeStamping as TimeStamp;*/
  provides interface ReceiveIndicator as EnergyIndicator;
  provides interface ReceiveIndicator as ByteIndicator;
  
  uses interface Alarm<T32khz,uint32_t> as BackoffTimer;
/*  uses interface Alarm<T62500hz,uint32_t> as AckAlarm;*/
/*  uses interface CC2420Packet;*/
/*  uses interface CC2420PacketBody;*/
  uses interface GpioCapture as CaptureSFD;
  uses interface GeneralIO as CCA;
  uses interface GeneralIO as CSN;
  uses interface GeneralIO as SFD;
  uses interface GeneralIO as FIFO;

  uses interface Resource as SpiResource;
  uses interface ChipSpiResource;
  uses interface CC2420Fifo as TXFIFO;
  uses interface CC2420Ram as TXFIFO_RAM;
  uses interface CC2420Register as TXCTRL;
  uses interface CC2420Register as RSSI;
  uses interface CC2420Strobe as SNOP;
  uses interface CC2420Strobe as STXON;
  uses interface CC2420Strobe as STXONCCA;
  uses interface CC2420Strobe as SFLUSHTX;
  uses interface CC2420Strobe as SRXON;
  uses interface CC2420Strobe as SRFOFF;
  uses interface CC2420Strobe as SFLUSHRX;
  uses interface CC2420Strobe as SACK; 
  uses interface CC2420Register as MDMCTRL1;
/*  uses interface CaptureTime;*/

  uses interface CC2420Receive;
  uses interface Leds;
}

implementation {

  typedef enum {
    S_STOPPED,
    S_STARTED,
    S_LOAD,
    S_SAMPLE_CCA,
    S_BEGIN_TRANSMIT,
    S_SFD,
    S_EFD,
    S_ACK_WAIT,
    S_CANCEL,
  } cc2420_transmit_state_t;

  // This specifies how many jiffies the stack should wait after a
  // TXACTIVE to receive an SFD interrupt before assuming something is
  // wrong and aborting the send. There seems to be a condition
  // on the micaZ where the SFD interrupt is never handled.
  enum {
    CC2420_ABORT_PERIOD = 320
  };
  
/*  norace message_t *m_msg;*/
  norace uint8_t *m_data;
  norace uint8_t m_txFrameLen;
  norace bool m_lockOnFail;
  norace uint16_t m_time;
  
  norace bool m_cca;
  
  norace uint8_t m_tx_power;
  
  cc2420_transmit_state_t m_state = S_STOPPED;
  
  bool m_receiving = FALSE;
  
  uint16_t m_prev_time;
  typedef enum {
    TS_IDLE,
    TS_LOADED,
    TS_INVALID,
  } timestamp_state_t;
  timestamp_state_t m_timestampState;  

  /** Byte reception/transmission indicator */
  bool sfdHigh;
  
  norace bool m_tx_power_changed;

  /** Let the CC2420 driver keep a lock on the SPI while waiting for an ack */
  norace bool abortSpiRelease;
  
  /** Total CCA checks that showed no activity before the NoAck LPL send */
  norace int8_t totalCcaChecks;
  
  /** The initial backoff period */
  norace uint16_t myInitialBackoff;
  
  /** The congestion backoff period */
  norace uint16_t myCongestionBackoff;
  

  /***************** Prototypes ****************/
  error_t resend( bool cca );
  void loadTXFIFO();
  error_t acquireSpiResource();
  error_t releaseSpiResource();
  
  
  /***************** Init Commands *****************/
  command error_t Init.init() {
    call CCA.makeInput();
    call CSN.makeOutput();
    call SFD.makeInput();
    m_tx_power = CC2420_DEF_RFPOWER;
    m_tx_power_changed = FALSE;
    return SUCCESS;
  }

  /***************** StdControl Commands ****************/
  command error_t StdControl.start() {
    atomic {
      call CaptureSFD.captureRisingEdge();
      m_state = S_STARTED;
      m_timestampState = TS_IDLE;
      m_receiving = FALSE;
    }
    return SUCCESS;
  }

  command error_t StdControl.stop() {
    atomic {
      m_state = S_STOPPED;
      call CaptureSFD.disable();
    }
    return SUCCESS;
  }


  /**************** Send Commands ****************/

  async command error_t CC2420Tx.loadTXFIFO(uint8_t *data) {
    atomic {
      if (m_state == S_CANCEL) {
        return ECANCEL;
      }

      if ( m_state != S_STARTED ) {
        return FAIL;
      }

      m_state = S_LOAD;
      m_data = data;
    }

    if ( acquireSpiResource() == SUCCESS ) {
      loadTXFIFO();
    }

    return SUCCESS;
  }

  void loadTXFIFO() {
    call CSN.clr();
    // flush out what was there before
    call SFLUSHTX.strobe();
    call TXFIFO.write( m_data, m_data[0] - 1 );
  }

  async event void TXFIFO.writeDone( uint8_t* tx_buf, uint8_t tx_len,
                                     error_t error ) 
  {
    call CSN.set();
    m_state = S_BEGIN_TRANSMIT;
    releaseSpiResource();
    signal CC2420Tx.loadTXFIFODone(m_data, error);
  }

  async command void CC2420Tx.send()
  {
    // we are owner of the Spi !
    atomic {
      m_state = S_SFD;
      call CSN.clr();
      call STXON.strobe(); // w/o CCA
      call CSN.set(); 
      return;
    }
  }

  async command void CC2420Tx.sendImmediate()
  {
    // we are owner of the Spi !
    atomic {
      // don't set the state !
      // m_state = S_SFD;
      m_timestampState = TS_INVALID; // SFD is not reliable anymore
      call CSN.clr();
      call STXON.strobe(); // w/o CCA
      call CSN.set();
      return;
    }
  }

  async command bool CC2420Tx.cca()
  {
    return call CCA.get();
  }

  /*
  async command bool CC2420Tx.isTxActive()
  {
    cc2420_status_t status;
    atomic {
      call CSN.clr();
      status = call SNOP.strobe(); // w/o CCA
      call CSN.set(); 
    }
    if ( status & CC2420_STATUS_TX_ACTIVE )
      return TRUE;
    else
      return FALSE;
  }
  */
  
  async command error_t CC2420Tx.rssi(int8_t *rssi) {
    // we are owner of the Spi !
    uint16_t data;
    cc2420_status_t status;
    call CSN.set();
    call CSN.clr();
    status = call RSSI.read(&data);
    call CSN.set();
    if ((status & 0x02)){
      *rssi = (data & 0x00FF);
      return SUCCESS;
    } else
      return FAIL;
  }

  async command error_t CC2420Tx.rxOn()
  {
    atomic {
      if (!call SpiResource.isOwner()) {
        call Leds.led0On();
        return FAIL;
      }
      call CSN.set();
      call CSN.clr();
      call SRXON.strobe();
      call CSN.set();
      return SUCCESS;
    }
  }

  async command error_t CC2420Tx.rfOff()
  {
    atomic {
      if (!call SpiResource.isOwner()) {
        call Leds.led0On();
        return FAIL;
      }
      call CSN.set();
      call CSN.clr();
      call SRFOFF.strobe();
      call CSN.set();
    }
    return SUCCESS;    
  }

  async command error_t CC2420Tx.immediateSpiRequest()
  {
    return call SpiResource.immediateRequest();
  }

  async command error_t CC2420Tx.releaseSpi()
  {
    return call SpiResource.release();
  }

  async command error_t CC2420Tx.cancel() 
  {
    return FAIL;
  }

  async command error_t CC2420Tx.modify( uint8_t offset, uint8_t* buf, 
                                     uint8_t len ) {
    call CSN.clr();
    call TXFIFO_RAM.write( offset, buf, len );
    call CSN.set();
    return SUCCESS;
  }
  
  async command void CC2420Tx.lockChipSpi()
  {
    abortSpiRelease = TRUE;
  }
  async command void CC2420Tx.unlockChipSpi()
  {
    abortSpiRelease = FALSE;
  }

  /***************** Indicator Commands ****************/
  command bool EnergyIndicator.isReceiving() {
    return !(call CCA.get());
  }
  
  command bool ByteIndicator.isReceiving() {
    bool high;
    atomic high = sfdHigh;
    return high;
  }

  /**
   * The CaptureSFD event is actually an interrupt from the capture pin
   * which is connected to timing circuitry and timer modules.  This
   * type of interrupt allows us to see what time (being some relative value)
   * the event occurred, and lets us accurately timestamp our packets.  This
   * allows higher levels in our system to synchronize with other nodes.
   *
   * Because the SFD events can occur so quickly, and the interrupts go
   * in both directions, we set up the interrupt but check the SFD pin to
   * determine if that interrupt condition has already been met - meaning,
   * we should fall through and continue executing code where that interrupt
   * would have picked up and executed had our microcontroller been fast enough.
   */

  task void sendDoneTask()
  {
    atomic m_state = S_STARTED;
    signal CC2420Tx.sendDone(m_data, m_time, SUCCESS);
  }

  async event void CaptureSFD.captured( uint16_t time ) {
    // assumption: either Tx.send is called while the radio if off, or
    // radio is switched to Rx but then no Tx.send is called afterwards
    bool validRxTimestamp = TRUE;
    atomic {
      switch( m_state ) {

      case S_SFD:
        m_time = time;
        m_state = S_EFD;
        sfdHigh = TRUE;
        call CaptureSFD.captureFallingEdge();
        if ( call SFD.get() ) {
          break;
        }
        /** Fall Through because the next interrupt was already received */

      case S_EFD:
        m_state = S_ACK_WAIT;
        sfdHigh = FALSE;
        call CaptureSFD.captureRisingEdge();
        post sendDoneTask();

        if ( !call SFD.get() ) {
          break;
        }
        /** Fall Through because the next interrupt was already received */
        validRxTimestamp = FALSE; 

      default:
        // JH: we want incoming ACKs to be timestamped and need to change this a little
        // Problems with the original code:
        // (1) if send is called while receiving a packet, then the SFD states is mixed up...
        // (2) if after captureFallingEdge() but before SFD.get() the SFD (EFD) occurs, states
        //     are mixed up...
        // (3) in the fall through case the timestamp is wrong
        //
        // We are very conservative, since we only need a few Rx timestamps as anchros, i.e.
        // we drop a lot of timestamps even if they might be valid. but the few we take
        // are valid.
        if (!validRxTimestamp)
          m_timestampState = TS_INVALID;
        if ( !m_receiving ) {
          sfdHigh = TRUE;
          call CaptureSFD.captureFallingEdge();
          m_receiving = TRUE;
          if (m_timestampState == TS_IDLE && call SFD.get()){
            m_prev_time = time;
            m_timestampState = TS_LOADED;
          }
          if ( call SFD.get() ) {
            // wait for the next interrupt before moving on
            return;
          }
        }
        
        sfdHigh = FALSE;
        call CaptureSFD.captureRisingEdge();
        m_receiving = FALSE;
        if (m_timestampState == TS_LOADED){
          if (!call SFD.get() && ((time - m_prev_time) > 2))
            call CC2420Receive.efd( m_prev_time );
          m_timestampState = TS_INVALID;
        }
        //if ( time - m_prev_time < 10 ) {
        //  call CC2420Receive.sfd_dropped();
        // }
        break;
      
      }
    }

  }

  /***************** ChipSpiResource Events ****************/
  async event void ChipSpiResource.releasing() {
    if(abortSpiRelease) {
      call ChipSpiResource.abortRelease();
    }
  }

  async event void CC2420Receive.receive(  uint8_t type, message_t *ackFrame ){ }

  /***************** SpiResource Events ****************/
  event void SpiResource.granted() {
    atomic {
      switch( m_state ) {
        case S_LOAD:
          loadTXFIFO();
          break;
        default:
          call Leds.led0On();
          releaseSpiResource();
          break;
      }
    }
  }

  error_t acquireSpiResource() {
    error_t error = call SpiResource.immediateRequest();
    if ( error != SUCCESS ) {
      call SpiResource.request();
    }
    return error;
  }

  error_t releaseSpiResource() {
    call SpiResource.release();
    return SUCCESS;
  }


  async event void BackoffTimer.fired() {}
  async event void TXFIFO.readDone( uint8_t* tx_buf, uint8_t tx_len, 
      error_t error ) {
  }

  default async event void CC2420Tx.sendDone(uint8_t *data, uint16_t time, error_t error){}
  default async event void CC2420Tx.loadTXFIFODone(uint8_t *data, error_t error ){}
}

