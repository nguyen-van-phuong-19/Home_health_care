// lib/screens/sleep_detail_screen.dart

import 'dart:core';

import 'package:flutter/material.dart';
import 'package:table_calendar/table_calendar.dart';
import 'package:sleek_circular_slider/sleek_circular_slider.dart';
import 'package:wearable_app/services/firebase_service.dart';

class SleepDetailScreen extends StatefulWidget {
  final String userId;
  const SleepDetailScreen({super.key, required this.userId});

  @override
  _SleepDetailScreenState createState() => _SleepDetailScreenState();
}

class _SleepDetailScreenState extends State<SleepDetailScreen> {
  late Future<Map<DateTime, List<Map<String, dynamic>>>> _sessionsFuture;
  DateTime _focusedDay = DateTime.now();
  DateTime? _selectedDay;

  @override
  void initState() {
    super.initState();
    _selectedDay = DateTime.now();
    _sessionsFuture = _fetchSleepSessions();
  }

  /// 1) Load all sleep sessions và group theo “YYYY-MM-DD”
  Future<Map<DateTime, List<Map<String, dynamic>>>>
  _fetchSleepSessions() async {
    final snap = await FirebaseService().readOnce(
      'users/${widget.userId}/sleep',
    );
    final raw = snap.value;
    if (raw is! Map) return {};

    // Cast an toàn từ _Map<Object?,Object?> sang Map<String,dynamic>
    final Map<String, dynamic> rawMap = (raw).cast<String, dynamic>();

    final Map<DateTime, List<Map<String, dynamic>>> grouped = {};

    rawMap.forEach((key, val) {
      if (val is Map) {
        final m = (val).cast<String, dynamic>();
        final startStr = m['start_time'] as String; // "2025-04-19T23:40:00"
        final endStr = m['end_time'] as String;
        final duration = (m['duration_hours'] as num).toDouble();

        // Lấy chỉ “YYYY-MM-DD”
        final dateOnly = startStr.split('T').first;
        final dayKey = DateTime.parse(dateOnly); // DateTime(2025,4,19)

        // Convert thành DateTime đầy đủ cho hiển thị
        final startDt = DateTime.parse(startStr);
        final endDt = DateTime.parse(endStr);

        grouped.putIfAbsent(dayKey, () => []).add({
          'start': startDt,
          'end': endDt,
          'duration': duration,
        });
      }
    });
    return grouped;
  }

  /// 2) Calendar marker: nếu có sessions thì trả về list không rỗng
  List _eventLoader(
    DateTime day,
    Map<DateTime, List<Map<String, dynamic>>> map,
  ) {
    final key = DateTime(day.year, day.month, day.day);
    return map[key] ?? [];
  }

  /// Format “HH:mm” từ tổng phút
  String _formatHM(int totalMinutes) {
    final h = totalMinutes ~/ 60;
    final m = totalMinutes % 60;
    return '${h.toString().padLeft(2, '0')}:${m.toString().padLeft(2, '0')}';
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Chi tiết Giấc ngủ'),
        backgroundColor: Colors.deepPurple,
      ),
      body: FutureBuilder<Map<DateTime, List<Map<String, dynamic>>>>(
        future: _sessionsFuture,
        builder: (context, snap) {
          if (snap.connectionState == ConnectionState.waiting) {
            return const Center(child: CircularProgressIndicator());
          }
          if (snap.hasError) {
            return Center(child: Text('Lỗi: ${snap.error}'));
          }
          final sessionsMap = snap.data!;
          final allDates = sessionsMap.keys.toList()..sort();
          final firstDay =
              allDates.isNotEmpty
                  ? allDates.first
                  : DateTime.now().subtract(const Duration(days: 30));
          final lastDay = allDates.isNotEmpty ? allDates.last : DateTime.now();

          // Nếu selectedDay ngoài range thì điều chỉnh
          if (_selectedDay!.isBefore(firstDay) ||
              _selectedDay!.isAfter(lastDay)) {
            _selectedDay = lastDay;
          }

          final lookupKey = DateTime(
            _selectedDay!.year,
            _selectedDay!.month,
            _selectedDay!.day,
          );
          final sessions = sessionsMap[lookupKey] ?? [];

          return Padding(
            padding: const EdgeInsets.all(16),
            child: Column(
              children: [
                // --- Calendar để chọn ngày ---
                TableCalendar(
                  firstDay: firstDay,
                  lastDay: lastDay,
                  focusedDay: _focusedDay,
                  selectedDayPredicate: (d) => isSameDay(d, _selectedDay),
                  eventLoader: (d) => _eventLoader(d, sessionsMap),
                  onDaySelected: (sel, foc) {
                    // print([
                    //   'Selected: ${sel.toIso8601String()}',
                    //   'Focused: ${foc.toIso8601String()}',
                    // ]);
                    setState(() {
                      _selectedDay = sel;
                      _focusedDay = foc;
                    });
                  },
                  calendarBuilders: CalendarBuilders(
                    markerBuilder: (ctx, date, events) {
                      if (events.isNotEmpty) {
                        return Positioned(
                          bottom: 1,
                          child: Container(
                            width: 6,
                            height: 6,
                            decoration: const BoxDecoration(
                              shape: BoxShape.circle,
                              color: Colors.deepPurple,
                            ),
                          ),
                        );
                      }
                      return null;
                    },
                  ),
                ),

                const SizedBox(height: 24),

                // --- Danh sách các phiên ngủ của ngày đã chọn ---
                if (sessions.isEmpty)
                  const Center(child: Text('Chưa có dữ liệu cho ngày này'))
                else
                  Expanded(
                    child: ListView.separated(
                      itemCount: sessions.length,
                      separatorBuilder: (_, __) => const SizedBox(height: 24),
                      itemBuilder: (context, index) {
                        final sess = sessions[index];
                        final start = sess['start'] as DateTime;
                        final end = sess['end'] as DateTime;
                        final duration = sess['duration'] as double;
                        final totalMin = (duration * 60).toInt();
                        final startMin = start.hour * 60 + start.minute;
                        final endMin = end.hour * 60 + end.minute;

                        return Column(
                          crossAxisAlignment: CrossAxisAlignment.center,
                          children: [
                            Text(
                              'Phiên ngủ ${index + 1}',
                              style: const TextStyle(
                                fontSize: 18,
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                            const SizedBox(height: 8),
                            SleekCircularSlider(
                              min: 0,
                              max: 24 * 60, // 1440 phút
                              initialValue: totalMin.toDouble(),
                              appearance: CircularSliderAppearance(
                                infoProperties: InfoProperties(
                                  mainLabelStyle: const TextStyle(
                                    fontSize: 20,
                                    fontWeight: FontWeight.bold,
                                  ),
                                  modifier: (val) {
                                    final h = val ~/ 60;
                                    final m = (val % 60).toInt();
                                    return '$h giờ ${m.toString().padLeft(2, '0')} phút';
                                  },
                                ),
                                customColors: CustomSliderColors(
                                  progressBarColor: Colors.deepPurple,
                                  trackColor: Colors.deepPurple.withAlpha(50),
                                  dotColor: Colors.deepPurple,
                                ),
                              ),
                            ),
                            const SizedBox(height: 8),
                            Row(
                              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                              children: [
                                Column(
                                  children: [
                                    const Text('Bắt đầu'),
                                    Text(_formatHM(startMin)),
                                  ],
                                ),
                                Column(
                                  children: [
                                    const Text('Kết thúc'),
                                    Text(_formatHM(endMin)),
                                  ],
                                ),
                              ],
                            ),
                          ],
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
