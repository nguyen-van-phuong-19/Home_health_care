import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;

const String serverUrl = 'http://localhost:5000';

class QAScreen extends StatefulWidget {
  const QAScreen({super.key});

  @override
  State<QAScreen> createState() => _QAScreenState();
}

class _QAScreenState extends State<QAScreen> {
  final TextEditingController _controller = TextEditingController();
  String _answer = '';
  bool _loading = false;

  Future<void> _ask() async {
    final question = _controller.text.trim();
    if (question.isEmpty) return;
    setState(() {
      _loading = true;
      _answer = '';
    });
    try {
      final resp = await http.post(
        Uri.parse('$serverUrl/qa'),
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({'question': question}),
      );
      if (resp.statusCode == 200) {
        final data = jsonDecode(resp.body);
        setState(() => _answer = data['answer'] ?? '');
      } else {
        setState(() => _answer = 'Error: ${resp.statusCode}');
      }
    } catch (e) {
      setState(() => _answer = 'Error: $e');
    }
    setState(() => _loading = false);
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Hỏi đáp')),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          children: [
            TextField(
              controller: _controller,
              decoration: const InputDecoration(labelText: 'Nhập câu hỏi'),
            ),
            const SizedBox(height: 16),
            ElevatedButton(
              onPressed: _loading ? null : _ask,
              child: const Text('Gửi'),
            ),
            const SizedBox(height: 16),
            if (_loading) const CircularProgressIndicator(),
            if (_answer.isNotEmpty)
              Expanded(
                child: SingleChildScrollView(child: Text(_answer)),
              ),
          ],
        ),
      ),
    );
  }
}
