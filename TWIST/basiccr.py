#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
basiccr.py: simple cognitive radio example

    In this example three nodes and python based control applications running
    on the PC are used. On two nodes one application providing radio interface
    should be installed. This application provides serial line interface to
    control the radio. Namely it is possible to set radio parameters by
    sending control messages over serial interface and send any message over
    the radio interface. On the third node the spectrum sensing application
    should be installed. Finally central controller application running on the
    PC connects to all the nodes, collects information about spectrum usage
    from sensor application and configures the best channel on the other two
    nodes.

Usage:
   basiccr.py [options] [-q | -v] <sensor> <transmitter> <receiver>
   basiccr.py --config

Devices:
   <sensor>            device for connection
   <transmitter>       device for connection
   <receiver>          device for connection
                       examples include:
                          /dev/ttyUSB0
                          localhost:9002
                          10

Options:
   -c <channel>        set channel
   -p <power>          set tx power
   -t                  send packet


Other options:
   -h, --help          show this help message and exit
   -q, --quiet         print less text
   -v, --verbose       print more text
   --version           show version and exit
"""

__author__ = "Mikolaj Chwalisz"
__copyright__ = "Copyright (c) 2015, Technische Universität Berlin"
__version__ = "0.1.0"
__email__ = "chwalisz@tkn.tu-berlin.de"

# copied from http://mail.millennium.berkeley.edu/pipermail/tinyos-help/2007-September/028165.html
# This is a quick and dirty example of how to use the MoteIF interface in Python

import sys
import os
sys.path.append(os.environ.get('TINYOS_ROOT_DIR') + "/tools/tinyos/python")
import time
import logging
import twist
from threading import Thread
from getpass import getpass
from math import log10

from tinyos.message import MoteIF
# from messages.printf import printfMsg
from radioapp.radio_conf_msg import radio_conf_msg
from radioapp.radio_msg import radio_msg
from sensorapp.CBSweepDataMsg import CBSweepDataMsg

class Controller(Thread):

    def __init__(self, sensor, tx, rx):
        Thread.__init__(self)
        self.setDaemon(True)
        self.sensor = sensor
        self.tx = tx
        self.rx = rx
        self.log = logging.getLogger("basiccr.ctrl")
        self.log.info("init")

    def run(self):
        # Create a MoteIF
        self.mif = MoteIF.MoteIF()
        # Attach a source to it
        self.ssr = self.mif.addSource(self.sensor)
        self.stx = self.mif.addSource(self.tx)
        self.srx = self.mif.addSource(self.rx)

        self.mif.addListener(self, CBSweepDataMsg)
        self.mif.addListener(self, radio_conf_msg)
        self.mif.addListener(self, radio_msg)
        self.log.info("run")

        b_channel = 11
        last_channel = 11
        self.configure(self.stx, channel=b_channel,power=31)
        self.configure(self.srx, channel=b_channel,power=31)

        ##########################################
        ###          Controller logic          ###
        while True:
            if self.channel_utilization is None:
                continue
            else:
                print ""
            self.channel_info()
            b_rssi = min(self.channel_utilization)
            b_channel = self.channel_utilization.index(b_rssi)+11
            print "Best channel: {:2d} (RSSI: {:.3f})".format(b_channel,b_rssi)

            if last_channel != b_channel:
                print "Configuring TX for channel {}.".format(b_channel)
                self.configure(self.stx, channel=b_channel)
            else:
                print "Staying on previous channel. It is still the best"
            print "Sending message"
            self.send_messge()
            time.sleep(1)
            if last_channel != b_channel:
                print "Configuring RX for channel {}.".format(b_channel)
                self.configure(self.srx, channel=b_channel)
            else:
                print "Staying on previous channel. It is still the best"
            print "Sending message"
            self.send_messge()
            last_channel = b_channel
            time.sleep(5)
        ###          Controller logic          ###
        ##########################################


    def receive(self, src, msg):
        msgType = msg.get_amType()
        if msgType == CBSweepDataMsg.get_amType():
            # self.log.info("sensing data: {}".format(str(msg.get_rssi())))
            self.update_utilization(msg.get_rssi())
        elif msgType == radio_msg.get_amType():
            self.log.info("Node: {} - radio msg".format(msg.getAddr()))
        elif msgType == radio_conf_msg.get_amType():
            self.log.info("radio conf msg: {}".format(msg.getAddr(),msg))

    def configure(self, target, channel=None, power=None):
        msg = radio_conf_msg()
        if channel is not None:
            msg.set_channel(channel)
        if power is not None:
            msg.set_txpower(power)
        self.mif.sendMsg(target, 0xFFFF, msg.get_amType(), 0, msg)

    def send_messge(self):
        msg = radio_msg()
        msg.set_srcid(255)
        msg.set_dstid(64)
        self.mif.sendMsg(self.stx, 0xFEFF, msg.get_amType(), 0, msg)

    def channel_info(self):
        print "Current channel utilization:\nChannel: {}\nPower:   {}".format(
            ", ".join("{:6d}".format(x) for x in range(11, 27)),
            ", ".join("{:6.1f}".format(x) for x in self.channel_utilization))
    # def channel_info

    channel_utilization = None
    def update_utilization(self, data):
        if self.channel_utilization is None:
            self.channel_utilization = data

        self.channel_utilization = map(avg_dbm, zip(data, self.channel_utilization))


def main(args):
    """Run the code for basiccr"""
    log = logging.getLogger('basiccr.main')
    log.debug(args)
    signal.signal(signal.SIGINT, signal_handler)

    ctrl = Controller(sensor = guess_sfsource(args["<sensor>"]),
                    tx = guess_sfsource(args["<transmitter>"]),
                    rx = guess_sfsource(args["<receiver>"]))
    ctrl.run()

    signal.pause()
# def main

##############################################################################
# Supporting functions

def avg_dbm(dBm):
    a = 0.1
    n = 10**((dBm[0])/10.)
    o = 10**((dBm[1])/10.)
    r = a * n + (1-a) * o
    return 10.*log10(r)

__ssh_password__ = None
def guess_sfsource(sfsource):
    """Returns proper serial forwarder source guessed from simplified argument

    Possibilities are:

    /dev/ttyUSB0       Local mote connected over USB
    localhost:9010     External serial forwarder with host and port
    151                TWIST Node (start ssh in this case)

    """
    if "/" in sfsource:
        return "serial@{}:115200".format(sfsource)
    elif ":" in sfsource:
        return "sf@{}".format(sfsource)
    elif sfsource.isdigit():
        global __ssh_password__
        if __ssh_password__ is None:
            print "Provide TWIST ssh password: "
            __ssh_password__ = getpass()
        tunnel = Thread(target = twist.ssh_tunnel, args = ([int(sfsource)], __ssh_password__,))
        tunnel.daemon = True
        tunnel.start()
        time.sleep(3)
        return "sf@localhost:9{0:03d}".format(int(sfsource))
    else:
        raise AttributeError("Could not recognize which Serial Forwarder to use.")
# def guess_sfsource

def signal_handler(signal, frame):
    log = logging.getLogger('basiccr.sinal')
    log.debug("You pressed Ctrl+C!")
    sys.exit(0)


if __name__ == "__main__":
    import signal
    try:
        from docopt import docopt
    except:
        print("""
        Please install docopt using:
            pip install docopt==0.6.1
        For more refer to:
        https://github.com/docopt/docopt
        """)
        raise

    args = docopt(__doc__, version=__version__)

    log_level = logging.INFO  # default
    if args['--verbose']:
        log_level = logging.DEBUG
    elif args['--quiet']:
        log_level = logging.ERROR
    logging.basicConfig(level=log_level,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    main(args)
