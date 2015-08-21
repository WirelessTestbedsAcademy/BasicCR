About
=====

The Wireless Testbed Academy contains source code that can be run on open wireless experimental testbeds. The following open experimentation/testing facilities are supported:
* iMinds [w-iLab.t](http://doc.ilabt.iminds.be/ilabt-documentation/wilabfacility.html)
* TUB [TWIST](http://www.twist.tu-berlin.de/wiki)
* TCD IRIS
* TUD
* JSI [LOG-a-TEC](http://log-a-tec.eu/overview.html)

Cognitive Radio
===============
Cognitive radio (CR) is a form of wireless communication in which a transceiver can detect or find out which communication channels are in use and which are not, and instantly move into vacant channels while avoiding occupied ones.

Experiment description
======================
The simple CR experiment consists of a Tx and an Rx working in a given band. There is also a spectrum sensor that monitors that band. When the spectrum sensor detects a new transmission in the band, it directly or indirectly (through a database) conveys this information to the initial Tx-Rx pair. Consequently, the Tx-Rx pair switch to another band.

A more complex CR experiment would use a more advanced spectrum sensing, detection and switching mechanism.

Simple cognitive radio source code
==================================
This code gives a preview on how a cognitive radio (CR) experiment is implemented using the APIs and hardware available on the individual testbeds. Note that you need to request an account to obtain access right to the testbeds in order to be able to try and run the code. The access to most of the testbeds is open.

For more details, browse the testbed specific CR code, including the respective README file and comments from the source code.


Other resources
===============
Videos of demos and tutorials on the facilities are available on the project [web site](http://www.crew-project.eu/demos) and on the [Youtube channel](https://www.youtube.com/user/FP7ictCREW). 


Acknowledgements
================
This work was partly funded by the EC under grant agreement n°258301 - Cognitive Radio Experimentation World. 
