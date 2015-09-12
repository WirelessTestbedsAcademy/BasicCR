# This example shows how to simulate a cognitive terminal and a legacy
# terminal using two nodes in the LOG-a-TEC testbed.

# Import some classes from vesna-alh-tools. Follow the instructions at
# https://github.com/sensorlab/vesna-alh-tools if you don't have it installed
# yet.
from vesna import alh
from vesna.alh.spectrumsensor import SpectrumSensor, SpectrumSensorProgram
from vesna.alh.signalgenerator import SignalGenerator, SignalGeneratorProgram, TxConfig

# Some standard Python modules.
import logging
import os
import time

def main():
	# Turn on logging so that we can see requests and responses in the
	# terminal.
	logging.basicConfig(level=logging.INFO)

	# We must first create an object representing the coordinator node.
	coor_campus = alh.ALHWeb("https://crn.log-a-tec.eu/communicator", 9501)

	# Our experiment will start 15 seconds into the future - this should
	# give us plenty of time to set up everything.
	time_start = time.time() + 15

	# First we set up the two transmitting nodes:
	#
	# Node 53 in the LOG-a-TEC Campus Zone will be playing the role of
	# the cognitive terminal.
	cognitive_terminal = SignalGenerator(alh.ALHProxy(coor_campus, 53))

	# All nodes we will be using are equipped with SNE-ISMTV-2400 radio
	# boards with CC2500 transceivers. From the available list of
	# signal generation configurations supported by this hardware, we
	# choose the first one: It allows to transmit in 200 kHz wide channels.
	config_list = cognitive_terminal.get_config_list()
	device_config = config_list.get_config(0, 0)

	# Here we program the "cognitive_terminal" node to transmit for 25
	# seconds on channel 110 (2422.0 MHz) and then change to channel 225
	# (2445.0 MHz). Transmission power is set to 0 dBm.
	cognitive_terminal.program( SignalGeneratorProgram(
		TxConfig(device_config, 110, 0),
		time_start=(time_start+5), 
		time_duration=25))

	cognitive_terminal.program( SignalGeneratorProgram(
		TxConfig(device_config, 225, 0),
		time_start=(time_start+32), 
		time_duration=23))


	# Node 54 in the LOG-a-TEC Campus Zone will be playing the role of
	# the legacy terminal.
	legacy_terminal = SignalGenerator(alh.ALHProxy(coor_campus, 54))

	# The legacy terminal, lacking the capability to change channels on
	# demand, is programmed to just transmit for 30 seconds on channel 114
	# (2422.8 MHz)
	legacy_terminal.program( SignalGeneratorProgram(
		TxConfig(device_config, 114, 0),
		time_start=(time_start+25),
		time_duration=30))

	# Now we setup some sensing nodes so that we can observe the
	# transmissions from different points in the testbed.

	# We will use nodes 51 and 58 for this purpose.
	sensor_node_ids = [ 51, 58 ]

	sensor_nodes = [ alh.ALHProxy(coor_campus, id) for id in sensor_node_ids ]
	sensors = [ SpectrumSensor(sensor_node) for sensor_node in sensor_nodes ]

	# For sensing, we will use the first sensing configuration (255
	# channels starting at 2.4 GHz with 400 kHz bandwidth)
	config_list = sensors[0].get_config_list()
	sweep_config = config_list.get_config(0, 0).get_full_sweep_config()

	# We instruct the nodes to perform spectrum sensing for 60 seconds.
	program = SpectrumSensorProgram(sweep_config, time_start, time_duration=60, slot_id=5)
	for sensor in sensors:
		sensor.program(program)

	# All nodes have been programmed at this point - wait for the experiment to finish.

	for sensor in sensors:
		while not sensor.is_complete(program):
			print "waiting..."
			time.sleep(2)

			if time.time() > (program.time_start + program.time_duration + 60):
				raise Exception("Something went wrong")

		print "experiment is finished. retrieving data."

		result = sensor.retrieve(program)

		# We will save the sensing results in a "data/" subdirectory.
		# Create it, if it doesn't already exist.
		try:
			os.mkdir("data")
		except OSError:
			pass

		# For each sensing node, we write the results into a CSV file.
		# The CSV file can be easily imported and plotted into tools
		# like MatLab. For example, to plot the recorded spectrogram
		# using GNU Plot, use the following commands:
		#
		# gnuplot> set pm3d map
		# gnuplot> set xlabel "frequency [MHz]"
		# gnuplot> set ylabel "time [s]"
		# gnuplot> set cblabel "power [dBm]"
		# gnuplot> unset key
		# gnuplot> splot "node17.csv" using ($2/1e6):1:3
		result.write("data/node_%d.csv" % (sensor.alh.addr,))

main()
