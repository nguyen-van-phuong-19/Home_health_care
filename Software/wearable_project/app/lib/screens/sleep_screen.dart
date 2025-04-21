import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:provider/provider.dart';
import 'package:wearable_app/services/user_provider.dart';
import 'package:wearable_app/widgets/line_chart_widget.dart';
import 'package:intl/intl.dart';

class SleepScreen extends StatefulWidget {
  const SleepScreen({Key? key}) : super(key: key);

  @override
  State<SleepScreen> createState() => _SleepScreenState();
}

class _SleepScreenState extends State<SleepScreen> {
  Map<String, dynamic> sleepData = {
    'latest': {'start_time': '', 'end_time': '', 'duration_hours': 0},
    'weekly': [],
    'average': 0
  };

  @override
  void initState() {
    super.initState();
    _fetchSleepData();
    _fetchSleepHistory();
  }

  void _fetchSleepData() {
    final userId = Provider.of<UserProvider>(context, listen: false).userId;
    
    FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .snapshots()
        .listen((docSnapshot) {
      if (docSnapshot.exists) {
        final data = docSnapshot.data()!;
        if (data['latest_sleep'] != null) {
          setState(() {
            sleepData['latest'] = data['latest_sleep'];
          });
        }
      }
    });
  }

  void _fetchSleepHistory() {
    final userId = Provider.of<UserProvider>(context, listen: false).userId;
    
    // Get sleep history for the last 7 days
    final now = DateTime.now();
    final sevenDaysAgo = now.subtract(const Duration(days: 7));
    
    FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .collection('sleep')
        .where('start_time', isGreaterThanOrEqualTo: sevenDaysAgo.toIso8601String())
        .orderBy('start_time')
        .snapshots()
        .listen((querySnapshot) {
      final sleepHistory = querySnapshot.docs.map((doc) {
        final data = doc.data();
        final startDate = DateTime.parse(data['start_time']).toLocal();
        return {
          'timestamp': DateFormat('MM/dd').format(startDate),
          'value': data['duration_hours']
        };
      }).toList();
      
      // Calculate average
      double totalDuration = 0;
      for (var sleep in sleepHistory) {
        totalDuration += sleep['value'] as double;
      }
      
      final avgDuration = sleepHistory.isNotEmpty ? totalDuration / sleepHistory.length : 0;
      
      setState(() {
        sleepData['weekly'] = sleepHistory;
        sleepData['average'] = avgDuration.toStringAsFixed(1);
      });
    });
  }

  String _formatDateTime(String timestamp) {
    if (timestamp.isEmpty) return 'N/A';
    try {
      final dateTime = DateTime.parse(timestamp);
      return DateFormat.yMd().add_jm().format(dateTime);
    } catch (e) {
      return 'Invalid date';
    }
  }

  Map<String, dynamic> _getSleepQuality(double hours) {
    if (hours < 6) return {'quality': 'Poor', 'color': Colors.red};
    if (hours < 7) return {'quality': 'Fair', 'color': Colors.amber};
    if (hours < 9) return {'quality': 'Good', 'color': Colors.green};
    return {'quality': 'Excellent', 'color': Colors.blue};
  }

  @override
  Widget build(BuildContext context) {
    final durationHours = sleepData['latest']['duration_hours'] as double? ?? 0;
    final sleepQuality = _getSleepQuality(durationHours);
    
    return Scaffold(
      appBar: AppBar(
        title: const Text('Sleep Tracking'),
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Latest sleep
            Card(
              elevation: 4,
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(12),
              ),
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Row(
                  children: [
                    Container(
                      width: 100,
                      height: 100,
                      decoration: BoxDecoration(
                        shape: BoxShape.circle,
                        border: Border.all(
                          color: sleepQuality['color'] as Color,
                          width: 3,
                        ),
                      ),
                      child: Center(
                        child: Column(
                          mainAxisSize: MainAxisSize.min,
                          children: [
                            Text(
                              durationHours.toStringAsFixed(1),
                              style: const TextStyle(
                                fontSize: 32,
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                            const Text(
                              'hours',
                              style: TextStyle(
                                fontSize: 14,
                              ),
                            ),
                          ],
                        ),
                      ),
                    ),
                    const SizedBox(width: 16),
                    Expanded(
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Text(
                            '${sleepQuality['quality']} Quality',
                            style: TextStyle(
                              fontSize: 18,
                              fontWeight: FontWeight.bold,
                              color: sleepQuality['color'] as Color,
                            ),
                          ),
                          const SizedBox(height: 8),
                          Text(
                            'From: ${_formatDateTime(sleepData['latest']['start_time'] ?? '')}',
                            style: const TextStyle(fontSize: 14),
                          ),
                          Text(
                            'To: ${_formatDateTime(sleepData['latest']['end_time'] ?? '')}',
                            style: const TextStyle(fontSize: 14),
                          ),
                        ],
                      ),
                    ),
                  ],
                ),
              ),
            ),
            
            const SizedBox(height: 16),
            
            // Stats
            Row(
              children: [
                Expanded(
                  child: Card(
                    elevation: 2,
                    child: Padding(
                      padding: const EdgeInsets.all(16.0),
                      child: Column(
                        children: [
                          const Text(
                            'Weekly Average',
                            style: TextStyle(
                              fontSize: 14,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                          const SizedBox(height: 8),
                          Text(
                            '${sleepData['average']}',
                            style: const TextStyle(
                              fontSize: 20,
                            ),
                          ),
                          const Text('hours'),
                        ],
                      ),
                    ),
                  ),
                ),
                Expanded(
                  child: Card(
                    elevation: 2,
                    child: Padding(
                      padding: const EdgeInsets.all(16.0),
                      child: Column(
                        children: [
                          const Text(
                            'Recommended',
                            style: TextStyle(
                              fontSize: 14,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                          const SizedBox(height: 8),
                          const Text(
                            '7-9',
                            style: TextStyle(
                              fontSize: 20,
                            ),
                          ),
                          const Text('hours'),
                        ],
                      ),
                    ),
                  ),
                ),
              ],
            ),
            
            const SizedBox(height: 24),
            
            // Sleep chart
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
                      'Weekly Sleep Duration',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 16),
                    SizedBox(
                      height: 200,
                      child: (sleepData['weekly'] as List).isEmpty
                          ? const Center(child: Text('No data available'))
                          : LineChartWidget(
                              data: sleepData['weekly'] as List<Map<String, dynamic>>,
                              color: Colors.purple,
                            ),
                    ),
                  ],
                ),
              ),
            ),
            
            const SizedBox(height: 24),
            
            // Info card
            Card(
              elevation: 2,
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'About Sleep Tracking',
                      style: TextStyle(
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 8),
                    const Text(
                      'Sleep tracking uses your heart rate and movement patterns to estimate when you fall asleep and wake up. '
                      'The wearable device monitors these patterns to calculate your total sleep duration.',
                    ),
                    const SizedBox(height: 16),
                    const Text(
                      'Sleep Quality Guidelines:',
                      style: TextStyle(
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 8),
                    _buildQualityRow('Poor', 'Less than 6 hours', Colors.red),
                    _buildQualityRow('Fair', '6-7 hours', Colors.amber),
                    _buildQualityRow('Good', '7-9 hours', Colors.green),
                    _buildQualityRow('Excellent', 'More than 9 hours', Colors.blue),
                    const SizedBox(height: 16),
                    const Text(
                      'Most adults need 7-9 hours of quality sleep per night for optimal health. '
                      'Consistent sleep patterns help improve overall well-being and cognitive function.',
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
  
  Widget _buildQualityRow(String quality, String range, Color color) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4.0),
      child: Row(
        children: [
          Container(
            width: 12,
            height: 12,
            decoration: BoxDecoration(
              color: color,
              shape: BoxShape.circle,
            ),
          ),
          const SizedBox(width: 8),
          Text(
            quality,
            style: TextStyle(
              fontWeight: FontWeight.bold,
              color: color,
            ),
          ),
          const SizedBox(width: 8),
          Text(range),
        ],
      ),
    );
  }
}
