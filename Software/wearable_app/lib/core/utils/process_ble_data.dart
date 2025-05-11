import 'dart:async';
import 'dart:convert';

import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:wearable_app/core/utils/process_mqtt.dart';
import 'package:wearable_app/services/ble_service.dart';
import 'package:wearable_app/models/topic_model.dart';

class ProcessBleDataService {
  final Guid serviceUuid = Guid('9ABCDEF0-5678-1234-3412-785634125678');
  final Guid hrCharUuid = Guid('9ABCDEF1-5678-1234-3412-785634125678');
  final Guid spo2CharUuid = Guid('9ABCDEF2-5678-1234-3412-785634125678');
  final Guid accCharUuid = Guid('9ABCDEF3-5678-1234-3412-785634125678');
  final Guid gpsCharUuid = Guid('9ABCDEF4-5678-1234-3412-785634125678');

  final BleService _ble = BleService();
  final ProcessMqttService _mqtt = ProcessMqttService.instance;

  void startProcessing() {
    print("start listening-----------------------------------");
    _listenHeartRate();
    _listenSpo2();
    _listenAccelerometer();
    _listenGps();
  }

  Future<void> _listenHeartRate() async {
    final Stream<List<int>> stream = await _ble.listenCharacteristic(
      serviceUuid,
      hrCharUuid,
    );
    stream.listen((bytes) {
      try {
        final jsonString = utf8.decode(bytes);
        final Map<String, dynamic> map = jsonDecode(jsonString);
        final hr = HeartRateTopic(
          userId: map['user_id'] as String,
          bpm: map['bpm'] as int,
        );
        _mqtt.sendHeartRate(hr);
      } catch (e) {}
    });
  }

  Future<void> _listenSpo2() async {
    final Stream<List<int>> stream = await _ble.listenCharacteristic(
      serviceUuid,
      spo2CharUuid,
    );
    stream.listen((bytes) {
      try {
        final jsonString = utf8.decode(bytes);
        final Map<String, dynamic> map = jsonDecode(jsonString);
        final spo2 = Spo2Topic(
          userId: map['user_id'] as String,
          percentage: (map['percentage'] as num).toDouble(),
        );
        _mqtt.sendSpO2(spo2);
      } catch (e) {}
    });
  }

  Future<void> _listenAccelerometer() async {
    final Stream<List<int>> stream = await _ble.listenCharacteristic(
      serviceUuid,
      accCharUuid,
    );
    stream.listen((bytes) {
      try {
        final jsonString = utf8.decode(bytes);
        final Map<String, dynamic> map = jsonDecode(jsonString);
        print(map);
        final acc = AccelerometerTopic(
          userId: map['user_id'] as String,
          totalVector: (map['total_vector'] as num).toDouble(),
        );
        _mqtt.sendAccelerometer(acc);
      } catch (e) {
        // TODO: Thay print bằng logging nếu cần
      }
    });
  }

  Future<void> _listenGps() async {
    final Stream<List<int>> stream = await _ble.listenCharacteristic(
      serviceUuid,
      gpsCharUuid,
    );
    stream.listen((bytes) {
      try {
        final jsonString = utf8.decode(bytes);
        final Map<String, dynamic> map = jsonDecode(jsonString);
        final gps = GpsTopic(
          userId: map['user_id'] as String,
          latitude: (map['lat'] as num).toDouble(),
          longitude: (map['lon'] as num).toDouble(),
          altitude: (map['alt'] as num).toDouble(),
        );
        _mqtt.sendGps(gps);
      } catch (e) {
        // TODO: Thay print bằng logging nếu cần
      }
    });
  }
}
