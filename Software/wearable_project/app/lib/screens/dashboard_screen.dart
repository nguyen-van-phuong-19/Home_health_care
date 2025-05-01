import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:provider/provider.dart';
import 'package:wearable_app/services/user_provider.dart';
import 'package:wearable_app/widgets/stat_card.dart';
import 'package:wearable_app/widgets/line_chart_widget.dart';
import 'package:intl/intl.dart';

class DashboardScreen extends StatefulWidget {
  const DashboardScreen({Key? key}) : super(key: key);

  @override
  State<DashboardScreen> createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen> {
  Map<String, dynamic> userData = {
    'heartRate': {'bpm': 0, 'timestamp': ''},
    'spo2': {'percentage': 0, 'timestamp': ''},
    'calories': {'calories_burned_today': 0, 'timestamp': ''},
    'sleep': {'duration_hours': 0, 'timestamp': ''},
    'location': {'latitude': 0, 'longitude': 0, 'timestamp': ''}
  };

  List<Map<String, dynamic>> heartRateHistory = [];

  @override
  void initState() {
    super.initState();
    _fetchUserData();
    _fetchHeartRateHistory();
  }

  void _fetchUserData() {
    final userId = Provider.of<UserProvider>(context, listen: false).userId;
    
    FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .snapshots()
        .listen((docSnapshot) {
      if (docSnapshot.exists) {
        final data = docSnapshot.data()!;
        setState(() {
          userData = {
            'heartRate': data['latest_heart_rate'] ?? {'bpm': 0, 'timestamp': ''},
            'spo2': data['latest_spo2'] ?? {'percentage': 0, 'timestamp': ''},
            'calories': data['latest_activity'] ?? {'calories_burned_today': 0, 'timestamp': ''},
            'sleep': data['latest_sleep'] ?? {'duration_hours': 0, 'timestamp': ''},
            'location': data['latest_location'] ?? {'latitude': 0, 'longitude': 0, 'timestamp': ''}
          };
        });
      }
    });
  }

  void _fetchHeartRateHistory() {
    final userId = Provider.of<UserProvider>(context, listen: false).userId;
    
    // Get heart rate history for today
    final today = DateTime.now();
    final startOfDay = DateTime(today.year, today.month, today.day);
    
    FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .collection('heart_rate')
        .where('timestamp', isGreaterThanOrEqualTo: startOfDay.toIso8601String())
        .orderBy('timestamp')
        .snapshots()
        .listen((querySnapshot) {
      final heartRateData = querySnapshot.docs.map((doc) {
        final data = doc.data();
        return {
          'timestamp': data['timestamp'],
          'value': data['bpm']
        };
      }).toList();
      
      setState(() {
        heartRateHistory = heartRateData;
      });
    });
  }

  String _formatTimestamp(String timestamp) {
    if (timestamp.isEmpty) return 'N/A';
    try {
      final dateTime = DateTime.parse(timestamp);
      return DateFormat.jm().format(dateTime);
    } catch (e) {
      return 'Invalid date';
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Health Dashboard'),
        actions: [
          IconButton(
            icon: const Icon(Icons.settings),
            onPressed: () {
              Navigator.pushNamed(context, '/settings');
            },
          ),
        ],
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              'Last updated: ${_formatTimestamp(userData['heartRate']['timestamp'])}',
              style: TextStyle(color: Colors.grey[600]),
            ),
            const SizedBox(height: 16),
            
            // Stat cards
            GridView.count(
              crossAxisCount: 2,
              crossAxisSpacing: 10,
              mainAxisSpacing: 10,
              shrinkWrap: true,
              physics: const NeverScrollableScrollPhysics(),
              children: [
                StatCard(
                  title: 'Heart Rate',
                  value: userData['heartRate']['bpm'].toString(),
                  unit: 'bpm',
                  icon: Icons.favorite,
                  color: Colors.redAccent,
                ),
                StatCard(
                  title: 'SpO2',
                  value: userData['spo2']['percentage'].toString(),
                  unit: '%',
                  icon: Icons.air,
                  color: Colors.blueAccent,
                ),
                StatCard(
                  title: 'Calories',
                  value: userData['calories']['calories_burned_today'].toString(),
                  unit: 'kcal',
                  icon: Icons.local_fire_department,
                  color: Colors.greenAccent,
                ),
                StatCard(
                  title: 'Sleep',
                  value: userData['sleep']['duration_hours'].toString(),
                  unit: 'hours',
                  icon: Icons.nightlight,
                  color: Colors.purpleAccent,
                ),
              ],
            ),
            
            const SizedBox(height: 24),
            
            // Heart rate chart
            Card(
              elevation: 4,
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(12),
              ),
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'Heart Rate Today',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 16),
                    SizedBox(
                      height: 200,
                      child: heartRateHistory.isEmpty
                          ? const Center(child: Text('No data available'))
                          : LineChartWidget(
                              data: heartRateHistory,
                              color: Colors.redAccent,
                            ),
                    ),
                  ],
                ),
              ),
            ),
            
            const SizedBox(height: 24),
            
            // Location card
            Card(
              elevation: 4,
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(12),
              ),
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'Last Known Location',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 16),
                    Text(
                      'Latitude: ${userData['location']['latitude'].toStringAsFixed(6)}\n'
                      'Longitude: ${userData['location']['longitude'].toStringAsFixed(6)}',
                      style: const TextStyle(fontSize: 16),
                    ),
                    const SizedBox(height: 8),
                    Text(
                      'Recorded at: ${_formatTimestamp(userData['location']['timestamp'])}',
                      style: TextStyle(color: Colors.grey[600]),
                    ),
                    const SizedBox(height: 8),
                    ElevatedButton.icon(
                      icon: const Icon(Icons.map),
                      label: const Text('View on Map'),
                      onPressed: () {
                        Navigator.pushNamed(context, '/location');
                      },
                    ),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
