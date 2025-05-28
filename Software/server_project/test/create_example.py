import random
from datetime import datetime, timedelta

import firebase_admin
from firebase_admin import credentials, db

def initialize_firebase():
    """
    Khởi tạo kết nối tới Firebase Realtime Database
    """
    cred = credentials.Certificate('env/sleep-system-7d563-firebase-adminsdk-fbsvc-df9f2e8fd0.json')
    firebase_admin.initialize_app(cred, {
        "databaseURL": "https://sleep-system-7d563-default-rtdb.asia-southeast1.firebasedatabase.app/"
    })

def generate_sample_data(user_id: str, days: int = 10):
    ref = db.reference(f'users/{user_id}')
    today = datetime.now().replace(hour=0, minute=0, second=0, microsecond=0)

    for i in range(days):
        # --- tính ngày data ---
        day_dt = today - timedelta(days=i)
        date_str = day_dt.strftime('%Y-%m-%d')

        # 1) HEART RATE & SPO2 & GPS đều dùng timestamp giả (cách nhau 5 phút)
        for j in range(3):
            ts_dt = day_dt + timedelta(hours=6 + j, minutes=5 * j)
            ts = ts_dt.strftime('%Y-%m-%dT%H:%M:%S')

            # heart_rate
            ref.child('heart_rate').child(ts).set({
                'bpm': random.randint(60, 100)
            })

            # spo2
            ref.child('spo2').child(ts).set({
                'percentage': round(random.uniform(90, 100), 1)
            })

        # latest_spo2 & latest_location
        ref.child('latest_spo2').set({
            'percentage': round(random.uniform(90, 100), 1)
        })
        ref.child('latest_heart_rate').set({
            'bpm': round(random.uniform(90, 100), 1)
        })

        # calories_by_heart_rate (theo ngày)
        ref.child('calories_by_heart_rate').child(date_str).set({
            'bpm': random.randint(60, 100),
            'calories_burned_hr': round(random.uniform(50, 300), 2)
        })

        # calorise_by_accelerometer (theo ngày)
        ref.child('calorise_by_accelerometer').child(date_str).set({
            'total_calories': round(random.uniform(1000, 3000), 2)
        })

        # daily_sleep: tổng số giờ ngủ trong ngày
        sleep_duration = round(random.uniform(5, 8), 2)
        ref.child('daily_sleep').child(date_str).set({
            'sleep_duration': sleep_duration
        })

        # === SLEEP: 1 bản ghi mỗi ngày ===
        # ngủ bắt đầu giữa 21:00–23:00
        start_hour = random.randint(21, 22)
        start_min  = random.randint(0, 59)
        start_dt   = day_dt + timedelta(hours=start_hour, minutes=start_min)
        # kết thúc sau 5–8 giờ
        end_dt     = start_dt + timedelta(hours=random.uniform(5, 8))

        start_str = start_dt.strftime("%Y-%m-%dT%H:%M:%S")
        end_str   = end_dt.strftime("%Y-%m-%dT%H:%M:%S")
        key       = f"{start_str}_to_{end_str}"

        ref.child('sleep').child(key).set({
            'start_time':     start_str,
            'end_time':       end_str,
            'duration_hours': round((end_dt - start_dt).total_seconds() / 3600, 2)
        })

        # calories_daily (kết hợp cả hai nguồn)
        ref.child('calories_daily').child(date_str).set({
            'combined_daily_calories': round(random.uniform(2000, 3500), 2),
            'heart_rate_calories':      round(random.uniform(200, 500), 2),
            'accelerometer_calories':   round(random.uniform(800, 2500), 2)
        })

if __name__ == '__main__':
    initialize_firebase()
    generate_sample_data('2mrSt8vHRQd6kpPiHjuLobCrwK13')
    print("Đã tạo xong 10 bản ghi mẫu cho 2mrSt8vHRQd6kpPiHjuLobCrwK13.")
