// lib/services/process_mqtt.dart

import 'dart:async';
import 'dart:convert';

import 'package:mqtt_client/mqtt_client.dart';
import 'package:wearable_app/models/topic_model.dart';
import 'package:wearable_app/services/mqtt_service.dart';

/// Callback typedefs for incoming messages
typedef HeartRateListener = void Function(HeartRateTopic data);
typedef Spo2Listener = void Function(Spo2Topic data);
typedef AccelerometerListener = void Function(AccelerometerTopic data);
typedef GpsListener = void Function(GpsTopic data);

class ProcessMqttService {
  ProcessMqttService._internal();
  static final ProcessMqttService instance = ProcessMqttService._internal();

  final MQTTService _mqtt = MQTTService();

  /// Các listener tự gán từ bên ngoài
  HeartRateListener? onHeartRate;
  Spo2Listener? onSpo2;
  AccelerometerListener? onAccelerometer;
  GpsListener? onGps;

  /// Khởi tạo MQTT, subscribe và lắng nghe message
  Future<void> initialize() async {
    // Kết nối
    await _mqtt.connect();

    // Đăng ký topic
    _mqtt.subscribe(MQTTService.TOPIC_HEART_RATE, MqttQos.atLeastOnce);
    _mqtt.subscribe(MQTTService.TOPIC_SPO2, MqttQos.atLeastOnce);
    _mqtt.subscribe(MQTTService.TOPIC_ACCELEROMETER, MqttQos.atLeastOnce);
    _mqtt.subscribe(MQTTService.TOPIC_GPS, MqttQos.atLeastOnce);

    // Lắng nghe tất cả message
    _mqtt.client?.updates?.listen(_onMessage);
  }

  /// Xử lý message đến
  void _onMessage(List<MqttReceivedMessage<MqttMessage>> events) {
    for (final event in events) {
      final recMess = event.payload as MqttPublishMessage;
      final payload = MqttPublishPayload.bytesToStringAsString(
        recMess.payload.message,
      );
      final topic = event.topic;

      switch (topic) {
        case MQTTService.TOPIC_HEART_RATE:
          final json = jsonDecode(payload) as Map<String, dynamic>;
          final data = HeartRateTopic.fromJson(json);
          onHeartRate?.call(data);
          break;

        case MQTTService.TOPIC_SPO2:
          final json = jsonDecode(payload) as Map<String, dynamic>;
          final data = Spo2Topic.fromJson(json);
          onSpo2?.call(data);
          break;

        case MQTTService.TOPIC_ACCELEROMETER:
          final json = jsonDecode(payload) as Map<String, dynamic>;
          final data = AccelerometerTopic.fromJson(json);
          onAccelerometer?.call(data);
          break;

        case MQTTService.TOPIC_GPS:
          final json = jsonDecode(payload) as Map<String, dynamic>;
          final data = GpsTopic.fromJson(json);
          onGps?.call(data);
          break;
      }
    }
  }

  /// Publish dữ liệu HeartRateTopic
  Future<void> sendHeartRate(HeartRateTopic data) async {
    _mqtt.publishJson(
      MQTTService.TOPIC_HEART_RATE,
      data.toJson(),
      qos: MqttQos.atLeastOnce,
    );
  }

  /// Publish dữ liệu Spo2Topic
  Future<void> sendSpO2(Spo2Topic data) async {
    _mqtt.publishJson(
      MQTTService.TOPIC_SPO2,
      data.toJson(),
      qos: MqttQos.atLeastOnce,
    );
  }

  /// Publish dữ liệu AccelerometerTopic
  Future<void> sendAccelerometer(AccelerometerTopic data) async {
    _mqtt.publishJson(
      MQTTService.TOPIC_ACCELEROMETER,
      data.toJson(),
      qos: MqttQos.atLeastOnce,
    );
  }

  /// Publish dữ liệu GpsTopic
  Future<void> sendGps(GpsTopic data) async {
    _mqtt.publishJson(
      MQTTService.TOPIC_GPS,
      data.toJson(),
      qos: MqttQos.atLeastOnce,
    );
  }
}
