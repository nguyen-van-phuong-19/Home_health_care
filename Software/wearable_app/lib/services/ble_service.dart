import 'dart:async';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

/// A singleton service to manage BLE operations using flutter_blue_plus.
class BleService {
  BleService._privateConstructor();
  static final BleService _instance = BleService._privateConstructor();
  factory BleService() => _instance;

  BluetoothDevice? _connectedDevice;
  StreamSubscription<BluetoothConnectionState>? _deviceStateSubscription;

  /// Starts scanning for BLE devices with optional filters.
  Future<void> startScan({
    List<Guid> withServices = const [],
    Duration timeout = const Duration(seconds: 5),
    bool continuousUpdates = false,
    Duration? removeIfGone,
    int continuousDivisor = 1,
    bool oneByOne = false,
    bool androidLegacy = false,
    AndroidScanMode androidScanMode = AndroidScanMode.lowLatency,
  }) async {
    await FlutterBluePlus.startScan(
      withServices: withServices,
      timeout: timeout,
      continuousUpdates: continuousUpdates,
      removeIfGone: removeIfGone,
      continuousDivisor: continuousDivisor,
      oneByOne: oneByOne,
      androidLegacy: androidLegacy,
      androidScanMode: androidScanMode,
    );
  }

  /// Stream of list of ScanResult from ongoing scan.
  Stream<List<ScanResult>> get scanResults => FlutterBluePlus.scanResults;

  /// Stops scanning for BLE devices.
  Future<void> stopScan() async {
    await FlutterBluePlus.stopScan();
  }

  /// Connects to the given [device] with an optional [timeout].
  Future<void> connectToDevice(
    BluetoothDevice device, {
    Duration timeout = const Duration(seconds: 10),
  }) async {
    print(device);
    _connectedDevice = device;
    // Listen to connection state changes
    _deviceStateSubscription = device.state.listen((
      BluetoothConnectionState state,
    ) {
      print('Connection State: $state');
    });
    await device.connect(timeout: timeout, autoConnect: false);
  }

  /// Disconnects from the currently connected device.
  Future<void> disconnectDevice() async {
    if (_connectedDevice != null) {
      await _deviceStateSubscription?.cancel();
      await _connectedDevice!.disconnect();
      _connectedDevice = null;
    }
  }

  /// Discovers services and characteristics of the connected device.
  Future<List<BluetoothService>> discoverServices() async {
    final services = _connectedDevice!.discoverServices();
    if (_connectedDevice == null) {
      throw Exception('No device connected');
    }
    return services;
  }

  /// Reads the value of the given [characteristic].
  Future<List<int>> readCharacteristic(
    BluetoothCharacteristic characteristic,
  ) async {
    return characteristic.read();
  }

  /// Writes [value] to the given [characteristic].
  /// If [withoutResponse] is true, uses write without response.
  Future<void> writeCharacteristic(
    BluetoothCharacteristic characteristic,
    List<int> value, {
    bool withoutResponse = false,
  }) async {
    await characteristic.write(value, withoutResponse: withoutResponse);
  }

  /// Sets up notifications for the given [characteristic].
  /// Returns a stream of byte values.
  Future<Stream<List<int>>> setNotification(
    BluetoothCharacteristic characteristic, {
    bool enable = true,
  }) async {
    await characteristic.setNotifyValue(enable);
    return characteristic.value;
  }

  /// Listens to notifications of a characteristic specified by service UUID and characteristic UUID.
  Future<Stream<List<int>>> listenCharacteristic(
    Guid serviceUuid,
    Guid charUuid,
  ) async {
    print("jhsdbvsvj");
    final services = await discoverServices();
    print('Discovered ${services.length} services');

    // In ra tất cả UUID của services và characteristics để bạn lựa chọn
    for (var s in services) {
      print('Service UUID: ${s.uuid}');
      for (var c in s.characteristics) {
        print('  Characteristic UUID: ${c.uuid}');
      }
    }
    final service = services.firstWhere((s) => s.uuid == serviceUuid);
    final characteristic = service.characteristics.firstWhere(
      (c) => c.uuid == charUuid,
    );
    await characteristic.setNotifyValue(true);
    return characteristic.value;
  }

  /// Reads the RSSI (signal strength) of the connected device.
  Future<int> readRssi() async {
    if (_connectedDevice == null) {
      throw Exception('No device connected');
    }
    return _connectedDevice!.readRssi();
  }

  /// Checks if BLE is available on the device.
  Future<bool> isAvailable() async => FlutterBluePlus.isAvailable;

  /// Checks if BLE is currently powered on.
  Future<bool> isOn() async => FlutterBluePlus.isOn;

  /// Stream of connection state changes for the connected device.
  Stream<BluetoothConnectionState>? get deviceState => _connectedDevice?.state;

  /// Stream of overall Bluetooth adapter state.
  Stream<BluetoothAdapterState> get state => FlutterBluePlus.adapterState;

  /// Scans and automatically connects to a device matching [deviceId] (MAC string).
  Future<BluetoothDevice> scanAndConnectById(
    String deviceId, {
    Duration timeout = const Duration(seconds: 10),
  }) async {
    BluetoothDevice? found;
    await startScan(timeout: timeout);
    late final StreamSubscription<List<ScanResult>> sub;
    sub = scanResults.listen((results) async {
      for (var r in results) {
        if (r.device.id.id.toUpperCase() == deviceId.toUpperCase()) {
          found = r.device;
          await stopScan();
          await connectToDevice(r.device);
          await sub.cancel();
          break;
        }
      }
    });
    final start = DateTime.now();
    while (found == null && DateTime.now().difference(start) < timeout) {
      await Future.delayed(Duration(milliseconds: 200));
    }
    if (found == null) throw Exception('Device $deviceId not found');
    return found!;
  }

  /// Dispose resources when no longer needed.
  Future<void> dispose() async {
    await stopScan();
    await _deviceStateSubscription?.cancel();
    if (_connectedDevice != null) await _connectedDevice!.disconnect();
  }
}
