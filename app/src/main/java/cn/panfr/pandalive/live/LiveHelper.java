package cn.panfr.pandalive.live;

/**
 * Created by panda on 2017/2/21.
 */

public class LiveHelper {
    public static native String helloFromFFmpeg();

    static {
        System.loadLibrary("panda-jni");
    }
}
