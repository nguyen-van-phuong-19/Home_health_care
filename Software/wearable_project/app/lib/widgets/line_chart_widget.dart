import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:intl/intl.dart';

class LineChartWidget extends StatelessWidget {
  final List<Map<String, dynamic>> data;
  final Color color;

  const LineChartWidget({
    Key? key,
    required this.data,
    required this.color,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    if (data.isEmpty) {
      return const Center(child: Text('No data available'));
    }

    return LineChart(
      LineChartData(
        gridData: FlGridData(
          show: true,
          drawVerticalLine: true,
          horizontalInterval: 1,
          verticalInterval: 1,
        ),
        titlesData: FlTitlesData(
          show: true,
          rightTitles: AxisTitles(
            sideTitles: SideTitles(showTitles: false),
          ),
          topTitles: AxisTitles(
            sideTitles: SideTitles(showTitles: false),
          ),
          bottomTitles: AxisTitles(
            sideTitles: SideTitles(
              showTitles: true,
              reservedSize: 30,
              interval: _calculateInterval(),
              getTitlesWidget: (value, meta) {
                if (value.toInt() >= data.length || value < 0) {
                  return const Text('');
                }
                final timestamp = data[value.toInt()]['timestamp'];
                return Padding(
                  padding: const EdgeInsets.only(top: 8.0),
                  child: Text(
                    _formatTimestamp(timestamp),
                    style: const TextStyle(
                      fontSize: 10,
                    ),
                  ),
                );
              },
            ),
          ),
          leftTitles: AxisTitles(
            sideTitles: SideTitles(
              showTitles: true,
              interval: _calculateYInterval(),
              getTitlesWidget: (value, meta) {
                return Text(
                  value.toInt().toString(),
                  style: const TextStyle(
                    fontSize: 10,
                  ),
                );
              },
              reservedSize: 40,
            ),
          ),
        ),
        borderData: FlBorderData(
          show: true,
          border: Border.all(color: const Color(0xff37434d), width: 1),
        ),
        minX: 0,
        maxX: data.length.toDouble() - 1,
        minY: _getMinY(),
        maxY: _getMaxY(),
        lineBarsData: [
          LineChartBarData(
            spots: _createSpots(),
            isCurved: true,
            color: color,
            barWidth: 3,
            isStrokeCapRound: true,
            dotData: FlDotData(
              show: true,
              getDotPainter: (spot, percent, barData, index) {
                return FlDotCirclePainter(
                  radius: 4,
                  color: color,
                  strokeWidth: 1,
                  strokeColor: Colors.white,
                );
              },
            ),
            belowBarData: BarAreaData(
              show: true,
              color: color.withOpacity(0.2),
            ),
          ),
        ],
      ),
    );
  }

  List<FlSpot> _createSpots() {
    return List.generate(data.length, (index) {
      return FlSpot(index.toDouble(), (data[index]['value'] as num).toDouble());
    });
  }

  double _getMinY() {
    if (data.isEmpty) return 0;
    double minY = (data[0]['value'] as num).toDouble();
    for (var item in data) {
      final value = (item['value'] as num).toDouble();
      if (value < minY) minY = value;
    }
    return minY > 10 ? minY - 10 : 0;
  }

  double _getMaxY() {
    if (data.isEmpty) return 100;
    double maxY = (data[0]['value'] as num).toDouble();
    for (var item in data) {
      final value = (item['value'] as num).toDouble();
      if (value > maxY) maxY = value;
    }
    return maxY + 10;
  }

  double _calculateInterval() {
    if (data.length <= 5) return 1;
    return (data.length / 5).ceil().toDouble();
  }

  double _calculateYInterval() {
    final range = _getMaxY() - _getMinY();
    if (range <= 50) return 10;
    if (range <= 100) return 20;
    return (range / 5).ceil().toDouble();
  }

  String _formatTimestamp(String timestamp) {
    try {
      // Check if timestamp is a date string or just a time
      if (timestamp.contains(':')) {
        if (timestamp.contains('T') || timestamp.contains('-')) {
          // ISO format date
          final dateTime = DateTime.parse(timestamp);
          return DateFormat('HH:mm').format(dateTime);
        } else {
          // Already a time string
          return timestamp;
        }
      } else {
        // Just return as is for other formats (like MM/dd)
        return timestamp;
      }
    } catch (e) {
      return timestamp;
    }
  }
}
