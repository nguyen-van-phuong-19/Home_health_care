import 'package:flutter/material.dart';
import 'package:shared_preferences/shared_preferences.dart';

class UserProvider extends ChangeNotifier {
  String _userId = 'default_user';
  
  String get userId => _userId;
  
  UserProvider() {
    _loadUserId();
  }
  
  Future<void> _loadUserId() async {
    final prefs = await SharedPreferences.getInstance();
    final savedUserId = prefs.getString('user_id');
    if (savedUserId != null && savedUserId.isNotEmpty) {
      _userId = savedUserId;
      notifyListeners();
    }
  }
  
  Future<void> setUserId(String userId) async {
    if (userId.isEmpty) return;
    
    _userId = userId;
    
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString('user_id', userId);
    
    notifyListeners();
  }
}
