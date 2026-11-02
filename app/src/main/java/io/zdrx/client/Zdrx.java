package io.zdrx.client;

import android.content.Context;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.FloatBuffer;

/** Ponte com libzdrx.so. A UI se monta a partir da tabela nativa. */
public final class Zdrx {

    public static final String NAME = "ZDRX Client";
    public static final String VER  = "1.0.0";
    public static final String PKG  = "io.zdrx.client";
    public static final String GAME = "com.dts.freefireth";

    public static final String TMP    = "/data/local/tmp";
    public static final String STATUS = TMP + "/zdrx.status";
    public static final String ENG    = TMP + "/zdrx.eng";

    public static final int ESP_CAP = 8 + 96 * 6;

    static { System.loadLibrary("zdrx"); }

    public static native void boot(boolean enable);
    public static native void wipe();
    public static native void tick();
    public static native boolean live();

    public static native void set(int f, int v);
    public static native void setJson(String json);
    public static native String cfgJson();

    public static native String statusJson();
    public static native String traceDrain();

    public static native int featCount();
    public static native String featKey(int i);
    public static native String featLabel(int i);
    public static native int featType(int i);
    public static native int featDef(int i);
    public static native int featLo(int i);
    public static native int featHi(int i);

    public static native boolean looksLikeGame(byte[] pkt);
    public static native byte[] onS2C(byte[] pkt);
    public static native byte[] onC2S(byte[] pkt);
    public static native byte[][] drainOut();

    public static native int espInto(FloatBuffer buf);

    public static native boolean firing();

    /** Caminho executável da engine, já copiada e com permissão aplicada. */
    public static String caminho(Context ctx) {
        File dst = new File(ENG);
        try {
            File src = new File(ctx.getApplicationInfo().nativeLibraryDir, "libzdrx.so");
            if (!dst.exists() || dst.length() != src.length()) copiar(src, dst);
            dst.setExecutable(true, false);
        } catch (Exception ignored) {
        }
        return dst.getAbsolutePath();
    }

    public static boolean pronto(Context ctx) {
        File f = new File(ENG);
        return f.exists() && f.canExecute();
    }

    private static void copiar(File src, File dst) throws Exception {
        try (InputStream in = new FileInputStream(src);
             OutputStream out = new FileOutputStream(dst)) {
            byte[] b = new byte[8192];
            int n;
            while ((n = in.read(b)) > 0) out.write(b, 0, n);
        }
    }

    private Zdrx() {}
}
