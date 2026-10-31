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
import java.io.IOException;

/** Sobe a engine sozinha e acompanha o status, sem clique. */
public class ZdrxServico extends Service {

    private static final int ID = 4242;
    private Process proc;
    private volatile boolean vivo = true;

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
        try {
            proc = new ProcessBuilder(Zdrx.caminho(this)).redirectErrorStream(true).start();
        } catch (IOException e) {
            proc = null;
        }
    }

    private void vigiar() {
        new Thread(() -> {
            File f = new File("/data/local/tmp/zdrx.status");
            while (vivo) {
                try {
                    String txt = "aguardando jogo";
                    if (f.exists()) {
                        byte[] b = new byte[128];
                        try (java.io.FileInputStream in = new java.io.FileInputStream(f)) {
                            int n = in.read(b);
                            if (n > 0) txt = new String(b, 0, n, "UTF-8").trim();
                        }
                    }
                    atualizar(txt);
                    if (proc != null && !vivo(proc)) arrancar();
                } catch (Exception ignored) {}
                try { Thread.sleep(1500); } catch (InterruptedException ignored) {}
            }
        }).start();
    }

    private static boolean vivo(Process p) {
        try { p.exitValue(); return false; } catch (IllegalThreadStateException e) { return true; }
    }

    private void atualizar(String txt) {
        NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        if (nm != null) nm.notify(ID, notificacao(txt));
    }

    @Override
    public int onStartCommand(Intent i, int flags, int id) { return START_STICKY; }

    @Override
    public void onDestroy() {
        vivo = false;
        if (proc != null) proc.destroy();
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent i) { return null; }
}
