package com.zxx.aaudio;

import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {
    static {
        System.loadLibrary("aaudio_player");
    }

    private TextView statusText;

    private native void nativeStart();
    private native void nativeStop();
    private native void nativeRelease();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        statusText = findViewById(R.id.statusText);
        Button btnStart = findViewById(R.id.btnStart);
        Button btnStop = findViewById(R.id.btnStop);

        btnStart.setOnClickListener(v -> {
            nativeStart();
            statusText.setText(R.string.status_playing);
        });

        btnStop.setOnClickListener(v -> {
            nativeStop();
            statusText.setText(R.string.status_stopped);
        });
    }

    @Override
    protected void onStop() {
        nativeStop();
        statusText.setText(R.string.status_stopped);
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        nativeRelease();
        super.onDestroy();
    }
}
