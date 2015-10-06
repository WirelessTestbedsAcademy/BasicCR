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

/**
 * @author Onur Ergin
 */

#ifndef NODEIFMESSAGES_H__
#define NODEIFMESSAGES_H__

enum {
  AM_NODEIFMSG = 0x13
};

enum {
  SETTX = 0,
  GETTX = 1,
  SETFREQ = 2,
  GETFREQ = 3,
  DATA = 4,
  IDQUERY = 5
};

enum {
  UART_QUEUE_LEN = 12,
  RADIO_QUEUE_LEN = 12,
};


typedef nx_struct NodeIfMsg {
  nx_uint16_t appid;
  nx_uint16_t nodeid;
  nx_uint16_t senderid;
  nx_uint16_t receiverid;
  nx_uint8_t pcktype;
  nx_uint16_t value;
  nx_int8_t rssi;
  nx_uint8_t txpower;
  nx_uint8_t lqi;
  nx_uint8_t channel;
} NodeIfMsg,nodeifMsg;

#endif //NODEIFMESSAGES_H__
