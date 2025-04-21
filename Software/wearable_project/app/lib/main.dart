import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:wearable_app/screens/dashboard_screen.dart';
import 'package:wearable_app/screens/heart_rate_screen.dart';
import 'package:wearable_app/screens/spo2_screen.dart';
import 'package:wearable_app/screens/calories_screen.dart';
import 'package:wearable_app/screens/sleep_screen.dart';
import 'package:wearable_app/screens/location_screen.dart';
import 'package:wearable_app/screens/settings_screen.dart';
import 'package:wearable_app/services/user_provider.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp();
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (context) => UserProvider(),
      child: MaterialApp(
        title: 'Wearable Device App',
        theme: ThemeData(
          primarySwatch: Colors.blue,
          visualDensity: VisualDensity.adaptivePlatformDensity,
          brightness: Brightness.light,
          appBarTheme: const AppBarTheme(
            elevation: 0,
            backgroundColor: Colors.white,
            foregroundColor: Colors.black,
          ),
        ),
        darkTheme: ThemeData(
          primarySwatch: Colors.blue,
          visualDensity: VisualDensity.adaptivePlatformDensity,
          brightness: Brightness.dark,
        ),
        themeMode: ThemeMode.system,
        home: const WearableApp(),
        routes: {
          '/dashboard': (context) => const DashboardScreen(),
          '/heart-rate': (context) => const HeartRateScreen(),
          '/spo2': (context) => const SpO2Screen(),
          '/calories': (context) => const CaloriesScreen(),
          '/sleep': (context) => const SleepScreen(),
          '/location': (context) => const LocationScreen(),
          '/settings': (context) => const SettingsScreen(),
        },
      ),
    );
  }
}

class WearableApp extends StatefulWidget {
  const WearableApp({Key? key}) : super(key: key);

  @override
  State<WearableApp> createState() => _WearableAppState();
}

class _WearableAppState extends State<WearableApp> {
  int _selectedIndex = 0;
  
  static const List<Widget> _screens = [
    DashboardScreen(),
    HeartRateScreen(),
    SpO2Screen(),
    CaloriesScreen(),
    SleepScreen(),
    LocationScreen(),
  ];
  
  void _onItemTapped(int index) {
    setState(() {
      _selectedIndex = index;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: _screens[_selectedIndex],
      bottomNavigationBar: BottomNavigationBar(
        type: BottomNavigationBarType.fixed,
        items: const <BottomNavigationBarItem>[
          BottomNavigationBarItem(
            icon: Icon(Icons.dashboard),
            label: 'Dashboard',
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.favorite),
            label: 'Heart Rate',
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.air),
            label: 'SpO2',
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.local_fire_department),
            label: 'Calories',
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.nightlight),
            label: 'Sleep',
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.location_on),
            label: 'Location',
          ),
        ],
        currentIndex: _selectedIndex,
        onTap: _onItemTapped,
      ),
    );
  }
}
