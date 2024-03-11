package com.example.front_camera_continuous_recording_with_service;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.view.View;
import android.widget.CheckBox;

public class MainActivity extends AppCompatActivity
{
    private final static int CODE_PERM_CAMERA = 0;
    private final static int CODE_PERM_SYSTEM_ALERT_WINDOW = 1;
    CheckBox mCheckBox;

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (null != intent.getAction() && intent.getAction().equals(CameraService.ACTION_STOPPED)) {
                flipButtonVisibility(false);
            }
        }
    };

    private void flipButtonVisibility(boolean running) {
        View buttonStart = findViewById(R.id.buttonStart);
        View buttonStop = findViewById(R.id.buttonStop);
        buttonStart.setVisibility(running ? View.GONE : View.VISIBLE);
        buttonStop.setVisibility(!running ? View.GONE : View.VISIBLE);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initView();

        mCheckBox = findViewById(R.id.checkBox);

//        String permission = Manifest.permission.CAMERA;
//        if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
//            // We don't have camera permission yet. Request it from the user.
//            ActivityCompat.requestPermissions(this, new String[]{permission}, CODE_PERM_CAMERA);
//        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        registerReceiver(mBroadcastReceiver, new IntentFilter(CameraService.ACTION_STOPPED));

        boolean running = isServiceRunning();
        flipButtonVisibility(running);

        askPermission();
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterReceiver(mBroadcastReceiver);
    }

    private void notifyService(String action) {
        Intent intent = new Intent(this, CameraService.class);
        intent.setAction(action);
        startService(intent);
    }

    private void initView() {
        View buttonStart = findViewById(R.id.buttonStart);
        View buttonStop = findViewById(R.id.buttonStop);

        buttonStart.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                if (!isServiceRunning()) {
                    notifyService(CameraService.ACTION_START + (mCheckBox.isChecked() ? "_true" : "_false"));
                    flipButtonVisibility(true);
//                    finish();
                }
            }
        });

        buttonStop.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                stopService(new Intent(MainActivity.this, CameraService.class));
                flipButtonVisibility(false);
            }
        });
    }

    private boolean isServiceRunning() {
        ActivityManager activityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningServiceInfo service : activityManager.getRunningServices(Integer.MAX_VALUE)) {
            if (CameraService.class.getName().equals(service.service.getClassName())) {
                return true;
            }
        }
        return false;
    }

    private void askPermission()
    {
        // 检查manifest中需要申请的权限是否都被授予，如果没有，则逐一要求用户授权
        String[] permissions = new String[] {
                Manifest.permission.CAMERA,
                Manifest.permission.RECORD_AUDIO,
                Manifest.permission.WRITE_EXTERNAL_STORAGE,
        };

        for (String permission : permissions)
        {
            if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED)
            {
                ActivityCompat.requestPermissions(this, new String[]{permission}, 1);
            }
        }

        // ask for permission of overlay
        if (!Settings.canDrawOverlays(this)) {
            // Don't have permission to draw over other apps yet - ask user to give permission
            Intent settingsIntent = new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION);
            startActivityForResult(settingsIntent, CODE_PERM_SYSTEM_ALERT_WINDOW);
        }
    }
}