import 'dart:async';

import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:wearable_app/core/config/firebase_options.dart';
import 'package:wearable_app/services/ble_service.dart';
import 'package:wearable_app/services/location_publisher.dart';
import 'package:wearable_app/services/mqtt_service.dart';
import 'package:wearable_app/core/utils/process_ble_data.dart';
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

class MyApp extends StatelessWidget {
  const MyApp({super.key});

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
  final BleService _bleService = BleService();
  final ProcessBleDataService _processBle = ProcessBleDataService();
  StreamSubscription<BluetoothConnectionState>? _connSub;

  @override
  void initState() {
    super.initState();
    _setupBle();
  }

  void _setupBle() {
    // Bắt đầu kết nối và lắng nghe lại khi thất bại hoặc ngắt kết nối
    _attemptConnection();
    // Nghe trạng thái kết nối để tự động reconnect
    _connSub = _bleService.deviceState?.listen((state) {
      if (state == BluetoothConnectionState.disconnected) {
        debugPrint('BLE disconnected, retrying...');
        _attemptConnection();
      }
    });
  }

  Future<void> _attemptConnection() async {
    const macAddress = 'CC:BA:97:0B:61:0E';
    const retryInterval = Duration(seconds: 10);
    BluetoothDevice? device;

    while (device == null && mounted) {
      try {
        device = await _bleService.scanAndConnectById(macAddress);
        // ignore: deprecated_member_use
        debugPrint('BLE connected: ${device.name} (${device.id})');
        _processBle.startProcessing();
      } catch (e) {
        debugPrint(
          'BLE connect failed, retry in \${retryInterval.inSeconds}s: $e',
        );
        await Future.delayed(retryInterval);
      }
    }
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
