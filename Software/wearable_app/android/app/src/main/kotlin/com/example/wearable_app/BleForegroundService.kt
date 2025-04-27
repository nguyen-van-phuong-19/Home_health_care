package com.example.wearable_app

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.IBinder
import android.util.Log
import androidx.core.app.NotificationCompat

class BleForegroundService : Service() {
    companion object {
        const val ACTION_START = "ACTION_START_BLE_SERVICE"
        const val EXTRA_MAC    = "EXTRA_MAC_ADDRESS"
        const val EXTRA_UUIDS  = "EXTRA_UUIDS"
        const val CHANNEL_ID   = "ble_service_channel"
        const val NOTIF_ID     = 1
    }

    override fun onCreate() {
        super.onCreate()
        createNotificationChannel()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        intent?.let {
            if (it.action == ACTION_START) {
                val mac   = it.getStringExtra(EXTRA_MAC)
                val uuids = it.getStringArrayListExtra(EXTRA_UUIDS)
                startForeground(NOTIF_ID, buildNotification("BLE service running"))
                startBleLoop(mac, uuids)
            }
        }
        return START_STICKY
    }

    private fun startBleLoop(mac: String?, uuids: List<String>?) {
        val adapter = BluetoothAdapter.getDefaultAdapter()
        val scanner = adapter.bluetoothLeScanner

        val callback = object : ScanCallback() {
            override fun onScanResult(callbackType: Int, result: ScanResult) {
                val dev = result.device
                if (dev.address == mac) {
                    Log.i("BleFGService", "Found target device, connecting…")
                    scanner.stopScan(this)
                    // TODO: connect GATT, listen disconnect & reconnect…
                }
            }

            override fun onScanFailed(errorCode: Int) {
                Log.e("BleFGService", "Scan failed with code $errorCode")
            }
        }

        Thread {
            while (true) {
                try {
                    scanner.startScan(callback)
                    Thread.sleep(5000)
                    scanner.stopScan(callback)
                    Thread.sleep(5000)
                } catch (t: Throwable) {
                    Log.e("BleFGService", "BLE loop error", t)
                }
            }
        }.start()
    }
    
    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val chan = NotificationChannel(
                CHANNEL_ID,
                "BLE Background Service",
                NotificationManager.IMPORTANCE_LOW
            )
            val mgr = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            mgr.createNotificationChannel(chan)
        }
    }

    private fun buildNotification(contentText: String): Notification {
        return NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("HealthTracker BLE")
            .setContentText(contentText)
            .setSmallIcon(R.drawable.ic_ble)      // thay icon phù hợp
            .setOngoing(true)
            .build()
    }

    override fun onBind(intent: Intent?): IBinder? = null
}
