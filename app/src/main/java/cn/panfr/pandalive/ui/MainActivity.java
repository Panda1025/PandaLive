package cn.panfr.pandalive.ui;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import java.io.File;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import cn.panfr.pandalive.R;
import cn.panfr.pandalive.live.LiveHelper;

/**
 * Created by panda on 2017/2/21.
 */

public class MainActivity extends AppCompatActivity {
    @BindView(R.id.test)
    TextView textView;
    private String filePath;
    private String savePath;
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);
        textView.setText(LiveHelper.helloFromFFmpeg());
        filePath = Environment.getExternalStorageDirectory() + File.separator + "IMG_0164.JPG";
        savePath = Environment.getExternalStorageDirectory() + File.separator + "test.jpg";
    }

    @OnClick({R.id.live, R.id.look,R.id.libjpeg})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.live:
                //发起直播
                Intent intent = new Intent(this, LiveActivity.class);
                startActivity(intent);
                break;
            case R.id.look:
                //观看直播
                break;
            case R.id.libjpeg:
                //测试libjpeg
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        long time = System.currentTimeMillis();
                        Log.e("Panda", filePath);
                        Log.e("Panda", savePath);
                        Bitmap bitmap = BitmapFactory.decodeFile(filePath);
                        LiveHelper.compressBitmap(bitmap, bitmap.getWidth(), bitmap.getHeight(), 20, savePath.getBytes(), true);
                        Log.e("Panda", "time:" + (System.currentTimeMillis() - time));
                    }
                }).start();
                break;
        }
    }
}
