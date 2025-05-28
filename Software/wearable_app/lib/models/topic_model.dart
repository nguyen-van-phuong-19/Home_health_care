// lib/models/topic_model.dart

class HeartRateTopic {
  final String userId;
  final int bpm;
  final double? weightKg;
  final int? age;
  final int? epochMinutes;

  HeartRateTopic({
    required this.userId,
    required this.bpm,
    this.weightKg,
    this.age,
    this.epochMinutes,
  });

  factory HeartRateTopic.fromJson(Map<String, dynamic> json) => HeartRateTopic(
    userId: json['user_id'] as String,
    bpm: json['bpm'] as int,
    weightKg: (json['weight_kg'] as num?)?.toDouble(), // null nếu không có
    age: json['age'] as int?,
    epochMinutes: json['epoch_minutes'] as int?,
  );

  Map<String, dynamic> toJson() {
    final map = <String, dynamic>{'user_id': userId, 'bpm': bpm};
    if (weightKg != null) map['weight_kg'] = weightKg;
    if (age != null) map['age'] = age;
    if (epochMinutes != null) map['epoch_minutes'] = epochMinutes;
    return map;
  }
}

class Spo2Topic {
  final String userId;
  final double percentage;

  Spo2Topic({required this.userId, required this.percentage});

  factory Spo2Topic.fromJson(Map<String, dynamic> json) => Spo2Topic(
    userId: json['user_id'] as String,
    percentage: (json['percentage'] as num).toDouble(),
  );

  Map<String, dynamic> toJson() => {
    'user_id': userId,
    'percentage': percentage,
  };
}

class AccelerometerTopic {
  final String userId;
  final double totalVector;
  final double? weightKg;
  final int? epochMinutes;

  AccelerometerTopic({
    required this.userId,
    required this.totalVector,
    this.weightKg,
    this.epochMinutes,
  });

  factory AccelerometerTopic.fromJson(Map<String, dynamic> json) =>
      AccelerometerTopic(
        userId: json['user_id'] as String,
        totalVector: (json['total_vector'] as num).toDouble(),
        weightKg: (json['weight_kg'] as num?)?.toDouble(),
        epochMinutes: json['epoch_minutes'] as int?,
      );

  Map<String, dynamic> toJson() {
    final map = <String, dynamic>{
      'user_id': userId,
      'total_vector': totalVector,
    };
    if (weightKg != null) map['weight_kg'] = weightKg;
    if (epochMinutes != null) map['epoch_minutes'] = epochMinutes;
    return map;
  }
}

class GpsTopic {
  final String userId;
  final double latitude;
  final double longitude;
  final double altitude;

  GpsTopic({
    required this.userId,
    required this.latitude,
    required this.longitude,
    required this.altitude,
  });

  factory GpsTopic.fromJson(Map<String, dynamic> json) => GpsTopic(
    userId: json['user_id'] as String,
    latitude: (json['latitude'] as num).toDouble(),
    longitude: (json['longitude'] as num).toDouble(),
    altitude: (json['altitude'] as num).toDouble(),
  );

  Map<String, dynamic> toJson() => {
    'user_id': userId,
    'latitude': latitude,
    'longitude': longitude,
    'altitude': altitude,
  };
}

class CaloriesTopic {
  final String userId;
  final double caloTotal;
  final double caloHr;
  final double caloMotion;

  CaloriesTopic({
    required this.userId,
    required this.caloTotal,
    required this.caloHr,
    required this.caloMotion,
  });

  factory CaloriesTopic.fromJson(Map<String, dynamic> json) => CaloriesTopic(
    userId: json['user_id'] as String,
    caloTotal: (json['calo_total'] as num).toDouble(),
    caloHr: (json['calo_hr'] as num).toDouble(),
    caloMotion: (json['calo_motion'] as num).toDouble(),
  );

  Map<String, dynamic> toJson() => {
    'user_id': userId,
    'calo_total': caloTotal,
    'calo_hr': caloHr,
    'calo_motion': caloMotion,
  };
}
