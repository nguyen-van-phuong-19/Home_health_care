import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:provider/provider.dart';
import 'package:wearable_app/services/user_provider.dart';
import 'package:wearable_app/widgets/line_chart_widget.dart';
import 'package:intl/intl.dart';

class HeartRateScreen extends StatefulWidget {
  const HeartRateScreen({Key? key}) : super(key: key);

  @override
  State<HeartRateScreen> createState() => _HeartRateScreenState();
}

class _HeartRateScreenState extends State<HeartRateScreen> {
  Map<String, dynamic> heartRateData = {
    'current': {'bpm': 0, 'timestamp': ''},
    'dailyAvg': 0,
    'min': 0,
    'max': 0
  };
  
  List<Map<String, dynamic>> heartRateHistory = [];

  @override
  void initState() {
    super.initState();
    _fetchHeartRateData();
    _fetchHeartRateHistory();
  }

  void _fetchHeartRateData() {
    final userId = Provider.of<UserProvider>(context, listen: false).userId;
    
    FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .snapshots()
        .listen((docSnapshot) {
      if (docSnapshot.exists) {
        final data = docSnapshot.data()!;
        if (data['latest_heart_rate'] != null) {
          setState(() {
            heartRateData['current'] = data['latest_heart_rate'];
          });
        }
      }
    });
  }

  void _fetchHeartRateHistory() {
    final userId = Provider.of<UserProvider>(context, listen: false).userId;
    
    // Get heart rate history for the last 24 hours
    final now = DateTime.now();
    final yesterday = now.subtract(const Duration(hours: 24));
    
    FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .collection('heart_rate')
        .where('timestamp', isGreaterThanOrEqualTo: yesterday.toIso8601String())
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
      
      // Calculate stats
      int sum = 0;
      int min = 999;
      int max = 0;
      
      for (var data in heartRateData) {
        final bpm = data['value'] as int;
        sum += bpm;
        min = min > bpm ? bpm : min;
        max = max < bpm ? bpm : max;
      }
      
      final avg = heartRateData.isNotEmpty ? (sum / heartRateData.length).round() : 0;
      
      setState(() {
        heartRateHistory = heartRateData;
        this.heartRateData['dailyAvg'] = avg;
        this.heartRateData['min'] = min == 999 ? 0 : min;
        this.heartRateData['max'] = max;
      });
    });
  }

  String _formatTimestamp(String timestamp) {
    if (timestamp.isEmpty) return 'N/A';
    try {
      final dateTime = DateTime.parse(timestamp);
      return DateFormat.yMd().add_jm().format(dateTime);
    } catch (e) {
      return 'Invalid date';
    }
  }

  Map<String, dynamic> _getHeartRateZone(int bpm) {
    if (bpm < 60) return {'zone': 'Resting', 'color': Colors.blue};
    if (bpm < 100) return {'zone': 'Normal', 'color': Colors.green};
    if (bpm < 140) return {'zone': 'Moderate', 'color': Colors.amber};
    if (bpm < 170) return {'zone': 'Intense', 'color': Colors.orange};
    return {'zone': 'Maximum', 'color': Colors.red};
  }

  @override
  Widget build(BuildContext context) {
    final currentBpm = heartRateData['current']['bpm'] as int? ?? 0;
    final heartRateZone = _getHeartRateZone(currentBpm);
    
    return Scaffold(
      appBar: AppBar(
        title: const Text('Heart Rate'),
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Current heart rate
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
                          color: heartRateZone['color'] as Color,
                          width: 3,
                        ),
                      ),
                      child: Center(
                        child: Column(
                          mainAxisSize: MainAxisSize.min,
                          children: [
                            Text(
                              currentBpm.toString(),
                              style: const TextStyle(
                                fontSize: 32,
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                            const Text(
                              'BPM',
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
                            '${heartRateZone['zone']} Zone',
                            style: TextStyle(
                              fontSize: 18,
                              fontWeight: FontWeight.bold,
                              color: heartRateZone['color'] as Color,
                            ),
                          ),
                          const SizedBox(height: 8),
                          Text(
                            'Last updated: ${_formatTimestamp(heartRateData['current']['timestamp'] ?? '')}',
                            style: TextStyle(color: Colors.grey[600]),
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
                            'Daily Average',
                            style: TextStyle(
                              fontSize: 14,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                          const SizedBox(height: 8),
                          Text(
                            '${heartRateData['dailyAvg']}',
                            style: const TextStyle(
                              fontSize: 20,
                            ),
                          ),
                          const Text('BPM'),
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
                            'Minimum',
                            style: TextStyle(
                              fontSize: 14,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                          const SizedBox(height: 8),
                          Text(
                            '${heartRateData['min']}',
                            style: const TextStyle(
                              fontSize: 20,
                            ),
                          ),
                          const Text('BPM'),
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
                            'Maximum',
                            style: TextStyle(
                              fontSize: 14,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                          const SizedBox(height: 8),
                          Text(
                            '${heartRateData['max']}',
                            style: const TextStyle(
                              fontSize: 20,
                            ),
                          ),
                          const Text('BPM'),
                        ],
                      ),
                    ),
                  ),
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
                      '24-Hour Heart Rate',
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
            
            // Info card
            Card(
              elevation: 2,
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'About Heart Rate',
                      style: TextStyle(
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 8),
                    const Text(
                      'Your heart rate is the number of times your heart beats per minute (BPM). '
                      'A normal resting heart rate for adults ranges from 60 to 100 BPM. '
                      'Athletes and people with high cardiovascular fitness may have resting heart rates as low as 40 BPM.',
                    ),
                    const SizedBox(height: 16),
                    const Text(
                      'Heart Rate Zones:',
                      style: TextStyle(
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 8),
                    _buildZoneRow('Resting', 'Below 60 BPM', Colors.blue),
                    _buildZoneRow('Normal', '60-100 BPM', Colors.green),
                    _buildZoneRow('Moderate', '100-140 BPM', Colors.amber),
                    _buildZoneRow('Intense', '140-170 BPM', Colors.orange),
                    _buildZoneRow('Maximum', 'Above 170 BPM', Colors.red),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
  
  Widget _buildZoneRow(String zone, String range, Color color) {
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
            zone,
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
