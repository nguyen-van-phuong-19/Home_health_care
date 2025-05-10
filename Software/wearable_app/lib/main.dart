import 'dart:async';
import 'dart:typed_data';

import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:wearable_app/core/config/firebase_options.dart';
import 'package:wearable_app/services/location_publisher.dart';
import 'package:wearable_app/services/mqtt_service.dart';
import 'package:wearable_app/screens/home_screen/home_screen.dart';

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();

  // Khởi tạo Firebase
  await Firebase.initializeApp(options: DefaultFirebaseOptions.currentPlatform);

  // Khởi tạo và kết nối MQTT broker
  await MQTTService().init();
  await MQTTService().connect();

  // Bắt đầu chia sẻ vị trí định kỳ (nếu cần)
  await LocationPublisher.instance.startPeriodic('user123', intervalMinutes: 1);

  // Gọi runApp ngay để UI hiển thị nhanh, kết nối BLE trong widget
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

  @override
  void initState() {
    super.initState();
    _listenScan();
  }

  /// Lắng nghe sự kiện scan và kết nối với thiết bị
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

  void handleScanResult(ScanResult r) {
    final m = r.advertisementData.manufacturerData;
    // Key là Company ID (0xABCD)
    final data = m[0xABCD];
    if (data == null || data.length < 6) return;

    final bd = ByteData.sublistView(Uint8List.fromList(data));
    final hr = bd.getUint8(0);
    final spo2 = bd.getUint8(1);
    final motion = bd.getFloat32(2, Endian.little);

    print('HR: $hr, SpO2: $spo2, Motion: ${motion.toStringAsFixed(2)}');
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
