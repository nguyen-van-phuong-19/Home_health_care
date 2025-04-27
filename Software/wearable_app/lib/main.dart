import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/material.dart';
import 'package:wearable_app/core/config/firebase_options.dart';
import 'package:wearable_app/core/utils/process_ble_data.dart';
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
  // Khởi tạo BLE và ProcessBleData
  final bleService = BleService();
  final processBle = ProcessBleDataService();

  try {
    // Quét và kết nối đến ESP32-S3 theo MAC
    final device = await bleService.scanAndConnectById("CC:BA:97:0B:61:0E");
    print("Kết nối thành công đến: ${device.name} (${device.id})");

    // Bắt đầu lắng nghe và xử lý dữ liệu BLE
    processBle.startProcessing();
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
