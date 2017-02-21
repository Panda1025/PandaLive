package cn.panfr.pandalive.ui;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import butterknife.BindView;
import butterknife.ButterKnife;
import cn.panfr.pandalive.R;

public class LiveActivity extends AppCompatActivity implements SurfaceHolder.Callback, Camera.PreviewCallback{

    @BindView(R.id.live_surface)
    SurfaceView mSurfaceView;
    private Camera mCamera;

    private SurfaceHolder mSurfaceHolder;
    private int mCameraId = 0;
    private int width = 1280;
    private int height = 720;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_live);
        ButterKnife.bind(this);
        mSurfaceHolder = mSurfaceView.getHolder();
        mSurfaceHolder.setFixedSize(width, height);
        mSurfaceHolder.addCallback(this);

    }

    @Override
    public void onPreviewFrame(byte[] bytes, Camera camera) {

    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        initCamera();
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
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
}
