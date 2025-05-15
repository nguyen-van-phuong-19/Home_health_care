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

# Constants for calculations
CALORIES_MET_FACTORS = {
    "resting": 1.0,
    "light": 2.5,
    "moderate": 5.0,
    "vigorous": 8.0
}

# Globals for sleep detection
last_activity_state = "resting"
is_sleeping = False
sleep_start_time: datetime = None

# Initialize Firebase service
CREDENTIAL_PATH = "env/sleep-system-7d563-firebase-adminsdk-fbsvc-df9f2e8fd0.json"
DATABASE_URL = (
    "https://sleep-system-7d563-default-rtdb.asia-southeast1.firebasedatabase.app/"
)
service = BaseService(CREDENTIAL_PATH, DATABASE_URL)

# MQTT callbacks
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe(topics.TOPIC_HEART_RATE)
    client.subscribe(topics.TOPIC_SPO2)
    client.subscribe(topics.TOPIC_ACCELEROMETER)
    client.subscribe(topics.TOPIC_GPS)


def on_message(client, userdata, msg):
    global is_sleeping, sleep_start_time
    payload = json.loads(msg.payload.decode())
    user_id = payload.get("user_id", "user123")
    topic = msg.topic
    last_bpm = 0;
    timestamp = payload.get(
        "timestamp",
        datetime.now().strftime("%Y-%m-%dT%H:%M:%S")
    )

    print(f"Received message on topic {topic}: {payload}")

    if topic == topics.TOPIC_HEART_RATE:
        bpm = payload.get("bpm", 0)
        last_bpm = bpm
        # store heart rate
        service.add_heart_rate(user_id, timestamp, bpm)

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
            service.add_daily_sleep(
                user_id,
                sleep_start_time.strftime("%Y-%m-%d"),
                duration_h,
            )

        # calories by heart rate
        weight = payload.get("weight_kg", 75)
        age = payload.get("age", 23)
        epoch_min = payload.get("epoch_minutes", 1)
        cal_per_min = (
            -55.0969 + 0.6309 * bpm + 0.1988 * weight + 0.2017 * age
        ) / 4.184
        cal_per_min = max(cal_per_min, 0)
        cal_burned = cal_per_min * epoch_min
        today = datetime.fromisoformat(timestamp).strftime("%Y-%m-%d")
        service.add_calories_by_hr(user_id, today, bpm, cal_burned)
        service.set_latest_in4_hr(user_id, bpm)

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
        weight = payload.get("weight_kg", 70)
        epoch_min = payload.get("epoch_minutes", 1)
        cal_per_min_acc = (
            0.001064 * total_vector + 0.087512 * weight - 5.500229
        )
        cal_per_min_acc = max(cal_per_min_acc, 0)
        cal_acc = cal_per_min_acc * epoch_min
        today = datetime.fromisoformat(timestamp).strftime("%Y-%m-%d")
        service.add_calories_by_accel(user_id, today, cal_acc)

    elif topic == topics.TOPIC_GPS:
        lat = payload.get("latitude", 0)
        lon = payload.get("longitude", 0)
        alt = payload.get("altitude", 0)
        service.add_gps(user_id, timestamp, lat, lon, alt)
        service.set_latest_location(user_id, lat, lon, alt, timestamp)


def main():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    # Connect to broker
    client.connect("localhost", 1883, 60)
    client.loop_forever()


if __name__ == '__main__':
    main()
