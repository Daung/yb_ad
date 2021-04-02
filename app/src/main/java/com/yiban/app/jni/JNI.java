package com.yiban.app.jni;

import android.content.Context;

public class JNI {

    private static volatile JNI INSTANCE;

    private final AntiDebug mJni;

    public static JNI getInstance() {
        if (INSTANCE == null) {
            synchronized (JNI.class) {
                if (INSTANCE == null) {
                    INSTANCE = new JNI();
                }
            }
        }
        return INSTANCE;
    }

    private JNI() {
        mJni = new AntiDebug();
    }



    public void startAntiDebug(Context context) {
        mJni.startAntiDebug(context);
    }

}
