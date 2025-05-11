import 'dart:async';
import 'dart:typed_data';

import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:wearable_app/core/config/firebase_options.dart';
import 'package:wearable_app/core/utils/process_mqtt.dart';
import 'package:wearable_app/models/topic_model.dart';
import 'package:wearable_app/services/location_publisher.dart';
import 'package:wearable_app/services/mqtt_service.dart';
import 'package:wearable_app/screens/home_screen/home_screen.dart';

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();

  await Firebase.initializeApp(options: DefaultFirebaseOptions.currentPlatform);

  await MQTTService().init();
  await MQTTService().connect();

  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  @override
  Widget build(BuildContext context) {
    return BleInitializer(
      child: MaterialApp(
        title: 'Health Tracker',
        theme: ThemeData(
          primarySwatch: Colors.green,
          scaffoldBackgroundColor: const Color(0xFFF7FFF0),
        ),
        home: HomeScreen(),
        debugShowCheckedModeBanner: false,
      ),
    );
  }
}

/// Widget chỉ khởi tạo BLE và tự động reconnect khi mất kết nối, không chặn UI
class BleInitializer extends StatefulWidget {
  final Widget child;
  const BleInitializer({super.key, required this.child});

  @override
  // ignore: library_private_types_in_public_api
  _BleInitializerState createState() => _BleInitializerState();
}

class _BleInitializerState extends State<BleInitializer> {
  StreamSubscription<BluetoothConnectionState>? _connSub;
  late int hrLast;
  late int spo2Last;
  late int motionLast;

  @override
  void initState() {
    super.initState();
    hrLast = 0;
    spo2Last = 0;
    motionLast = 0;
    _listenScan();
  }

  Future<void> _listenScan() async {
    FlutterBluePlus.scanResults.listen((results) {
      for (final r in results) {
        handleScanResult(r);
      }
    });
    await FlutterBluePlus.startScan(
      androidScanMode: AndroidScanMode.lowLatency,
    );
  }

  Future<void> handleScanResult(ScanResult r) async {
    final m = r.advertisementData.manufacturerData;
    // Key là Company ID (0xABCD)
    final data = m[0xABCD];
    if (data == null || data.length < 6) {
      return;
    }

    final bd = ByteData.sublistView(Uint8List.fromList(data));
    final hr = bd.getUint8(0);
    final spo2 = bd.getUint8(1);
    final motion = bd.getUint8(2);

    if (hr == hrLast && spo2 == spo2Last && motion == motionLast) {
      return;
    }

    setState(() {
      hrLast = hr;
      spo2Last = spo2;
      motionLast = motion;
    });

    print('HR: $hr, SpO2: $spo2, Motion: ${motion.toStringAsFixed(2)}');
    _publishBleData(hr, spo2, motion);
    await LocationPublisher.instance.startPeriodic(
      'user123',
      intervalMinutes: 1,
    );
  }

  /// Hàm tổng hợp & gửi dữ liệu BLE qua MQTT
  ///
  /// Function này được gọi khi có dữ liệu BLE mới, nó sẽ tổng hợp và gửi qua MQTT
  ///
  /// [hr] là số nhịp tim
  /// [spo2] là số SpO2
  /// [motion] là số gia tốc
  ///
  /// Returns [Future<void>] là promise cho việc gửi dữ liệu
  Future<void> _publishBleData(int hr, int spo2, int motion) async {
    // Lấy userId từ chỗ bạn lưu (ví dụ SharedPreferences, Auth service, v.v.)
    final String userId = await _getCurrentUserId();

    // Tính số phút kể từ epoch
    final int epochMinutes = 1;

    // 2.1 Tạo model HeartRateTopic
    final hrTopic = HeartRateTopic(
      userId: userId,
      bpm: hr,
      epochMinutes: epochMinutes,
    );

    // 2.2 Tạo model Spo2Topic
    final spo2Topic = Spo2Topic(userId: userId, percentage: spo2.toDouble());

    // 2.3 Tạo model AccelerometerTopic
    final accelTopic = AccelerometerTopic(
      userId: userId,
      totalVector: motion.toDouble(),
      epochMinutes: epochMinutes,
    );

    // 2.4 Gửi lên broker
    final mqtt = ProcessMqttService.instance;
    try {
      await mqtt.sendHeartRate(hrTopic);
      await mqtt.sendSpO2(spo2Topic);
      await mqtt.sendAccelerometer(accelTopic);
    } catch (e) {
      // Xử lý lỗi network/MQTT ở đây
      print('❌ Lỗi khi publish MQTT: $e');
    }
  }

  /// Lấy userId hiện tại từ chỗ lưu trữ (ví dụ SharedPreferences, Auth service, v.v.)
  ///
  /// Bạn cần phải implement logic lấy userId trong hàm này.
  ///
  /// Returns [String] là userId
  Future<String> _getCurrentUserId() async {
    // TODO: implement logic lấy userId
    return 'user123';
  }

  @override
  void dispose() {
    _connSub?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return widget.child;
  }
}
