#!/usr/bin/env python3
"""
Wearable Device MQTT Server
---------------------------
This server connects to an MQTT broker, subscribes to topics from the wearable device,
processes the sensor data, and stores the results in Firebase using BaseService.
"""
import json
import math
from datetime import datetime, timedelta
import paho.mqtt.client as mqtt
import mqtt.topics as topics
from db.base import BaseService
from db.firebase_manager import fetch_daily_sleep

# Constants for calculations
CALORIES_MET_FACTORS = {
    "resting": 1.0,
    "light": 2.5,
    "moderate": 5.0,
    "vigorous": 8.0
}

# Globals for sleep detection
is_sleeping = False
sleep_start_time: datetime | None = None

calo_pr = 0
calo_total = 0

timestamp = datetime.now().strftime("%Y-%m-%dT%H:%M:%S")
today_pr = datetime.fromisoformat(timestamp).strftime("%Y-%m-%d")

is_new_day = False
configured = False

duration_h = 0.000001
last_duration = 0
last_duration = duration_h


# Initialize Firebase service
CREDENTIAL_PATH = "env/sleep-system-7d563-firebase.json"
DATABASE_URL = (
    "https://sleep-system-7d563-default-rtdb.asia-southeast1.firebasedatabase.app/"
)
service = BaseService(CREDENTIAL_PATH, DATABASE_URL)

# Load existing sleep information when starting the server
DEFAULT_USER_ID = "2mrSt8vHRQd6kpPiHjuLobCrwK13"
today_str = datetime.now().strftime("%Y-%m-%d")
try:
    existing = fetch_daily_sleep(DEFAULT_USER_ID, today_str)
    if existing:
        # restore previous sleep start time and sleeping state
        if existing.get("sleep_start_time"):
            sleep_start_time = datetime.fromisoformat(existing["sleep_start_time"])
            is_sleeping = existing.get("is_sleeping", False)
        # pick up accumulated duration for today
        duration_h = existing.get("sleep_duration", 0.000001)
        last_duration = duration_h
except Exception as e:
    print(f"Failed to fetch initial sleep state: {e}")

# MQTT callbacks
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe(topics.TOPIC_HEART_RATE)
    client.subscribe(topics.TOPIC_SPO2)
    client.subscribe(topics.TOPIC_ACCELEROMETER)
    client.subscribe(topics.TOPIC_GPS)
    client.subscribe(topics.MQTT_TOPIC_CALORIES)
    client.subscribe(topics.MQTT_TOPIC_SLEEP)


def on_message(client, userdata, msg):
    global is_sleeping, sleep_start_time, today_pr, calo_pr, calo_total, duration_h, is_new_day, configured, last_duration
    topic = msg.topic
    if topic == topics.MQTT_TOPIC_SLEEP:
        return
    payload = json.loads(msg.payload.decode())
    user_id = payload.get("user_id", "user123")
    timestamp = datetime.now().strftime("%Y-%m-%dT%H:%M:%S")

    today = datetime.fromisoformat(timestamp).strftime("%Y-%m-%d")

    print(f"Received message on topic {topic}: {payload}")

    if topic == topics.TOPIC_HEART_RATE:
        bpm = payload.get("bpm", 0)

        # store heart rate
        service.add_heart_rate(user_id, timestamp, bpm)
        service.set_latest_in4_hr(user_id, bpm)

        # sleep detection
        if bpm < 60 and not is_sleeping:
            is_sleeping = True
            sleep_start_time = datetime.fromisoformat(timestamp)
            print(f"Sleep detected at {sleep_start_time}")
        elif bpm > 70 and is_sleeping:
            is_sleeping = False
            end_time = datetime.fromisoformat(timestamp)
            duration_h = (end_time - sleep_start_time).total_seconds() / 3600
            service.add_sleep_record(
                user_id,
                sleep_start_time.strftime("%Y-%m-%dT%H:%M:%S"),
                timestamp,
                duration_h,
            )
            duration_h = duration_h + last_duration
            last_duration = duration_h
            service.add_daily_sleep(
                user_id,
                today,
                duration_h,
                False,
                None,
            )
        if today_pr != today:
            is_new_day = True
        if is_new_day:
            client.publish(topics.MQTT_TOPIC_SLEEP, "0", qos=1, retain=False)
            print("new day")
            is_new_day = False
            configured = True

        if calo_total > calo_pr and configured:
            client.publish(topics.MQTT_TOPIC_SLEEP, "0", qos=1, retain=False)
            print("new day")
        else:
            client.publish(topics.MQTT_TOPIC_SLEEP, str(duration_h), qos=1, retain=False)
            print("old day")
            configured = False
        today_pr = today
        if sleep_start_time != None and is_sleeping:
            now_time = datetime.fromisoformat(timestamp)
            duration_h = (now_time - sleep_start_time).total_seconds() / 3600
            service.add_daily_sleep(
                user_id,
                today,
                duration_h + last_duration,
                is_sleeping,
                sleep_start_time.strftime("%Y-%m-%dT%H:%M:%S") if is_sleeping and sleep_start_time else None,
        )
        # after accelerometer processed too, combine daily calories
        # leave combine step to message ordering or implement separately

    elif topic == topics.TOPIC_SPO2:
        spo2 = payload.get("percentage", 0)
        service.add_spo2(user_id, timestamp, spo2)
        service.set_latest_in4_spo2(user_id, spo2)

    elif topic == topics.TOPIC_ACCELEROMETER:
        total_vector = payload.get("total_vector")
        if total_vector is None:
            print("No 'total_vector' in payload")
            return

    elif topic == topics.TOPIC_GPS:
        lat = payload.get("latitude", 0)
        lon = payload.get("longitude", 0)
        alt = payload.get("altitude", 0)
        service.add_gps(user_id, timestamp, lat, lon, alt)
        service.set_latest_location(user_id, lat, lon, alt, timestamp)

    elif topic == topics.MQTT_TOPIC_CALORIES:
        calo_hr = payload.get("cal_hr", 0)
        calo_motion = payload.get("cal_motion", 0)
        calo_total = payload.get("cal_total", 0)
        service.add_calories_daily(user_id, today, calo_total, calo_hr, calo_motion)
        calo_pr = calo_total







def main():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    # Connect to broker
    client.connect("localhost", 1883, 60)
    client.loop_forever()


if __name__ == '__main__':
    main()
