I-Rob
================================
Content of this project is a semi-autonomous robot system for academic teaching. The robot is remotely controlled from the base via the MQTT communication interface. 


Contents
--------

* Installation
* Limitations
* Basestation
    * RobotEsp32

* Roboter
    * I-Rob
* Hardeware


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
-----------------

The following feature are unimplemented.

Calibration of the line-sensors.

Basestation
-------------

Detailed API documentation is available through **pydoc**. Samples are available in the **examples** directory.

The package provides two modules, a full client and a helper for simple publishing.

RobotEsp32 userinstruction
**************************

The RobotEsp32 class contains all necessary functions for controlling and reading sensor data.

.. code:: python

    from paho.mqtt import client as mqtt_client
    import RobotEsp32 as classexample

    # Initatisation
    examplebot = classexample.RobotEsp32( your broker IP-Adress, Port (optional if different from port 1883), subscriber-topic, publicher-topic) 

