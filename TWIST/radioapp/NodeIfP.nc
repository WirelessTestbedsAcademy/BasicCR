/*
 *   NodeIF: Node InterFace, a tinyOS(c) software to command telosB-family nodes
 *   as radio interfaces from one Java process.
 *
 *   Copyright (C) 2014  M. Onur Ergin - monurergin@gmail.com
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program, in the file <license-gpl-3.0.txt>.
 *   If not, see <http://www.gnu.org/licenses/>.
 *
 */

// $Id: NodeIf.nc,v 1.10 2008/06/23 20:25:14 regehr Exp $
// Author: Onur Ergin

/*
 * NodeIfP allows programmers to use the nodes as a Radio interface.
 * Uses message queueing function from BaseStation(c) application.
 */

#include "CC2420.h"

#include "AM.h"
#include "Serial.h"

//#include "printf.h"
	#define printf(...) ;
	#define printfflush(...) ;

#define RSSI_OFFSET (-45)
#include "NodeIfMessages.h"


module NodeIfP @safe() {
  uses {
    interface Boot;
    interface SplitControl as SerialControl;
    interface SplitControl as RadioControl;

    interface AMSend as UartSend[am_id_t id];
    interface Receive as UartReceive[am_id_t id];
    interface Packet as UartPacket;
    interface AMPacket as UartAMPacket;

    interface AMSend as RadioSend[am_id_t id];
    interface Receive as RadioReceive[am_id_t id];
    interface Receive as RadioSnoop[am_id_t id];
    interface Packet as RadioPacket;
    interface AMPacket as RadioAMPacket;

    interface CC2420Packet;

    interface Leds;

  }

  uses {
	interface GeneralIO as CSN;

	interface Resource as SpiResource;

	interface CC2420Ram as TXFIFO_RAM;
	interface CC2420Register as TXCTRL;
	interface CC2420Strobe as SNOP;
	interface CC2420Strobe as STXON;
	interface CC2420Strobe as STXONCCA;
	interface CC2420Strobe as SFLUSHTX;
	interface CC2420Register as MDMCTRL1;
	interface CC2420Register as FSCTRL;

	interface CC2420Strobe as SRFOFF;
	interface CC2420Strobe as SRXON;
  }

}

implementation
{

  enum { APPID = 132 };

  norace uint16_t m_tx_power = CC2420_DEF_RFPOWER;
  norace uint16_t curChannel = CC2420_DEF_CHANNEL;

  message_t pkt;


  /* Prototypes */
  message_t *addUartQ   (message_t *);
  message_t *addRadioQ  (message_t *);
  uint16_t setTxPower   (uint8_t);
  uint16_t setTxChannel (uint16_t);
  uint16_t getTxChannel ();
  uint16_t getTxPower   ();
  uint16_t ch2freq      (uint16_t);

  message_t  uartQueueBufs[UART_QUEUE_LEN];
  message_t  * ONE_NOK uartQueue[UART_QUEUE_LEN];
  uint8_t    uartIn, uartOut;
  bool       uartBusy, uartFull;

  message_t  radioQueueBufs[RADIO_QUEUE_LEN];
  message_t  * ONE_NOK radioQueue[RADIO_QUEUE_LEN];
  uint8_t    radioIn, radioOut;
  bool       radioBusy, radioFull;

  task void uartSendTask();
  task void radioSendTask();

  void dropBlink() {
    call Leds.led2Toggle();
  }

  void failBlink() {
    call Leds.led2Toggle();
  }

  event void Boot.booted() {
    uint8_t i;

    for (i = 0; i < UART_QUEUE_LEN; i++)
      uartQueue[i] = &uartQueueBufs[i];
    uartIn = uartOut = 0;
    uartBusy = FALSE;
    uartFull = TRUE;

    for (i = 0; i < RADIO_QUEUE_LEN; i++)
      radioQueue[i] = &radioQueueBufs[i];
    radioIn = radioOut = 0;
    radioBusy = FALSE;
    radioFull = TRUE;

    call RadioControl.start();
    call SerialControl.start();
  }

  event void RadioControl.startDone(error_t error) {
    if (error == SUCCESS) {
      radioFull = FALSE;
    }
  }

  event void SerialControl.startDone(error_t error) {
    if (error == SUCCESS) {
      uartFull = FALSE;
    }
  }

  event void SerialControl.stopDone(error_t error) {}
  event void RadioControl.stopDone(error_t error) {}

  uint8_t count = 0;

  message_t* ONE receive(message_t* ONE msg, void* payload, uint8_t len);

  event message_t *RadioSnoop.receive[am_id_t id](message_t *msg, void *payload, uint8_t len) {
    return receive(msg, payload, len);
  }

  event message_t *RadioReceive.receive[am_id_t id](message_t *msg, void *payload, uint8_t len) {
    return receive(msg, payload, len);
  }

  message_t* receive(message_t *msg, void *payload, uint8_t len) {
    message_t *ret = msg;

	nodeifMsg *incoming = (nodeifMsg*)(call RadioPacket.getPayload(ret, (int) NULL));

	if (incoming->appid != APPID) return ret;
	incoming->receiverid = TOS_NODE_ID;
	incoming->nodeid = TOS_NODE_ID;
	incoming->rssi = call CC2420Packet.getRssi(ret) + RSSI_OFFSET;
	incoming->lqi = call CC2420Packet.getLqi(ret);
	// incoming->txpower = m_tx_power; // bu uartReceive'de gönderici tarafindan set edilsin
	incoming->channel = curChannel; // ekle.
	ret = addUartQ(msg);

    return ret;
  }

  task void uartSendTask() {
    uint8_t len;
    am_id_t id;
    am_addr_t addr, src;
    message_t* msg;
    atomic
      if (uartIn == uartOut && !uartFull)
	{
	  uartBusy = FALSE;
	  return;
	}

    msg = uartQueue[uartOut];
    len = sizeof(nodeifMsg);
    id = call RadioAMPacket.type(msg);
    addr = call RadioAMPacket.destination(msg);
    src = call RadioAMPacket.source(msg);
    call UartPacket.clear(msg);
    call UartAMPacket.setSource(msg, src);

    if (call UartSend.send[id](addr, uartQueue[uartOut], len) == SUCCESS)
      call Leds.led1Toggle();
    else
      {
	failBlink();
	post uartSendTask();
      }
  }

  event void UartSend.sendDone[am_id_t id](message_t* msg, error_t error) {
    if (error != SUCCESS)
      failBlink();
    else
      atomic
	if (msg == uartQueue[uartOut])
	  {
	    if (++uartOut >= UART_QUEUE_LEN)
	      uartOut = 0;
	    if (uartFull)
	      uartFull = FALSE;
	  }
    post uartSendTask();
  }

  event message_t *UartReceive.receive[am_id_t id](message_t *msg,
						   void *payload,
						   uint8_t len) {
    message_t *ret = msg;
    uint16_t value;

	nodeifMsg *incoming = (nodeifMsg*)(call RadioPacket.getPayload(msg, (int) NULL));

	if (incoming->appid != APPID) // || incoming->nodeid == TOS_NODE_ID )// || incoming->senderid == TOS_NODE_ID )
			return ret;

	incoming->nodeid = TOS_NODE_ID;
	switch(incoming->pcktype) {
		case SETTX:
			setTxPower(incoming->value);
			value = getTxPower();
			incoming->value = value;
			ret = addUartQ(msg);
			break;
		case GETTX:
			value = getTxPower();
			incoming->value = value;
			ret = addUartQ(msg);
			break;
		case SETFREQ:
			setTxChannel(incoming->value);
			value = getTxChannel();
			incoming->value = value;
			ret = addUartQ(msg);
			break;
		case GETFREQ:
			value = getTxChannel();
			incoming->value = value;
			ret = addUartQ(msg);
			break;
		case DATA:
		    incoming->senderid = TOS_NODE_ID;
		    incoming->txpower = m_tx_power;
		    incoming->channel = curChannel;
			ret = addRadioQ(msg);	// ONLY DATA MESSAGE IS SENT TO RADIO
			break;
		case IDQUERY:
		    incoming->nodeid = TOS_NODE_ID;
		    ret = addUartQ(msg);
			break;
		default:
			break;
	} // switch

    return ret;
  }

  /* Adds the message to the Uart Queue
   * Returns a message pointer
   */
  message_t *addUartQ (message_t *tMsg) {
    message_t *ret = tMsg;

    atomic {
		if (!uartFull)
		{
		  ret = uartQueue[uartIn];
		  uartQueue[uartIn] = tMsg;

		  uartIn = (uartIn + 1) % UART_QUEUE_LEN;

		  if (uartIn == uartOut)
			uartFull = TRUE;

		  if (!uartBusy)
			{
			  post uartSendTask();
			  uartBusy = TRUE;
			}
		}
		else {
			dropBlink();
		}
	} // atomic

	return ret;
  }

  /* Adds the message to the radio Queue
   * Returns a message pointer
   */
  message_t *addRadioQ (message_t *tMsg) {
    message_t *ret = tMsg;

  	atomic {
		if (!radioFull)
		{
		  ret = radioQueue[radioIn];
		  radioQueue[radioIn] = tMsg;
		  if (++radioIn >= RADIO_QUEUE_LEN)
			radioIn = 0;
		  if (radioIn == radioOut)
			radioFull = TRUE;

		  if (!radioBusy)
			{
			  post radioSendTask();
			  radioBusy = TRUE;
			}
		}
		else
		dropBlink();
	} // atomic

	return ret;
  }


  task void radioSendTask() {
    uint8_t len;
    am_id_t id;
    am_addr_t addr,source;
    message_t* msg;

    atomic
      if (radioIn == radioOut && !radioFull)
	{
	  radioBusy = FALSE;
	  return;
	}

    msg = radioQueue[radioOut];
    len = sizeof(nodeifMsg);
    addr = call UartAMPacket.destination(msg);
    source = call UartAMPacket.source(msg);
    id = call UartAMPacket.type(msg);

    call RadioPacket.clear(msg);
    call RadioAMPacket.setSource(msg, source);

    if (call RadioSend.send[id](addr, msg, len) == SUCCESS)
      call Leds.led2Toggle();
    else
      {
	failBlink();
	post radioSendTask();
      }
  }

  event void RadioSend.sendDone[am_id_t id](message_t* msg, error_t error) {
    if (error != SUCCESS)
      failBlink();
    else
      atomic
	if (msg == radioQueue[radioOut])
	  {
	    if (++radioOut >= RADIO_QUEUE_LEN)
	      radioOut = 0;
	    if (radioFull)
	      radioFull = FALSE;
	  }

    post radioSendTask();
  }

  uint16_t getTxChannel () {
		uint16_t channel = 0;
		uint16_t freq = 0;
		uint16_t rd_channel = 0;

		error_t error = call SpiResource.immediateRequest();

		if ( error == SUCCESS ) {
			atomic{
				do {
					call CSN.clr();
					call FSCTRL.read(&rd_channel);
					call CSN.set();
		       } while (rd_channel & 0x1000);
			}
		}

		call SpiResource.release();

		//channel = ((rd_channel & 0x03FF) -2405)/5 + 11;
		freq = (rd_channel & 0x03FF) + 2048;
		channel = (freq - 2405)/5 + 11;


		return channel;
  }

  uint16_t ch2freq (uint16_t tch) {
  		return 2405 + 5*(tch-11);
  }

  uint16_t getTxPower () {
		uint16_t power = m_tx_power;
		uint16_t rd_power;

		error_t error = call SpiResource.immediateRequest();

		if ( error == SUCCESS ) {
			atomic{
					call CSN.clr();
					call TXCTRL.read(&rd_power);
					call CSN.set();
			}
			power = rd_power & 0x1F;
		}

		call SpiResource.release();

		return power;
  }

  uint16_t setTxPower(uint8_t tpower) {
		uint8_t tx_power = tpower;
		uint16_t wr_power = 0;
		uint16_t rd_power = 1;
		cc2420_status_t status;

		error_t error = call SpiResource.immediateRequest();

		if (error != SUCCESS)
		{
			return 0xFF;
		}

		call CSN.clr();
		call SRFOFF.strobe();
		call CSN.set();

		if ( !tx_power ) {
		  tx_power = CC2420_DEF_RFPOWER;
		}
		printf ("Now will set power to %d\n", tx_power); printfflush();
		atomic{
			call CSN.clr();
			if ( m_tx_power != tx_power ) {
			   wr_power = ( 2 << CC2420_TXCTRL_TXMIXBUF_CUR ) |
								 ( 3 << CC2420_TXCTRL_PA_CURRENT ) |
								 ( 1 << CC2420_TXCTRL_RESERVED ) |
								 ( (tx_power & 0x1F) << CC2420_TXCTRL_PA_LEVEL );

				status = call TXCTRL.write( wr_power );
			}
			call TXCTRL.read(&rd_power);
			call CSN.set();
		}

		call CSN.clr();
		call SRXON.strobe();
		call CSN.set();

		call SpiResource.release();

		printf("Written: %d(%X), Read:%d(%X)", wr_power, wr_power, rd_power, rd_power); printfflush();

		if ( rd_power != wr_power)
			call Leds.led0On();
		else {
			call Leds.led0Off();

			if (m_tx_power != tx_power) {
				printf("\t M_TX_POWER is set to %d from %d\n", tx_power, m_tx_power);
				printfflush();
			}

			m_tx_power = tx_power;
		}
		printfflush();

		return m_tx_power;

	}

	uint16_t setTxChannel (uint16_t tChannel) {
		uint8_t channel = tChannel;
		uint16_t wr_channel = 0, rd_channel = 0;

		cc2420_status_t status;

		error_t error = call SpiResource.immediateRequest();

		if (error != SUCCESS)
		{
			return 0xFF;
		}

		call CSN.clr();
		call SRFOFF.strobe();
		call CSN.set();

		if(!channel) {
			channel = CC2420_DEF_CHANNEL;
		}

		printf("set to channel=%d curChannel=%d\n",channel, curChannel);

		atomic {
			    call CSN.clr();
			    call FSCTRL.read(&wr_channel);
			    wr_channel = (wr_channel & 0xFE00) | ( ( (channel - 11)*5+357 ) << CC2420_FSCTRL_FREQ );
			    call CSN.set();
			    call CSN.clr();
				//wr_channel = ( 1 << CC2420_FSCTRL_LOCK_THR ) | ( ( (channel - 11)*5+357 ) << CC2420_FSCTRL_FREQ );
				status = call FSCTRL.write( wr_channel );
				call CSN.set();

			do {
			    call CSN.clr();
				call FSCTRL.read(&rd_channel);
				call CSN.set();
	       } while (rd_channel & 0x1000);
		}

		call CSN.clr();
		call SRXON.strobe();
		call CSN.set();

		call SpiResource.release();

		if (rd_channel != wr_channel) {
			printf("Problem: rd_channel=%d(0x%X) != wr_channel=%d(0x%X) && status = 0x%X SPI.owner=%d\n",rd_channel, rd_channel, wr_channel, wr_channel, status, call SpiResource.isOwner());
			//call Leds.led0On();
		}
		else {
			curChannel = channel;
		}

		printfflush();
		return curChannel;
	}

	event void SpiResource.granted() {
			printf("Request Granted\n"); printfflush();
	}

}
