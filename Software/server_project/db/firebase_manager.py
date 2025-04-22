import json
import os
from typing import Any, Dict, Optional

import firebase_admin
from firebase_admin import credentials, db

# Initialize Firebase app (singleton)
_firebase_app: Optional[firebase_admin.App] = None


def init_firebase(credential_path: str, database_url: str) -> None:
    """
    Initialize the Firebase Admin SDK.

    Args:
        credential_path: Path to the service account JSON file.
        database_url: URL of the Realtime Database, e.g. https://<project>.firebaseio.com
    """
    global _firebase_app
    if _firebase_app is None:
        cred = credentials.Certificate(credential_path)
        _firebase_app = firebase_admin.initialize_app(cred, {
            'databaseURL': database_url
        })


def get_reference(path: str):
    """
    Get a DatabaseReference to the given path.
    """
    if _firebase_app is None:
        raise RuntimeError("Firebase has not been initialized. Call init_firebase first.")
    return db.reference(path)


# CRUD operations for user data

def get_user_data(user_id: str) -> Dict[str, Any]:
    """
    Retrieve all data for a given user.
    """
    ref = get_reference(f"users/{user_id}")
    return ref.get() or {}


def update_heart_rate(user_id: str, timestamp: str, bpm: float) -> None:
    """
    Write a heart rate record.
    """
    path = f"users/{user_id}/heart_rate/{timestamp}"
    get_reference(path).set({'bpm': bpm})


def update_spo2(user_id: str, timestamp: str, percentage: float) -> None:
    """
    Write an SpO2 record.
    """
    path = f"users/{user_id}/spo2/{timestamp}"
    get_reference(path).set({'percentage': percentage})


def update_latest_spo2(user_id: str, percentage: float) -> None:
    """
    Update the latest SpO2 value.
    """
    path = f"users/{user_id}/latest_spo2"
    get_reference(path).set({'percentage': percentage})


def update_gps(user_id: str, timestamp: str, latitude: float, longitude: float, altitude: float) -> None:
    """
    Write a GPS location record.
    """
    path = f"users/{user_id}/gps/{timestamp}"
    get_reference(path).set({
        'latitude': latitude,
        'longitude': longitude,
        'altitude': altitude
    })


def update_latest_location(user_id: str, latitude: float, longitude: float, altitude: float, timestamp: str) -> None:
    """
    Update the latest GPS location.
    """
    path = f"users/{user_id}/latest_location"
    get_reference(path).set({
        'latitude': latitude,
        'longitude': longitude,
        'altitude': altitude,
        'timestamp': timestamp
    })


def update_calories_by_heart_rate(user_id: str, date: str, bpm: float, calories_burned_hr: float) -> None:
    """
    Write daily calories by heart rate.
    """
    path = f"users/{user_id}/calories_by_heart_rate/{date}"
    get_reference(path).set({
        'bpm': bpm,
        'calories_burned_hr': calories_burned_hr
    })


def update_calories_by_accelerometer(user_id: str, date: str, total_calories: float) -> None:
    """
    Write daily calories by accelerometer.
    """
    path = f"users/{user_id}/calorise_by_accelerometer/{date}"
    get_reference(path).set({'total_calories': total_calories})


def update_calories_daily(user_id: str, date: str, combined_daily_calories: float,
                          heart_rate_calories: float, accelerometer_calories: float) -> None:
    """
    Write combined daily calories summary.
    """
    path = f"users/{user_id}/calories_daily/{date}"
    get_reference(path).set({
        'combined_daily_calories': combined_daily_calories,
        'heart_rate_calories': heart_rate_calories,
        'accelerometer_calories': accelerometer_calories
    })


def update_sleep_record(user_id: str, start_time: str, end_time: str, duration_hours: float) -> None:
    """
    Write a sleep session record.

    The key format is "{start_time}_to_{end_time}".
    """
    key = f"{start_time}_to_{end_time}"
    path = f"users/{user_id}/sleep/{key}"
    get_reference(path).set({
        'start_time': start_time,
        'end_time': end_time,
        'duration_hours': duration_hours
    })


def update_daily_sleep(user_id: str, date: str, sleep_duration: float) -> None:
    """
    Write total sleep duration for a specific date.
    """
    path = f"users/{user_id}/daily_sleep/{date}"
    get_reference(path).set({'sleep_duration': sleep_duration})


# Example usage
if __name__ == '__main__':
    # Load config from environment or file
    cred_path = os.getenv('FIREBASE_CREDENTIAL_PATH', 'path/to/serviceAccount.json')
    db_url = os.getenv('FIREBASE_DATABASE_URL', 'https://<project>.firebaseio.com')
    init_firebase(cred_path, db_url)

    # Example writes
    update_heart_rate('user123', '2025-04-22T18:00:00', 72.0)
    update_latest_spo2('user123', 98.2)
    update_gps('user123', '2025-04-22T18:00:00', 21.0278, 105.8342, 12.0)
    update_latest_location('user123', 21.0278, 105.8342, 12.0, '2025-04-22T18:00:00')
    update_sleep_record('user123', '2025-04-21T22:00:00', '2025-04-22T06:00:00', 8.0)
    update_daily_sleep('user123', '2025-04-22', 8.0)
    update_calories_by_heart_rate('user123', '2025-04-22', 72.0, 300.5)
    update_calories_by_accelerometer('user123', '2025-04-22', 450.2)
    update_calories_daily('user123', '2025-04-22', 750.7, 300.5, 450.2)
