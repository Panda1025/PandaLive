package cn.panfr.pandalive.ui;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.lang.ref.WeakReference;

import butterknife.BindView;
import butterknife.ButterKnife;
import cn.panfr.pandalive.R;
import cn.panfr.pandalive.live.LiveHelper;

public class LiveActivity extends AppCompatActivity implements SurfaceHolder.Callback, Camera.PreviewCallback{

    @BindView(R.id.live_surface)
    SurfaceView mSurfaceView;
    private Camera mCamera;

    private SurfaceHolder mSurfaceHolder;
    private int mCameraId = 0;
    private int width = 1280;
    private int height = 720;

    private HandlerThread mHandlerThread;

    private LiveHandler mHandler;


    private static final int STREAMER_INIT = 1;
    private static final int STREAMER_HANDLE = 2;
    private static final int STREAMER_FLUSH = 3;
    private static final int STREAMER_RELEASE = 4;

    /**
     * 判断有没有初始化成功，不成功不不进行后续的编码处理
     */
    private int liveInitResult = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_live);
        ButterKnife.bind(this);
        mSurfaceHolder = mSurfaceView.getHolder();
        mSurfaceHolder.setFixedSize(width, height);
        mSurfaceHolder.addCallback(this);

        mHandlerThread = new HandlerThread("liveHandlerThread");
        mHandlerThread.start();
        mHandler = new LiveHandler(this, mHandlerThread.getLooper());
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        /**
         * 如果初始化成功，那就把数据发送到Handler，然后再调用native方法
         */
        if(liveInitResult == 0 && data != null && data.length > 0) {
            Message msg = Message.obtain();
            Bundle bundle = new Bundle();
            bundle.putByteArray("frame_data", data);
            msg.what = STREAMER_HANDLE;
            msg.setData(bundle);
            mHandler.sendMessage(msg);
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        liveInitResult = streamerInit(width, height);
        if(liveInitResult == -1) {
            mHandler.sendEmptyMessage(STREAMER_RELEASE);
        } else {
            Log.e("LiveActivity", "streamer init result: " + liveInitResult);
        }
        initCamera();
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        /**
         * 在surface销毁的时候清空缓冲帧（在直播成功开启的情况下）
         * 清空后就进行资源的释放
         * 并且把HandlerThread退出
         */
        if(liveInitResult == 0) {
            mHandler.sendEmptyMessage(STREAMER_FLUSH);
        }
        mHandler.sendEmptyMessage(STREAMER_RELEASE);
        mHandlerThread.quitSafely();
        destroyCamera();
    }

    private void initCamera() {
        try {
            mCamera = Camera.open(mCameraId);
            mCamera.setPreviewDisplay(mSurfaceHolder);
            Camera.Parameters params = mCamera.getParameters();
            //设置预览大小
            params.setPreviewSize(width, height);
            //设置生成的照片大小
            params.setPictureSize(width, height);
            params.setPreviewFormat(ImageFormat.NV21);
            mCamera.setDisplayOrientation(90);
            mCamera.setParameters(params);
            mCamera.setPreviewCallback(this);
            mCamera.startPreview();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void destroyCamera() {
        if(mCamera == null) {
            return;
        }

        mCamera.setPreviewCallback(null);
        mCamera.stopPreview();
        mCamera.release();
        mCamera = null;
    }
    private static class LiveHandler extends Handler{
        private WeakReference<LiveActivity> mActivity;

        public LiveHandler(LiveActivity activity, Looper looper) {
            super(looper);
            mActivity = new WeakReference<LiveActivity>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            LiveActivity activity = mActivity.get();
            if(activity == null) {
                return;
            }

            switch (msg.what){
                case STREAMER_INIT:
                    break;

                case STREAMER_HANDLE:
                    Bundle bundle = msg.getData();
                    if(bundle != null) {
                        byte[] data = bundle.getByteArray("frame_data");
                        if(data != null && data.length > 0) {
                            activity.streamerHandle(data);
                        } else {
                            Log.e("LiveActivity", "byte data null");
                        }
                    } else {
                        Log.e("LiveActivity", "bundle null");
                    }
                    break;

                case STREAMER_FLUSH:
                    activity.streamerFlush();
                    break;

                case STREAMER_RELEASE:
                    activity.streamerRelease();
                    break;
            }
        }
    }

    private int streamerInit(int width, int height) {
        return LiveHelper.streamerInit(width, height,"rtmp://live.simope.com:1935/live/90a3c65d07?liveID=100029116&accessCode=f43348c2");
    }

    private void streamerRelease() {
        LiveHelper.streamerRelease();
    }

    private void streamerFlush() {
        LiveHelper.streamerFlush();
    }

    private void streamerHandle(byte[] data) {
        LiveHelper.streamerHandle(data);
    }
}
