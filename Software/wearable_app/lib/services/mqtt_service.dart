// lib/services/mqtt_service.dart

import 'dart:async';
import 'dart:convert'; // để dùng jsonEncode
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';
import 'package:typed_data/typed_data.dart'; // Import for Uint8Buffer

/// Service MQTT duy nhất cho toàn app
class MQTTService {
  // Singleton
  static final MQTTService _instance = MQTTService._internal();
  factory MQTTService() => _instance;
  MQTTService._internal();

  static const String TOPIC_HEART_RATE = 'wearable_02420/heart_rate';
  static const String TOPIC_SPO2 = 'wearable_02420/spo2';
  static const String TOPIC_ACCELEROMETER = 'wearable_02420/accelerometer';
  static const String TOPIC_GPS = 'wearable_02420/gps';

  late MqttServerClient _client;
  final StreamController<MqttReceivedMessage<MqttMessage>> _msgController =
      StreamController.broadcast();
  Timer? _reconnectTimer;

  MqttClient? get client => _client; // Public getter

  /// Cấu hình broker (tùy chỉnh trước khi init)
  String server = 'broker.emqx.io';
  int port = 1883;
  String clientId = 'flutter_client_${DateTime.now().millisecondsSinceEpoch}';
  bool autoReconnect = true;
  Duration reconnectInterval = const Duration(seconds: 5);

  /// Stream để UI hoặc controller lắng nghe message đến
  Stream<MqttReceivedMessage<MqttMessage>> get messages =>
      _msgController.stream;

  /// 1️⃣ Khởi tạo client và callback
  Future<void> init() async {
    _client =
        MqttServerClient(server, clientId)
          ..port = port
          ..keepAlivePeriod = 20
          ..logging(on: false)
          ..onConnected = _onConnected
          ..onDisconnected = _onDisconnected
          ..onSubscribed = _onSubscribed;
  }

  /// 2️⃣ Kết nối tới broker
  Future<void> connect() async {
    try {
      await _client.connect(); // tự động throw nếu lỗi
      _startMessageListener();
    } catch (e) {
      _client.disconnect();
      if (autoReconnect) _startAutoReconnect();
      rethrow;
    }
    // Nếu kết nối không thành công nhưng không throw, khởi động lại
    if (_client.connectionStatus?.state != MqttConnectionState.connected &&
        autoReconnect) {
      _startAutoReconnect();
    }
  }

  /// 3️⃣ Lắng nghe các message trả về
  void _startMessageListener() {
    _client.updates?.listen((updates) {
      for (var msg in updates) {
        _msgController.add(msg);
      }
    });
  }

  /// 4️⃣ Gửi payload bytes
  void publishBytes(
    String topic,
    List<int> payload, {
    MqttQos qos = MqttQos.atLeastOnce,
    bool retain = false,
  }) {
    final builder = MqttClientPayloadBuilder();
    builder.addBuffer(
      Uint8Buffer()..addAll(payload),
    ); // chuyển List<int> → buffer :contentReference[oaicite:0]{index=0}
    _client.publishMessage(topic, qos, builder.payload!, retain: retain);
  }

  /// 5️⃣ Gửi chuỗi text (plain string)
  void publishString(
    String topic,
    String message, {
    MqttQos qos = MqttQos.atLeastOnce,
    bool retain = false,
  }) {
    final builder =
        MqttClientPayloadBuilder()..addString(
          message,
        ); // addString có sẵn :contentReference[oaicite:1]{index=1}
    _client.publishMessage(topic, qos, builder.payload!, retain: retain);
  }

  /// 6️⃣ **Mới**: Gửi JSON object
  void publishJson(
    String topic,
    Map<String, dynamic> jsonMap, {
    MqttQos qos = MqttQos.atLeastOnce,
    bool retain = false,
  }) {
    final jsonString = jsonEncode(
      jsonMap,
    ); // stringify JSON :contentReference[oaicite:2]{index=2}
    final builder = MqttClientPayloadBuilder()..addString(jsonString);
    _client.publishMessage(topic, qos, builder.payload!, retain: retain);
  }

  /// 7️⃣ Subscribe & unsubscribe
  void subscribe(
    String topic,
    MqttQos atLeastOnce, {
    MqttQos qos = MqttQos.atLeastOnce,
  }) {
    _client.subscribe(topic, qos);
  }

  void unsubscribe(String topic) {
    _client.unsubscribe(topic);
  }

  /// 8️⃣ Ngắt kết nối thủ công
  Future<void> disconnect() async {
    autoReconnect = false;
    _cancelReconnectTimer();
    _client.disconnect();
  }

  // ─── Callbacks ─────────────────────────────────────────────

  void _onConnected() {
    _cancelReconnectTimer();
    // print('MQTT: Đã kết nối thành công');
  }

  void _onDisconnected() {
    // print('MQTT: Đã mất kết nối');
    if (autoReconnect) _startAutoReconnect();
  }

  void _onSubscribed(String topic) {
    // print('MQTT: Đã subscribe $topic');
  }

  // ─── Tái kết nối tự động ───────────────────────────────────

  void _startAutoReconnect() {
    _cancelReconnectTimer();
    _reconnectTimer = Timer.periodic(
      reconnectInterval,
      (_) => _attemptReconnect(),
    );
  }

  void _attemptReconnect() async {
    if (_client.connectionStatus?.state != MqttConnectionState.connected) {
      try {
        await _client.connect();
      } catch (_) {
        // bỏ qua, để timer thử lại
      }
    } else {
      _cancelReconnectTimer();
    }
  }

  void _cancelReconnectTimer() {
    _reconnectTimer?.cancel();
    _reconnectTimer = null;
  }
}
