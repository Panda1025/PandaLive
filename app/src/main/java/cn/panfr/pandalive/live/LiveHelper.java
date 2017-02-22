package cn.panfr.pandalive.live;

import android.graphics.Bitmap;

/**
 * Created by panda on 2017/2/21.
 */

public class LiveHelper {
    public static native String helloFromFFmpeg();

    /**
     * libjpeg压缩图片方法
     * @param bitmap 原位图
     * @param width 需要压缩的宽度
     * @param height 需要压缩的高度
     * @param quality 压缩率
     * @param fileName 保存的文件地址
     * @param optimize 是否开始哈弗曼算法
     * @return 0 成功 1 失败
     */
    public static native String compressBitmap(Bitmap bitmap, int width, int height, int quality, byte[] fileName, boolean optimize);

    /**
     * 初始化编码
     * @param width 编码视频的宽度
     * @param height 编码视频的高度
     * @return 0 成功  小于0失败
     */
    public static native int streamerInit(int width, int height, String address);

    /**
     * 对每一帧进行编码推流
     * @param data NV21格式的数据
     * @return 0成功，小于0失败
     */
    public static native int streamerHandle(byte[] data);

    /**
     * 把缓冲帧的数据清空
     * @return 0成功，小于0失败
     */
    public static native int streamerFlush();

    /**
     * 释放资源，比如编码器这些
     * @return 0成功，小于0失败
     */
    public static native int streamerRelease();

    static {
        System.loadLibrary("jpeg");// libjpeg
        System.loadLibrary("panda-jni");
    }
}
