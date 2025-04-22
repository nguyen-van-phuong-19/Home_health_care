// lib/services/ble_service.dart

import 'dart:async';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

class BLEService {
  // Singleton
  static final BLEService _instance = BLEService._internal();
  factory BLEService() => _instance;
  BLEService._internal();

  /// Thiết bị đang kết nối
  BluetoothDevice? _device;

  /// Map lưu characteristic để dễ truy cập
  final Map<Guid, BluetoothCharacteristic> _chars = {};

  /// StreamController để phát sự kiện giá trị mới
  final StreamController<List<int>> _valueController =
      StreamController.broadcast();

  Stream<List<int>> get onDataReceived => _valueController.stream;

  /// 1️⃣ Quét thiết bị ESP32-S3 (tùy chỉnh filter theo tên/UUID)
  Future<List<ScanResult>> scan({
    Duration timeout = const Duration(seconds: 5),
  }) async {
    final results = <ScanResult>[];
    FlutterBluePlus.startScan(timeout: timeout);
    await for (final scanResults in FlutterBluePlus.scanResults) {
      for (final res in scanResults) {
        // Lọc theo tên hoặc service UUID
        if (res.device.name.contains('ESP32-S3') ||
            res.advertisementData.serviceUuids.contains('YOUR_SERVICE_UUID')) {
          results.add(res);
        }
      }
    }
    await FlutterBluePlus.stopScan();
    return results;
  }

  /// 2️⃣ Kết nối đến thiết bị và discover dịch vụ/characteristics
  Future<void> connect(BluetoothDevice device) async {
    _device = device;
    await device.connect(); // tự reconnect=false
    final services = await device.discoverServices();
    for (var svc in services) {
      for (var chr in svc.characteristics) {
        _chars[chr.uuid] = chr;
      }
    }
  }

  /// 3️⃣ Đọc dữ liệu từ characteristic
  Future<List<int>> read(Guid charUuid) async {
    final chr = _chars[charUuid]!;
    final data =
        await chr
            .read(); // trả về List<int> :contentReference[oaicite:6]{index=6}
    return data;
  }

  /// 4️⃣ Ghi bytes vào characteristic
  Future<void> writeBytes(
    Guid charUuid,
    List<int> data, {
    bool withResponse = true,
  }) async {
    final chr = _chars[charUuid]!;
    await chr.write(data, withoutResponse: !withResponse);
  }

  /// 5️⃣ Ghi string (JSON hoặc text) vào characteristic
  Future<void> writeString(
    Guid charUuid,
    String text, {
    bool withResponse = true,
  }) async {
    final chr = _chars[charUuid]!;
    final bytes = text.codeUnits;
    await chr.write(bytes, withoutResponse: !withResponse);
  }

  /// 6️⃣ Đăng ký nhận notify từ characteristic
  Future<void> subscribe(Guid charUuid) async {
    final chr = _chars[charUuid]!;
    await chr.setNotifyValue(true);
    chr.value.listen((data) {
      _valueController.add(
        data,
      ); // phát ra stream :contentReference[oaicite:7]{index=7}
    });
  }

  /// 7️⃣ Hủy đăng ký notify
  Future<void> unsubscribe(Guid charUuid) async {
    final chr = _chars[charUuid]!;
    await chr.setNotifyValue(false);
  }

  /// 8️⃣ Ngắt kết nối
  Future<void> disconnect() async {
    await _device?.disconnect();
    _chars.clear();
    _device = null;
  }
}
