package io.zdrx.client;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

/** Sobe a engine sozinha e acompanha o status, sem clique. */
public class ZdrxServico extends Service {

    private static final int ID = 4242;
    private static final long ESPERA_MIN = 1500L;
    private static final long ESPERA_MAX = 30000L;

    private Process proc;
    private volatile boolean rodando = true;

    public static void iniciar(Context ctx) {
        Intent i = new Intent(ctx, ZdrxServico.class);
        if (Build.VERSION.SDK_INT >= 26) ctx.startForegroundService(i);
        else ctx.startService(i);
    }

    public static void parar(Context ctx) {
        ctx.stopService(new Intent(ctx, ZdrxServico.class));
    }

    @Override
    public void onCreate() {
        super.onCreate();
        canal();
        startForeground(ID, notificacao("aguardando o jogo"));
        arrancar();
        vigiar();
    }

    private void canal() {
        if (Build.VERSION.SDK_INT < 26) return;
        NotificationManager nm = getSystemService(NotificationManager.class);
        if (nm == null) return;
        nm.createNotificationChannel(
                new NotificationChannel("zdrx", "ZDRX", NotificationManager.IMPORTANCE_LOW));
    }

    private Notification notificacao(String txt) {
        Notification.Builder b = Build.VERSION.SDK_INT >= 26
                ? new Notification.Builder(this, "zdrx")
                : new Notification.Builder(this);
        return b.setContentTitle("ZDRX")
                .setContentText(txt)
                .setSmallIcon(android.R.drawable.ic_menu_compass)
                .setOngoing(true)
                .build();
    }

    private void arrancar() {
        String p = Zdrx.caminho(this);
        if (!Zdrx.pronto(this)) {
            proc = null;
            return;
        }
        try {
            proc = new ProcessBuilder(p).redirectErrorStream(true).start();
        } catch (IOException direto) {
            try {
                proc = new ProcessBuilder("/system/bin/linker64", p)
                        .redirectErrorStream(true).start();
            } catch (IOException viaLinker) {
                proc = null;
            }
        }
    }

    private void vigiar() {
        new Thread(() -> {
            File f = new File(Zdrx.STATUS);
            long espera = ESPERA_MIN;
            while (rodando) {
                String txt = "aguardando jogo";
                try {
                    if (f.exists() && f.canRead()) {
                        byte[] b = new byte[160];
                        try (FileInputStream in = new FileInputStream(f)) {
                            int n = in.read(b);
                            if (n > 0) txt = new String(b, 0, n, "UTF-8").trim();
                        }
                        espera = ESPERA_MIN;
                    }
                } catch (Exception ignored) {
                }
                atualizar(txt);

                if (proc == null || !vivo(proc)) {
                    arrancar();
                    try {
                        Thread.sleep(espera);
                    } catch (InterruptedException ignored) {
                    }
                    espera = Math.min(espera * 2, ESPERA_MAX);
                    continue;
                }
                try {
                    Thread.sleep(ESPERA_MIN);
                } catch (InterruptedException ignored) {
                }
            }
        }).start();
    }

    private static boolean vivo(Process p) {
        try {
            p.exitValue();
            return false;
        } catch (IllegalThreadStateException e) {
            return true;
        }
    }

    private void atualizar(String txt) {
        NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        if (nm != null) nm.notify(ID, notificacao(txt));
    }

    @Override
    public int onStartCommand(Intent i, int flags, int id) {
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        rodando = false;
        if (proc != null) proc.destroy();
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent i) {
        return null;
    }
}
