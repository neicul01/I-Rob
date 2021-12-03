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


Installation (Windows 10)
----------------------

1. Install Mosquitto-Broker on your PC ( a helpful tutorial can be found `here <http://www.steves-internet-guide.com/install-mosquitto-broker/>`_ )
2. Customize permission for Mosquitto folders (Depending on the setting, the folder may have limited access rights for the user. These must be changed to full access rights.)
3. edit the config-file with the following lines

::

	allow_anonymous true
	listeners 1883

        
Now the Broker is available on Port 1883 and anonymous clients are allowed

4. open the application "service" and stop the service "Mosquitto-Broker"
5. open cmd as admin and navigate to mosquitto folder. The following command is entered there:

::

   mosquitto -v -c mosquitto.conf

The mosquitto-python-library can be installed from `paho.mqtt.python <https://github.com/eclipse/paho.mqtt.python>`_ .
More information can be found at `Mosquitto Website <https://mosquitto.org/documentation/authentication-methods/>`_ .
A helpful tutorial concerning the mossquitto-config-file can be found `here <http://www.steves-internet-guide.com/mossquitto-conf-file/>`_ .

Limitations
-----------------

The following feature are unimplemented.

Calibration of the line-sensors.

Basestation
-------------



RobotEsp32 userinstruction
**************************

The RobotEsp32 class contains all necessary functions for controlling and reading sensor data.

.. code:: python

    from paho.mqtt import client as mqtt_client
    import RobotEsp32 as classexample

'classexample' can be individually adapted

To initialize a connection the following function is called:

.. code:: python

    examplebot = classexample.RobotEsp32( your broker ip-adress, port (optional if different from port 1883), subscriber-topic, publicher-topic)
Here are two examples for the initialization:  

example 1 ( default settings are used: Port = 1883, sub_topic="channel_1" and pub_topic="channel_2")

.. code:: python

    examplebot = classexample.RobotEsp32("123.456.789.01")  
    


example 2 (individual settings are used: Port= 1883, sub_topic="mytpoic1", pub_topic="mytopic2")

.. code:: python

    examplebot = classexample.RobotEsp32("123.456.789.01", 1883, "mytpoic1", "mytopic2")

Starting the MQTT- Client :    

.. code:: python

    client = examplebot.connect_mqtt()
    client.loop_start()
    
Now all funktions are available 

for example: 

.. code:: python

    examplebot.drive(motor_left, motor_right,client)


Roboter
-------------

The robot software can be uploaded via the Arduino IDE.

1. Installing ESP32 in Arduino IDE (helpful tutorial can be found `here <https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/>`_ ).

2. Installing the necessary librarys: 
                                      WiFi by Arduino
                                      
                                      PubSubClient by Nick O'Leary
                                      
                                      ESP32Encoder by Kevin Harrington

3. Make sure the roboter is off, than connecting the ESP32 via USB and upload the skretch
4. To make it easier for the user to set the Wi-Fi connection as well as the connection to the Mosquitto broker, a Python script can be used instead of changing the Arduino script.
