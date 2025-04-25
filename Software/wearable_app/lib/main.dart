import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:wearable_app/core/config/firebase_options.dart';
import 'package:wearable_app/screens/home_screen/home_screen.dart';
import 'package:wearable_app/services/ble_service.dart';
import 'package:wearable_app/services/location_publisher.dart';
import 'package:wearable_app/services/mqtt_service.dart';

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp(options: DefaultFirebaseOptions.currentPlatform);
  await MQTTService().init(); // kết nối broker
  await MQTTService().connect(); // kết nối broker
  await LocationPublisher.instance.startPeriodic(
    'user123',
    intervalMinutes: 1,
  ); // thay 'user123' bằng userId thực
  // Instantiate the BLE service
  final bleService = BleService();

  try {
    final device = await bleService.scanAndConnectById("CC:BA:97:0B:61:0E");
    print("Kết nối thành công đến: ${device.name} (${device.id})");
  } catch (e) {
    print("Không tìm thấy thiết bị hoặc kết nối thất bại: $e");
  }
  runApp(MyApp(bleService: bleService));
}

class MyApp extends StatelessWidget {
  final BleService bleService;
  const MyApp({super.key, required this.bleService});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Health Tracker',
      theme: ThemeData(
        primarySwatch: Colors.green,
        scaffoldBackgroundColor: const Color(0xFFF7FFF0), // Màu nền nhẹ nhàng
      ),
      home: HomeScreen(bleService: bleService),
      debugShowCheckedModeBanner: false,
    );
  }
}
