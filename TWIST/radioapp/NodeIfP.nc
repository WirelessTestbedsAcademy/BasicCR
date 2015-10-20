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

#include "printf.h"
// #define printf(...) ;
// #define printfflush(...) ;

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

#ifdef CC2420_CCA_OVERRIDE
	interface RadioBackoff as CcaOverride;
#endif

	interface Timer<TMilli> as TimerR;
	interface Timer<TMilli> as TimerG;
	interface Timer<TMilli> as TimerB;
	}

}

implementation {

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

	void infoBlink() {
		call Leds.led1Toggle(); // D5 Green on telosb
		call TimerG.startOneShot(50);
	}

	event void TimerG.fired() {
		call Leds.led1Toggle(); // D5 Green on telosb
	}

	void warningBlink() {
		call Leds.led2Toggle(); // D6 Blue on telosb
		call TimerB.startOneShot(50);
	}

	event void TimerB.fired() {
		call Leds.led2Toggle(); // D6 Blue on telosb
	}

	void failBlink() {
		call Leds.led0Toggle(); // D4 Red on telosb
		call TimerR.startOneShot(50);
	}

	event void TimerR.fired() {
		call Leds.led0Toggle(); // D4 Red on telosb
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

		call SerialControl.start();
		call RadioControl.start();
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

	message_t* ONE receive(message_t* ONE msg, void* payload, uint8_t len);

	event message_t *RadioSnoop.receive[am_id_t id](message_t *msg, void *payload, uint8_t len) {
		message_t *ret = msg;
		ret = addUartQ(msg);
		return ret;
	}

	event message_t *RadioReceive.receive[am_id_t id](message_t *msg, void *payload, uint8_t len) {
		message_t *ret = msg;
		ret = addUartQ(msg);
		return ret;
	}

	task void uartSendTask() {
		uint8_t len;
		am_id_t id;
		am_addr_t addr, src;
		message_t* msg;
		error_t ecode;
		atomic {
			if (uartIn == uartOut && !uartFull) {
				uartBusy = FALSE;
				return;
			}
		}

		msg = uartQueue[uartOut];
		len = call RadioPacket.payloadLength(msg);
		id = call RadioAMPacket.type(msg);
		addr = call RadioAMPacket.destination(msg);
		src = call RadioAMPacket.source(msg);
		call RadioPacket.clear(msg);
		call UartAMPacket.setSource(msg, src);
		ecode = call UartSend.send[id](addr, uartQueue[uartOut], len);
		if (ecode == SUCCESS) {
			infoBlink();
		}	else {
			failBlink();
			post uartSendTask();
		}
	}

	event void UartSend.sendDone[am_id_t id](message_t* msg, error_t error) {
		if (error != SUCCESS) {
			failBlink();
		} else {
			atomic {
				if (msg == uartQueue[uartOut]) {
					if (++uartOut >= UART_QUEUE_LEN)
						uartOut = 0;
					if (uartFull)
						uartFull = FALSE;
				}
			}
		}
		post uartSendTask();
	}

	event message_t *UartReceive.receive[am_id_t id](message_t *msg,
							 void *payload,
							 uint8_t len) {
		message_t *ret = msg;

		if (id == AM_RADIO_CONF_MSG){
			radio_conf_msg_t *incoming = (radio_conf_msg_t*)(call UartPacket.getPayload(msg, (int) NULL));

			incoming->nodeid = TOS_NODE_ID;
			if (0 < incoming->txpower && incoming->txpower <= 31) {
				setTxPower(incoming->txpower);
			}
			if (11 <= incoming->channel && incoming->channel <= 16) {
				setTxChannel(incoming->channel);
			}
			incoming->txpower = getTxPower();
			incoming->channel = getTxChannel();
			// ret = addUartQ(msg);
		} else {
			failBlink();
			ret = addRadioQ(msg);
		}

		return ret;
	}

	/* Adds the message to the Uart Queue
	 * Returns a message pointer
	 */
	message_t *addUartQ (message_t *tMsg) {
		message_t *ret = tMsg;

		atomic {
			if (!uartFull) {
				ret = uartQueue[uartIn];
				uartQueue[uartIn] = tMsg;

				uartIn = (uartIn + 1) % UART_QUEUE_LEN;

				if (uartIn == uartOut)
					uartFull = TRUE;

				if (!uartBusy) {
					post uartSendTask();
					uartBusy = TRUE;
				}
			} else {
				failBlink();
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
			if (!radioFull)	{
				ret = radioQueue[radioIn];
				radioQueue[radioIn] = tMsg;
				if (++radioIn >= RADIO_QUEUE_LEN)
					radioIn = 0;
				if (radioIn == radioOut)
					radioFull = TRUE;

				if (!radioBusy) {
					post radioSendTask();
					radioBusy = TRUE;
				}
			}	else {
				failBlink();
			}
		} // atomic

	return ret;
	}


	task void radioSendTask() {
		uint8_t len;
		am_id_t id;
		am_addr_t addr,source;
		message_t* msg;

		atomic {
			if (radioIn == radioOut && !radioFull) {
				radioBusy = FALSE;
				return;
			}
		}

		msg = radioQueue[radioOut];
		len = call UartPacket.payloadLength(msg);
		addr = call UartAMPacket.destination(msg);
		source = call UartAMPacket.source(msg);
		id = call UartAMPacket.type(msg);

		call RadioPacket.clear(msg);
		call RadioAMPacket.setSource(msg, source);

		if (call RadioSend.send[id](addr, msg, len) == SUCCESS) {
			infoBlink();
		}	else {
			failBlink();
			post radioSendTask();
		}
	}

	event void RadioSend.sendDone[am_id_t id](message_t* msg, error_t error) {
		if (error != SUCCESS) {
			failBlink();
		} else {
			atomic {
				if (msg == radioQueue[radioOut]) {
					if (++radioOut >= RADIO_QUEUE_LEN)
						radioOut = 0;
					if (radioFull)
						radioFull = FALSE;
				}
			}
		}

		post radioSendTask();
	}

	uint16_t getTxChannel() {
		uint16_t channel = 0;
		uint16_t freq = 0;
		uint16_t rd_channel = 0;

		error_t error = call SpiResource.immediateRequest();

		if ( error == SUCCESS ) {
			atomic {
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

	uint16_t getTxPower() {
		uint16_t power = m_tx_power;
		uint16_t rd_power;

		error_t error = call SpiResource.immediateRequest();

		if ( error == SUCCESS ) {
			atomic {
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

		if (error != SUCCESS) {
			return 0xFF;
		}

		call CSN.clr();
		call SRFOFF.strobe();
		call CSN.set();

		if ( !tx_power ) {
			tx_power = CC2420_DEF_RFPOWER;
		}

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

		if ( rd_power != wr_power) {
			warningBlink();
		} else {
			m_tx_power = tx_power;
		}

		return m_tx_power;
	}

	uint16_t setTxChannel(uint16_t tChannel) {
		uint8_t channel = tChannel;
		uint16_t wr_channel = 0, rd_channel = 0;

		cc2420_status_t status;

		error_t error = call SpiResource.immediateRequest();

		if (error != SUCCESS)	{
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
			warningBlink();
		}
		else {
			curChannel = channel;
		}

		printfflush();
		return curChannel;
	}

	event void SpiResource.granted() {
	}

#ifdef CC2420_CCA_OVERRIDE
	async event void CcaOverride.requestCca(message_t *msg) {call CcaOverride.setCca(FALSE);}
	async event void CcaOverride.requestInitialBackoff(message_t *msg) {}
	async event void CcaOverride.requestCongestionBackoff(message_t *msg) {}
#endif

}
