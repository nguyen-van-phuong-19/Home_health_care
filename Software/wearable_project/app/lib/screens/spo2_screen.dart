import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:provider/provider.dart';
import 'package:wearable_app/services/user_provider.dart';
import 'package:wearable_app/widgets/line_chart_widget.dart';
import 'package:intl/intl.dart';

class SpO2Screen extends StatefulWidget {
  const SpO2Screen({Key? key}) : super(key: key);

  @override
  State<SpO2Screen> createState() => _SpO2ScreenState();
}

class _SpO2ScreenState extends State<SpO2Screen> {
  Map<String, dynamic> spo2Data = {
    'current': {'percentage': 0, 'timestamp': ''},
    'dailyAvg': 0,
    'min': 0,
    'max': 0
  };
  
  List<Map<String, dynamic>> spo2History = [];

  @override
  void initState() {
    super.initState();
    _fetchSpO2Data();
    _fetchSpO2History();
  }

  void _fetchSpO2Data() {
    final userId = Provider.of<UserProvider>(context, listen: false).userId;
    
    FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .snapshots()
        .listen((docSnapshot) {
      if (docSnapshot.exists) {
        final data = docSnapshot.data()!;
        if (data['latest_spo2'] != null) {
          setState(() {
            spo2Data['current'] = data['latest_spo2'];
          });
        }
      }
    });
  }

  void _fetchSpO2History() {
    final userId = Provider.of<UserProvider>(context, listen: false).userId;
    
    // Get SpO2 history for the last 24 hours
    final now = DateTime.now();
    final yesterday = now.subtract(const Duration(hours: 24));
    
    FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .collection('spo2')
        .where('timestamp', isGreaterThanOrEqualTo: yesterday.toIso8601String())
        .orderBy('timestamp')
        .snapshots()
        .listen((querySnapshot) {
      final spo2Data = querySnapshot.docs.map((doc) {
        final data = doc.data();
        return {
          'timestamp': data['timestamp'],
          'value': data['percentage']
        };
      }).toList();
      
      // Calculate stats
      int sum = 0;
      int min = 101;
      int max = 0;
      
      for (var data in spo2Data) {
        final percentage = data['value'] as int;
        sum += percentage;
        min = min > percentage ? percentage : min;
        max = max < percentage ? percentage : max;
      }
      
      final avg = spo2Data.isNotEmpty ? (sum / spo2Data.length).round() : 0;
      
      setState(() {
        spo2History = spo2Data;
        this.spo2Data['dailyAvg'] = avg;
        this.spo2Data['min'] = min == 101 ? 0 : min;
        this.spo2Data['max'] = max;
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

  Map<String, dynamic> _getSpO2Status(int percentage) {
    if (percentage < 90) return {'status': 'Low', 'color': Colors.red};
    if (percentage < 95) return {'status': 'Acceptable', 'color': Colors.amber};
    return {'status': 'Normal', 'color': Colors.green};
  }

  @override
  Widget build(BuildContext context) {
    final currentPercentage = spo2Data['current']['percentage'] as int? ?? 0;
    final spo2Status = _getSpO2Status(currentPercentage);
    
    return Scaffold(
      appBar: AppBar(
        title: const Text('Blood Oxygen (SpO2)'),
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Current SpO2
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
                          color: spo2Status['color'] as Color,
                          width: 3,
                        ),
                      ),
                      child: Center(
                        child: Column(
                          mainAxisSize: MainAxisSize.min,
                          children: [
                            Text(
                              currentPercentage.toString(),
                              style: const TextStyle(
                                fontSize: 32,
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                            const Text(
                              '%',
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
                            spo2Status['status'] as String,
                            style: TextStyle(
                              fontSize: 18,
                              fontWeight: FontWeight.bold,
                              color: spo2Status['color'] as Color,
                            ),
                          ),
                          const SizedBox(height: 8),
                          Text(
                            'Last updated: ${_formatTimestamp(spo2Data['current']['timestamp'] ?? '')}',
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
                            '${spo2Data['dailyAvg']}',
                            style: const TextStyle(
                              fontSize: 20,
                            ),
                          ),
                          const Text('%'),
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
                            '${spo2Data['min']}',
                            style: const TextStyle(
                              fontSize: 20,
                            ),
                          ),
                          const Text('%'),
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
                            '${spo2Data['max']}',
                            style: const TextStyle(
                              fontSize: 20,
                            ),
                          ),
                          const Text('%'),
                        ],
                      ),
                    ),
                  ),
                ),
              ],
            ),
            
            const SizedBox(height: 24),
            
            // SpO2 chart
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
                      '24-Hour SpO2 Levels',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 16),
                    SizedBox(
                      height: 200,
                      child: spo2History.isEmpty
                          ? const Center(child: Text('No data available'))
                          : LineChartWidget(
                              data: spo2History,
                              color: Colors.blueAccent,
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
                      'About Blood Oxygen Saturation (SpO2)',
                      style: TextStyle(
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 8),
                    const Text(
                      'SpO2 is a measure of the amount of oxygen-carrying hemoglobin in the blood relative to the amount of hemoglobin not carrying oxygen. '
                      'Normal SpO2 levels are typically between 95% and 100%.',
                    ),
                    const SizedBox(height: 16),
                    const Text(
                      'SpO2 Levels:',
                      style: TextStyle(
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 8),
                    _buildLevelRow('Normal', '95-100%', Colors.green),
                    _buildLevelRow('Acceptable', '90-94%', Colors.amber),
                    _buildLevelRow('Low', 'Below 90%', Colors.red),
                    const SizedBox(height: 16),
                    const Text(
                      'Low SpO2 levels can indicate hypoxemia (low blood oxygen), which may be caused by various conditions including respiratory disorders, sleep apnea, or high altitude.',
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
  
  Widget _buildLevelRow(String level, String range, Color color) {
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
            level,
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
