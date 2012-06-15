README for SpectrumMonitorApp

Description:

This application periodically samples RF noise power in the 2.4 GHz band on a
set of user-defined frequencies (FREQUENCY_VECTOR in spectrummonitor.h) and
continuously outputs the result over the USB interface. Each value represents
RF power over a time interval defined by DWELL_TIME paramter on a given
frequency. Depeding on the mode you select the value represents the
maximum/minimum/...  value over the DWELL_TIME period (see spectrummonitor.h).
By default the DWELL_TIME is 10 ms, the maximum value is taken, the default
sampling period is 1/16 sec and the bandwidth (IEEE 802.15.4 channel width) is
2 MHz (fixed). This app is only available for the TelosB (Tmote Sky) platform.
During operation, LED 2 (blue) should toggle periodically. 

For testing purposes the app will also generate a message (cb_repo_query_msg_t
/ CBRepoQueryMsg.py) every time the user button on the sensor node is pressed.
Upon reception of this message the backend should reply (send to the node) a
cb_channelmask_msg (CBChannelMaskMsg.py) message that includes a bitmask of the
free channels.  Every time the node receives such a message it will toggle its
second (yellow) LED and forward it over the wireless channel. 

The RED LED should never turn on, it indicates an error. 

Usage: 

1. Install the image on a telosb node:

   $ make telosb install

2. Start a serialforwarder and python test script:

   $ java net.tinyos.sf.SerialForwarder -comm serial@/dev/ttyUSB0:115200 -no-gui -port 9002

   ... and run the python test program to see the results from the node: 

   $ python test.py

