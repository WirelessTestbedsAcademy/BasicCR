Simple CR example
===================

TCD 
-----

The 'alohamaccrew_liquidofdm_tx.xml' file (in the examples/alohamac
folder) contains the flowgraph for the CR transmitter

The 'alohamaccrew_liquidofdm_rx.xml' file (in the examples/alohamac
folder) contains the flowgraph for the CR receiver

Together these describe a cognitive link. The radios of this link swap
channels once the packet error rate rises above a certain level. We
only consider two channels for now.

The tx decides to swap channel after a certain number of packets
didn't get an ack. The rx decides to swap channel if it does not detect
a packet after a N times the acktimeout time interval, where acktimeout and N
are set by the user.

The software to implement the radios themselves, included here, is a
modified instance of the Iris software defined radio libraries to
provide the described functionality.

Requirements:

  * [Included Iris code](Installtion instructions at https://github.com/softwareradiosystems)

To run the transmitter:

    $ iris alohamaccrew_liquidofdm_tx.xml

To run the transmitter:

    $ iris alohamaccrew_liquidofdm_rx.xml
    

This alohamaccrew_liquidofdm_tx.xml.xml file consists of  seven components:
  * filerawreader0
    * Reads the input data from the file "testdata.bin".
  * alohamac0
    * cordinates the access to the channel
  * liquidofdmmod1
    * Performs the OFDM modulation of the signal to be transmitted
  * signalscaler1
    * Scales the output signal for transmission by a gain of 0.9
  * usrptx1
    * Tells the USRP frontend to transmit at a centre frequency of 5.01 GHz and transmission rate of 2.5 Msample/s 
  * ursprx1
      * Tells the USRP frontend to receive at a centre frequency of 5.01 GHz and rate of 2.5 Msample/s
  * liquidofmdemod1
      * Performs the OFDM demodulation for ACK reception


This alohamaccrew_liquidofdm_tx.xml.xml file consists of  seven components:
  * filewriter0
    * Writes received data into the file "output.bin".
  * alohamac0
    * cordinates the access to the channel
  * ursprx1
      * Tells the USRP frontend to receive at a centre frequency of 5.01 GHz and rate of 2.5 Msample/s
  * ofmdemod1
      * Performs the OFDM demodulation for ACK reception
  * ofdmmod1
    * Performs the OFDM modulation of the signal to be transmitted
  * signalscaler1
    * Scales the output signal for transmission by a gain of 0.9
  * usrptx1
    * Tells the USRP frontend to transmit at a centre frequency of 5.01 GHz and transmission rate of 2.5 Msample/s 
      