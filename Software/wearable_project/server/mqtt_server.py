#!/usr/bin/env python3
"""
Wearable Device MQTT Server
---------------------------
This server connects to an MQTT broker, subscribes to topics from the wearable device,
processes the sensor data, and stores the results in Firebase.

Sensors:
- MAX30102: Heart rate and SpO2
- LIS2DH12TR: Accelerometer for activity tracking
- GPS 511-TESEO-LIV3R: Location tracking

Computed metrics:
- Heart rate (bpm)
- SpO2 (%)
- Calories burned
- Sleep duration
- GPS location
"""

import json
import time
import math
from datetime import datetime, timedelta
import paho.mqtt.client as mqtt
import firebase_admin
from firebase_admin import credentials
from firebase_admin import firestore

# Constants for calculations
CALORIES_MET_FACTORS = {
    "resting": 1.0,
    "light": 2.5,
    "moderate": 5.0,
    "vigorous": 8.0
}

# Global variables
last_activity_state = "resting"
last_activity_time = datetime.now()
sleep_start_time = None
is_sleeping = False

# Firebase setup
def setup_firebase():
    """Initialize Firebase connection"""
    # In production, use a secure method to store credentials
    cred = credentials.Certificate("firebase_credentials.json")
    firebase_admin.initialize_app(cred)
    db = firestore.client()
    return db

# MQTT callbacks
def on_connect(client, userdata, flags, rc):
    """Callback for when the client connects to the MQTT broker"""
    print(f"Connected with result code {rc}")
    
    # Subscribe to all relevant topics from the wearable device
    client.subscribe("wearable/heartrate")
    client.subscribe("wearable/spo2")
    client.subscribe("wearable/accelerometer")
    client.subscribe("wearable/gps")
    client.subscribe("wearable/battery")

def on_message(client, userdata, msg):
    """Callback for when a message is received from the MQTT broker"""
    topic = msg.topic
    payload = json.loads(msg.payload.decode())
    
    print(f"Received message on topic {topic}: {payload}")
    
    # Process data based on topic
    if topic == "wearable/heartrate":
        process_heart_rate(payload, userdata["db"])
    elif topic == "wearable/spo2":
        process_spo2(payload, userdata["db"])
    elif topic == "wearable/accelerometer":
        process_accelerometer(payload, userdata["db"])
    elif topic == "wearable/gps":
        process_gps(payload, userdata["db"])
    elif topic == "wearable/battery":
        process_battery(payload, userdata["db"])

# Data processing functions
def process_heart_rate(data, db):
    """Process heart rate data and store in Firebase"""
    user_id = data.get("user_id", "default_user")
    heart_rate = data.get("bpm", 0)
    timestamp = data.get("timestamp", datetime.now().isoformat())
    
    # Validate heart rate data
    if heart_rate < 30 or heart_rate > 220:
        print(f"Invalid heart rate value: {heart_rate}")
        return
    
    # Check for potential sleep state based on heart rate
    global is_sleeping, sleep_start_time
    if heart_rate < 60 and not is_sleeping:
        # Low heart rate might indicate sleep onset
        is_sleeping = True
        sleep_start_time = datetime.fromisoformat(timestamp)
        print(f"Sleep detected at {sleep_start_time}")
    elif heart_rate > 70 and is_sleeping:
        # Higher heart rate might indicate waking up
        is_sleeping = False
        if sleep_start_time:
            sleep_duration = datetime.fromisoformat(timestamp) - sleep_start_time
            store_sleep_data(user_id, sleep_start_time.isoformat(), 
                            timestamp, sleep_duration.total_seconds() / 3600, db)
            sleep_start_time = None
    
    # Store heart rate data in Firebase
    heart_rate_ref = db.collection("users").document(user_id).collection("heart_rate").document(timestamp)
    heart_rate_ref.set({
        "bpm": heart_rate,
        "timestamp": timestamp
    })
    
    # Update latest values
    db.collection("users").document(user_id).set({
        "latest_heart_rate": {
            "bpm": heart_rate,
            "timestamp": timestamp
        }
    }, merge=True)

def process_spo2(data, db):
    """Process SpO2 data and store in Firebase"""
    user_id = data.get("user_id", "default_user")
    spo2 = data.get("percentage", 0)
    timestamp = data.get("timestamp", datetime.now().isoformat())
    
    # Validate SpO2 data
    if spo2 < 80 or spo2 > 100:
        print(f"Invalid SpO2 value: {spo2}")
        return
    
    # Store SpO2 data in Firebase
    spo2_ref = db.collection("users").document(user_id).collection("spo2").document(timestamp)
    spo2_ref.set({
        "percentage": spo2,
        "timestamp": timestamp
    })
    
    # Update latest values
    db.collection("users").document(user_id).set({
        "latest_spo2": {
            "percentage": spo2,
            "timestamp": timestamp
        }
    }, merge=True)

def process_accelerometer(data, db):
    """Process accelerometer data, calculate activity level and calories, and store in Firebase"""
    user_id = data.get("user_id", "default_user")
    x = data.get("x", 0)
    y = data.get("y", 0)
    z = data.get("z", 0)
    timestamp = data.get("timestamp", datetime.now().isoformat())
    
    # Calculate magnitude of acceleration
    magnitude = math.sqrt(x**2 + y**2 + z**2)
    
    # Determine activity state based on acceleration magnitude
    # These thresholds would need to be calibrated for the specific device
    activity_state = "resting"
    if magnitude > 1.5:
        activity_state = "light"
    if magnitude > 2.5:
        activity_state = "moderate"
    if magnitude > 4.0:
        activity_state = "vigorous"
    
    # Calculate calories burned since last measurement
    global last_activity_state, last_activity_time
    current_time = datetime.fromisoformat(timestamp)
    time_diff = (current_time - last_activity_time).total_seconds() / 3600  # in hours
    
    # Assume a default weight of 70kg if not provided
    weight_kg = data.get("weight_kg", 70)
    
    # Calculate calories using MET values
    # Formula: Calories = MET * weight(kg) * time(hours)
    calories_burned = CALORIES_MET_FACTORS[activity_state] * weight_kg * time_diff
    
    # Store accelerometer and activity data in Firebase
    activity_ref = db.collection("users").document(user_id).collection("activity").document(timestamp)
    activity_ref.set({
        "x": x,
        "y": y,
        "z": z,
        "magnitude": magnitude,
        "activity_state": activity_state,
        "calories_burned": calories_burned,
        "timestamp": timestamp
    })
    
    # Update cumulative calories for the day
    today = current_time.strftime("%Y-%m-%d")
    daily_stats_ref = db.collection("users").document(user_id).collection("daily_stats").document(today)
    
    # Get current daily total or start with 0
    daily_stats = daily_stats_ref.get()
    if daily_stats.exists:
        current_calories = daily_stats.to_dict().get("total_calories", 0)
    else:
        current_calories = 0
    
    # Update daily stats
    daily_stats_ref.set({
        "total_calories": current_calories + calories_burned,
        "last_updated": timestamp
    }, merge=True)
    
    # Update latest values
    db.collection("users").document(user_id).set({
        "latest_activity": {
            "state": activity_state,
            "calories_burned_today": current_calories + calories_burned,
            "timestamp": timestamp
        }
    }, merge=True)
    
    # Update global variables for next calculation
    last_activity_state = activity_state
    last_activity_time = current_time

def process_gps(data, db):
    """Process GPS data and store in Firebase"""
    user_id = data.get("user_id", "default_user")
    latitude = data.get("latitude", 0)
    longitude = data.get("longitude", 0)
    altitude = data.get("altitude", 0)
    timestamp = data.get("timestamp", datetime.now().isoformat())
    
    # Store GPS data in Firebase
    gps_ref = db.collection("users").document(user_id).collection("gps").document(timestamp)
    gps_ref.set({
        "latitude": latitude,
        "longitude": longitude,
        "altitude": altitude,
        "timestamp": timestamp
    })
    
    # Update latest values
    db.collection("users").document(user_id).set({
        "latest_location": {
            "latitude": latitude,
            "longitude": longitude,
            "altitude": altitude,
            "timestamp": timestamp
        }
    }, merge=True)

def process_battery(data, db):
    """Process battery data and store in Firebase"""
    user_id = data.get("user_id", "default_user")
    percentage = data.get("percentage", 0)
    timestamp = data.get("timestamp", datetime.now().isoformat())
    
    # Store battery data in Firebase
    db.collection("users").document(user_id).set({
        "device_battery": {
            "percentage": percentage,
            "timestamp": timestamp
        }
    }, merge=True)

def store_sleep_data(user_id, start_time, end_time, duration_hours, db):
    """Store sleep session data in Firebase"""
    sleep_id = f"{start_time}_to_{end_time}"
    
    sleep_ref = db.collection("users").document(user_id).collection("sleep").document(sleep_id)
    sleep_ref.set({
        "start_time": start_time,
        "end_time": end_time,
        "duration_hours": duration_hours
    })
    
    # Update latest values
    db.collection("users").document(user_id).set({
        "latest_sleep": {
            "start_time": start_time,
            "end_time": end_time,
            "duration_hours": duration_hours
        }
    }, merge=True)
    
    # Update daily stats
    start_date = datetime.fromisoformat(start_time).strftime("%Y-%m-%d")
    daily_stats_ref = db.collection("users").document(user_id).collection("daily_stats").document(start_date)
    daily_stats_ref.set({
        "sleep_duration": duration_hours
    }, merge=True)

def main():
    """Main function to run the MQTT server"""
    # Setup Firebase
    db = setup_firebase()
    
    # Setup MQTT client
    client = mqtt.Client()
    client.user_data_set({"db": db})
    client.on_connect = on_connect
    client.on_message = on_message
    
    # Connect to MQTT broker
    # In production, use secure connection parameters
    mqtt_broker = "mqtt.example.com"  # Replace with your MQTT broker address
    mqtt_port = 1883
    
    try:
        client.connect(mqtt_broker, mqtt_port, 60)
        print(f"Connected to MQTT broker at {mqtt_broker}:{mqtt_port}")
        
        # Start the MQTT loop
        client.loop_forever()
    except Exception as e:
        print(f"Error connecting to MQTT broker: {e}")

if __name__ == "__main__":
    main()
