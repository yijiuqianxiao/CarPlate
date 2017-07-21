package com.dongnao.carplate;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends Activity {


    private ImageView imageView;

    private TextView textView;

    private int index = 0;
    private int[] ids = {R.drawable.test1, R.drawable.test2, R.drawable.test3};
    static {
        System.loadLibrary("carplate");
        System.loadLibrary("opencv_java3");
    }

    native String recognition(Bitmap bitmap);
    native void init();
    native void release();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageView = (ImageView) findViewById(R.id.image_view);
        textView = (TextView) findViewById(R.id.result);
        imageView.setImageResource(ids[index]);
        init();
        try {
            InputStream is = getAssets().open("ANN_DATA.xml");
            FileOutputStream fos = new FileOutputStream("/sdcard/ANN_DATA.xml");
            byte[] b = new byte[2048];
            int len;
            while ((len = is.read(b)) != -1)
                fos.write(b, 0, len);
            is.close();
            fos.close();

            is = getAssets().open("ANN_ZH_DATA.xml");
            fos = new FileOutputStream("/sdcard/ANN_ZH_DATA.xml");
            while ((len = is.read(b)) != -1)
                fos.write(b, 0, len);
            is.close();
            fos.close();

            is = getAssets().open("SVM_DATA.xml");
            fos = new FileOutputStream("/sdcard/SVM_DATA.xml");
            while ((len = is.read(b)) != -1)
                fos.write(b, 0, len);
            is.close();
            fos.close();




        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void previous(View view) {
        textView.setText(null);
        index--;
        if (index < 0) {
            index = ids.length - 1;
        }
        imageView.setImageResource(ids[index]);
    }

    public void next(View view) {
        textView.setText(null);
        index++;
        if (index >= ids.length) {
            index = 0;
        }
        imageView.setImageResource(ids[index]);
    }

    public void click(View view) {
        Bitmap bitmap = BitmapFactory.decodeResource(getResources(), ids[index]);
        textView.setText(recognition(bitmap));
        bitmap.recycle();
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        release();
    }
}
