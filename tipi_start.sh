#!/bin/sh
sudo mosquitto -c /etc/mosquitto/mosquitto.conf
mosquitto &

/home/pi/sol/tipi/tipi &


