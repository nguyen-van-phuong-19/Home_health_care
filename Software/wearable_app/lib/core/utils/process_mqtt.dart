// lib/services/process_mqtt.dart

import 'dart:async';
import 'dart:convert';

import 'package:mqtt_client/mqtt_client.dart';
import 'package:wearable_app/models/topic_model.dart';
import 'package:wearable_app/services/mqtt_service.dart';

typedef HeartRateListener = void Function(HeartRateTopic data);
typedef Spo2Listener = void Function(Spo2Topic data);
typedef AccelerometerListener = void Function(AccelerometerTopic data);
typedef GpsListener = void Function(GpsTopic data);

class ProcessMqttService {
  ProcessMqttService._internal();
  static final ProcessMqttService instance = ProcessMqttService._internal();

  final MQTTService _mqtt = MQTTService();

  HeartRateListener? onHeartRate;
  Spo2Listener? onSpo2;
  AccelerometerListener? onAccelerometer;
  GpsListener? onGps;

  Future<void> initialize() async {
    // Kết nối
    await _mqtt.connect();

    _mqtt.subscribe(MQTTService.TOPIC_HEART_RATE, MqttQos.atLeastOnce);
    _mqtt.subscribe(MQTTService.TOPIC_SPO2, MqttQos.atLeastOnce);
    _mqtt.subscribe(MQTTService.TOPIC_ACCELEROMETER, MqttQos.atLeastOnce);
    _mqtt.subscribe(MQTTService.TOPIC_GPS, MqttQos.atLeastOnce);
    _mqtt.subscribe(MQTTService.TOPIC_CALORIES, MqttQos.atLeastOnce);

    _mqtt.client?.updates?.listen(_onMessage);
  }

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

  Future<void> sendHeartRate(HeartRateTopic data) async {
    _mqtt.publishJson(
      MQTTService.TOPIC_HEART_RATE,
      data.toJson(),
      qos: MqttQos.atLeastOnce,
    );
  }

  Future<void> sendSpO2(Spo2Topic data) async {
    _mqtt.publishJson(
      MQTTService.TOPIC_SPO2,
      data.toJson(),
      qos: MqttQos.atLeastOnce,
    );
  }

  Future<void> sendAccelerometer(AccelerometerTopic data) async {
    _mqtt.publishJson(
      MQTTService.TOPIC_ACCELEROMETER,
      data.toJson(),
      qos: MqttQos.atLeastOnce,
    );
  }

  Future<void> sendGps(GpsTopic data) async {
    _mqtt.publishJson(
      MQTTService.TOPIC_GPS,
      data.toJson(),
      qos: MqttQos.atLeastOnce,
    );
  }

  Future<void> sendCalories(CaloriesTopic data) async {
    _mqtt.publishJson(
      MQTTService.TOPIC_CALORIES,
      data.toJson(),
      qos: MqttQos.atLeastOnce,
    );
  }
}
