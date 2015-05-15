#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Cr Application
# Generated: Fri May 15 13:29:31 2015
##################################################

from PyQt4 import Qt
from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio import qtgui
from gnuradio import uhd
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from grc_gnuradio import blks2 as grc_blks2
from optparse import OptionParser
import PyQt4.Qwt5 as Qwt
import crew
import numpy
import sip
import sys
import threading
import time

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
        self.sps = sps = 4
        self.nfilts = nfilts = 32
        self.usrp_rf_freq = usrp_rf_freq = 2405000000
        self.usrp_int_freq = usrp_int_freq = 2400000000
        self.timing_loop_bw = timing_loop_bw = 6.28/100
        self.taps = taps = [1.0, 0.25-0.25j, 0.50 + 0.10j, -0.3 + 0.2j]
        self.samp_rate = samp_rate = 390625
        self.rrc_taps = rrc_taps = firdes.root_raised_cosine(nfilts, nfilts, 1.0/float(sps), 0.35, 11*sps*nfilts)
        self.qpsk = qpsk = digital.constellation_rect(([0.707+0.707j, -0.707+0.707j, -0.707-0.707j, 0.707-0.707j]), ([0, 1, 2, 3]), 4, 2, 2, 1, 1).base()
        self.phase_bw = phase_bw = 6.28/100
        self.excess_bw = excess_bw = 0.35
        self.eq_gain = eq_gain = 0.01
        self.arity = arity = 4

        ##################################################
        # Blocks
        ##################################################
        self.crew_db_channel_selector_0 = crew.db_channel_selector('test_may14_r1', 'zigbee', 'max', 0.05)
        def _usrp_rf_freq_probe():
        	while True:
        		val = self.crew_db_channel_selector_0.get_freq()
        		try: self.set_usrp_rf_freq(val)
        		except AttributeError, e: pass
        		time.sleep(1.0/(0.2))
        _usrp_rf_freq_thread = threading.Thread(target=_usrp_rf_freq_probe)
        _usrp_rf_freq_thread.daemon = True
        _usrp_rf_freq_thread.start()
        self._usrp_int_freq_layout = Qt.QVBoxLayout()
        self._usrp_int_freq_tool_bar = Qt.QToolBar(self)
        self._usrp_int_freq_layout.addWidget(self._usrp_int_freq_tool_bar)
        self._usrp_int_freq_tool_bar.addWidget(Qt.QLabel("usrp_int_freq"+": "))
        self._usrp_int_freq_counter = Qwt.QwtCounter()
        self._usrp_int_freq_counter.setRange(2400000000, 2480000000, 5000000)
        self._usrp_int_freq_counter.setNumButtons(2)
        self._usrp_int_freq_counter.setValue(self.usrp_int_freq)
        self._usrp_int_freq_tool_bar.addWidget(self._usrp_int_freq_counter)
        self._usrp_int_freq_counter.valueChanged.connect(self.set_usrp_int_freq)
        self._usrp_int_freq_slider = Qwt.QwtSlider(None, Qt.Qt.Horizontal, Qwt.QwtSlider.BottomScale, Qwt.QwtSlider.BgSlot)
        self._usrp_int_freq_slider.setRange(2400000000, 2480000000, 5000000)
        self._usrp_int_freq_slider.setValue(self.usrp_int_freq)
        self._usrp_int_freq_slider.setMinimumWidth(100)
        self._usrp_int_freq_slider.valueChanged.connect(self.set_usrp_int_freq)
        self._usrp_int_freq_layout.addWidget(self._usrp_int_freq_slider)
        self.top_layout.addLayout(self._usrp_int_freq_layout)
        self.uhd_usrp_source_0_0 = uhd.usrp_source(
        	device_addr="addr=192.168.30.2",
        	stream_args=uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_source_0_0.set_samp_rate(samp_rate)
        self.uhd_usrp_source_0_0.set_center_freq(usrp_rf_freq, 0)
        self.uhd_usrp_source_0_0.set_gain(10, 0)
        self.uhd_usrp_sink_1 = uhd.usrp_sink(
        	device_addr="",
        	stream_args=uhd.stream_args(
        		cpu_format="fc32",
        		args="addr=192.168.40.2",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_sink_1.set_samp_rate(samp_rate)
        self.uhd_usrp_sink_1.set_center_freq(usrp_int_freq, 0)
        self.uhd_usrp_sink_1.set_gain(40, 0)
        self.uhd_usrp_sink_0 = uhd.usrp_sink(
        	device_addr="addr=192.168.20.2",
        	stream_args=uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_sink_0.set_samp_rate(samp_rate)
        self.uhd_usrp_sink_0.set_center_freq(usrp_rf_freq, 0)
        self.uhd_usrp_sink_0.set_gain(10, 0)
        self.qtgui_time_sink_x_0 = qtgui.time_sink_f(
        	1024, #size
        	samp_rate/2, #samp_rate
        	"BER", #name
        	1 #number of inputs
        )
        self.qtgui_time_sink_x_0.set_update_time(0.10)
        self.qtgui_time_sink_x_0.set_y_axis(-1, 1)
        self.qtgui_time_sink_x_0.enable_tags(-1, True)
        self.qtgui_time_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "BER")
        self._qtgui_time_sink_x_0_win = sip.wrapinstance(self.qtgui_time_sink_x_0.pyqwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_time_sink_x_0_win)
        self.digital_pfb_clock_sync_xxx_0 = digital.pfb_clock_sync_ccf(sps, timing_loop_bw, (rrc_taps), nfilts, nfilts/2, 1.5, 2)
        self.digital_map_bb_0 = digital.map_bb(([0,1,2,3]))
        self.digital_lms_dd_equalizer_cc_0 = digital.lms_dd_equalizer_cc(20, 0.02, 2, qpsk)
        self.digital_gfsk_mod_0 = digital.gfsk_mod(
        	samples_per_symbol=2,
        	sensitivity=1.0,
        	bt=0.35,
        	verbose=False,
        	log=False,
        )
        self.digital_diff_decoder_bb_0 = digital.diff_decoder_bb(4)
        self.digital_costas_loop_cc_0 = digital.costas_loop_cc(phase_bw, arity)
        self.digital_constellation_receiver_cb_0 = digital.constellation_receiver_cb(qpsk, timing_loop_bw, 0.0001, 0.0002)
        self.digital_constellation_modulator_0 = digital.generic_mod(
          constellation=qpsk,
          differential=True,
          samples_per_symbol=4,
          pre_diff_code=True,
          excess_bw=0.35,
          verbose=False,
          log=False,
          )
        self.blocks_vector_source_x_0_0 = blocks.vector_source_b((170,170), True, 1, [])
        self.blocks_unpack_k_bits_bb_0_0 = blocks.unpack_k_bits_bb(8)
        self.blocks_unpack_k_bits_bb_0 = blocks.unpack_k_bits_bb(2)
        self.blks2_error_rate_0 = grc_blks2.error_rate(
        	type='BER',
        	win_size=1000,
        	bits_per_symbol=1,
        )
        self.analog_random_source_x_1 = blocks.vector_source_b(map(int, numpy.random.randint(0, 256, 1000)), True)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.digital_constellation_modulator_0, 0), (self.uhd_usrp_sink_0, 0))
        self.connect((self.blocks_vector_source_x_0_0, 0), (self.digital_constellation_modulator_0, 0))
        self.connect((self.uhd_usrp_source_0_0, 0), (self.digital_pfb_clock_sync_xxx_0, 0))
        self.connect((self.digital_costas_loop_cc_0, 0), (self.digital_constellation_receiver_cb_0, 0))
        self.connect((self.digital_map_bb_0, 0), (self.digital_diff_decoder_bb_0, 0))
        self.connect((self.digital_diff_decoder_bb_0, 0), (self.blocks_unpack_k_bits_bb_0, 0))
        self.connect((self.digital_lms_dd_equalizer_cc_0, 0), (self.digital_costas_loop_cc_0, 0))
        self.connect((self.digital_pfb_clock_sync_xxx_0, 0), (self.digital_lms_dd_equalizer_cc_0, 0))
        self.connect((self.blks2_error_rate_0, 0), (self.qtgui_time_sink_x_0, 0))
        self.connect((self.blocks_unpack_k_bits_bb_0, 0), (self.blks2_error_rate_0, 0))
        self.connect((self.blocks_unpack_k_bits_bb_0_0, 0), (self.blks2_error_rate_0, 1))
        self.connect((self.blocks_vector_source_x_0_0, 0), (self.blocks_unpack_k_bits_bb_0_0, 0))
        self.connect((self.analog_random_source_x_1, 0), (self.digital_gfsk_mod_0, 0))
        self.connect((self.digital_gfsk_mod_0, 0), (self.uhd_usrp_sink_1, 0))
        self.connect((self.digital_constellation_receiver_cb_0, 0), (self.digital_map_bb_0, 0))
        self.connect((self.blks2_error_rate_0, 0), (self.crew_db_channel_selector_0, 0))


# QT sink close method reimplementation
    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "cr_application")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_sps(self):
        return self.sps

    def set_sps(self, sps):
        self.sps = sps
        self.set_rrc_taps(firdes.root_raised_cosine(self.nfilts, self.nfilts, 1.0/float(self.sps), 0.35, 11*self.sps*self.nfilts))

    def get_nfilts(self):
        return self.nfilts

    def set_nfilts(self, nfilts):
        self.nfilts = nfilts
        self.set_rrc_taps(firdes.root_raised_cosine(self.nfilts, self.nfilts, 1.0/float(self.sps), 0.35, 11*self.sps*self.nfilts))

    def get_usrp_rf_freq(self):
        return self.usrp_rf_freq

    def set_usrp_rf_freq(self, usrp_rf_freq):
        self.usrp_rf_freq = usrp_rf_freq
        self.uhd_usrp_sink_0.set_center_freq(self.usrp_rf_freq, 0)
        self.uhd_usrp_source_0_0.set_center_freq(self.usrp_rf_freq, 0)

    def get_usrp_int_freq(self):
        return self.usrp_int_freq

    def set_usrp_int_freq(self, usrp_int_freq):
        self.usrp_int_freq = usrp_int_freq
        self._usrp_int_freq_counter.setValue(self.usrp_int_freq)
        self._usrp_int_freq_slider.setValue(self.usrp_int_freq)
        self.uhd_usrp_sink_1.set_center_freq(self.usrp_int_freq, 0)

    def get_timing_loop_bw(self):
        return self.timing_loop_bw

    def set_timing_loop_bw(self, timing_loop_bw):
        self.timing_loop_bw = timing_loop_bw
        self.digital_pfb_clock_sync_xxx_0.set_loop_bandwidth(self.timing_loop_bw)

    def get_taps(self):
        return self.taps

    def set_taps(self, taps):
        self.taps = taps

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.uhd_usrp_sink_0.set_samp_rate(self.samp_rate)
        self.uhd_usrp_source_0_0.set_samp_rate(self.samp_rate)
        self.uhd_usrp_sink_1.set_samp_rate(self.samp_rate)
        self.qtgui_time_sink_x_0.set_samp_rate(self.samp_rate/2)

    def get_rrc_taps(self):
        return self.rrc_taps

    def set_rrc_taps(self, rrc_taps):
        self.rrc_taps = rrc_taps
        self.digital_pfb_clock_sync_xxx_0.set_taps((self.rrc_taps))

    def get_qpsk(self):
        return self.qpsk

    def set_qpsk(self, qpsk):
        self.qpsk = qpsk

    def get_phase_bw(self):
        return self.phase_bw

    def set_phase_bw(self, phase_bw):
        self.phase_bw = phase_bw
        self.digital_costas_loop_cc_0.set_loop_bandwidth(self.phase_bw)

    def get_excess_bw(self):
        return self.excess_bw

    def set_excess_bw(self, excess_bw):
        self.excess_bw = excess_bw

    def get_eq_gain(self):
        return self.eq_gain

    def set_eq_gain(self, eq_gain):
        self.eq_gain = eq_gain

    def get_arity(self):
        return self.arity

    def set_arity(self, arity):
        self.arity = arity

if __name__ == '__main__':
    import ctypes
    import os
    if os.name == 'posix':
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    (options, args) = parser.parse_args()
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

