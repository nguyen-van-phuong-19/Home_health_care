import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart' as fb_auth;
import 'package:firebase_ui_auth/firebase_ui_auth.dart' as ui_auth;
import 'package:firebase_ui_oauth_google/firebase_ui_oauth_google.dart'
    as ui_google;
import 'package:wearable_app/screens/home_screen/home_screen.dart';

class AuthenticationWrapper extends StatelessWidget {
  final String googleClientId;
  const AuthenticationWrapper({Key? key, required this.googleClientId})
    : super(key: key);

  @override
  Widget build(BuildContext context) {
    return StreamBuilder<fb_auth.User?>(
      stream: fb_auth.FirebaseAuth.instance.authStateChanges(),
      builder: (context, snapshot) {
        if (snapshot.connectionState == ConnectionState.waiting) {
          return const Scaffold(
            body: Center(child: CircularProgressIndicator()),
          );
        }

        if (snapshot.hasData) {
          return const HomeScreen();
        }

        return ui_auth.SignInScreen(
          providers: [
            ui_auth.EmailAuthProvider(),
            ui_google.GoogleProvider(clientId: googleClientId),
          ],
          actions: [
            ui_auth.AuthStateChangeAction<ui_auth.SignedIn>((context, state) {
              Navigator.of(context).pushReplacement(
                MaterialPageRoute(builder: (_) => const HomeScreen()),
              );
            }),
            ui_auth.AuthStateChangeAction<ui_auth.AuthFailed>((context, state) {
              final msg = state.exception.toString();
              ScaffoldMessenger.of(context).showSnackBar(
                SnackBar(content: Text('Đăng nhập thất bại: $msg')),
              );
            }),
            ui_auth.AuthStateChangeAction<ui_auth.UserCreated>((
              context,
              state,
            ) {
              Navigator.of(context).pushReplacement(
                MaterialPageRoute(builder: (_) => const HomeScreen()),
              );
            }),
          ],
          footerBuilder: (context, _) {
            return const Padding(
              padding: EdgeInsets.symmetric(vertical: 8),
              child: Text(
                'Đăng nhập hoặc Đăng ký để tiếp tục',
                style: TextStyle(color: Colors.grey),
              ),
            );
          },
        );
      },
    );
  }
}
