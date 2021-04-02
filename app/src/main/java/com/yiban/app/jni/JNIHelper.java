package com.yiban.app.jni;

import com.yiban.app.application.YibanApplication;

public class JNIHelper {


    public static void startAntiDebug() {
        JNI.getInstance().startAntiDebug(YibanApplication.getContext());
    }

}
