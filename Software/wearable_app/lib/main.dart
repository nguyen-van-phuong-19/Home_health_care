import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';

import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_localizations/flutter_localizations.dart';
import 'package:wearable_app/core/config/firebase_options.dart';
import 'package:wearable_app/core/utils/process_mqtt.dart';
import 'package:wearable_app/models/topic_model.dart';
import 'package:wearable_app/screens/authentication_wrapper.dart';
import 'package:wearable_app/services/ble_service.dart';
import 'package:wearable_app/services/location_publisher.dart';
import 'package:wearable_app/services/mqtt_service.dart';
import 'package:firebase_ui_localizations/firebase_ui_localizations.dart';

Future<String> _loadGoogleWebClientId() async {
  final raw = await rootBundle.loadString('android/app/google-services.json');
  final json = jsonDecode(raw) as Map<String, dynamic>;

  // tìm oauth_client có client_type = 3 (Web)
  final clients =
      (json['client'] as List)
          .expand((c) => (c['oauth_client'] as List))
          .cast<Map<String, dynamic>>();
  final web = clients.firstWhere((c) => c['client_type'] == 3);
  return web['client_id'] as String;
}

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp(options: DefaultFirebaseOptions.currentPlatform);

  await MQTTService().init();
  await MQTTService().connect();
  final webId = await _loadGoogleWebClientId();

  await LocationPublisher.instance.startPeriodic(
    FirebaseAuth.instance.currentUser!.uid,
    intervalMinutes: 1,
  );

  runApp(MyApp(googleClientId: webId));
}

class MyApp extends StatefulWidget {
  final String googleClientId;
  const MyApp({super.key, required this.googleClientId});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  @override
  Widget build(BuildContext context) {
    return BleInitializer(
      child: MaterialApp(
        localizationsDelegates: [
          GlobalMaterialLocalizations.delegate,
          GlobalWidgetsLocalizations.delegate,
          FirebaseUILocalizations.delegate, // thêm delegate cho Firebase UI
        ],
        supportedLocales: [Locale('vi'), Locale('en')],
        title: 'Health Tracker',
        theme: ThemeData(
          primarySwatch: Colors.green,
          scaffoldBackgroundColor: const Color(0xFFF7FFF0),
        ),
        home: AuthenticationWrapper(googleClientId: widget.googleClientId),
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
  late double spo2Last;
  late double motionLast;
  late double cl_hr;
  late double cl_total;
  late double cl_motion;

  @override
  void initState() {
    super.initState();
    hrLast = 0;
    spo2Last = 0;
    motionLast = 0;
    cl_hr = 0;
    cl_total = 0;
    cl_motion = 0;
    print(FirebaseAuth.instance.currentUser?.uid);
    print(FirebaseAuth.instance.currentUser?.uid);
    print(FirebaseAuth.instance.currentUser?.uid);
    print(FirebaseAuth.instance.currentUser?.uid);
    // _listenScan();
    BleService.instance.onEsp32Advertisement = (payload) async {
      // Xử lý payload từ ESP32-S3 ở đây
      final bd = ByteData.sublistView(Uint8List.fromList(payload));
      // print(bd.buffer.asUint8List().toString());
      final hr = bd.getUint8(0);
      final spo2 = bd.getFloat32(1, Endian.little);
      final motion = bd.getFloat32(5, Endian.little);
      final calo_hr = bd.getFloat32(9, Endian.little);
      final calo_motion = bd.getFloat32(13, Endian.little);
      final calo_total = bd.getFloat32(17, Endian.little);

      if (hr == hrLast &&
          spo2 == spo2Last &&
          motion == motionLast &&
          calo_hr == cl_hr &&
          calo_total == cl_total &&
          calo_motion == cl_motion) {
        return;
      }

      if (hr == 0 && spo2 == 0 && motion == 0) {
        return;
      }

      setState(() {
        hrLast = hr;
        spo2Last = spo2;
        motionLast = motion;
        cl_hr = calo_hr;
        cl_total = calo_total;
        cl_motion = calo_motion;
      });

      print(
        'HR: $hr, SpO2: ${spo2.toStringAsFixed(2)}, Motion: ${motion.toStringAsFixed(2)},\nCalo HR: ${calo_hr.toStringAsFixed(2)}, Calo Total: ${calo_total.toStringAsFixed(2)}, Calo Motion: ${calo_motion.toStringAsFixed(2)}',
      );
      _publishBleData(hr, spo2, motion, calo_hr, calo_total, calo_motion);
    };

    BleService.instance.start(FirebaseAuth.instance.currentUser?.uid);
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
  Future<void> _publishBleData(
    int hr,
    double spo2,
    double motion,
    double caloHr,
    double caloTotal,
    double caloMotion,
  ) async {
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
    final spo2Topic = Spo2Topic(
      userId: userId,
      percentage: double.parse(spo2.toStringAsFixed(2)),
    );

    // 2.3 Tạo model AccelerometerTopic
    final accelTopic = AccelerometerTopic(
      userId: userId,
      totalVector: double.parse(motion.toStringAsFixed(2)),
      epochMinutes: epochMinutes,
    );

    final caloriesTopic = CaloriesTopic(
      userId: userId,
      caloHr: double.parse(caloHr.toStringAsFixed(2)),
      caloTotal: double.parse(caloTotal.toStringAsFixed(2)),
      caloMotion: double.parse(caloMotion.toStringAsFixed(2)),
    );

    // 2.4 Gửi lên broker
    final mqtt = ProcessMqttService.instance;
    try {
      await mqtt.sendHeartRate(hrTopic);
      await mqtt.sendSpO2(spo2Topic);
      await mqtt.sendAccelerometer(accelTopic);
      await mqtt.sendCalories(caloriesTopic);
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
    return FirebaseAuth.instance.currentUser?.uid ?? 'default_user';
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
