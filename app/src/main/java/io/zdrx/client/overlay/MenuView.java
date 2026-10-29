package io.zdrx.client.overlay;

import android.content.Context;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.view.MotionEvent;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;

import io.zdrx.client.Prefs;
import io.zdrx.client.Zdrx;

/** Menu construido automaticamente a partir da tabela nativa de features. */
public class MenuView extends LinearLayout {

    private final LinearLayout body;
    private final TextView caret;
    private boolean open = true;

    private float downX, downY;
    private int startLeft, startTop;

    public MenuView(Context c) {
        super(c);
        setOrientation(VERTICAL);

        GradientDrawable bg = new GradientDrawable();
        bg.setColor(0xE60B1220);
        bg.setCornerRadius(20f);
        bg.setStroke(2, 0xFF22D3EE);
        setBackground(bg);
        setPadding(4, 4, 4, 10);

        body = new LinearLayout(c);
        body.setOrientation(VERTICAL);

        LinearLayout bar = new LinearLayout(c);
        bar.setOrientation(HORIZONTAL);
        bar.setPadding(18, 12, 12, 12);

        TextView title = new TextView(c);
        title.setText("ZDRX  " + Zdrx.VER);
        title.setTextColor(0xFF22D3EE);
        title.setTextSize(13f);
        title.setTypeface(Typeface.MONOSPACE);
        bar.addView(title, new LayoutParams(0, LayoutParams.WRAP_CONTENT, 1f));

        caret = new TextView(c);
        caret.setText("v");
        caret.setTextColor(0xFFFFFFFF);
        caret.setTextSize(13f);
        caret.setPadding(16, 0, 8, 0);
        bar.addView(caret);

        bar.setOnTouchListener(new Drag());
        addView(bar);
        addView(body);

        build();
        try { Prefs.loadCfg(); } catch (Throwable ignored) {}
    }

    private void build() {
        int n;
        try { n = Zdrx.featCount(); } catch (Throwable t) { return; }

        for (int i = 0; i < n; i++) {
            final int id = i;
            int type = Zdrx.featType(i);
            String label = Zdrx.featLabel(i);

            if (type == 0) {
                Switch sw = new Switch(getContext());
                sw.setText(label);
                sw.setTextColor(0xFFE5E7EB);
                sw.setTextSize(12f);
                sw.setPadding(18, 6, 18, 6);
                sw.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                    @Override public void onCheckedChanged(CompoundButton b, boolean v) {
                        Zdrx.set(id, v ? 1 : 0);
                        Prefs.saveCfg();
                    }
                });
                body.addView(sw);
            } else {
                LinearLayout row = new LinearLayout(getContext());
                row.setOrientation(VERTICAL);
                row.setPadding(18, 8, 18, 4);

                TextView lab = new TextView(getContext());
                lab.setText(label + "   " + Zdrx.featDef(id));
                lab.setTextColor(0xFF9CA3AF);
                lab.setTextSize(11f);
                row.addView(lab);

                SeekBar sb = new SeekBar(getContext());
                int lo = Zdrx.featLo(id);
                int hi = Zdrx.featHi(id);
                sb.setMax(Math.max(1, hi - lo));
                sb.setProgress(Zdrx.featDef(id) - lo);
                sb.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    @Override public void onProgressChanged(SeekBar s, int p, boolean u) {
                        int v = Zdrx.featLo(id) + p;
                        lab.setText(Zdrx.featLabel(id) + "   " + v);
                        Zdrx.set(id, v);
                    }
                    @Override public void onStartTrackingTouch(SeekBar s) {}
                    @Override public void onStopTrackingTouch(SeekBar s) { Prefs.saveCfg(); }
                });
                row.addView(sb);
                body.addView(row);
            }
        }

        TextView foot = new TextView(getContext());
        foot.setText("arraste o titulo para mover");
        foot.setTextColor(0xFF5B6B86);
        foot.setTextSize(10f);
        foot.setPadding(18, 10, 18, 4);
        body.addView(foot);
    }

    private class Drag implements OnTouchListener {
        @Override public boolean onTouch(View v, MotionEvent e) {
            FrameLayout.LayoutParams lp = (FrameLayout.LayoutParams) getLayoutParams();
            switch (e.getActionMasked()) {
                case MotionEvent.ACTION_DOWN:
                    downX = e.getRawX();
                    downY = e.getRawY();
                    startLeft = lp.leftMargin;
                    startTop = lp.topMargin;
                    return true;
                case MotionEvent.ACTION_MOVE:
                    lp.leftMargin = startLeft + (int) (e.getRawX() - downX);
                    lp.topMargin  = startTop  + (int) (e.getRawY() - downY);
                    setLayoutParams(lp);
                    return true;
                case MotionEvent.ACTION_UP:
                    if (Math.abs(e.getRawX() - downX) < 12
                            && Math.abs(e.getRawY() - downY) < 12) {
                        open = !open;
                        body.setVisibility(open ? VISIBLE : GONE);
                        caret.setText(open ? "v" : ">");
                    }
                    return true;
                default:
                    return false;
            }
        }
    }
}
