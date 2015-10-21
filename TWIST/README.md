Simple cognitive radio example
==============================

In this example three nodes and python based control applications running on the PC are used. On two nodes one application providing radio interface should be installed. This application provides serial line interface to control the radio. Namely it is possible to set radio parameters by sending control messages over serial interface and send any message over the radio interface. On the third node the spectrum sensing application should be installed. Finally central controller application running on the PC connects to all the nodes, collects information about spectrum usage from sensor application and configures the best channel on the other two nodes.

Requirements
------------

- Working TinyOS toolchain
- Python with additional packages

    To install required Python packages, run:

        $ pip install  -r requirements.txt

It is possible to pull the Docker image build:

    docker pull wirelesstestbedsacademy/basiccr-twist
    # or build it yourself:
    docker build -t wta/basiccr-twist .

and run:

    docker run -itP --rm wirelesstestbedsacademy/basiccr-twist bash


Execution of Experiment
-----------------------

1. Compile code for sensor nodes:

        cd sensorapp
        make telosb

        cd ../radioapp
        make telosb

2. Install both on the testbed (for testbed access and basic usage refer to [TWIST documentation](https://www.twist.tu-berlin.de/tutorials/twist-getting-started.html))

    Additionally it is possible to use `twist.py` interface to install sensor node images on the nodes. This method still requires node reservation on the testbed via web interface.

        cd ..
        ./twist.py -i ./radioapp/build/telosb/main.exe -n 13
        ./twist.py -i ./sensorapp/build/telosb/main.exe -n 151 -n 152

3. Run the application

        ./basiccr.py 13 151 152

    For demonstration purposes the cognitive radio controller first changes the channel on TX node, sends a packet and only then does it on RX node (and again sends packet)

This application was done for tutorial purposes, so we encourage to look into the code.
