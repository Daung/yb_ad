package com.yiban.app.jni;

import android.content.Context;

public class AntiDebug {

    static {
        System.loadLibrary("antidebug");
    }

    public native void startAntiDebug(Context context);

}
