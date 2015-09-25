/*
 * Copyright (c) 2009, Technische Universitaet Berlin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in the 
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the Technische Universitaet Berlin nor the names 
 *   of its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * - Revision -------------------------------------------------------------
 * $Revision: 1.1 $
 * $Date: 2009-06-10 09:23:45 $
 * @author: Jasper Buesch <buesch@tkn.tu-berlin.de>
 * ========================================================================
 */

#ifndef __APP_PROFILE_H
#define __APP_PROFILE_H

enum {
  RADIO_CHANNEL = 11,
  COORDINATOR_ADDRESS = 0x6287,
  DEVICE_ADDRESS = 0x6288,
  BAN_PAN_ID = 0x4927,
  BEACON_ORDER = 4,
  SUPERFRAME_ORDER = 4,
  TX_POWER_COORDINATOR = 0, // in dBm
  TX_POWER_DEVICE = -20, // in dBm

  DATA_PACKET_PERIOD_MS = 200,

  AM_STATUS_MSG = 81,
  AM_ISS_MSG = 82,
  AM_CONTROL_MODE_MSG = 83,
  AM_CONTROL_CHANNEL_MSG = 84,

  AM_BAN_DATA_MSG = 101,
  AM_CB_BEACON_MSG = 0XB3,
  PRR_WINDOW_SIZE = 10,

  LOCAL_SENSING_NOISE_THR = -50,
  LOCAL_SENSING_MIN_PRR = 10,    // in percentage
  LOCAL_SENSING_BACKOFF_TIME_MS = 1000*10,
  LOCAL_SENSING_MIN_CHANNEL = 11,
  LOCAL_SENSING_MAX_CHANNEL = 14,
  //IMEC_SENSING_BACKOFF_TIME_MS = 1000*3,
};

norace int8_t m_rfnoise;

enum {
  SENSING_MODE_MANUAL = 0,          // only user can swtich channel (via GUI)
  SENSING_MODE_LOCAL = 1,           // channel switch based on local RSSI readings
  SENSING_MODE_IMEC = 2,            // channel switch based on IMEC readings
  SENSING_MODE_INFRASTRUCTURE = 3,  // channel switch based on infrastructure readings
};

// this message is periodically (1Hz) sent from the node out over the serial line
typedef nx_struct status_msg {
  nx_uint8_t mode;     // current sensing mode: SENSING_MODE_MANUAL, SENSING_MODE_LOCAL or SENSING_MODE_INFRASTRUCTURE
  nx_uint8_t prr;      // packet reception rate (in percent) -> 0..100
  nx_uint8_t channel;  // current 802.15.4 channel -> 11..26
  nx_int8_t noise;     // RF noise on current channel in dBm (for debugging)
} status_msg_t;

// this message is sent from the node out over the serial line, whenever it has
// received a message from the infrastructure sensing nodes that tell the BAN
// to switch to another channel (useful for debugging)
typedef nx_struct iss_msg {
  nx_uint16_t src;          // id of the TWIST telos node, which sent the channelmask
  nx_uint16_t channelmask;  // received channelmask
  nx_uint8_t channel;       // next channel (derived from channelmask iff in SENSING_MODE_INFRASTRUCTURE)
} iss_msg_t;
 
// this message is sent from the laptop to the node to change the mode
typedef nx_struct control_mode_msg {
  nx_uint8_t mode;     // next sensing mode: SENSING_MODE_MANUAL, SENSING_MODE_LOCAL, 
                       // SENSING_MODE_IMEC or SENSING_MODE_INFRASTRUCTURE
} control_mode_msg_t;

// this message is sent from the laptop to the node to change the channel
// (e.g. for debugging purpose)
typedef nx_struct control_channel_msg {
  nx_uint8_t mode;        // one of the SENSING_MODE_X above: e.g. if this msg comes from
                          // IMEC then set SENSING_MODE_IMEC, if manual SENSING_MODE_MANUAL
  nx_uint8_t channel;     // next channel
} control_channel_msg_t;


// this message is sent over wireless from device to coordinator
typedef nx_struct data_msg {
  nx_uint8_t type; // must be AM_BAN_DATA_MSG
  nx_uint32_t seqno;
} data_msg_t;


typedef nx_struct beacon_payload {
  nx_uint8_t type; // must be AM_CB_BEACON_MSG
  nx_uint8_t countdown;
  nx_uint8_t nextChannel;
} beacon_payload_t;

/*typedef nx_struct  {*/
  //nxle_uint8_t length;
  //nxle_uint16_t fcf;
  //nxle_uint8_t dsn;
  //nxle_uint16_t destpan;
  //nxle_uint16_t dest;
/*} header_154_t;*/
#endif
