import 'dart:async';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

/// A service to manage Bluetooth LE operations using flutter_blue_plus.
class BleService {
  // Singleton instance
  BleService._();
  static final BleService _instance = BleService._();
  factory BleService() => _instance;

  BluetoothDevice? _connectedDevice;
  StreamSubscription<BluetoothConnectionState>? _connectionSubscription;
  List<BluetoothService>? _services; // Cache of discovered services

  /// Starts scanning for BLE devices without any OS-level UUID filters.
  /// This will scan for all BLE devices, and the [scanResults] stream will
  /// be updated with devices whose advertisement data includes at least one 128-bit service UUID.
  /// If [timeout] is provided, scanning stops after the given duration; otherwise, it continues until [stopScan] is called.
  /// Throws an [Exception] if scanning fails to start (e.g., if Bluetooth is off or permissions are missing).
  Future<void> startScan({Duration? timeout}) async {
    try {
      await FlutterBluePlus.startScan(timeout: timeout);
    } on Exception catch (e) {
      // Handle any exceptions that occur when starting the scan
      throw Exception('Failed to start BLE scan: $e');
    }
  }

  /// Stream of discovered BLE devices as a list of [ScanResult].
  /// Subscribe to this to receive continuous updates of scan results.
  /// Only devices that advertise at least one 128-bit UUID in their advertisement data are included.
  Stream<List<ScanResult>> get scanResults {
    return FlutterBluePlus.scanResults.map((results) {
      // Filter for devices advertising at least one 128-bit service UUID
      return results.where((r) {
        return r.advertisementData.serviceUuids.any(
          (uuid) => uuid.toString().contains('-'),
        );
      }).toList();
    });
  }

  // Define the connectionState getter
  Stream<BluetoothConnectionState> get connectionState {
    // Replace with actual implementation to return the connection state stream
    return FlutterBluePlus.state.map((state) {
      switch (state) {
        case BluetoothAdapterState.on:
          return BluetoothConnectionState.connected;
        case BluetoothAdapterState.off:
          return BluetoothConnectionState.disconnected;
        default:
          return BluetoothConnectionState.disconnected;
      }
    });
  }

  /// Stops scanning for BLE devices.
  Future<void> stopScan() async {
    await FlutterBluePlus.stopScan();
  }

  /// Connects to the given [BluetoothDevice].
  /// This will attempt to connect up to 3 times if the initial attempt fails, waiting 10 seconds between attempts.
  /// If the connection is successful, the device is set as the current connected device.
  /// If all attempts fail, an [Exception] is thrown.
  Future<void> connectToDevice(
    BluetoothDevice device, {
    Duration timeout = const Duration(seconds: 10),
  }) async {
    // Set the current device and listen for its connection state changes
    // Cancel any existing subscription to avoid multiple listeners
    await _connectionSubscription?.cancel();
    _connectionSubscription = device.connectionState.listen((state) {
      if (state == BluetoothConnectionState.disconnected) {
        // Handle unexpected disconnection
        print(
          'Device disconnected (${device.disconnectReason?.code}: ${device.disconnectReason?.description})',
        );
        _services = null;
        _connectedDevice = null;
        _connectionSubscription?.cancel();
        _connectionSubscription = null;
      }
    });

    Exception? lastError;
    for (int attempt = 1; attempt <= 3; attempt++) {
      try {
        await device.connect(timeout: timeout, autoConnect: false);
        _connectedDevice = device;
        // Connected successfully
        return;
      } catch (e) {
        lastError = Exception('Connection attempt $attempt failed: $e');
        if (attempt < 3) {
          // Wait 10 seconds before retrying
          await Future.delayed(const Duration(seconds: 10));
        }
      }
    }
    // If we reach here, all connection attempts failed
    await _connectionSubscription?.cancel();
    _connectionSubscription = null;
    _connectedDevice = null;
    throw Exception(
      'Failed to connect after 3 attempts. Last error: $lastError',
    );
  }

  /// Disconnects from the currently connected device, if any.
  /// Cancels active connection state listeners and clears the cached device and services.
  Future<void> disconnectDevice() async {
    if (_connectedDevice != null) {
      await _connectionSubscription?.cancel();
      _connectionSubscription = null;
      await _connectedDevice!.disconnect();
      _connectedDevice = null;
      _services = null;
    }
  }

  /// Discovers all services and characteristics of the connected device.
  /// Returns a list of [BluetoothService] objects (each containing its characteristics and descriptors).
  /// Throws an [Exception] if no device is connected or if service discovery fails.
  Future<List<BluetoothService>> discoverServices() async {
    if (_connectedDevice == null) {
      throw Exception('No device connected');
    }
    try {
      _services = await _connectedDevice!.discoverServices();
      print(_services);
      return _services!;
    } catch (e) {
      throw Exception('Failed to discover services: $e');
    }
  }

  /// Reads the RSSI (signal strength) of the currently connected device.
  /// Returns the RSSI value in dBm.
  /// Throws an [Exception] if no device is connected or if the RSSI read fails.
  Future<int> readRssi() async {
    if (_connectedDevice == null) {
      throw Exception('No device connected');
    }
    try {
      return await _connectedDevice!.readRssi();
    } catch (e) {
      throw Exception('Failed to read RSSI: $e');
    }
  }

  /// Subscribes to notifications from a characteristic specified by [serviceUuid] and [charUuid].
  /// Ensures the service is discovered (if not already), enables notifications on the characteristic,
  /// and returns a stream of values received from the device for that characteristic.
  /// The returned [Stream] emits a list of bytes each time the characteristic value changes.
  /// Throws an [Exception] if no device is connected, if the service/characteristic is not found, or if enabling notifications fails.
  Future<Stream<List<int>>> subscribeToCharacteristic(
    Guid serviceUuid,
    Guid charUuid,
  ) async {
    if (_connectedDevice == null) {
      throw Exception('No device connected');
    }
    // Ensure services are discovered (perform discovery if not done yet)
    _services ??= await _connectedDevice!.discoverServices();
    // Find the specified service and characteristic
    final service = _services!.firstWhere(
      (s) => s.uuid == serviceUuid,
      orElse: () => throw Exception('Service $serviceUuid not found on device'),
    );
    final characteristic = service.characteristics.firstWhere(
      (c) => c.uuid == charUuid,
      orElse:
          () => throw Exception('Characteristic $charUuid not found on device'),
    );
    // Enable notifications for the characteristic
    try {
      await characteristic.setNotifyValue(true);
    } catch (e) {
      throw Exception('Failed to enable notifications on $charUuid: $e');
    }
    // Return the stream of characteristic value updates
    return characteristic.value;
  }

  /// Reads the value of the given [BluetoothCharacteristic].
  /// Returns the value as a list of bytes.
  /// Throws an [Exception] if no device is connected or if the read fails.
  Future<List<int>> readCharacteristic(
    BluetoothCharacteristic characteristic,
  ) async {
    if (_connectedDevice == null) {
      throw Exception('No device connected');
    }
    try {
      return await characteristic.read();
    } catch (e) {
      throw Exception(
        'Failed to read characteristic ${characteristic.uuid}: $e',
      );
    }
  }

  /// Writes [value] to the given [BluetoothCharacteristic].
  /// If [withoutResponse] is true, the write is performed without waiting for an acknowledgment.
  /// Throws an [Exception] if no device is connected or if the write fails.
  Future<void> writeCharacteristic(
    BluetoothCharacteristic characteristic,
    List<int> value, {
    bool withoutResponse = false,
  }) async {
    if (_connectedDevice == null) {
      throw Exception('No device connected');
    }
    try {
      await characteristic.write(value, withoutResponse: withoutResponse);
    } catch (e) {
      throw Exception(
        'Failed to write characteristic ${characteristic.uuid}: $e',
      );
    }
  }

  /// Stream of connection state changes for the currently connected device.
  /// Listen to this to get real-time updates of the device's connection status
  /// (e.g., [BluetoothConnectionState.connected], [BluetoothConnectionState.disconnected]).
  Stream<BluetoothConnectionState>? get deviceConnectionState {
    return _connectedDevice?.connectionState;
  }

  /// Stream of the Bluetooth adapter state (e.g., on, off, turningOn, turningOff).
  /// This can be used to monitor whether the device's Bluetooth is enabled or not.
  Stream<BluetoothAdapterState> get bluetoothAdapterState {
    return FlutterBluePlus.adapterState;
  }

  /// Stream of the connected device's connection state.
  Stream<BluetoothConnectionState>? get deviceState =>
      _connectedDevice?.connectionState;

  /// Checks if BLE functionality is available on the current device.
  Future<bool> get isAvailable async => await FlutterBluePlus.isAvailable;

  /// Checks if the Bluetooth adapter is currently powered on.
  Future<bool> get isOn async => await FlutterBluePlus.isOn;

  /// Scans for a device matching [deviceId] (MAC address) and connects to it.
  /// Returns the connected [BluetoothDevice].
  Future<BluetoothDevice> scanAndConnectById(
    String deviceId, {
    Duration timeout = const Duration(seconds: 10),
  }) async {
    await startScan(timeout: timeout);
    try {
      final matchingList = await scanResults.firstWhere(
        (list) => list.any(
          (r) => r.device.id.id.toUpperCase() == deviceId.toUpperCase(),
        ),
      );
      final result = matchingList.firstWhere(
        (r) => r.device.id.id.toUpperCase() == deviceId.toUpperCase(),
      );
      print(result);
      await stopScan();
      await connectToDevice(result.device, timeout: timeout);
      return result.device;
    } catch (e) {
      await stopScan();
      throw Exception('Device \$deviceId not found: \$e');
    }
  }

  /// Alias for [subscribeToCharacteristic], providing a familiar name.
  Future<Stream<List<int>>> listenCharacteristic(
    Guid serviceUuid,
    Guid charUuid,
  ) {
    return subscribeToCharacteristic(serviceUuid, charUuid);
  }
}
