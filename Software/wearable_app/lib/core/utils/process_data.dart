import 'package:wearable_app/models/user_model.dart';
import 'package:wearable_app/services/firebase_service.dart';

class ProcessDataService {
  ProcessDataService._();
  static final instance = ProcessDataService._();

  final _firebase = FirebaseService();

  // ---------------------- WRITE METHODS ----------------------
  /// Xử lý và lưu nhịp tim
  Future<void> processHeartRate(Map<String, dynamic> data) async {
    final userId = data['user_id'] as String? ?? 'default_user';
    final bpm = data['bpm'] as int? ?? 0;
    final timestamp =
        data['timestamp'] as String? ?? DateTime.now().toIso8601String();
    final date = timestamp.substring(0, 10);

    final user = await _loadUser(userId);
    user.heartRate[timestamp] = HeartRateEntry(bpm: bpm);
    user.latestHeartRate = LatestHeartRate(bpm: bpm, timestamp: timestamp);

    final weight = (data['weight_kg'] as num?)?.toDouble() ?? 70.0;
    final age = (data['age'] as num?)?.toDouble() ?? 30.0;
    final epochMinutes = (data['epoch_minutes'] as num?)?.toDouble() ?? 1.0;
    final calPerMin =
        (-55.0969 + 0.6309 * bpm + 0.1988 * weight + 0.2017 * age) / 4.184;
    final caloriesBurned = calPerMin < 0 ? 0 : calPerMin * epochMinutes;
    user.caloriesByHeartRate[date] = CaloriesByHeartRateEntry(
      bpm: bpm,
      caloriesBurnedHr:
          (user.caloriesByHeartRate[date]?.caloriesBurnedHr ?? 0) +
          caloriesBurned,
    );
    await _saveUser(user);
  }

  /// Xử lý và lưu SpO2
  Future<void> processSpO2(Map<String, dynamic> data) async {
    final userId = data['user_id'] as String? ?? 'default_user';
    final spo2 = (data['percentage'] as num?)?.toDouble() ?? 0.0;
    final timestamp =
        data['timestamp'] as String? ?? DateTime.now().toIso8601String();

    final user = await _loadUser(userId);
    user.spo2[timestamp] = Spo2Entry(percentage: spo2);
    user.latestSpO2 = LatestSpO2(percentage: spo2);
    await _saveUser(user);
  }

  /// Xử lý và lưu accelerometer
  Future<void> processAccelerometer(Map<String, dynamic> data) async {
    final userId = data['user_id'] as String? ?? 'default_user';
    final totalVector = (data['total_vector'] as num?)?.toDouble();
    if (totalVector == null) return;
    final weight = (data['weight_kg'] as num?)?.toDouble() ?? 70.0;
    final epochMinutes = (data['epoch_minutes'] as num?)?.toDouble() ?? 1.0;
    final timestamp =
        data['timestamp'] as String? ?? DateTime.now().toIso8601String();
    final date = timestamp.substring(0, 10);

    final user = await _loadUser(userId);
    final calPerMin = 0.001064 * totalVector + 0.087512 * weight - 5.500229;
    final calories = calPerMin < 0 ? 0 : calPerMin * epochMinutes;
    user.caloriesByAccelerometer[date] = AccelerometerCaloriesEntry(
      totalCalories:
          (user.caloriesByAccelerometer[date]?.totalCalories ?? 0) + calories,
    );
    await _saveUser(user);
  }

  /// Xử lý và lưu GPS
  Future<void> processGps(Map<String, dynamic> data) async {
    final userId = data['user_id'] as String? ?? 'default_user';
    final latitude = (data['latitude'] as num?)?.toDouble() ?? 0.0;
    final longitude = (data['longitude'] as num?)?.toDouble() ?? 0.0;
    final altitude = (data['altitude'] as num?)?.toDouble() ?? 0.0;
    final timestamp =
        data['timestamp'] as String? ?? DateTime.now().toIso8601String();

    final user = await _loadUser(userId);
    user.gps[timestamp] = GpsEntry(
      latitude: latitude,
      longitude: longitude,
      altitude: altitude,
    );
    user.latestLocation = LatestLocation(
      latitude: latitude,
      longitude: longitude,
      altitude: altitude,
      timestamp: timestamp,
    );
    await _saveUser(user);
  }

  /// Lưu phiên ngủ
  Future<void> saveSleepSession(
    String userId,
    String sessionId,
    String startTime,
    String endTime,
    double durationHours,
  ) async {
    final user = await _loadUser(userId);
    user.sleep[sessionId] = SleepSession(
      startTime: startTime,
      endTime: endTime,
      durationHours: durationHours,
    );
    final date = startTime.substring(0, 10);
    user.dailySleep[date] = DailySleep(sleepDuration: durationHours);
    await _saveUser(user);
  }

  /// Tính và lưu tổng calo hàng ngày
  Future<void> calculateCombinedDailyCalories(String userId) async {
    final date = DateTime.now().toIso8601String().substring(0, 10);
    final user = await _loadUser(userId);
    final hrCal = user.caloriesByHeartRate[date]?.caloriesBurnedHr ?? 0;
    final accCal = user.caloriesByAccelerometer[date]?.totalCalories ?? 0;
    final combined = (hrCal + accCal) / 2;
    user.caloriesDaily[date] = DailyCalories(
      combinedDailyCalories: combined,
      heartRateCalories: hrCal,
      accelerometerCalories: accCal,
    );
    await _saveUser(user);
  }

  // ---------------------- READ METHODS ----------------------
  /// Lấy toàn bộ UserModel
  Future<UserModel> getUserModel(String userId) async {
    final snap = await _firebase.readOnce('users/$userId');
    final json = snap.value as Map<String, dynamic>? ?? {};
    return UserModel.fromJson(json, userId);
  }

  /// Lấy lịch sử HeartRateEntry
  Future<List<HeartRateEntry>> getHeartRateHistory(String userId) async {
    final user = await getUserModel(userId);
    return user.heartRate.entries.map((e) => e.value).toList();
  }

  /// Lấy CaloriesByHeartRateEntry cho ngày
  Future<CaloriesByHeartRateEntry?> getCaloriesByHeartRate(
    String userId,
    String date,
  ) async {
    final user = await getUserModel(userId);
    return user.caloriesByHeartRate[date];
  }

  /// Lấy lịch sử SpO2
  Future<List<Spo2Entry>> getSpO2History(String userId) async {
    final user = await getUserModel(userId);
    return user.spo2.entries.map((e) => e.value).toList();
  }

  /// Lấy SpO2 mới nhất
  Future<LatestSpO2> getLatestSpO2(String userId) async {
    final user = await getUserModel(userId);
    return user.latestSpO2;
  }

  /// Lấy nhịp tim mới nhất
  Future<LatestHeartRate> getLatestHeartRate(String userId) async {
    final user = await getUserModel(userId);
    return user.latestHeartRate;
  }

  /// Lấy calories từ accelerometer cho ngày
  Future<AccelerometerCaloriesEntry?> getAccelerometerCalories(
    String userId,
    String date,
  ) async {
    final user = await getUserModel(userId);
    return user.caloriesByAccelerometer[date];
  }

  /// Lấy vị trí GPS mới nhất
  Future<LatestLocation> getLatestLocation(String userId) async {
    final user = await getUserModel(userId);
    return user.latestLocation;
  }

  /// Lấy lịch sử GPS
  Future<List<GpsEntry>> getGpsHistory(String userId) async {
    final user = await getUserModel(userId);
    return user.gps.entries.map((e) => e.value).toList();
  }

  /// Lấy danh sách phiên ngủ
  Future<List<SleepSession>> getSleepSessions(String userId) async {
    final user = await getUserModel(userId);
    return user.sleep.entries.map((e) => e.value).toList();
  }

  /// Lấy DailySleep cho ngày
  Future<DailySleep?> getDailySleep(String userId, String date) async {
    final user = await getUserModel(userId);
    return user.dailySleep[date];
  }

  /// Lấy DailyCalories cho ngày
  Future<DailyCalories?> getDailyCalories(String userId, String date) async {
    final user = await getUserModel(userId);
    return user.caloriesDaily[date];
  }

  // ---------------------- INTERNAL HELPERS ----------------------
  /// Tải UserModel hiện tại
  Future<UserModel> _loadUser(String userId) async => getUserModel(userId);

  /// Lưu UserModel
  Future<void> _saveUser(UserModel user) async {
    await _firebase.setData('users/${user.userId}', user.toJson());
  }
}
