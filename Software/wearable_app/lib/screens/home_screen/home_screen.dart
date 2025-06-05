// import 'dart:async';

import 'package:firebase_auth/firebase_auth.dart';
import 'package:flutter/material.dart';
import 'package:flutter_map/flutter_map.dart';
import 'package:latlong2/latlong.dart';
import 'package:wearable_app/screens/history_screen/history_screen.dart';
import 'package:wearable_app/screens/home_screen/widgets/build_stat_card.dart';
import 'package:wearable_app/screens/sleep_screen/sleep_detail_screen.dart';
import 'package:wearable_app/services/firebase_service.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  final String _userId = FirebaseAuth.instance.currentUser?.uid ?? '';

  /// Hiển thị dialog thông tin user
  void _showUserInfo() {
    final user = FirebaseAuth.instance.currentUser;
    if (user == null) return;

    showDialog(
      context: context,
      builder:
          (_) => AlertDialog(
            title: const Text('Thông tin người dùng'),
            content: Column(
              mainAxisSize: MainAxisSize.min,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text('UID: ${user.uid}'),
                Text('Email: ${user.email ?? 'Không có'}'),
              ],
            ),
            actions: [
              TextButton(
                onPressed: () => Navigator.pop(context),
                child: const Text('Đóng'),
              ),
            ],
          ),
    );
  }

  /// Đăng xuất tài khoản
  Future<void> _signOut() async {
    await FirebaseAuth.instance.signOut();
    // Khi signOut, AuthenticationWrapper sẽ tự động quay về SignInScreen
  }

  int _heartRate = 0;
  double _spo2 = 0.0;
  double _calories = 0.0;
  double _sleepHours = 0.0;
  bool _isSleeping = false;

  double _latitude = 0.0;
  double _longitude = 0.0;

  @override
  void initState() {
    super.initState();
    // Listen to scan results and update UI
    // Listen latest heart rate
    FirebaseService().listenToChanges('users/$_userId/latest_heart_rate').listen((
      event,
    ) {
      final raw = event.snapshot.value;
      if (raw is Map && raw['bpm'] != null) {
        setState(() => _heartRate = (raw['bpm'] as num).toInt());
      }
    });

    // Listen latest SpO2
    FirebaseService().listenToChanges('users/$_userId/latest_spo2').listen((
      event,
    ) {
      final raw = event.snapshot.value;
      if (raw is Map && raw['percentage'] != null) {
        setState(() => _spo2 = (raw['percentage'] as num).toDouble());
      }
    });

    // Listen today's combined calories
    final today = DateTime.now().toIso8601String().split('T').first;
    FirebaseService()
        .listenToChanges('users/$_userId/calories_daily/$today')
        .listen((event) {
          final raw = event.snapshot.value;
          if (raw is Map && raw['combined_daily_calories'] != null) {
            setState(
              () =>
                  _calories =
                      (raw['combined_daily_calories'] as num).toDouble(),
            );
          }
        });

    // Listen today's sleep duration
    FirebaseService()
        .listenToChanges('users/$_userId/daily_sleep/$today')
        .listen((event) {
          final raw = event.snapshot.value;
          if (raw is Map) {
            if (raw['sleep_duration'] != null) {
              _sleepHours = (raw['sleep_duration'] as num).toDouble();
            }
            if (raw['is_sleeping'] != null) {
              _isSleeping = raw['is_sleeping'] as bool;
            }
            setState(() {});
          }
        });

    // Listen latest location
    FirebaseService().listenToChanges('users/$_userId/latest_location').listen((
      event,
    ) {
      final raw = event.snapshot.value;
      if (raw is Map && raw['latitude'] != null && raw['longitude'] != null) {
        setState(() {
          _latitude = (raw['latitude'] as num).toDouble();
          _longitude = (raw['longitude'] as num).toDouble();
        });
      }
    });
  }

  String _formatSleep(double hours) {
    final h = hours.floor();
    final m = ((hours - h) * 60).round();
    return '${h}h ${m.toString().padLeft(2, '0')}m';
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Dashboard'),
        actions: [
          PopupMenuButton<String>(
            onSelected: (value) {
              switch (value) {
                case 'info':
                  _showUserInfo();
                  break;
                case 'ble':
                  // TODO: chuyển tới màn BLE của bạn
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('Chức năng BLE chưa có')),
                  );
                  break;
                case 'settings':
                  // TODO: chuyển tới màn Cài đặt
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('Chức năng Cài đặt chưa có')),
                  );
                  break;
                case 'logout':
                  _signOut();
                  break;
              }
            },
            itemBuilder:
                (ctx) => const [
                  PopupMenuItem(value: 'info', child: Text('Thông tin')),
                  PopupMenuItem(value: 'ble', child: Text('BLE')),
                  PopupMenuItem(value: 'settings', child: Text('Cài đặt')),
                  PopupMenuItem(value: 'logout', child: Text('Đăng xuất')),
                ],
          ),
        ],
      ),
      body: SafeArea(
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: Column(
            children: [
              // Stats row
              Row(
                children: [
                  Expanded(
                    child: GestureDetector(
                      onTap:
                          () => Navigator.push(
                            context,
                            MaterialPageRoute(
                              builder:
                                  (_) =>
                                      HeartRateHistoryScreen(userId: _userId),
                            ),
                          ),
                      child: buildStatCard(
                        title: "Heart Rate",
                        value: "$_heartRate bpm",
                        color: Colors.redAccent,
                        iconData: Icons.favorite,
                      ),
                    ),
                  ),
                  const SizedBox(width: 16),
                  Expanded(
                    child: GestureDetector(
                      onTap:
                          () => Navigator.push(
                            context,
                            MaterialPageRoute(
                              builder:
                                  (_) => Spo2HistoryScreen(userId: _userId),
                            ),
                          ),
                      child: buildStatCard(
                        title: "SpO₂",
                        value: "${_spo2.toStringAsFixed(1)}%",
                        color: const Color(0xFFB3E5FC),
                        iconData: Icons.health_and_safety,
                      ),
                    ),
                  ),
                  const SizedBox(width: 16),
                  Expanded(
                    child: GestureDetector(
                      onTap:
                          () => Navigator.push(
                            context,
                            MaterialPageRoute(
                              builder:
                                  (_) => CaloriesHistoryScreen(userId: _userId),
                            ),
                          ),
                      child: buildStatCard(
                        title: "Calories",
                        value: "${_calories.toStringAsFixed(0)} kcal",
                        color: Colors.orangeAccent,
                        iconData: Icons.local_fire_department,
                      ),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 16),

              // Sleep card
              GestureDetector(
                onTap:
                    () => Navigator.push(
                      context,
                      MaterialPageRoute(
                        builder: (_) => SleepDetailScreen(userId: _userId),
                      ),
                    ),
                child: Container(
                  width: double.infinity,
                  padding: const EdgeInsets.all(16),
                  decoration: BoxDecoration(
                    color: Colors.deepPurple.shade100,
                    borderRadius: BorderRadius.circular(12),
                  ),
                  child: Row(
                    children: [
                      Icon(Icons.bedtime, size: 40, color: Colors.deepPurple),
                      const SizedBox(width: 16),
                      Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          const Text(
                            "Sleep",
                            style: TextStyle(
                              fontSize: 18,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                          const SizedBox(height: 4),
                          Text(
                          _formatSleep(_sleepHours),
                          style: const TextStyle(fontSize: 16),
                        ),
                        const SizedBox(height: 4),
                        Text(
                          _isSleeping ? 'Đang ngủ' : 'Đã thức',
                          style: const TextStyle(fontSize: 14),
                        ),
                      ],
                    ),
                  ],
                ),
              ),
              ),
              const SizedBox(height: 16),

              // Mini map with flutter_map
              if (_latitude == 0 && _longitude == 0)
                const Text('Chưa có dữ liệu vị trí')
              else
                Container(
                  height: 200,
                  decoration: BoxDecoration(
                    borderRadius: BorderRadius.circular(12),
                    border: Border.all(color: Colors.grey.shade300),
                  ),
                  clipBehavior: Clip.hardEdge,
                  child: FlutterMap(
                    options: MapOptions(
                      initialCenter: LatLng(_latitude, _longitude),
                    ),
                    children: [
                      TileLayer(
                        urlTemplate:
                            'https://tile.openstreetmap.org/{z}/{x}/{y}.png',
                      ),
                      MarkerLayer(
                        markers: [
                          Marker(
                            point: LatLng(_latitude, _longitude),
                            width: 48,
                            height: 48,
                            child: const Icon(
                              Icons.location_pin,
                              color: Colors.redAccent,
                              size: 36,
                            ),
                          ),
                        ],
                      ),
                    ],
                  ),
                ),
            ],
          ),
        ),
      ),
    );
  }
}
