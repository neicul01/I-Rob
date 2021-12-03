import RobotEsp32 as classbsp
import random
from paho.mqtt import client as mqtt_client
import time
import string
from math import trunc

myrobo = classbsp.RobotEsp32("192.168.178.51", 1883, "isdtfdw/channel_1", "isdtfdw/channel_2")

if __name__ == '__main__':
    client = myrobo.connect_mqtt()
    client.loop_start()
    i = 0
    z = 0
    s_int = 0
    time_now = 0
    pre_time = 0
    cycle_time = 0
    cycle_control=0
    store_value = 1
    s_intl=0;
    while True:
        # myrobo.get_distance(client)
        #data = myrobo.get_data(client)
        rot = myrobo.get_data(client)
        if z > 100:
            s = input('speed:')
            s_int = int(s)
            z = 0
        myrobo.drive(s_int, s_int,client,1,0,5,8,2)
        # print('hallo')
        time.sleep(0.005)
        #i += 1
        z += 1
    client.disconnect()
    
