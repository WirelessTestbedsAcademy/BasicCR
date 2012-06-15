#ifndef SPECTRUMMONITOR_H
#define SPECTRUMMONITOR_H

#ifndef FREQUENCY_VECTOR

// The center frequencies of the IEEE 802.15.4 channels in the 2.4 GHz band
#define FREQUENCY_VECTOR {2405, 2410, 2415, 2420, 2425, 2430, 2435, 2440, 2445, 2450, 2455, 2460, 2465, 2470, 2475, 2480}

//#define FREQUENCY_VECTOR {2400, 2402, 2404, 2406, 2408, 2410, 2412, 2414, 2416, 2418, 2420, 2422, 2424, 2426, 2428, 2430, 2432, 2434, 2436, 2438, 2440, 2442, 2444, 2446, 2448, 2450, 2452, 2454, 2456, 2458, 2460, 2462, 2464, 2466, 2468, 2470, 2472, 2474, 2476, 2478, 2480}

#endif

uint16_t dummy_[] = FREQUENCY_VECTOR;
#define NUM_FREQUENCIES (sizeof(dummy_) / sizeof(uint16_t))

typedef enum {
  MODE_MAX,     // max value of all samples obtained during DWELL_TIME
  MODE_MIN,     // min ...
  // MODE_MEAN, // 
} dwell_mode_t;

enum {
  SAMPLING_PERIOD = (32768U/NUM_FREQUENCIES), // ticks of ACLK, which runs at 32768 Hz, use values below 64 at your own risk!
  DWELL_TIME = 327, // the time we sample a given frequency, in ticks of which runs at 32768 Hz. MUST be smaller than SAMPLING_PERIOD! (set to zero if you only want a single sample)
  MSG_QUEUE_SIZE = 10,

  NO_ERROR = 0,
  OVERFLOW1_ERROR = 1,
  OVERFLOW2_ERROR = 1,

  FRAME_TYPE_DATA         = 0x01,
  FRAME_TYPE_ACK          = 0x02,
  DEST_MODE_SHORT         = 0x08,
  ACK_REQUEST             = 0x20,
  BROADCAST_ADDRESS       = 0xFFFF,
};

/*** no need to change anything below ***/

#define TOSH_DATA_LENGTH 200

enum {
  AM_CB_SWEEP_DATA_MSG = 0XB0,
  AM_CB_REPO_QUERY_MSG = 0XB1,
  AM_CB_CHANNELMASK_MSG = 0XB2,
};

typedef nx_struct cb_sweep_data_msg {
  nx_uint8_t src;        // the ID of the node where this message originates from
  nx_uint8_t errorcode;  // should always be 0
  nx_uint32_t seqno;     // incremented after every transmitted message
  nx_int8_t rssi[NUM_FREQUENCIES];
} cb_sweep_data_msg_t;

enum {
  MODE_CHANNEL_MASK, // a 16-bit bitmask, where each bit represents a 802.15.4 channel in the 2.4 GHz band
  // more to follow ...
};

typedef nx_struct cb_repo_query_msg {
  nx_uint8_t forwarder;   // TWIST node ID
  nx_uint16_t srcID;      // BAN src ID
  nx_uint16_t srcPANID;   // BAN src PAN ID
  nx_uint8_t mode;        // type of request: MODE_CHANNEL_MASK etc.
} cb_repo_query_msg_t;

typedef nx_struct cb_channelmask_msg {
  nx_uint16_t data;
} cb_channelmask_msg_t;


typedef nx_struct  {
  nxle_uint8_t length;
  nxle_uint16_t fcf;
  nxle_uint8_t dsn;
  nxle_uint16_t destpan;
  nxle_uint16_t dest;
} header_154_t;

#endif 

