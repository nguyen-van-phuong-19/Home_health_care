package com.example.wearable_app

import android.os.Build
import android.content.Intent
import io.flutter.embedding.android.FlutterActivity
import io.flutter.embedding.engine.FlutterEngine
import io.flutter.plugin.common.MethodChannel
import com.example.wearable_app.BleForegroundService


class MainActivity: FlutterActivity() {
    private val CHANNEL = "com.example.ble_service"

    override fun configureFlutterEngine(flutterEngine: FlutterEngine) {
        super.configureFlutterEngine(flutterEngine)
        MethodChannel(
            flutterEngine.dartExecutor.binaryMessenger,
            CHANNEL
        ).setMethodCallHandler { call, result ->
            when (call.method) {
                "startBleService" -> {
                    val mac   = call.argument<String>("mac")
                    val uuids = call.argument<List<String>>("uuids")
                    val intent = Intent(this, BleForegroundService::class.java).apply {
                        action = BleForegroundService.ACTION_START
                        putExtra(BleForegroundService.EXTRA_MAC, mac)
                        putStringArrayListExtra(
                            BleForegroundService.EXTRA_UUIDS,
                            ArrayList(uuids ?: emptyList())
                        )
                    }
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                        startForegroundService(intent)
                    } else {
                        startService(intent)
                    }
                    result.success(null)
                }
                else -> result.notImplemented()
            }
        }
    }
}
