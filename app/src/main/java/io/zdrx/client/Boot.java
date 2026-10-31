package io.zdrx.client;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/** Sobe o serviço da engine no boot do aparelho. */
public class Boot extends BroadcastReceiver {

    @Override
    public void onReceive(Context ctx, Intent i) {
        if (i == null || i.getAction() == null) return;
        String a = i.getAction();
        if (Intent.ACTION_BOOT_COMPLETED.equals(a) || "android.intent.action.QUICKBOOT_POWERON".equals(a)) {
            ZdrxServico.iniciar(ctx);
        }
    }
}
