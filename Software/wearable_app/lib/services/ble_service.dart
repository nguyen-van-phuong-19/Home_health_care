// lib/services/ble_service.dart

import 'dart:async';
import 'package:flutter_blue_background/flutter_blue_background.dart';
import 'package:shared_preferences/shared_preferences.dart';

/// Service quản lý kết nối BLE (Bluetooth Low Energy)
/// Sử dụng plugin flutter_blue_background để hỗ trợ hoạt động ngay cả khi ứng dụng chạy ngầm
class BLEService {
  // Singleton
  static final BLEService _instance = BLEService._internal();
  factory BLEService() => _instance;
  BLEService._internal();

  bool _serviceStarted = false;

  /// Bắt đầu service BLE nền
  /// [onBackgroundTask] là callback chạy khi service khởi động (thường dùng để giữ kết nối)
  Future<void> startService({required Function() onBackgroundTask}) async {
    if (!_serviceStarted) {
      await FlutterBlueBackground.startFlutterBackgroundService(
        onBackgroundTask,
      );
      _serviceStarted = true;
    }
  }

  /// Dừng service BLE nền
  Future<void> stopService() async {
    if (_serviceStarted) {
      await FlutterBlueBackground.stopFlutterBackgroundService();
      _serviceStarted = false;
    }
  }

  /// Kết nối đến thiết bị BLE theo tên và UUID
  /// Trả về true nếu kết nối thành công
  Future<bool> connect({
    required String deviceName,
    required String serviceUuid,
    required String characteristicUuid,
  }) async {
    await FlutterBlueBackground.connectToDevice(
      deviceName: deviceName,
      serviceUuid: serviceUuid,
      characteristicUuid: characteristicUuid,
    );
    // Assuming the connection is successful if no exception is thrown
    return true;
  }

  /// Đọc dữ liệu từ characteristic đã kết nối
  /// Trả về chuỗi dữ liệu (thường là string encoded)
  Future<String?> readData({
    required String serviceUuid,
    required String characteristicUuid,
  }) async {
    return await FlutterBlueBackground.readData(
      serviceUuid: serviceUuid,
      characteristicUuid: characteristicUuid,
    );
  }

  /// Ghi dữ liệu vào characteristic đã kết nối
  Future<void> writeData({
    required String serviceUuid,
    required String characteristicUuid,
    required String data,
  }) async {
    await FlutterBlueBackground.writeData(
      serviceUuid: serviceUuid,
      characteristicUuid: characteristicUuid,
      data: data,
    );
  }

  /// Lấy danh sách giá trị đã đọc (Android)
  /// Dữ liệu được lưu trong SharedPreferences dưới key 'getReadData'
  Future<List<String>> getReadDataAndroid() async {
    // Đảm bảo đã chạy service trước đó
    await FlutterBlueBackground.getReadDataAndroid();
    final prefs = await SharedPreferences.getInstance();
    await prefs.reload();
    return prefs.getStringList('getReadData') ?? <String>[];
  }

  /// Xóa toàn bộ dữ liệu đã đọc lưu trữ
  Future<void> clearReadStorage() async {
    await FlutterBlueBackground.clearReadStorage();
  }
}
