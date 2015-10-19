Simple cognitive radio example
==============================

In this example three nodes and python based control applications running on the PC are used. On two nodes one application providing radio interface should be installed. This application provides serial line interface to control the radio. Namely it is possible to set radio parameters by sending control messages over serial interface and send any message over the radio interface. On the third node the spectrum sensing application should be installed. Finally central controller application running on the PC connects to all the nodes, collects information about spectrum usage from sensor application and configures the best channel on the other two nodes.

Requirements
------------

- Working TinyOS toolchain
- Python with additional packages

    To install required Python packages, run:

        $ pip install  -r requirements.txt

It is possible to run the Docker image build from attached Dockerfile in this repository

    docker build -t wta/basiccr-twist .
    docker run -itP --rm wta/basiccr-twist bash


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
        ./twist.py -i ./radioapp/build/telosb/main.exe -a
        ./twist.py -i ./sensorapp/build/telosb/main.exe -n 151

3. Connect to SSH tunnels

        ./twist.py -s -a

4. Run the application
