package cn.panfr.pandalive.live;

import android.graphics.Bitmap;

/**
 * Created by panda on 2017/2/21.
 */

public class LiveHelper {
    public static native String helloFromFFmpeg();

    public static native String compressBitmap(Bitmap bitmap, int width, int height, int quality, byte[] fileName, boolean optimize);

    static {
        System.loadLibrary("jpeg");// libjpeg
        System.loadLibrary("panda-jni");
    }
}
