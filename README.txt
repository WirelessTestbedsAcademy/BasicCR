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

Whenever the app receives any message from the serial line, it broadcasts this
message out over the radio (and toggles it's green LED).

Optionally, you can set CTRL_CHANNEL_FREQUENCY_VECTOR (in spectrummonitor.h).
This variable represents a list of control channels, which the application will
listen briefly in between the sweeps. Whenever it receives an appropriate
message (a beacon from a BAN) on one of the control channels, the application
will forward this message over the USB interface. By default, however,
CTRL_CHANNEL_FREQUENCY_VECTOR is empty, i.e. the app will not listen for
messages on control channels; it will only sample RF noise.

The RED LED should never turn on, it indicates an error. 

Usage: 

1. Install the image on a telosb node:

   $ make telosb install

2. Start a serialforwarder and python test script:

   $ java net.tinyos.sf.SerialForwarder -comm serial@/dev/ttyUSB0:115200 -no-gui -port 9002

   ... and run the python test program to see the results from the node: 

   $ python test.py

