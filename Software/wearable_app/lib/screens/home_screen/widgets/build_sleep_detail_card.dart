import 'package:flutter/material.dart';

Widget buildSleepDetailCard(BuildContext context) {
  double sleepHours = 8.2; // 8 giờ 12 phút
  return Container(
    padding: const EdgeInsets.all(16),
    decoration: BoxDecoration(
      color: Colors.white,
      borderRadius: BorderRadius.circular(16),
      boxShadow: [
        BoxShadow(
          color: Color.fromRGBO(158, 158, 158, 0.1),
          blurRadius: 6,
          offset: const Offset(0, 3),
        ),
      ],
    ),
    child: Column(
      children: [
        const Text(
          "Sleep tracking",
          style: TextStyle(fontWeight: FontWeight.bold, fontSize: 18),
        ),
        const SizedBox(height: 16),
        // Hiển thị đồng hồ và thời gian đi ngủ/thức dậy
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceAround,
          children: [
            Column(
              children: const [
                Text(
                  "Go to bed",
                  style: TextStyle(fontWeight: FontWeight.w500),
                ),
                SizedBox(height: 6),
                Text("23:40", style: TextStyle(fontSize: 16)),
              ],
            ),
            // Đồng hồ hiển thị
            SizedBox(
              width: 100,
              height: 100,
              child: CircularProgressIndicator(
                value: sleepHours / 10, // Giả định 10h = 100%
                strokeWidth: 10,
                backgroundColor: Colors.grey.shade200,
                valueColor: const AlwaysStoppedAnimation<Color>(Colors.green),
              ),
            ),
            Column(
              children: const [
                Text("Wake up", style: TextStyle(fontWeight: FontWeight.w500)),
                SizedBox(height: 6),
                Text("08:00", style: TextStyle(fontSize: 16)),
              ],
            ),
          ],
        ),
      ],
    ),
  );
}
