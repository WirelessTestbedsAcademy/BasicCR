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
  AM_RADIO_CONF_MSG = 113,
  AM_RADIO_MSG = 114,
};


enum {
  UART_QUEUE_LEN = 12,
  RADIO_QUEUE_LEN = 12,
};

/* Transmission power (PA_LEVEL register, see CC2420 Datasheet):
 *
 *  31 -> 0 dBm
 *  27 -> -1 dBm
 *  23 -> -3 dBm
 *  19 -> -5 dBm
 *  15 -> -7 dBm
 *  11 -> -10 dBm
 *  7 -> -15 dBm
 *  3 -> -25 dBm
 **/


typedef nx_struct radio_conf_msg {
  nx_uint16_t nodeid;
  nx_uint8_t txpower;
  nx_uint8_t channel;
} radio_conf_msg_t;

typedef nx_struct radio_msg {
  nx_uint16_t srcid;
  nx_uint16_t dstid;
  nx_int8_t data[TOSH_DATA_LENGTH-2*sizeof(nx_uint16_t)];
} radio_msg_t;


#endif //NODEIFMESSAGES_H__
