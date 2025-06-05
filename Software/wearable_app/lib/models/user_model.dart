import 'package:firebase_auth/firebase_auth.dart';

class UserModel {
  final String userId = FirebaseAuth.instance.currentUser?.uid ?? '';
  final Map<String, HeartRateEntry> heartRate;
  final Map<String, CaloriesByHeartRateEntry> caloriesByHeartRate;
  final Map<String, Spo2Entry> spo2;
  LatestSpO2 latestSpO2;
  LatestHeartRate latestHeartRate;
  final Map<String, AccelerometerCaloriesEntry> caloriesByAccelerometer;
  final Map<String, GpsEntry> gps;
  late final LatestLocation latestLocation;
  final Map<String, SleepSession> sleep;
  final Map<String, DailySleep> dailySleep;
  final Map<String, DailyCalories> caloriesDaily;

  UserModel({
    required this.heartRate,
    required this.caloriesByHeartRate,
    required this.spo2,
    required this.latestSpO2,
    required this.latestHeartRate,
    required this.caloriesByAccelerometer,
    required this.gps,
    required this.latestLocation,
    required this.sleep,
    required this.dailySleep,
    required this.caloriesDaily,
  });

  factory UserModel.fromJson(Map<String, dynamic> json, String userId) {
    return UserModel(
      heartRate:
          (json['heart_rate'] as Map<String, dynamic>?)?.map(
            (k, v) => MapEntry(k, HeartRateEntry.fromJson(v)),
          ) ??
          {},
      caloriesByHeartRate:
          (json['calories_by_heart_rate'] as Map<String, dynamic>?)?.map(
            (k, v) => MapEntry(k, CaloriesByHeartRateEntry.fromJson(v)),
          ) ??
          {},
      spo2:
          (json['spo2'] as Map<String, dynamic>?)?.map(
            (k, v) => MapEntry(k, Spo2Entry.fromJson(v)),
          ) ??
          {},
      latestSpO2: LatestSpO2.fromJson(
        json['latest_spo2'] as Map<String, dynamic>? ?? {},
      ),
      latestHeartRate: LatestHeartRate.fromJson(
        json['latest_heart_rate'] as Map<String, dynamic>? ?? {},
      ),
      caloriesByAccelerometer:
          (json['calorise_by_accelerometer'] as Map<String, dynamic>?)?.map(
            (k, v) => MapEntry(k, AccelerometerCaloriesEntry.fromJson(v)),
          ) ??
          {},
      gps:
          (json['gps'] as Map<String, dynamic>?)?.map(
            (k, v) => MapEntry(k, GpsEntry.fromJson(v)),
          ) ??
          {},
      latestLocation: LatestLocation.fromJson(
        json['latest_location'] as Map<String, dynamic>? ?? {},
      ),
      sleep:
          (json['sleep'] as Map<String, dynamic>?)?.map(
            (k, v) => MapEntry(k, SleepSession.fromJson(v)),
          ) ??
          {},
      dailySleep:
          (json['daily_sleep'] as Map<String, dynamic>?)?.map(
            (k, v) => MapEntry(k, DailySleep.fromJson(v)),
          ) ??
          {},
      caloriesDaily:
          (json['calories_daily'] as Map<String, dynamic>?)?.map(
            (k, v) => MapEntry(k, DailyCalories.fromJson(v)),
          ) ??
          {},
    );
  }

  Map<String, dynamic> toJson() => {
    'heart_rate': heartRate.map((k, v) => MapEntry(k, v.toJson())),
    'calories_by_heart_rate': caloriesByHeartRate.map(
      (k, v) => MapEntry(k, v.toJson()),
    ),
    'spo2': spo2.map((k, v) => MapEntry(k, v.toJson())),
    'latest_spo2': latestSpO2.toJson(),
    'latest_heart_rate': latestHeartRate.toJson(),
    'calorise_by_accelerometer': caloriesByAccelerometer.map(
      (k, v) => MapEntry(k, v.toJson()),
    ),
    'gps': gps.map((k, v) => MapEntry(k, v.toJson())),
    'latest_location': latestLocation.toJson(),
    'sleep': sleep.map((k, v) => MapEntry(k, v.toJson())),
    'daily_sleep': dailySleep.map((k, v) => MapEntry(k, v.toJson())),
    'calories_daily': caloriesDaily.map((k, v) => MapEntry(k, v.toJson())),
  };
}

// Nested model classes

class HeartRateEntry {
  final int bpm;

  HeartRateEntry({required this.bpm});

  factory HeartRateEntry.fromJson(Map<String, dynamic> json) {
    return HeartRateEntry(bpm: json['bpm'] as int);
  }

  Map<String, dynamic> toJson() => {'bpm': bpm};
}

class CaloriesByHeartRateEntry {
  final int bpm;
  final double caloriesBurnedHr;

  CaloriesByHeartRateEntry({required this.bpm, required this.caloriesBurnedHr});

  factory CaloriesByHeartRateEntry.fromJson(Map<String, dynamic> json) {
    return CaloriesByHeartRateEntry(
      bpm: json['bpm'] as int,
      caloriesBurnedHr: (json['calories_burned_hr'] as num).toDouble(),
    );
  }

  Map<String, dynamic> toJson() => {
    'bpm': bpm,
    'calories_burned_hr': caloriesBurnedHr,
  };
}

class Spo2Entry {
  final num percentage;

  Spo2Entry({required this.percentage});

  factory Spo2Entry.fromJson(Map<String, dynamic> json) {
    return Spo2Entry(percentage: json['percentage'] as num);
  }

  Map<String, dynamic> toJson() => {'percentage': percentage};
}

class LatestSpO2 {
  final num percentage;

  LatestSpO2({required this.percentage});

  factory LatestSpO2.fromJson(Map<String, dynamic> json) {
    return LatestSpO2(percentage: json['percentage'] as num);
  }

  Map<String, dynamic> toJson() => {'percentage': percentage};
}

class LatestHeartRate {
  final num bpm;
  final String timestamp;

  LatestHeartRate({required this.bpm, this.timestamp = ''});

  factory LatestHeartRate.fromJson(Map<String, dynamic> json) {
    return LatestHeartRate(
      bpm: json['bpm'] as num? ?? 0,
      timestamp: json['timestamp'] as String? ?? '',
    );
  }

  Map<String, dynamic> toJson() => {
        'bpm': bpm,
        'timestamp': timestamp,
      };
}

class AccelerometerCaloriesEntry {
  final double totalCalories;

  AccelerometerCaloriesEntry({required this.totalCalories});

  factory AccelerometerCaloriesEntry.fromJson(Map<String, dynamic> json) {
    return AccelerometerCaloriesEntry(
      totalCalories: (json['total_calories'] as num).toDouble(),
    );
  }

  Map<String, dynamic> toJson() => {'total_calories': totalCalories};
}

class GpsEntry {
  final double latitude;
  final double longitude;
  final double altitude;

  GpsEntry({
    required this.latitude,
    required this.longitude,
    required this.altitude,
  });

  factory GpsEntry.fromJson(Map<String, dynamic> json) {
    return GpsEntry(
      latitude: (json['latitude'] as num).toDouble(),
      longitude: (json['longitude'] as num).toDouble(),
      altitude: (json['altitude'] as num).toDouble(),
    );
  }

  Map<String, dynamic> toJson() => {
    'latitude': latitude,
    'longitude': longitude,
    'altitude': altitude,
  };
}

class LatestLocation {
  final double latitude;
  final double longitude;
  final double altitude;
  final String timestamp;

  LatestLocation({
    required this.latitude,
    required this.longitude,
    required this.altitude,
    required this.timestamp,
  });

  factory LatestLocation.fromJson(Map<String, dynamic> json) {
    return LatestLocation(
      latitude: (json['latitude'] as num).toDouble(),
      longitude: (json['longitude'] as num).toDouble(),
      altitude: (json['altitude'] as num).toDouble(),
      timestamp: json['timestamp'] as String,
    );
  }

  Map<String, dynamic> toJson() => {
    'latitude': latitude,
    'longitude': longitude,
    'altitude': altitude,
    'timestamp': timestamp,
  };
}

class SleepSession {
  final String startTime;
  final String endTime;
  final double durationHours;

  SleepSession({
    required this.startTime,
    required this.endTime,
    required this.durationHours,
  });

  factory SleepSession.fromJson(Map<String, dynamic> json) {
    return SleepSession(
      startTime: json['start_time'] as String,
      endTime: json['end_time'] as String,
      durationHours: (json['duration_hours'] as num).toDouble(),
    );
  }

  Map<String, dynamic> toJson() => {
    'start_time': startTime,
    'end_time': endTime,
    'duration_hours': durationHours,
  };
}

class DailySleep {
  final double sleepDuration;
  final bool isSleeping;

  DailySleep({required this.sleepDuration, required this.isSleeping});

  factory DailySleep.fromJson(Map<String, dynamic> json) {
    return DailySleep(
      sleepDuration: (json['sleep_duration'] as num).toDouble(),
      isSleeping: (json['is_sleeping'] as bool? ?? false),
    );
  }

  Map<String, dynamic> toJson() => {
        'sleep_duration': sleepDuration,
        'is_sleeping': isSleeping,
      };
}

class DailyCalories {
  final double combinedDailyCalories;
  final double heartRateCalories;
  final double accelerometerCalories;

  DailyCalories({
    required this.combinedDailyCalories,
    required this.heartRateCalories,
    required this.accelerometerCalories,
  });

  factory DailyCalories.fromJson(Map<String, dynamic> json) {
    return DailyCalories(
      combinedDailyCalories:
          (json['combined_daily_calories'] as num).toDouble(),
      heartRateCalories: (json['heart_rate_calories'] as num).toDouble(),
      accelerometerCalories: (json['accelerometer_calories'] as num).toDouble(),
    );
  }

  Map<String, dynamic> toJson() => {
    'combined_daily_calories': combinedDailyCalories,
    'heart_rate_calories': heartRateCalories,
    'accelerometer_calories': accelerometerCalories,
  };
}
