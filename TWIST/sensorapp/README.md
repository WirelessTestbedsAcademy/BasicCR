SpectrumMonitorApp
==================

This application periodically samples RF noise power in the 2.4 GHz band on a
set of user-defined frequencies (`FREQUENCY_VECTOR` in `spectrummonitor.h`) and
continuously outputs the result over the USB interface. Each value represents
RF power over a time interval defined by `DWELL_TIME` parameter on a given
frequency. Depending on the mode you select the value represents the
maximum/minimum/...  value over the `DWELL_TIME` period (see `spectrummonitor.h`).
By default the `DWELL_TIME` is 10 ms, the maximum value is taken, the default
sampling period is 1/16 sec and the bandwidth (IEEE 802.15.4 channel width) is
2 MHz (fixed). This app is only available for the TelosB (Tmote Sky) platform.
During operation, LED 2 (blue) should toggle periodically.

Whenever the app receives any message from the serial line, it broadcasts this
message out over the radio (and toggles it's green LED).

The RED LED should never turn on, it indicates an error.

Usage:

0. Make sure you have TinyOS toolchain installed on the system

    You can use provided Dockerfile to create own docker image

        docker build -t wta/sensingtwist .

1. Build the image on a telosb node:

        make telosb

2. Install the code on the motes

    a. TWIST testbed (on all nodes, refer to `./twist.py --help` for more information):

            ./twist.py -i ./build/telosb/main.exe -a

    b. You can also install it on the local mote using:

            make telosb reinstall bsl,/dev/ttyUSB0

2. Start listening for the spectrum data:

    a. On the TWIST testbed you need to start SSH tunnels and connect to it:

            ./twit.py -a -s
            # In second console/screen
            ./sensorapp.py localhost:9151

    b. Running locally it is enough to point to USB interface

            ./sensorapp.py /dev/ttyUSB0
