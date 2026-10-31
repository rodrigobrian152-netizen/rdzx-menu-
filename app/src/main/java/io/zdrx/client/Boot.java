package io.zdrx.client;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.net.VpnService;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.text.InputType;
import android.util.TypedValue;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;


public class Boot extends Activity {

    private static final int REQ_PERM = 1001;
    private static final int REQ_VPN  = 1002;

    private TextView out;

    @Override protected void onCreate(Bundle b) {
        super.onCreate(b);
        setContentView(ui());
        ask();
    }

    private View ui() {
        ScrollView sc = new ScrollView(this);
        LinearLayout r = new LinearLayout(this);
        r.setOrientation(LinearLayout.VERTICAL);
        r.setPadding(dp(18), dp(18), dp(18), dp(18));
        r.setBackgroundColor(Color.parseColor("#0b1220"));
        sc.addView(r);

        TextView t = new TextView(this);
        t.setText(Zdrx.NAME + "  " + Zdrx.VER);
        t.setTextColor(Color.parseColor("#22d3ee"));
        t.setTextSize(TypedValue.COMPLEX_UNIT_SP, 22);
        r.addView(t);

        TextView hw = new TextView(this);
        hw.setText("HWID  " + Lic.hwid(this).substring(0, 16));
        hw.setTextColor(Color.parseColor("#8b9ab5"));
        hw.setTextSize(TypedValue.COMPLEX_UNIT_SP, 11);
        r.addView(hw);

        final EditText key = new EditText(this);
        key.setHint("chave  (ZDRX-TRIAL libera)");
        key.setTextColor(Color.WHITE);
        key.setHintTextColor(Color.parseColor("#5b6b86"));
        key.setInputType(InputType.TYPE_CLASS_TEXT);
        r.addView(key);

        Button gen = new Button(this);
        gen.setText("GERAR MINHA CHAVE");
        gen.setOnClickListener(v -> {
            key.setText(Lic.keyFor(Boot.this));
            say("chave: " + Lic.keyFor(Boot.this));
        });
        r.addView(gen);

        Button start = new Button(this);
        start.setText("ATIVAR E INICIAR");
        start.setOnClickListener(v -> activate(key.getText().toString()));
        r.addView(start);

        Button stop = new Button(this);
        stop.setText("PARAR");
        stop.setOnClickListener(v -> {
            Zdrx.boot(false);
            stopService(new Intent(this, ZdrxVpn.class));
            stopService(new Intent(this, ZdrxOverlay.class));
            say("parado");
        });
        r.addView(stop);

        out = new TextView(this);
        out.setTextColor(Color.parseColor("#9ca3af"));
        out.setTextSize(TypedValue.COMPLEX_UNIT_SP, 11);
        out.setPadding(0, dp(10), 0, 0);
        r.addView(out);

        loop();
        return sc;
    }

    private void loop() {
        out.postDelayed(new Runnable() {
            @Override public void run() {
                try {
                    Zdrx.tick();
                    String tr = Zdrx.traceDrain();
                    if (tr != null && !tr.trim().isEmpty()) say(tr.trim());
                } catch (Throwable ignored) {}
                out.postDelayed(this, 1500);
            }
        }, 1500);
    }

    private void say(String s) { out.setText(s + "\n" + out.getText()); }

    private void ask() {
        List<String> need = new ArrayList<>();
        if (Build.VERSION.SDK_INT >= 33
                && checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS)
                   != PackageManager.PERMISSION_GRANTED) {
            need.add(Manifest.permission.POST_NOTIFICATIONS);
        }
        if (!need.isEmpty()) requestPermissions(need.toArray(new String[0]), REQ_PERM);

        if (!Settings.canDrawOverlays(this)) {
            say("conceda \"Sobrepor a outros apps\"");
            startActivity(new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION));
        }
    }

    private void activate(String key) {
        if (Lic.check(this, key) != Lic.OK) {
            Toast.makeText(this, "chave invalida", Toast.LENGTH_SHORT).show();
            say("licenca: INVALIDA");
            return;
        }
        say("licenca: OK");

        if (!Settings.canDrawOverlays(this)) {
            startActivity(new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION));
            return;
        }

        Intent vpn = VpnService.prepare(this);
        if (vpn != null) { startActivityForResult(vpn, REQ_VPN); return; }
        launch();
    }

    private void launch() {
        Zdrx.boot(true);
        Prefs.loadCfg();
        startService(new Intent(this, ZdrxVpn.class));
        startService(new Intent(this, ZdrxOverlay.class));
        say("motor ONLINE - VPN + overlay ativos");
    }

    @Override protected void onActivityResult(int req, int res, Intent d) {
        super.onActivityResult(req, res, d);
        if (req == REQ_VPN) {
            if (res == RESULT_OK) launch();
            else say("VPN recusada");
        }
    }

    @Override protected void onPause() {
        super.onPause();
        try { Prefs.saveCfg(); } catch (Throwable ignored) {}
    }

    private int dp(int v) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, v,
                getResources().getDisplayMetrics());
    }
}
