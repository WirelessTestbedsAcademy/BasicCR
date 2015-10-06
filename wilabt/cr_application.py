#!/usr/bin/env python2
##################################################
# GNU Radio Python Flow Graph
# Title: Cr Application
# Generated: Tue Oct  6 10:33:49 2015
##################################################

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

import os
import sys
sys.path.append(os.environ.get('GRC_HIER_PATH', os.path.expanduser('~/.grc_gnuradio')))

from PyQt4 import Qt
from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio import qtgui
from gnuradio import uhd
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.qtgui import Range, RangeWidget
from optparse import OptionParser
from packet_gen import packet_gen
from sync_blocks import sync_blocks
import crew
import numpy
import sip
import threading
import time

from distutils.version import StrictVersion
class cr_application(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Cr Application")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Cr Application")
        try:
             self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
             pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "cr_application")
        self.restoreGeometry(self.settings.value("geometry").toByteArray())


        ##################################################
        # Variables
        ##################################################
        self.preamble = preamble = [1,-1,1,-1,1,1,-1,-1,1,1,-1,1,1,1,-1,1,1,-1,1,-1,-1,1,-1,-1,1,1,1,-1,-1,-1,1,-1,1,1,1,1,-1,-1,1,-1,1,-1,-1,-1,1,1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,1,1,1,1,1,1,-1,-1]
        self.usrp_rf_freq = usrp_rf_freq = 2435000000
        self.usrp_int_freq = usrp_int_freq = 2400000000
        self.tx_pkt_rate = tx_pkt_rate = 157
        self.samp_rate = samp_rate = 200000
        self.preamble_qpsk = preamble_qpsk = map(lambda x: x*(1+1j)/pow(2,0.5), preamble)
        self.payload_size = payload_size = 50
        self.nfilts = nfilts = 32
        self.eb = eb = 0.35
        self.digital_gain = digital_gain = 1.0/8

        ##################################################
        # Blocks
        ##################################################
        self.crew_db_channel_selector_0 = crew.db_channel_selector('test_crew', 'zigbee', 'max', 0.05)
        def _usrp_rf_freq_probe():
            while True:
                val = self.crew_db_channel_selector_0.get_freq()
                try:
                    self.set_usrp_rf_freq(val)
                except AttributeError:
                    pass
                time.sleep(1.0 / (10))
        _usrp_rf_freq_thread = threading.Thread(target=_usrp_rf_freq_probe)
        _usrp_rf_freq_thread.daemon = True
        _usrp_rf_freq_thread.start()
        self._usrp_int_freq_range = Range(2400000000, 2480000000, 5000000, 2400000000, 100)
        self._usrp_int_freq_win = RangeWidget(self._usrp_int_freq_range, self.set_usrp_int_freq, "usrp_int_freq", "counter_slider")
        self.top_layout.addWidget(self._usrp_int_freq_win)
        self.uhd_usrp_source_0_0 = uhd.usrp_source(
        	",".join(("addr=192.168.20.2", "")),
        	uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_source_0_0.set_samp_rate(samp_rate)
        self.uhd_usrp_source_0_0.set_center_freq(usrp_rf_freq, 0)
        self.uhd_usrp_source_0_0.set_gain(28, 0)
        self.uhd_usrp_source_0_0.set_antenna("J1", 0)
        self.uhd_usrp_sink_1 = uhd.usrp_sink(
        	",".join(("", "")),
        	uhd.stream_args(
        		cpu_format="fc32",
        		args="addr=192.168.60.2",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_sink_1.set_samp_rate(samp_rate*8)
        self.uhd_usrp_sink_1.set_center_freq(usrp_int_freq, 0)
        self.uhd_usrp_sink_1.set_gain(30, 0)
        self.uhd_usrp_sink_0_0 = uhd.usrp_sink(
        	",".join(("addr=192.168.50.2", "")),
        	uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_sink_0_0.set_samp_rate(samp_rate)
        self.uhd_usrp_sink_0_0.set_center_freq(usrp_rf_freq, 0)
        self.uhd_usrp_sink_0_0.set_gain(26, 0)
        self.uhd_usrp_sink_0_0.set_antenna("J1", 0)
        self.sync_blocks_0 = sync_blocks(
            digital_gain=19,
        )
        self.qtgui_number_sink_0 = qtgui.number_sink(
                gr.sizeof_float,
                0,
                qtgui.NUM_GRAPH_VERT,
        	1
        )
        self.qtgui_number_sink_0.set_update_time(0.2)
        self.qtgui_number_sink_0.set_title("Packet Reception Rate")
        
        labels = [" ", "", "", "", "",
                  "", "", "", "", ""]
        units = ["%", "", "", "", "",
                  "", "", "", "", ""]
        colors = [("black", "black"), ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black"),
                  ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black")]
        factor = [100, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        for i in xrange(1):
            self.qtgui_number_sink_0.set_min(i, 0)
            self.qtgui_number_sink_0.set_max(i, 100)
            self.qtgui_number_sink_0.set_color(i, colors[i][0], colors[i][1])
            if len(labels[i]) == 0:
                self.qtgui_number_sink_0.set_label(i, "Data {0}".format(i))
            else:
                self.qtgui_number_sink_0.set_label(i, labels[i])
            self.qtgui_number_sink_0.set_unit(i, units[i])
            self.qtgui_number_sink_0.set_factor(i, factor[i])
        
        self.qtgui_number_sink_0.enable_autoscale(False)
        self._qtgui_number_sink_0_win = sip.wrapinstance(self.qtgui_number_sink_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_number_sink_0_win, 0,1,1,1)
        self.qtgui_const_sink_x_0_0 = qtgui.const_sink_c(
        	20000, #size
        	"Rx Constellation", #name
        	1 #number of inputs
        )
        self.qtgui_const_sink_x_0_0.set_update_time(0.10)
        self.qtgui_const_sink_x_0_0.set_y_axis(-2, 2)
        self.qtgui_const_sink_x_0_0.set_x_axis(-2, 2)
        self.qtgui_const_sink_x_0_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, "")
        self.qtgui_const_sink_x_0_0.enable_autoscale(False)
        self.qtgui_const_sink_x_0_0.enable_grid(True)
        
        if not False:
          self.qtgui_const_sink_x_0_0.disable_legend()
        
        labels = ["", "", "", "", "",
                  "", "", "", "", ""]
        widths = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        colors = ["blue", "red", "red", "red", "red",
                  "red", "red", "red", "red", "red"]
        styles = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        markers = [0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]
        for i in xrange(1):
            if len(labels[i]) == 0:
                self.qtgui_const_sink_x_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_const_sink_x_0_0.set_line_label(i, labels[i])
            self.qtgui_const_sink_x_0_0.set_line_width(i, widths[i])
            self.qtgui_const_sink_x_0_0.set_line_color(i, colors[i])
            self.qtgui_const_sink_x_0_0.set_line_style(i, styles[i])
            self.qtgui_const_sink_x_0_0.set_line_marker(i, markers[i])
            self.qtgui_const_sink_x_0_0.set_line_alpha(i, alphas[i])
        
        self._qtgui_const_sink_x_0_0_win = sip.wrapinstance(self.qtgui_const_sink_x_0_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_const_sink_x_0_0_win, 0,0,1,1)
        self.packet_gen_0 = packet_gen(
            payload_size=payload_size,
        )
        self.digital_gfsk_mod_0 = digital.gfsk_mod(
        	samples_per_symbol=2,
        	sensitivity=1.0,
        	bt=0.35,
        	verbose=False,
        	log=False,
        )
        self.crew_packet_decoder_cb_0 = crew.packet_decoder_cb((preamble_qpsk))
        self.blocks_null_sink_0 = blocks.null_sink(gr.sizeof_char*1)
        self.blocks_multiply_const_vxx_1_0 = blocks.multiply_const_vff((1.0/(tx_pkt_rate), ))
        self.blocks_multiply_const_vxx_1 = blocks.multiply_const_vff((-1.0/(tx_pkt_rate), ))
        self.blocks_multiply_const_vxx_0_0 = blocks.multiply_const_vcc((1.0/4, ))
        self.blocks_file_source_0 = blocks.file_source(gr.sizeof_char*1, "/users/lwei/GITfolder/wirelessacademy/BasicTx/wilabt/file_sent.txt", True)
        self.blocks_add_const_vxx_0 = blocks.add_const_vff((1.0, ))
        self.analog_random_source_x_1 = blocks.vector_source_b(map(int, numpy.random.randint(0, 256, 1000)), True)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_random_source_x_1, 0), (self.digital_gfsk_mod_0, 0))    
        self.connect((self.blocks_add_const_vxx_0, 0), (self.crew_db_channel_selector_0, 0))    
        self.connect((self.blocks_file_source_0, 0), (self.packet_gen_0, 0))    
        self.connect((self.blocks_multiply_const_vxx_0_0, 0), (self.uhd_usrp_sink_1, 0))    
        self.connect((self.blocks_multiply_const_vxx_1, 0), (self.blocks_add_const_vxx_0, 0))    
        self.connect((self.blocks_multiply_const_vxx_1_0, 0), (self.qtgui_number_sink_0, 0))    
        self.connect((self.crew_packet_decoder_cb_0, 1), (self.blocks_multiply_const_vxx_1, 0))    
        self.connect((self.crew_packet_decoder_cb_0, 1), (self.blocks_multiply_const_vxx_1_0, 0))    
        self.connect((self.crew_packet_decoder_cb_0, 0), (self.blocks_null_sink_0, 0))    
        self.connect((self.digital_gfsk_mod_0, 0), (self.blocks_multiply_const_vxx_0_0, 0))    
        self.connect((self.packet_gen_0, 0), (self.uhd_usrp_sink_0_0, 0))    
        self.connect((self.sync_blocks_0, 0), (self.crew_packet_decoder_cb_0, 0))    
        self.connect((self.sync_blocks_0, 0), (self.qtgui_const_sink_x_0_0, 0))    
        self.connect((self.uhd_usrp_source_0_0, 0), (self.sync_blocks_0, 0))    

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "cr_application")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_preamble(self):
        return self.preamble

    def set_preamble(self, preamble):
        self.preamble = preamble
        self.set_preamble_qpsk(map(lambda x: x*(1+1j)/pow(2,0.5), self.preamble))

    def get_usrp_rf_freq(self):
        return self.usrp_rf_freq

    def set_usrp_rf_freq(self, usrp_rf_freq):
        self.usrp_rf_freq = usrp_rf_freq
        self.uhd_usrp_source_0_0.set_center_freq(self.usrp_rf_freq, 0)
        self.uhd_usrp_sink_0_0.set_center_freq(self.usrp_rf_freq, 0)

    def get_usrp_int_freq(self):
        return self.usrp_int_freq

    def set_usrp_int_freq(self, usrp_int_freq):
        self.usrp_int_freq = usrp_int_freq
        self.uhd_usrp_sink_1.set_center_freq(self.usrp_int_freq, 0)

    def get_tx_pkt_rate(self):
        return self.tx_pkt_rate

    def set_tx_pkt_rate(self, tx_pkt_rate):
        self.tx_pkt_rate = tx_pkt_rate
        self.blocks_multiply_const_vxx_1_0.set_k((1.0/(self.tx_pkt_rate), ))
        self.blocks_multiply_const_vxx_1.set_k((-1.0/(self.tx_pkt_rate), ))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.uhd_usrp_source_0_0.set_samp_rate(self.samp_rate)
        self.uhd_usrp_sink_0_0.set_samp_rate(self.samp_rate)
        self.uhd_usrp_sink_1.set_samp_rate(self.samp_rate*8)

    def get_preamble_qpsk(self):
        return self.preamble_qpsk

    def set_preamble_qpsk(self, preamble_qpsk):
        self.preamble_qpsk = preamble_qpsk

    def get_payload_size(self):
        return self.payload_size

    def set_payload_size(self, payload_size):
        self.payload_size = payload_size
        self.packet_gen_0.set_payload_size(self.payload_size)

    def get_nfilts(self):
        return self.nfilts

    def set_nfilts(self, nfilts):
        self.nfilts = nfilts

    def get_eb(self):
        return self.eb

    def set_eb(self, eb):
        self.eb = eb

    def get_digital_gain(self):
        return self.digital_gain

    def set_digital_gain(self, digital_gain):
        self.digital_gain = digital_gain


if __name__ == '__main__':
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    (options, args) = parser.parse_args()
    if(StrictVersion(Qt.qVersion()) >= StrictVersion("4.5.0")):
        Qt.QApplication.setGraphicsSystem(gr.prefs().get_string('qtgui','style','raster'))
    qapp = Qt.QApplication(sys.argv)
    tb = cr_application()
    tb.start()
    tb.show()
    def quitting():
        tb.stop()
        tb.wait()
    qapp.connect(qapp, Qt.SIGNAL("aboutToQuit()"), quitting)
    qapp.exec_()
    tb = None #to clean up Qt widgets
