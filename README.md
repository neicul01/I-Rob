# I-Rob
Remote Roboter on ESP32 with MQTT
I-Rob
================================
Content of this project is a semi-autonomous robot system for academic teaching. The robot is remotely controlled from the base via the MQTT communication interface. 


Contents
--------

* Installation_
* `Limitations`_
* `Basestation`_
    * `RobotEsp32`_

* `Roboter`_
    * `I-Rob`_
* `Hardeware`_


Installation
------------

1. Install Mosquitto on your PC
2. Customize permission for Mosquitto folders
3. edit the config-file with the following lines

::
	allow_anonymous true
	listeners 1883
Now the Broker is available on Port 1883 and anonymous clients are allowed
4. open the application service and deactivate mosquitto
5. open cmd as admin and navigate to mosquitto folder. The following command is entered there:

::
   mosquitto -v -c mosquitto.conf

The mosquitto-python-library can be installed from here `paho.mqtt.python <https://github.com/eclipse/paho.mqtt.python>'_
More information can be found at `Mosquitto Website <https://mosquitto.org/documentation/authentication-methods/>`_ .
A helpfull tutorial can be found `here <http://www.steves-internet-guide.com/mossquitto-conf-file/>`_ .


Limitations
