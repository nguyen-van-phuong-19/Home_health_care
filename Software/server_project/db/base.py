from typing import Dict, Optional

from db.firebase_manager import (
    init_firebase,
    get_reference,
    get_user_data,
    update_heart_rate,
    update_spo2,
    update_latest_in4_spo2,
    update_latest_in4_hr,
    update_gps,
    update_latest_location,
    update_calories_daily,
    update_sleep_record,
    update_daily_sleep,
)
from db.models import (
    UserData,
    HeartRateRecord,
    Spo2Record,
    LocationRecord,
    LatestLocation,
    LatestHeartRate,
    CaloriesByHeartRate,
    CaloriesByAccelerometer,
    CaloriesDaily,
    SleepRecord,
    DailySleep,
)


class BaseService:
    def __init__(self, cred_path: str, db_url: str):
        """
        Initialize Firebase and set up BaseService.
        """
        init_firebase(cred_path, db_url)

    def get_user(self, user_id: str) -> UserData:
        """
        Fetch raw user data and convert to UserData model.
        """
        raw = get_user_data(user_id)
        user = UserData()

        # Heart rate history
        for ts, rec in raw.get("heart_rate", {}).items():
            user.heart_rate[ts] = HeartRateRecord(**rec)

        # SpO2 history
        for ts, rec in raw.get("spo2", {}).items():
            user.spo2[ts] = Spo2Record(**rec)
        # Latest SpO2
        lat_spo2 = raw.get("latest_spo2")
        if lat_spo2:
            user.latest_spo2 = Spo2Record(**lat_spo2)
        # Latest heart rate
        lat_hr = raw.get("latest_heart_rate")
        if lat_hr:
            user.latest_heart_rate = LatestHeartRate(**lat_hr)

        # GPS history
        for ts, rec in raw.get("gps", {}).items():
            user.gps[ts] = LocationRecord(**rec)
        # Latest location
        lat_loc = raw.get("latest_location")
        if lat_loc:
            user.latest_location = LatestLocation(**lat_loc)

        # Calories by heart rate
        for date, rec in raw.get("calories_by_heart_rate", {}).items():
            user.calories_by_heart_rate[date] = CaloriesByHeartRate(**rec)
        # Calories by accelerometer
        for date, rec in raw.get("calorise_by_accelerometer", {}).items():
            user.calorise_by_accelerometer[date] = CaloriesByAccelerometer(**rec)
        # Combined daily calories
        for date, rec in raw.get("calories_daily", {}).items():
            user.calories_daily[date] = CaloriesDaily(**rec)

        # Sleep records
        for key, rec in raw.get("sleep", {}).items():
            user.sleep[key] = SleepRecord(**rec)
        # Daily sleep
        for date, rec in raw.get("daily_sleep", {}).items():
            user.daily_sleep[date] = DailySleep(**rec)

        return user

    def save_user(self, user_id: str, user: UserData) -> None:
        """
        Overwrite entire user node with the provided UserData.
        """
        ref = get_reference(f"users/{user_id}")
        ref.set(user.to_dict())

    # Wrapper methods for individual fields
    def add_heart_rate(self, user_id: str, timestamp: str, bpm: float) -> None:
        update_heart_rate(user_id, timestamp, bpm)

    def add_spo2(self, user_id: str, timestamp: str, percentage: float) -> None:
        update_spo2(user_id, timestamp, percentage)

    def set_latest_in4_spo2(self, user_id: str, percentage: float) -> None:
        update_latest_in4_spo2(user_id, percentage)

    def set_latest_in4_hr(self, user_id: str, bpm: float) -> None:
        update_latest_in4_hr(user_id, bpm)

    def add_gps(self, user_id: str, timestamp: str, latitude: float, longitude: float, altitude: float) -> None:
        update_gps(user_id, timestamp, latitude, longitude, altitude)

    def set_latest_location(self, user_id: str, latitude: float, longitude: float, altitude: float, timestamp: str) -> None:
        update_latest_location(user_id, latitude, longitude, altitude, timestamp)

    def add_calories_daily(self, user_id: str, date: str, combined: float, hr_cal: float, accel_cal: float) -> None:
        update_calories_daily(user_id, date, combined, hr_cal, accel_cal)

    def add_sleep_record(self, user_id: str, start_time: str, end_time: str, duration_hours: float) -> None:
        update_sleep_record(user_id, start_time, end_time, duration_hours)

    def add_daily_sleep(
        self,
        user_id: str,
        date: str,
        sleep_duration: float,
        is_sleeping: bool,
        sleep_start_time: Optional[str] | None = None,
    ) -> None:
        update_daily_sleep(
            user_id,
            date,
            sleep_duration,
            is_sleeping,
            sleep_start_time,
        )

# Example Usage
if __name__ == '__main__':
    service = BaseService('path/to/creds.json', 'https://<project>.firebaseio.com')
    uid = 'user123'
    # Add a heart rate entry
    service.add_heart_rate(uid, '2025-04-22T18:00:00', 75.0)
    # Fetch and print user data
    user_data = service.get_user(uid)
    print(user_data.to_dict())
