package com.yiban.app.jni;

import android.util.Log;

import java.util.Timer;
import java.util.TimerTask;

/**
 * 定时任务管理器
 *
 * @author dyh
 */
public class TimerManager {

    private static final String TAG = "YIBAN_SO";

    /**
     * 单例模式
     */
    private static TimerManager timerManager;


    private Timer timer;

    private static class MyTask extends TimerTask {

        @Override
        public void run() {
            JNIHelper.startAntiDebug();
            Log.d(TAG, "handleMessage:  name = " + Thread.currentThread().getName());
        }
    }


    private static final int TIMER_DURATION = 5000;

    private TimerManager() {

    }

    public static TimerManager getInstance() {
        if (timerManager == null) {
            synchronized (TimerManager.class) {
                if (timerManager == null) {
                    timerManager = new TimerManager();
                }
            }
        }
        return timerManager;
    }


    /**
     * 启动定时任务
     */
    public void startTimerTask() {
        if (timer == null) {
            timer = new Timer("timer");
            timer.purge();
            timer.schedule(new MyTask(), 0, TIMER_DURATION);
        }
    }

    /**
     * 定时任务取消
     */
    public void stopTimerTask() {
        if (timer != null) {
            timer.cancel();
            timer = null;
        }
    }

}
