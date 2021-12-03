import random
import time
import string
from math import trunc
from paho.mqtt import client as mqtt_client

l_left = 0
l_right = 0
dist = 0
f1 = 0
f2 = 0
motor_stl = 0
motor_str = 0
u_akku = 1600


# Instructions for using the RobotEsp32 class :
#                                               - import RobotEsp32 as classexample
#                                               - examplebot = classexample.RobotEsp32( your broker IP-Adress, Port (optional if different from port 1883), subscriber-topic, publicher-topic)
#                                                   - example1 examplebot = classexample.RobotEsp32("123.456.789.01")  now Port = 1883, sub_topic="channel_1" and pub_topic="channel_2"
#                                                   - example2 examplebot = classexample.RobotEsp32("123.456.789.01", 1883, "mytpoic1", "mytopic2")
#                                               -Starting the MQTT- Client :    client = examplebot.connect_mqtt()
#                                                                               client.loop_start()
#                                               -Calling Function, for example: examplebot.drive(motor_left, motor_right,client)
class RobotEsp32:

    def __init__(self, ip, port=1883, sub_topic='channel_1', pub_topic='channel_2'):
        # self.__broker = 'test.mosquitto.org'
        # self.__broker = '192.168.178.51'
        self.__broker = ip
        # self.__port = 1883
        self.__port = port
        # self.__sub_topic = "isdtfdw/channel_1"
        self.__sub_topic = sub_topic
        # self.__pub_topic = "isdtfdw/channel_2"
        self.__pub_topic = pub_topic

    def connect_mqtt(self):  # -> mqtt_client:
        def on_connect(client, userdata, flags, rc):
            if rc == 0:
                print("Connected to MQTT Broker!")
            else:
                print("Failed to connect, return code %d\n", rc)

        # client.username_pw_set(username, password)
        # generate client ID with pub prefix randomly
        client_id = f'python-mqtt-{random.randint(0, 1000)}'
        client = mqtt_client.Client(client_id)
        client.on_connect = on_connect
        client.connect(self.__broker, self.__port)
        return client

    # function drive(motor left, motor right, mode, calibrate, Kp, Ki, Kd)
    # motor right and left: 0-300 in [rpm]
    # mode: 1-3 (mode = 1 Motor left is stronger; mode = 2 Motor right is stronger; mode = 3 motors have the same torque)
    # calibrate: 1 (the PID-Parameter Kp, Ki,Kd are passed by this function), 0 (the PID-Parameter of the arduino-code are used)
    def drive(self, motor_left, motor_right, client, mode=3, calib=0, p_calib=0, i_calib=0, d_calib=0) -> object:
        msg_count = 0
        if calib > 1:
            calib = 1
        if motor_left > 450:
            motor_left = 450
        elif motor_left < -450:
            motor_left = -450
        if motor_right > 450:
            motor_right = 450
        elif motor_right < -450:
            motor_right = -450
        motor_left_cor = motor_left + 450
        motor_right_cor = motor_right + 450
        ml = list(map(int, str(motor_left_cor)))
        mr = list(map(int, str(motor_right_cor)))
        p = list(map(int, str(p_calib)))
        i = list(map(int, str(i_calib)))
        d = list(map(int, str(d_calib)))
        m_s = list(map(int, str(mode)))
        c_s = list(map(int, str(calib)))
        if motor_left_cor < 10:
            ml.insert(0, 0)
            ml.insert(0, 0)
        elif motor_left_cor < 100:
            ml.insert(0, 0)
        if motor_right_cor < 10:
            mr.insert(0, 0)
            mr.insert(0, 0)
        elif motor_right_cor < 100:
            mr.insert(0, 0)
        if i_calib < 10:
            i.insert(0, 0)
        if p_calib < 10:
            p.insert(0, 0)
        if d_calib < 10:
            d.insert(0, 0)
            d.insert(0, 0)
        if d_calib < 100:
            d.insert(0, 0)
        msg = f"{ml[0]}{ml[1]}{ml[2]}{mr[0]}{mr[1]}{mr[2]}{m_s[0]}{c_s[0]}{p[0]}{p[1]}{i[0]}{i[1]}{d[0]}{d[1]}{d[2]}"
        print(msg)
        result = client.publish(self.__pub_topic, msg)
        status = result[0]

    def get_data(self, client):

        def on_message(client, userdata, msg):
            global f1
            global f2
            global motor_str
            global motor_stl
            # print(f"{msg.payload.decode()}")
            data = msg.payload.decode()
            length = len(data)
            f1 = (int(data[0]) * 1000 + int(data[1]) * 100 + int(data[2]) * 10 + int(data[3])) - 2500
            f2 = (int(data[4]) * 1000 + int(data[5]) * 100 + int(data[6]) * 10 + int(data[7])) - 2500
            motor_stl = (int(data[8]) * 1000 + int(data[9]) * 100 + int(data[10]) * 10 + int(data[11])) - 3000
            motor_str = (int(data[12]) * 1000 + int(data[13]) * 100 + int(data[14]) * 10 + int(data[15])) - 3000
            u_akku = int(data[16]) * 1000 + int(data[17]) * 100 + int(data[18]) * 10 + int(data[19])
            dist = int(data[20]) * 1000 + int(data[21]) * 100 + int(data[22]) * 10 + int(data[23])
            l_left = int(data[24]) * 1000 + int(data[25]) * 100 + int(data[26]) * 10 + int(data[27])
            l_right = int(data[28]) * 1000 + int(data[29]) * 100 + int(data[30]) * 10 + int(data[31])

            if u_akku < 1500:
                print("______battery is empty!!!______")
            # print(f"left: {line_l} right: {line_r} dist: {dist}")
            print(
                f'right: {f1} left: {f2} mo_r: {motor_stl} mo_l: {motor_str} battery:{u_akku} distance:{dist} line_left:{l_left} line_right: {l_right} time: {time.time()}')
            # client.disconnect()

        client.subscribe(self.__sub_topic)
        client.on_message = on_message
        global f1
        global f2
        global motor_str
        global motor_stl
        global l_left
        global l_right
        global dist
        return f1, f2, motor_stl, motor_str, l_left, l_right, dist
