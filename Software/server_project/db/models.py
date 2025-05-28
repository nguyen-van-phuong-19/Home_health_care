from dataclasses import dataclass, field, asdict
from typing import Dict, Optional

@dataclass
class HeartRateRecord:
    bpm: float

    def to_dict(self) -> dict:
        return asdict(self)

@dataclass
class Spo2Record:
    percentage: float

    def to_dict(self) -> dict:
        return asdict(self)

@dataclass
class LocationRecord:
    latitude: float
    longitude: float
    altitude: float

    def to_dict(self) -> dict:
        return asdict(self)

@dataclass
class LatestLocation:
    latitude: float
    longitude: float
    altitude: float
    timestamp: str

    def to_dict(self) -> dict:
        return asdict(self)

@dataclass
class SleepRecord:
    start_time: str
    end_time: str
    duration_hours: float

    def to_dict(self) -> dict:
        return asdict(self)

@dataclass
class DailySleep:
    sleep_duration: float

    def to_dict(self) -> dict:
        return asdict(self)

@dataclass
class CaloriesByHeartRate:
    bpm: float
    calories_burned_hr: float

    def to_dict(self) -> dict:
        return asdict(self)

@dataclass
class CaloriesByAccelerometer:
    total_calories: float

    def to_dict(self) -> dict:
        return asdict(self)

@dataclass
class CaloriesDaily:
    combined_daily_calories: float
    heart_rate_calories: float
    accelerometer_calories: float

    def to_dict(self) -> dict:
        return asdict(self)

@dataclass
class UserData:
    heart_rate: Dict[str, HeartRateRecord] = field(default_factory=dict)
    calories_by_heart_rate: Dict[str, CaloriesByHeartRate] = field(default_factory=dict)
    spo2: Dict[str, Spo2Record] = field(default_factory=dict)
    latest_spo2: Optional[Spo2Record] = None
    calorise_by_accelerometer: Dict[str, CaloriesByAccelerometer] = field(default_factory=dict)
    gps: Dict[str, LocationRecord] = field(default_factory=dict)
    latest_location: Optional[LatestLocation] = None
    sleep: Dict[str, SleepRecord] = field(default_factory=dict)
    daily_sleep: Dict[str, DailySleep] = field(default_factory=dict)
    calories_daily: Dict[str, CaloriesDaily] = field(default_factory=dict)

    def to_dict(self) -> dict:
        """
        Convert the entire user data structure into nested dicts ready for Firebase.
        """
        data = {
            "heart_rate": {k: v.to_dict() for k, v in self.heart_rate.items()},
            "calories_by_heart_rate": {k: v.to_dict() for k, v in self.calories_by_heart_rate.items()},
            "spo2": {k: v.to_dict() for k, v in self.spo2.items()},
            "latest_spo2": self.latest_spo2.to_dict() if self.latest_spo2 else {},
            "calorise_by_accelerometer": {k: v.to_dict() for k, v in self.calorise_by_accelerometer.items()},
            "gps": {k: v.to_dict() for k, v in self.gps.items()},
            "latest_location": self.latest_location.to_dict() if self.latest_location else {},
            "sleep": {k: v.to_dict() for k, v in self.sleep.items()},
            "daily_sleep": {k: v.to_dict() for k, v in self.daily_sleep.items()},
            "calories_daily": {k: v.to_dict() for k, v in self.calories_daily.items()},
        }
        return data
