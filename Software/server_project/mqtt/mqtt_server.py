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
import math
from datetime import datetime, timedelta
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
import mqtt.topics as topics

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
    cred = credentials.Certificate("sleep-system-7d563-firebase-adminsdk-fbsvc-df9f2e8fd0.json")
    firebase_admin.initialize_app(cred, {
        "databaseURL": "https://sleep-system-7d563-default-rtdb.asia-southeast1.firebasedatabase.app/"
    })
    db_v = db.reference("/")
    return db_v

# MQTT callbacks
def on_connect(client, userdata, flags, rc):
    """Callback for when the client connects to the MQTT broker"""
    print(f"Connected with result code {rc}")
    
    # Subscribe to all relevant topics from the wearable device
    client.subscribe(topics.TOPIC_HEART_RATE)
    client.subscribe(topics.TOPIC_SPO2)
    client.subscribe(topics.TOPIC_ACCELEROMETER)
    client.subscribe(topics.TOPIC_GPS)

def on_message(client, userdata, msg):
    """Callback for when a message is received from the MQTT broker"""
    topic = msg.topic
    payload = json.loads(msg.payload.decode())
    counter = 0
    
    # print(f"Received message on topic {topic}: {payload}")
    
    # Process data based on topic
    if topic == topics.TOPIC_HEART_RATE:
        process_heart_rate(payload, userdata["db"])
        if counter == 0:
            counter += 1
    elif topic == topics.TOPIC_SPO2:
        process_spo2(payload, userdata["db"])
    elif topic == topics.TOPIC_ACCELEROMETER:
        process_accelerometer(payload, userdata["db"])
        if counter == 1:
            counter += 1
    elif topic == topics.TOPIC_GPS:
        process_gps(payload, userdata["db"])

    
    if counter == 2:
        # Calculate combined daily calories
        user_id = payload.get("user_id", "default_user")
        combined_daily_calories = calculate_combined_daily_calories(userdata["db"], user_id)
        # print(f"Combined daily calories for user {user_id}: {combined_daily_calories}")
        counter = 0

# Data processing functions
def process_heart_rate(data, root_ref):
    """Process heart rate data, calculate calories burned based on heart rate, and store in Firebase"""
    user_id = data.get("user_id", "default_user")
    heart_rate = data.get("bpm", 0)
    timestamp = data.get("timestamp", datetime.now().strftime("%Y-%m-%dT%H:%M:%S"))
    
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
            store_sleep_data(user_id, sleep_start_time.strftime("%Y-%m-%dT%H:%M:%S"), 
                             timestamp, sleep_duration.total_seconds() / 3600, db)
            sleep_start_time = None

    # Tính toán lượng calo tiêu thụ dựa trên nhịp tim
    # Lấy các thông số bổ sung (nếu có)
    weight_kg = data.get("weight_kg", 70)  # sử dụng giá trị mặc định 70kg nếu không có dữ liệu
    age = data.get("age", 30)              # sử dụng giá trị mặc định 30 tuổi nếu không có
    epoch_minutes = data.get("epoch_minutes", 1)  # khoảng thời gian đo, mặc định 1 phút

    # Tính calo tiêu hao theo công thức:
    # cal/min = (-55.0969 + 0.6309 × heart_rate + 0.1988 × weight_kg + 0.2017 × age) / 4.184
    cal_per_min_hr = (-55.0969 + (0.6309 * heart_rate) + (0.1988 * weight_kg) + (0.2017 * age)) / 4.184
    # Đảm bảo giá trị không âm
    cal_per_min_hr = max(cal_per_min_hr, 0)
    calories_burned_hr = cal_per_min_hr * epoch_minutes
    heart_rate_ref = root_ref.child(f"users/{user_id}/heart_rate/{timestamp}")
    write_data(heart_rate_ref, {
        "bpm": heart_rate,
        })
    # Store heart rate data in Firebase (bao gồm calo tiêu hao)
    today = datetime.fromisoformat(timestamp).strftime("%Y-%m-%d")
    
    # Update latest values in the user document
    update_data_ref = root_ref.child(f"users/{user_id}/calories_by_heart_rate/{today}")
    user_data = read_data(update_data_ref)
    calories_burned_hr_prev = user_data.get("calories_burned_hr", 0) if user_data else 0
    update_data(update_data_ref, {
        "bpm": heart_rate,
        "calories_burned_hr": calories_burned_hr + calories_burned_hr_prev,
    })


def process_spo2(data, root_ref):
    """Process SpO2 data and store in Firebase"""
    user_id = data.get("user_id", "default_user")
    spo2 = data.get("percentage", 0)
    timestamp = data.get("timestamp", datetime.now().strftime("%Y-%m-%dT%H:%M:%S"))
    
    # Validate SpO2 data
    if spo2 < 80 or spo2 > 100:
        print(f"Invalid SpO2 value: {spo2}")
        return
    spo2_ref = root_ref.child(f"users/{user_id}/spo2/{timestamp}")
    write_data(spo2_ref, {
        "percentage": spo2,
        })
    # Update latest values
    update_data(root_ref.child(f"users/{user_id}/latest_spo2"), {
        "percentage": spo2,
    })

def process_accelerometer(data, root_ref):
    """
    Process accelerometer data using the aggregated total vector acceleration,
    calculate activity level and calories burned, then store the results in Firebase.

    This function assumes that the wearable device sends an aggregated "total_vector" value
    (i.e. the sum of acceleration magnitudes over an epoch, e.g. 1 minute) instead of individual
    acceleration samples for each axis.
    """
    user_id = data.get("user_id", "default_user")
    timestamp = data.get("timestamp", datetime.now().strftime("%Y-%m-%dT%H:%M:%S"))
    
    # Lấy giá trị tổng của vector gia tốc (đã được tính toán trên thiết bị)
    total_vector = data.get("total_vector")
    if total_vector is None:
        print("No 'total_vector' found in accelerometer data.")
        return

    # Assume weight in kilograms; nếu không có, mặc định 70kg
    weight_kg = data.get("weight_kg", 70)
    
    # Duration (epoch) tính bằng phút (mặc định là 1 phút nếu không gửi kèm)
    epoch_minutes = data.get("epoch_minutes", 1)
    
    # Xác định trạng thái hoạt động dựa trên tổng vector gia tốc
    if total_vector < 100:
        activity_state = "resting"
    elif total_vector < 200:
        activity_state = "light"
    elif total_vector < 400:
        activity_state = "moderate"
    else:
        activity_state = "vigorous"

    # Tính lượng calo tiêu hao dựa trên công thức Freedson VM3:
    # EE (kcal/phút) = 0.001064 * total_vector + 0.087512 * weight - 5.500229
    cal_per_min_acc = 0.001064 * total_vector + 0.087512 * weight_kg - 5.500229
    cal_per_min_acc = max(cal_per_min_acc, 0)  # đảm bảo không nhận được giá trị âm
    calories_burned = cal_per_min_acc * epoch_minutes
    
    # Cập nhật tổng số calo cho ngày hiện tại
    current_time = datetime.fromisoformat(timestamp)
    today = current_time.strftime("%Y-%m-%d")
    daily_stats_ref = root_ref.child(f"users/{user_id}/calorise_by_accelerometer/{today}")
    
    daily_stats = daily_stats_ref.get()
    if daily_stats is not None:
        current_calories = daily_stats.to_dict().get("total_calories", 0)
    else:
        current_calories = 0
    
    update_data(daily_stats_ref, {
        "total_calories": current_calories + calories_burned,
    })
    

def process_gps(data, root_ref):
    """Process GPS data and store in Firebase"""
    user_id = data.get("user_id", "default_user")
    latitude = data.get("latitude", 0)
    longitude = data.get("longitude", 0)
    altitude = data.get("altitude", 0)
    timestamp = data.get("timestamp", datetime.now().strftime("%Y-%m-%dT%H:%M:%S"))
    
    # Store GPS data in Firebase
    # gps_ref = db.collection("users").document(user_id).collection("gps").document(timestamp)
    gps_ref = root_ref.child(f"users/{user_id}/gps/{timestamp}")
    gps_ref.set({
        "latitude": latitude,
        "longitude": longitude,
        "altitude": altitude,
        "timestamp": timestamp
    })
    
    # Update latest values
    update_data(root_ref.child(f"users/{user_id}/latest_location"), {
        "latitude": latitude,
        "longitude": longitude,
        "altitude": altitude,
        "timestamp": timestamp
    })

def store_sleep_data(user_id, start_time, end_time, duration_hours, root_ref):
    """Store sleep session data in Firebase"""
    sleep_id = f"{start_time}_to_{end_time}"
    
    # sleep_ref = db.collection("users").document(user_id).collection("sleep").document(sleep_id)
    sleep_ref = root_ref.child(f"users/{user_id}/sleep/{sleep_id}")
    sleep_ref.set({
        "start_time": start_time,
        "end_time": end_time,
        "duration_hours": duration_hours
    })
    
    # Update daily stats
    start_date = datetime.fromisoformat(start_time).strftime("%Y-%m-%d")
    daily_stats_ref = root_ref.child(f"users/{user_id}/daily_sleep/{start_date}")
    update_data(daily_stats_ref, {
        "sleep_duration": duration_hours
    })


# Hàm để lấy tham chiếu đến một vị trí cụ thể trong cơ sở dữ liệu
def get_database_reference(path='/'):
    """Trả về một tham chiếu đến vị trí được chỉ định trong Realtime Database."""
    return db.reference(path)


# Hàm để ghi dữ liệu vào cơ sở dữ liệu
def write_data(ref, data):
    """Ghi dữ liệu vào vị trí được chỉ định."""
    try:
        ref.set(data)
        print("Dữ liệu đã được ghi thành công!")
    except Exception as e:
        print(f"Lỗi khi ghi dữ liệu: {e}")


def read_data(ref):
    """Read data from the specified location in the database.
    
    Args:
        ref: A database reference object pointing to the location to read from.
    
    Returns:
        dict or None: The data read from the database, or None if no data is found.
    """
    try:
        data = ref.get()
        if data:
            return data
        else:
            print("No data found at this location.")
            return None
    except Exception as e:
        print(f"Error while reading data: {e}")
        return None


def update_data(ref, data):
    """
    Cập nhật dữ liệu tại vị trí được chỉ định.

    Args:
        ref (Reference): Tham chiếu đến vị trí cần cập nhật.
        data (dict): Dữ liệu cần cập nhật.

    Returns:
        None
    """
    try:
        ref.update(data)
        print("Dữ liệu đã được cập nhật thành công!")
    except Exception as e:
        print(f"Lỗi khi cập nhật dữ liệu: {e}")


# Hàm để xóa dữ liệu từ cơ sở dữ liệu
def delete_data(ref):
    """
    Xóa dữ liệu tại vị trí được chỉ định.

    Args:
        ref (Reference): Tham chiếu đến vị trí cần xóa.
    """
    try:
        ref.delete()
        print("Dữ liệu đã được xóa thành công!")
    except Exception as e:
        print(f"Lỗi khi xóa dữ liệu: {e}")

def calculate_combined_daily_calories(root_ref, user_id):
    """
    Tính toán tổng số calo tiêu hao hàng ngày từ dữ liệu nhịp tim và gia tốc.
    
    Args:
        root_ref: Tham chiếu đến cơ sở dữ liệu Firebase.
        user_id: ID của người dùng cần tính toán.
    
    Returns:
        dict: Tổng số calo tiêu hao hàng ngày từ nhịp tim và gia tốc.
    """
    today = datetime.now().strftime("%Y-%m-%d")
    heart_rate_ref = root_ref.child(f"users/{user_id}/calories_by_heart_rate/{today}")
    accelerometer_ref = root_ref.child(f"users/{user_id}/calorise_by_accelerometer/{today}")
    
    heart_rate_data = read_data(heart_rate_ref)
    accelerometer_data = read_data(accelerometer_ref)
    
    total_calories_heart_rate = heart_rate_data.get("calories_burned_hr", 0) if heart_rate_data else 0
    total_calories_accelerometer = accelerometer_data.get("total_calories", 0) if accelerometer_data else 0

    combined_daily_calories = (total_calories_heart_rate + total_calories_accelerometer) / 2

    calories_ref = root_ref.child(f"users/{user_id}/calories_daily/{today}")
    update_data(calories_ref, {
        "combined_daily_calories": combined_daily_calories,
        "heart_rate_calories": total_calories_heart_rate,
        "accelerometer_calories": total_calories_accelerometer
    })

    return combined_daily_calories

    