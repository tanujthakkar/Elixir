package com.stellarlabs.elixir;

import android.media.Image;
import android.os.AsyncTask;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.google.firebase.FirebaseApp;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

import org.w3c.dom.Text;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Timer;
import java.util.TimerTask;

public class MainActivity extends AppCompatActivity {

    Button btnOn;
    Button btnOff;
    public TextView terminal;

    char storage_tank;
    char supply_tank;
    char relay;

    ImageView storage;
    ImageView supply;

    public final static String IP_ADDR = "http://192.168.0.200";
    public final static String PORT_ADDR = "80";
    public final static String RELAY_ON = "http://192.168.0.200/relay_on";
    public final static String RELAY_OFF = "http://192.168.0.200/relay_off";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        FirebaseApp.initializeApp(this);
        setContentView(R.layout.activity_main);

        btnOn = (Button)findViewById(R.id.btnOn);
        btnOff = (Button)findViewById(R.id.btnOff);

        storage = (ImageView)findViewById(R.id.storage_tank);
        supply = (ImageView)findViewById(R.id.supply_tank);

        FirebaseDatabase database = FirebaseDatabase.getInstance();
        DatabaseReference myRef = database.getReference();

        btnOn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                new ServerTask().execute(RELAY_ON);

            }
        });

        btnOff.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                new ServerTask().execute(RELAY_OFF);

            }
        });

        setRepeatingAsyncTask();

    }

    public class ServerTask extends AsyncTask<String, String, String> {


        @Override
        protected String doInBackground(String... urls) {

            HttpURLConnection urlConnection = null;
            BufferedReader reader = null;

            try {
                URL url = new URL(urls[0]);
                urlConnection = (HttpURLConnection) url.openConnection();
                urlConnection.connect();

                InputStream in = urlConnection.getInputStream();

                reader = new BufferedReader(new InputStreamReader(in));

                StringBuffer buffer = new StringBuffer();

                String line = "";
                while((line = reader.readLine()) != null) {
                    buffer.append(line);
                }

                return buffer.toString();

            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                if(urlConnection != null) {
                    urlConnection.disconnect();
                }
                try {
                    if(reader != null) {
                        reader.close();
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            return null;
        }

        @Override
        protected  void onPostExecute(String result) {
            super.onPostExecute(result);

            try {

                storage_tank = result.charAt(1);
                supply_tank = result.charAt(3);
                relay = result.charAt(5);
                if(relay == '1') {
                    btnOn.setBackgroundResource(R.drawable.status_active);
                    btnOff.setBackgroundResource(R.drawable.button_default);
                } else if(relay == '0') {
                    btnOff.setBackgroundResource(R.drawable.status_inactive);
                    btnOn.setBackgroundResource(R.drawable.button_default);
                }

                if(storage_tank == 'E') {
                    storage.setBackgroundResource(R.drawable.level_e);
                } else if (storage_tank == 'A') {
                    storage.setBackgroundResource(R.drawable.level_m);
                } else if (storage_tank == 'F') {
                    storage.setBackgroundResource(R.drawable.level_h);
                }

                if(supply_tank == 'E') {
                    supply.setBackgroundResource(R.drawable.level_e);
                } else if (supply_tank == 'L') {
                    supply.setBackgroundResource(R.drawable.level_l);
                } else if (supply_tank == 'M') {
                    supply.setBackgroundResource(R.drawable.level_m);
                } else if(supply_tank == 'H') {
                    supply.setBackgroundResource(R.drawable.level_h);
                }

//                terminal.setText(result);

            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private void setRepeatingAsyncTask() {

        final Handler handler = new Handler();
        Timer timer = new Timer();

        TimerTask task = new TimerTask() {
            @Override
            public void run() {
                handler.post(new Runnable() {
                    public void run() {
                        try {
                            ServerTask data = new ServerTask();
                            data.execute(IP_ADDR);
                        } catch (Exception e) {
                            // error, do something
                        }
                    }
                });
            }
        };

        timer.schedule(task, 0, 60*1);  // interval of one minute

    }

}