package io.zdrx.client;

import android.content.Context;
import android.content.SharedPreferences;

public final class Prefs {

    private static final String STORE = "zdrx";
    private static SharedPreferences sp;

    public static void init(Context c) {
        sp = c.getApplicationContext().getSharedPreferences(STORE, Context.MODE_PRIVATE);
    }

    public static String get(String k, String def) {
        return sp == null ? def : sp.getString(k, def);
    }

    public static void put(String k, String v) {
        if (sp != null) sp.edit().putString(k, v).apply();
    }

    public static void saveCfg() { put("cfg", Zdrx.cfgJson()); }

    public static void loadCfg() {
        String j = get("cfg", null);
        if (j != null && j.length() > 2) Zdrx.setJson(j);
    }

    private Prefs() {}
}
