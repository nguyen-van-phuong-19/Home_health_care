// lib/screens/history_screen.dart

import 'package:flutter/material.dart';
import 'package:fl_chart/fl_chart.dart';
import 'package:table_calendar/table_calendar.dart';
import 'package:wearable_app/services/firebase_service.dart';

/// ------------------ HEART RATE HISTORY ------------------
class HeartRateHistoryScreen extends StatelessWidget {
  final String userId;
  const HeartRateHistoryScreen({super.key, required this.userId});

  /// Trả về list map: [{ 'timestamp': String, 'bpm': double }, ...]
  Future<List<Map<String, dynamic>>> _fetchHeartRateData() async {
    final snapshot = await FirebaseService().readOnce(
      'users/$userId/heart_rate',
    );
    final raw = snapshot.value;
    final Map<String, dynamic> hrMap =
        raw is Map ? (raw).cast<String, dynamic>() : <String, dynamic>{};

    final keys = hrMap.keys.toList()..sort();
    return keys.map((k) {
      final entry = hrMap[k];
      final bpm = entry is Map ? (entry['bpm'] as num).toDouble() : 0.0;
      return {'timestamp': k, 'bpm': bpm};
    }).toList();
  }

  @override
  Widget build(BuildContext context) {
    final chartHeight = MediaQuery.of(context).size.height * 0.4;
    return Scaffold(
      appBar: AppBar(
        title: const Text('Lịch sử Nhịp tim'),
        backgroundColor: Colors.redAccent,
      ),
      body: FutureBuilder<List<Map<String, dynamic>>>(
        future: _fetchHeartRateData(),
        builder: (context, snap) {
          if (snap.connectionState == ConnectionState.waiting) {
            return const Center(child: CircularProgressIndicator());
          }
          if (snap.hasError) {
            return Center(child: Text('Lỗi: ${snap.error}'));
          }
          final data = snap.data!;
          if (data.isEmpty) {
            return const Center(child: Text('Không có dữ liệu Nhịp tim'));
          }
          // Tạo spots cho chart
          final spots =
              data.asMap().entries.map((e) {
                return FlSpot(e.key.toDouble(), e.value['bpm'] as double);
              }).toList();

          return Padding(
            padding: const EdgeInsets.all(16),
            child: Column(
              children: [
                SizedBox(
                  height: chartHeight,
                  child: LineChart(
                    LineChartData(
                      lineBarsData: [
                        LineChartBarData(
                          spots: spots,
                          isCurved: false,
                          dotData: FlDotData(show: false),
                        ),
                      ],
                      titlesData: FlTitlesData(
                        bottomTitles: AxisTitles(
                          sideTitles: SideTitles(showTitles: false),
                        ),
                        leftTitles: AxisTitles(
                          sideTitles: SideTitles(
                            showTitles: true,
                            interval: 10,
                          ),
                        ),
                      ),
                    ),
                  ),
                ),
                const SizedBox(height: 16),
                const Text(
                  'Chi tiết lịch sử',
                  style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
                ),
                const SizedBox(height: 8),
                Expanded(
                  child: ListView.builder(
                    itemCount: data.length,
                    itemBuilder: (context, index) {
                      final item = data[index];
                      final ts = item['timestamp'] as String;
                      final bpm = item['bpm'] as double;
                      return ListTile(
                        leading: const Icon(
                          Icons.favorite,
                          color: Colors.redAccent,
                        ),
                        title: Text('${bpm.toStringAsFixed(0)} bpm'),
                        subtitle: Text(ts),
                      );
                    },
                  ),
                ),
              ],
            ),
          );
        },
      ),
    );
  }
}

/// ------------------ SpO₂ HISTORY ------------------
class Spo2HistoryScreen extends StatelessWidget {
  final String userId;
  const Spo2HistoryScreen({super.key, required this.userId});

  Future<List<Map<String, dynamic>>> _fetchSpO2Data() async {
    final snapshot = await FirebaseService().readOnce('users/$userId/spo2');
    final raw = snapshot.value;
    final Map<String, dynamic> spo2Map =
        raw is Map ? (raw).cast<String, dynamic>() : <String, dynamic>{};

    final keys = spo2Map.keys.toList()..sort();
    return keys.map((k) {
      final entry = spo2Map[k];
      final pct = entry is Map ? (entry['percentage'] as num).toDouble() : 0.0;
      return {'timestamp': k, 'percentage': pct};
    }).toList();
  }

  @override
  Widget build(BuildContext context) {
    final chartHeight = MediaQuery.of(context).size.height * 0.4;
    return Scaffold(
      appBar: AppBar(
        title: const Text('Lịch sử SpO₂'),
        backgroundColor: Colors.blueAccent,
      ),
      body: FutureBuilder<List<Map<String, dynamic>>>(
        future: _fetchSpO2Data(),
        builder: (context, snap) {
          if (snap.connectionState == ConnectionState.waiting) {
            return const Center(child: CircularProgressIndicator());
          }
          if (snap.hasError) {
            return Center(child: Text('Lỗi: ${snap.error}'));
          }
          final data = snap.data!;
          if (data.isEmpty) {
            return const Center(child: Text('Không có dữ liệu SpO₂'));
          }
          final spots =
              data.asMap().entries.map((e) {
                return FlSpot(
                  e.key.toDouble(),
                  e.value['percentage'] as double,
                );
              }).toList();

          return Padding(
            padding: const EdgeInsets.all(16),
            child: Column(
              children: [
                SizedBox(
                  height: chartHeight,
                  child: LineChart(
                    LineChartData(
                      lineBarsData: [
                        LineChartBarData(
                          spots: spots,
                          isCurved: false,
                          dotData: FlDotData(show: false),
                        ),
                      ],
                      titlesData: FlTitlesData(
                        bottomTitles: AxisTitles(
                          sideTitles: SideTitles(showTitles: false),
                        ),
                        leftTitles: AxisTitles(
                          sideTitles: SideTitles(showTitles: true, interval: 1),
                        ),
                      ),
                    ),
                  ),
                ),
                const SizedBox(height: 16),
                const Text(
                  'Chi tiết lịch sử',
                  style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
                ),
                const SizedBox(height: 8),
                Expanded(
                  child: ListView.builder(
                    itemCount: data.length,
                    itemBuilder: (context, index) {
                      final item = data[index];
                      final ts = item['timestamp'] as String;
                      final pct = item['percentage'] as double;
                      return ListTile(
                        leading: const Icon(
                          Icons.health_and_safety,
                          color: Colors.blueAccent,
                        ),
                        title: Text('${pct.toStringAsFixed(1)} %'),
                        subtitle: Text(ts),
                      );
                    },
                  ),
                ),
              ],
            ),
          );
        },
      ),
    );
  }
}

/// ------------------ CALORIES HISTORY ------------------
class CaloriesHistoryScreen extends StatefulWidget {
  final String userId;
  const CaloriesHistoryScreen({super.key, required this.userId});

  @override
  State<CaloriesHistoryScreen> createState() => _CaloriesHistoryScreenState();
}

class _CaloriesHistoryScreenState extends State<CaloriesHistoryScreen> {
  late Future<Map<DateTime, double>> _futureEvents;
  DateTime _focusedDay = DateTime.now();
  DateTime? _selectedDay;
  Map<DateTime, double> _events = {};
  Map<DateTime, double> _eventsHr = {};
  Map<DateTime, double> _eventsAcc = {};

  @override
  void initState() {
    super.initState();
    _selectedDay = _focusedDay;
    _futureEvents = _fetchCaloriesEvents();
  }

  Future<Map<DateTime, double>> _fetchCaloriesEvents() async {
    final snapshot = await FirebaseService().readOnce(
      'users/${widget.userId}/calories_daily',
    );
    final raw = snapshot.value;
    final Map<String, dynamic> calMap =
        raw is Map ? (raw).cast<String, dynamic>() : <String, dynamic>{};

    final Map<DateTime, double> result = {};
    final Map<DateTime, double> dataCalHr = {};
    final Map<DateTime, double> dataCalAcc = {};
    calMap.forEach((dateStr, value) {
      if (value is Map && value['combined_daily_calories'] != null) {
        final dt = DateTime.parse(dateStr);
        final cal = (value['combined_daily_calories'] as num).toDouble();
        final calHr = (value['heart_rate_calories'] as num).toDouble();
        final calAcc = (value['accelerometer_calories'] as num).toDouble();
        result[DateTime(dt.year, dt.month, dt.day)] = cal;
        dataCalHr[DateTime(dt.year, dt.month, dt.day)] = calHr;
        dataCalAcc[DateTime(dt.year, dt.month, dt.day)] = calAcc;
      }
    });
    _events = result;
    _eventsHr = dataCalHr;
    _eventsAcc = dataCalAcc;
    return result;
  }

  List<double> _eventsForDay(DateTime day) {
    final key = DateTime(day.year, day.month, day.day);
    final cal = _events[key];
    final calHr = _eventsHr[key];
    final calAcc = _eventsAcc[key];
    if (cal != null && calHr != null && calAcc != null) {
      return [cal, calHr, calAcc];
    }
    return cal != null && cal > 0 ? [cal] : [];
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Lịch sử Calo hàng tháng'),
        backgroundColor: Colors.orangeAccent,
      ),
      body: FutureBuilder<Map<DateTime, double>>(
        future: _futureEvents,
        builder: (context, snap) {
          if (snap.connectionState == ConnectionState.waiting) {
            return const Center(child: CircularProgressIndicator());
          }
          if (snap.hasError) {
            return Center(child: Text('Lỗi: ${snap.error}'));
          }
          return Column(
            children: [
              TableCalendar<double>(
                firstDay: DateTime(
                  DateTime.now().year,
                  DateTime.now().month,
                  1,
                ),
                lastDay: DateTime(
                  DateTime.now().year,
                  DateTime.now().month + 1,
                  0,
                ),
                focusedDay: _focusedDay,
                selectedDayPredicate: (d) => isSameDay(d, _selectedDay),
                eventLoader: _eventsForDay,
                onDaySelected: (sel, foc) {
                  setState(() {
                    _selectedDay = sel;
                    _focusedDay = foc;
                  });
                },
                calendarBuilders: CalendarBuilders(
                  markerBuilder: (context, date, events) {
                    if (events.isNotEmpty) {
                      return Positioned(
                        bottom: 4,
                        child: Container(
                          width: 6,
                          height: 6,
                          decoration: const BoxDecoration(
                            shape: BoxShape.circle,
                            color: Colors.red,
                          ),
                        ),
                      );
                    }
                    return null;
                  },
                ),
              ),
              const SizedBox(height: 8),
              Padding(
                padding: const EdgeInsets.symmetric(horizontal: 16),
                child: Column(
                  children: [
                    Text(
                      _eventsForDay(_selectedDay!).isNotEmpty
                          ? 'Calo ngày ${_selectedDay!.day}: ${_eventsForDay(_selectedDay!).first.toStringAsFixed(1)} kcal'
                          : 'Không có dữ liệu',
                      style: const TextStyle(
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    Text(
                      _eventsForDay(_selectedDay!).isNotEmpty
                          ? 'Calo theo nhịp tim ngày ${_selectedDay!.day}: ${_eventsForDay(_selectedDay!).elementAt(1).toStringAsFixed(1)} kcal'
                          : 'Không có dữ liệu',
                      style: const TextStyle(
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    Text(
                      _eventsForDay(_selectedDay!).isNotEmpty
                          ? 'Calo theo hoạt động ngày ${_selectedDay!.day}: ${_eventsForDay(_selectedDay!).last.toStringAsFixed(1)} kcal'
                          : 'Không có dữ liệu',
                      style: const TextStyle(
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                  ],
                ),
              ),
            ],
          );
        },
      ),
    );
  }
}
