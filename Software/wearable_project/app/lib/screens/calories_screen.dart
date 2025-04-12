import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:provider/provider.dart';
import 'package:wearable_app/services/user_provider.dart';
import 'package:wearable_app/widgets/line_chart_widget.dart';
import 'package:intl/intl.dart';

class CaloriesScreen extends StatefulWidget {
  const CaloriesScreen({Key? key}) : super(key: key);

  @override
  State<CaloriesScreen> createState() => _CaloriesScreenState();
}

class _CaloriesScreenState extends State<CaloriesScreen> {
  Map<String, dynamic> caloriesData = {
    'current': {'calories_burned_today': 0, 'state': 'resting', 'timestamp': ''},
    'goal': 2000, // Default goal
    'weeklyData': {}
  };
  
  List<Map<String, dynamic>> activityHistory = [];

  @override
  void initState() {
    super.initState();
    _fetchCaloriesData();
    _fetchActivityHistory();
    _fetchWeeklyData();
  }

  void _fetchCaloriesData() {
    final userId = Provider.of<UserProvider>(context, listen: false).userId;
    
    FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .snapshots()
        .listen((docSnapshot) {
      if (docSnapshot.exists) {
        final data = docSnapshot.data()!;
        if (data['latest_activity'] != null) {
          setState(() {
            caloriesData['current'] = data['latest_activity'];
          });
        }
      }
    });
  }

  void _fetchActivityHistory() {
    final userId = Provider.of<UserProvider>(context, listen: false).userId;
    
    // Get activity history for today
    final today = DateTime.now();
    final startOfDay = DateTime(today.year, today.month, today.day);
    
    FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .collection('activity')
        .where('timestamp', isGreaterThanOrEqualTo: startOfDay.toIso8601String())
        .orderBy('timestamp')
        .snapshots()
        .listen((querySnapshot) {
      final activityData = querySnapshot.docs.map((doc) {
        final data = doc.data();
        return {
          'timestamp': data['timestamp'],
          'value': data['calories_burned'],
          'activity': data['activity_state']
        };
      }).toList();
      
      setState(() {
        activityHistory = activityData;
      });
    });
  }

  void _fetchWeeklyData() {
    final userId = Provider.of<UserProvider>(context, listen: false).userId;
    
    // Get weekly stats
    final now = DateTime.now();
    final sevenDaysAgo = now.subtract(const Duration(days: 7));
    
    FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .collection('daily_stats')
        .where('timestamp', isGreaterThanOrEqualTo: sevenDaysAgo.toIso8601String())
        .orderBy('timestamp')
        .snapshots()
        .listen((querySnapshot) {
      final weeklyData = {};
      
      querySnapshot.docs.forEach((doc) {
        final data = doc.data();
        final date = doc.id; // Using the document ID as the date (YYYY-MM-DD)
        
        weeklyData[date] = {
          'total_calories': data['total_calories'] ?? 0
        };
      });
      
      setState(() {
        caloriesData['weeklyData'] = weeklyData;
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

  Color _getActivityColor(String activity) {
    switch (activity) {
      case 'resting': return Colors.blue;
      case 'light': return Colors.green;
      case 'moderate': return Colors.amber;
      case 'vigorous': return Colors.red;
      default: return Colors.blue;
    }
  }

  // Prepare weekly chart data
  List<Map<String, dynamic>> _getWeeklyChartData() {
    return caloriesData['weeklyData'].entries.map<Map<String, dynamic>>((entry) {
      return {
        'timestamp': entry.key,
        'value': entry.value['total_calories']
      };
    }).toList();
  }

  // Calculate progress percentage
  double _getProgressPercentage() {
    final currentCalories = caloriesData['current']['calories_burned_today'] as int? ?? 0;
    final goal = caloriesData['goal'] as int;
    return (currentCalories / goal).clamp(0.0, 1.0);
  }

  @override
  Widget build(BuildContext context) {
    final currentActivity = caloriesData['current']['state'] as String? ?? 'resting';
    final activityColor = _getActivityColor(currentActivity);
    final progressPercentage = _getProgressPercentage();
    final weeklyChartData = _getWeeklyChartData();
    
    return Scaffold(
      appBar: AppBar(
        title: const Text('Calories Burned'),
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Calories progress
            Card(
              elevation: 4,
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(12),
              ),
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  children: [
                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        const Text(
                          'Today\'s Progress',
                          style: TextStyle(
                            fontSize: 18,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                        Text(
                          '${caloriesData['current']['calories_burned_today'] ?? 0} / ${caloriesData['goal']} kcal',
                          style: const TextStyle(
                            fontSize: 16,
                          ),
                        ),
                      ],
                    ),
                    const SizedBox(height: 16),
                    LinearProgressIndicator(
                      value: progressPercentage,
                      minHeight: 10,
                      backgroundColor: Colors.grey[300],
                      valueColor: AlwaysStoppedAnimation<Color>(Colors.green),
                    ),
                    const SizedBox(height: 16),
                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            const Text(
                              'Current Activity',
                              style: TextStyle(
                                fontSize: 14,
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                            const SizedBox(height: 4),
                            Text(
                              currentActivity.capitalize(),
                              style: TextStyle(
                                fontSize: 16,
                                color: activityColor,
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                          ],
                        ),
                        Text(
                          'Updated: ${_formatTimestamp(caloriesData['current']['timestamp'] ?? '')}',
                          style: TextStyle(
                            fontSize: 12,
                            color: Colors.grey[600],
                          ),
                        ),
                      ],
                    ),
                  ],
                ),
              ),
            ),
            
            const SizedBox(height: 24),
            
            // Today's activity chart
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
                      'Today\'s Calorie Burn',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 16),
                    SizedBox(
                      height: 200,
                      child: activityHistory.isEmpty
                          ? const Center(child: Text('No data available'))
                          : LineChartWidget(
                              data: activityHistory,
                              color: Colors.green,
                            ),
                    ),
                  ],
                ),
              ),
            ),
            
            const SizedBox(height: 24),
            
            // Weekly calories chart
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
                      'Weekly Calories',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 16),
                    SizedBox(
                      height: 200,
                      child: weeklyChartData.isEmpty
                          ? const Center(child: Text('No data available'))
                          : LineChartWidget(
                              data: weeklyChartData,
                              color: Colors.green,
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
                      'About Calorie Expenditure',
                      style: TextStyle(
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 8),
                    const Text(
                      'Calorie expenditure is the amount of energy your body uses during physical activities and basic bodily functions. '
                      'The wearable device calculates calories burned based on your heart rate, movement patterns, and personal metrics.',
                    ),
                    const SizedBox(height: 16),
                    const Text(
                      'Activity Levels:',
                      style: TextStyle(
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 8),
                    _buildActivityRow('Resting', 'Minimal movement, sitting or lying down', Colors.blue),
                    _buildActivityRow('Light', 'Walking, light household chores', Colors.green),
                    _buildActivityRow('Moderate', 'Brisk walking, cycling at moderate pace', Colors.amber),
                    _buildActivityRow('Vigorous', 'Running, high-intensity workouts', Colors.red),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
  
  Widget _buildActivityRow(String activity, String description, Color color) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4.0),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Container(
            width: 12,
            height: 12,
            margin: const EdgeInsets.only(top: 4),
            decoration: BoxDecoration(
              color: color,
              shape: BoxShape.circle,
            ),
          ),
          const SizedBox(width: 8),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  activity,
                  style: TextStyle(
                    fontWeight: FontWeight.bold,
                    color: color,
                  ),
                ),
                Text(
                  description,
                  style: const TextStyle(fontSize: 12),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

extension StringExtension on String {
  String capitalize() {
    return "${this[0].toUpperCase()}${this.substring(1)}";
  }
}
