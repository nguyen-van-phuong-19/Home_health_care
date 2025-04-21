// lib/services/firebase_service.dart

import 'package:firebase_database/firebase_database.dart';

class FirebaseService {
  // Singleton
  static final FirebaseService _instance = FirebaseService._internal();
  factory FirebaseService() => _instance;
  FirebaseService._internal();

  // Tham chiếu đến root của Realtime Database
  final DatabaseReference _dbRef = FirebaseDatabase.instance.ref();

  /// Đọc dữ liệu một lần tại đường dẫn [path]
  Future<DataSnapshot> readOnce(String path) async {
    try {
      final snapshot = await _dbRef.child(path).once();
      return snapshot.snapshot;
    } catch (e) {
      // Xử lý lỗi (network, permission, ...)
      rethrow;
    }
  }

  /// Ghi đè (set) dữ liệu [value] tại [path]
  Future<void> setData(String path, Object value) async {
    try {
      await _dbRef.child(path).set(value);
    } catch (e) {
      rethrow;
    }
  }

  /// Cập nhật (update) các trường con tại [path]
  Future<void> updateData(String path, Map<String, Object> values) async {
    try {
      await _dbRef.child(path).update(values);
    } catch (e) {
      rethrow;
    }
  }

  /// Thêm mới (push) một node con vào [path]
  Future<DatabaseReference> pushData(String path, Object value) async {
    try {
      final newRef = _dbRef.child(path).push();
      await newRef.set(value);
      return newRef;
    } catch (e) {
      rethrow;
    }
  }

  /// Xóa dữ liệu tại [path]
  Future<void> removeData(String path) async {
    try {
      await _dbRef.child(path).remove();
    } catch (e) {
      rethrow;
    }
  }

  /// Lắng nghe sự thay đổi realtime tại [path]
  Stream<DatabaseEvent> listenToChanges(String path) {
    return _dbRef.child(path).onValue;
  }
}
