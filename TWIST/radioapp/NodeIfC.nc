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
 */

// $Id: NodeIfC.nc,v 1.6 2008/09/25 04:08:09 regehr Exp $

//#define NEW_PRINTF_SEMANTICS
//#include "printf.h"

configuration NodeIfC {
}
implementation {
  components MainC, NodeIfP, LedsC;
  components ActiveMessageC as Radio, SerialActiveMessageC as Serial;
  components CC2420ActiveMessageC;

  MainC.Boot <- NodeIfP;

  NodeIfP.RadioControl -> Radio;
  NodeIfP.SerialControl -> Serial;

  NodeIfP.UartSend -> Serial;
  NodeIfP.UartReceive -> Serial.Receive;
  NodeIfP.UartPacket -> Serial;
  NodeIfP.UartAMPacket -> Serial;

  NodeIfP.RadioSend -> Radio;
  NodeIfP.RadioReceive -> Radio.Receive;
  NodeIfP.RadioSnoop -> Radio.Snoop;
  NodeIfP.RadioPacket -> Radio;
  NodeIfP.RadioAMPacket -> Radio;

  NodeIfP -> CC2420ActiveMessageC.CC2420Packet;

  NodeIfP.Leds -> LedsC;

  components HplCC2420PinsC as Pins;
  NodeIfP.CSN -> Pins.CSN;

  components new CC2420SpiC() as Spi;
  NodeIfP.SpiResource -> Spi;
  NodeIfP.SNOP        -> Spi.SNOP;
  NodeIfP.STXON       -> Spi.STXON;
  NodeIfP.STXONCCA    -> Spi.STXONCCA;
  NodeIfP.SFLUSHTX    -> Spi.SFLUSHTX;
  NodeIfP.TXCTRL      -> Spi.TXCTRL;
  NodeIfP.MDMCTRL1    -> Spi.MDMCTRL1;
  NodeIfP.FSCTRL 	  -> Spi.FSCTRL;

  // To let the changes take effect use these:
  NodeIfP.SRXON -> Spi.SRXON;
  NodeIfP.SRFOFF -> Spi.SRFOFF;


}
