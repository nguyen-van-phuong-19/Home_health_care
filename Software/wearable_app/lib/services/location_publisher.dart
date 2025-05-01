// lib/services/location_publisher.dart

import 'dart:async';
import 'package:geolocator/geolocator.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:wearable_app/services/mqtt_service.dart';

class LocationPublisher {
  LocationPublisher._();
  static final instance = LocationPublisher._();

  final _mqtt = MQTTService();
  Timer? _timer;

  /// Bắt đầu lấy vị trí mỗi [interval] phút và publish lên MQTT
  Future<void> startPeriodic(String userId, {int intervalMinutes = 1}) async {
    // 1) Kiểm tra & yêu cầu quyền vị trí
    LocationPermission perm = await Geolocator.checkPermission();
    if (perm == LocationPermission.denied) {
      perm = await Geolocator.requestPermission();
      if (perm == LocationPermission.denied) {
        throw Exception('Location permissions are denied');
      }
    }
    if (perm == LocationPermission.deniedForever) {
      throw Exception('Location permissions are permanently denied');
    }

    // 2) Hủy timer cũ nếu có
    _timer?.cancel();

    // 3) Tạo timer để chạy định kỳ
    _timer = Timer.periodic(Duration(minutes: intervalMinutes), (_) async {
      try {
        // Lấy vị trí hiện tại
        final pos = await Geolocator.getCurrentPosition(
          desiredAccuracy: LocationAccuracy.high,
        );

        // Chuẩn bị payload
        final payload = <String, dynamic>{
          'user_id': userId,
          'latitude': pos.latitude,
          'longitude': pos.longitude,
          'altitude': pos.altitude,
        };

        // Publish lên topic GPS
        _mqtt.publishJson(
          MQTTService.TOPIC_GPS,
          payload,
          qos: MqttQos.atLeastOnce,
        );
      } catch (e) {
        print('LocationPublisher error: $e');
      }
    });

    // 4) (Tùy chọn) publish ngay lập tức lần đầu, không phải chờ interval
    _timer?.tick == 0 ? await _publishOnce(userId) : null;
  }

  /// Gửi một lần ngay lập tức
  Future<void> _publishOnce(String userId) async {
    final pos = await Geolocator.getCurrentPosition(
      desiredAccuracy: LocationAccuracy.high,
    );
    final payload = <String, dynamic>{
      'user_id': userId,
      'latitude': pos.latitude,
      'longitude': pos.longitude,
      'altitude': pos.altitude,
    };
    _mqtt.publishJson(MQTTService.TOPIC_GPS, payload, qos: MqttQos.atLeastOnce);
  }

  /// Dừng gửi vị trí
  Future<void> stop() async {
    _timer?.cancel();
    _timer = null;
  }
}
