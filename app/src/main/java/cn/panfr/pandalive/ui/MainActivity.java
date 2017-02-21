package cn.panfr.pandalive.ui;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.TextView;

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

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);
        textView.setText(LiveHelper.helloFromFFmpeg());
    }

    @OnClick({R.id.live, R.id.look})
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
        }
    }
}
