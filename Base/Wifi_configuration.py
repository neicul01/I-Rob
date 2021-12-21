# todo's : 0. make sure the roboter is off !
#          1.adapt SSID;Passwort;Broker-ip; in line 17
#          2.adapt COM-Port-number in line 29 and 31 ->if you do not now the port-number than:  I set 'com_port_known' in line 19 to False
#                                                                                               II run the script
#                                                                                               III connect the ESP32 via USB (ATTENTION: never conect via usb while the roboter is on)
#                                                                                               IV read the COM-Port-number from output of the script and adapt the COM-Port-number in line 29 and 31
#                                                                                               V set 'com_port_known' in line 19 to True
#          3.run the script
#          4.connect the ESP32 via USB with the Computer(ATTENTION: never connect via usb while the roboter is on)
#          5.if the connection to the WLAN and the Broker succeeded, the script will print 'Connection succeeded' -> if not check your SSID;Passwort;Broker-ip; in line 17 and your Mqtt-Brokersettings
#          6.the Roboter is ready to use


import time
import serial.tools.list_ports

SSID = 'FRITZ!Box 7530;12345678912345678910;123.456.123.45;'  # 'SSID;Passwort;Broker-ip;'
msg_SSID = bytes(SSID, 'utf-8')
com_port_known = True

while True:
    myport = [tuple(p) for p in list(serial.tools.list_ports.comports())]
    if myport == []:
        print("no devices connected")
    else:
        time.sleep(0.5)
        print(myport)
        if com_port_known:
            esp32_port = [port for port in myport if 'COM4' in port][0]
            if esp32_port in myport:
                ser = serial.Serial(port='COM4',
                                    baudrate=115200,
                                    bytesize=serial.EIGHTBITS,
                                    parity=serial.PARITY_NONE,
                                    timeout=2)

                while True:
                    if ser.isOpen():
                        ser.write(msg_SSID)
                        msg = []
                        msg2 = []
                        i = 0
                        try:
                            while i < 10:
                                # msg.insert(i,ser.read().decode())
                                msg.insert(i, ser.read())
                                i += 1
                            # print(msg)
                            msg2 = [x.decode('utf-8') for x in msg]
                            message = ''.join(map(str, msg2))
                            print(message)
                        except Exception:
                            print("error communicating...: " + str())
                    else:
                        print("cannot open serial port ")

