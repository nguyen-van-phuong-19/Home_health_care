// ble_service.dart
// Service to scan ESP32-S3 BLE advertisements (manufacturerId 0xABCD)
// and continuously advertise phone BLE data (manufacturerId 0xABEF)

import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_ble_peripheral/flutter_ble_peripheral.dart';

/// Callback signature for receiving ESP32-S3 manufacturer data
typedef OnEsp32Advertisement = void Function(List<int> data);

class BleService {
  BleService._();
  static final BleService instance = BleService._();

  final FlutterBlePeripheral _peripheral = FlutterBlePeripheral();

  StreamSubscription<List<ScanResult>>? _scanSubscription;

  /// Called when an ESP32-S3 BLE advertisement is received
  OnEsp32Advertisement? onEsp32Advertisement;

  /// Starts BLE advertising and scanning
  void start(Object? initialPayload) {
    _startAdvertising(initialPayload);
    _startScanning();
  }

  /// Stops BLE advertising and scanning
  void stop() {
    _stopAdvertising();
    _stopScanning();
  }

  void _startAdvertising([Object? payload]) {
    final List<int> bytes = _buildPayloadBytes(payload);
    final AdvertiseData data = AdvertiseData(
      includeDeviceName: true,
      manufacturerId: 0xABEF,
      manufacturerData: Uint8List.fromList(
        bytes,
      ), // initial payload, update if needed
    );
    _peripheral.start(
      advertiseData: data,
      advertiseSettings: AdvertiseSettings(
        advertiseMode: AdvertiseMode.advertiseModeLowLatency,
        txPowerLevel: AdvertiseTxPower.advertiseTxPowerHigh,
        timeout: 18000000,
      ),
    );
  }

  /// Convert payload to byte list for advertising
  List<int> _buildPayloadBytes(Object? payload) {
    if (payload == null) {
      return <int>[];
    } else if (payload is String) {
      return utf8.encode(payload);
    } else if (payload is double) {
      final bd = ByteData(4)..setFloat32(0, payload, Endian.little);
      return bd.buffer.asUint8List();
    } else if (payload is int) {
      final bd = ByteData(4)..setInt32(0, payload, Endian.little);
      return bd.buffer.asUint8List();
    } else if (payload is List<int>) {
      return payload;
    } else {
      throw ArgumentError('Unsupported payload type: ${payload.runtimeType}');
    }
  }

  /// Update the advertised manufacturer data payload
  void updateAdvertiseData(Object payload) {
    _peripheral.stop();
    _startAdvertising(payload);
  }

  void _stopAdvertising() {
    _peripheral.stop();
  }

  void _startScanning() {
    // 0 duration = infinite scan until stopped
    FlutterBluePlus.startScan(timeout: Duration(days: 365));
    _scanSubscription = FlutterBluePlus.scanResults.listen((results) {
      for (final ScanResult r in results) {
        final mfg = r.advertisementData.manufacturerData;
        // print(mfg);
        if (mfg.containsKey(0xABCD)) {
          final data = mfg[0xABCD]!;
          onEsp32Advertisement?.call(data);
        }
      }
    });
  }

  void _stopScanning() {
    _scanSubscription?.cancel();
    FlutterBluePlus.stopScan();
  }
}
