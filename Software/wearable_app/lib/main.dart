import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/material.dart';
import 'package:wearable_app/core/config/firebase_options.dart';
import 'package:wearable_app/screens/home_screen/home_screen.dart';
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
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Health Tracker',
      theme: ThemeData(
        primarySwatch: Colors.green,
        scaffoldBackgroundColor: const Color(0xFFF7FFF0), // Màu nền nhẹ nhàng
      ),
      home: const HomeScreen(),
      debugShowCheckedModeBanner: false,
    );
  }
}
