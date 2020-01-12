#include <ArduinoJson.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include <HTTP_Method.h>
#include <time.h>
  #define JST 3600*9
#include <WebServer.h>
#include <WiFi.h>

///////////Configurable Value///////////
#define NUM_LED 600
#define BRIGHTNESS 64
#define PIN 0
#define Oclock 15

const char* ssid = "SIAF-FREE-WiFi";
const char* pass = "siafsiaf";
float durationMin = 3;
//0=>red, 1=>green, 2=>blue
int color_temp[3] = {0, 149, 237};
int color_hum[3] = {0, 191, 255};
int color_drops[3] = {0, 191, 255};
///////////Configurable Value///////////

const String server = "http://opendata.artful.co.jp/get/?output=json";
CRGB leds[NUM_LED];

//environment_values
String envVls;
float temp, hum, air_press;
//sep => separate
float env_sep[2];
float env_length[2] = {0, 0};
bool flag_mills = true;
bool flag_getHttp = true;
unsigned long sttTime;
int i_hum_forDrops;
int duration_bounce[4] = {0, 0, 0, 0};
float duration_speed_bounce [4]= {0.1, 0.1, 0.1, 0.1};

void setup()
{
  Serial.begin(115200);
  delay(100);
  wifi_connect(ssid, pass);
  configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
  FastLED.addLeds<WS2812B, PIN, GRB>(leds, NUM_LED);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.showColor(CRGB::Black);
  Serial.println("init");
}

void loop()
{

  if(WiFi.status() == WL_CONNECTED)
  {
    led(Oclock, durationMin);
    FastLED.show();
  }
  delay(5);
}

//===================wifi=====================//

void wifi_connect(const char* ssid, const char* password)
{
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//===================get_http=====================//

String get_http()
{
  HTTPClient http;
  http.begin(server);
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    return http.getString();
  }
  http.end();
}

//===================get_envData=====================//

//(data you got with http, place{0=>place1, 1=>place2, 2=>place3}, data you want{"hum", "temp", "air_press"})
float get_envData(String data_http, int place, String data_name)
{
  const size_t capacity = JSON_ARRAY_SIZE(3) + 3*JSON_OBJECT_SIZE(5) + 290;
  DynamicJsonDocument doc(capacity);
  const String json = data_http;
  deserializeJson(doc, json);
  const char* data;
  JsonObject object = doc[place];
  if(data_name == "hum")
  {
    data = object["humidity"];
  }
  else if(data_name == "temp")
  {
    data = object["temperature"];
  }
  else if(data_name == "air_press")
  {
    data = object["air_pressure"];
  }
  return atof(data);
}

int get_crntTime()
{
  time_t t;
  struct tm *tm;
  t = time(NULL);
  tm = localtime(&t);
  return tm->tm_hour;
}

//===================led=====================//

void led(int oclock, float durationMin)
{
  if(true)//||get_crntTime() == Oclock)
  {
    if(flag_getHttp)
    {
      envVls = get_http();
      temp = get_envData(envVls, 2, "temp");
      constrain(temp, 0, -10);
      hum = get_envData(envVls, 2, "hum");
      constrain(hum, 35, 60);
      air_press = get_envData(envVls, 2, "air_press");
      for(int sep_i = 0; sep_i <= sizeof(env_sep); sep_i++)
      {
        switch (sep_i)
        {
        case 0:
          env_sep[sep_i] = temp * -10 / (durationMin * 60);
          break;
        case 1:
          env_sep[sep_i] = (hum - 35) * 2 / (durationMin * 60);
          break;
        }
      }
      flag_getHttp = false;
    }
    if(flag_mills)
    {
      sttTime = millis();
      flag_mills = false;
    }
    int eTime_led = millis() - sttTime;
    if(eTime_led > 1000)
    {
      for(int length_i = 0; length_i < 2; length_i++)
      {
        switch(length_i)
        {
          case 0:
            if(env_length[0] / 10 <= temp * -1)
            {
              env_length[0] += env_sep[0];
            }
            break;
          case 1:
            if(env_length[1] <= env_sep[1] * 60 * durationMin)
            {
              env_length[length_i] += env_sep[length_i];
            }
            break;
        }
      }
      flag_mills = true;
    }
    temp_bar((int)env_length[0], color_temp);
    hum_bar((int)env_length[1], color_hum);
    drops(1000, 10000, 10, color_drops, i_hum_forDrops);
    //drops(1000, 2000, 10, color_drops, i_hum_forDrops, 3);
  }
}

void setPix(int num,  CRGB rin[], int r, int g, int b)
{
  rin[num].r = r;
  rin[num].g = g;
  rin[num].b = b;
}

void setall(){
  for(int i = 0; i < 599; i++)
  {
    setPix(i, leds, 255, 255, 255);
  }
}

//===================env=====================//

void temp_bar(int length_temp, int color[3])
{
  for(int i_temp_0 = 0; i_temp_0 <= length_temp; i_temp_0++)
  {
    setPix(i_temp_0, leds, color[0], color[1], color[2]);
    setPix(i_temp_0 + 300, leds, color[0], color[1], color[2]);
  }
 for(int i_temp_1 = 299; i_temp_1 >= 299 - length_temp; i_temp_1--)
 {
   setPix(i_temp_1, leds, color[0], color[1], color[2]);
   setPix(i_temp_1 + 300, leds, color[0], color[1], color[2]);
 }
}

void hum_bar(int length_hum, int color[3])
{
  for(int i_hum_0 = 149; i_hum_0 >= 149 - length_hum; i_hum_0--)
  {
    i_hum_forDrops = i_hum_0;
    setPix(i_hum_0, leds, color[0], color[1], color[2]);
    setPix(i_hum_0 + 300, leds, color[0], color[1], color[2]);
  }

  for(int i_hum_1 = 150; i_hum_1 <= 150 + length_hum; i_hum_1++)
  {
    setPix(i_hum_1, leds, color[0], color[1], color[2]);
    setPix(i_hum_1 + 300, leds, color[0], color[1], color[2]);
  }
}

bool flag_drops[4] = {true, true, true, true};
bool flag_dropsSpeed[4] = {true, true, true, true};
bool flag_drops_start[4] = {true, true, true, true};
unsigned long time_drops[4];
unsigned int time_dropsSpeed[4];
int duration_drops[4];
int i_temp_forDrops[4];
int i_drops[4];
int n[4] = {1, 1, 1, 1};
int i_bounce[4] = {0, 0, 0, 0};
int i_hum_forBounce[4];
float duration_dropsGravity[4] = {0.0, 0.0, 0.0, 0.0};
bool flag_bounce[4] = {true, true, true, true};
bool flag_initBounce[4] = {true, true, true, true};
unsigned int time_bounce[4];
bool flag_bounce_forDrops[4] = {true, true, true, true};
bool flag_i_hum_forBounce[4] = {true, true, true, true};
int color_drops_forBounce[3];



void drops(int minDuration, int maxDuration, float duration_dropsSpeed, int color[3], int i_hum)
{
  for(int i = 0; i < 4; i++)
  {
    if(flag_drops[i])
    {
      time_drops[i] = millis();
      duration_drops[i] = random(minDuration, maxDuration);
      flag_drops[i] = false;
    }
    if(flag_dropsSpeed[i])
    {
      time_dropsSpeed[i] = millis();
      flag_dropsSpeed[i] = false;
    }
    int eTime_dropsSpeed[4];
    eTime_dropsSpeed[i] = millis() - time_dropsSpeed[i];
    int eTime_drops[4];
    eTime_drops[i] = millis() - time_drops[i];
    if(eTime_drops[i] >= duration_drops[i])
    {
      if(eTime_dropsSpeed[i] >= duration_dropsSpeed - duration_dropsGravity[i])
      {
        if(flag_drops_start[i])
        {
          i_temp_forDrops[i] = (int)env_length[0] + 1;
          i_drops[i] = 0;
          flag_drops_start[i] = false;
        }
        if(flag_bounce_forDrops[i])
        {
          setPix(299 - i_drops[0] - i_temp_forDrops[0], leds, color[0], color[1], color[2]);
          setPix(299 - i_drops[1] - i_temp_forDrops[1], leds, color[0], color[1], color[2]);
          setPix(300 + i_drops[2] + i_temp_forDrops[2], leds, color[0], color[1], color[2]);
          setPix(300 + i_drops[3] + i_temp_forDrops[3], leds, color[0], color[1], color[2]);
        }
        if(!( i_drops[i] == 0 )&& flag_bounce_forDrops[i])
        {
          setPix(299 - i_drops[0] - i_temp_forDrops[0] + 1, leds, 0, 0, 0);
          setPix(299 - i_drops[1] - i_temp_forDrops[1] + 1, leds, 0, 0, 0);
          setPix(i_drops[2] + i_temp_forDrops[2] + 299, leds, 0, 0, 0);
          setPix(i_drops[3] + i_temp_forDrops[3] + 299, leds, 0, 0, 0);
        }
        if(i_drops[i] + i_temp_forDrops[i] + 1 >= i_hum)
        {
          if( flag_i_hum_forBounce[i] )
          {
            flag_i_hum_forBounce[i] = false;
            for(int i_array_bounce = 0; i_array_bounce < 3; i_array_bounce++)
            {
              color_drops_forBounce[i_array_bounce] = color_drops[i_array_bounce];
            }
            i_hum_forBounce[i] = i_hum;
          }
          if(flag_bounce[i])
          {
            flag_bounce[i] = false;
            time_bounce[i] = millis();
          }
          int eTime_bounce[4];
          eTime_bounce[i] = millis() - time_bounce[i];
          if( eTime_bounce[i] >= duration_bounce[i] && i_bounce[i] <= 8)
          {
            if(i_bounce[i] == 0)
            {
              setPix(i_drops[i] + i_temp_forDrops[i], leds, 0, 0, 0);
            }
            
            if(!i_bounce[i] == 0)
            {
              setPix(299 - i_hum_forBounce[0] + i_bounce[0], leds, color_drops_forBounce[0], color_drops_forBounce[1], color_drops_forBounce[2]);
              setPix(299 - i_hum_forBounce[1] + i_bounce[1], leds, color_drops_forBounce[0], color_drops_forBounce[1], color_drops_forBounce[2]);
              setPix(300 + i_hum_forBounce[2] - i_bounce[2], leds, color_drops_forBounce[0], color_drops_forBounce[1], color_drops_forBounce[2]);
              setPix(300 + i_hum_forBounce[3] - i_bounce[3], leds, color_drops_forBounce[0], color_drops_forBounce[1], color_drops_forBounce[2]);
            }
            if(!(i_bounce[i] <= 1))
            {
              setPix(299 - i_hum_forBounce[1] + i_bounce[1] - 1, leds, 0, 0, 0);
              setPix(299 - i_hum_forBounce[2] + i_bounce[2] - 1, leds, 0, 0, 0);
              setPix(300 + i_hum_forBounce[3] - i_bounce[3] + 1, leds, 0, 0, 0);
              setPix(300 + i_hum_forBounce[4] - i_bounce[4] + 1, leds, 0, 0, 0);
            }
            flag_bounce[i] = true;
            eTime_bounce[i] = 0;
            duration_bounce[i] = duration_bounce[i] + duration_speed_bounce[i];
            duration_speed_bounce[i] = duration_speed_bounce[i] * 2;
            i_bounce[i]++;
            for(int i_array_bounce = 0; i_array_bounce < 3; i_array_bounce++)
            {
              color_drops_forBounce[i_array_bounce] = color_drops_forBounce[i_array_bounce] * 0.75;
            }

          }else if( eTime_bounce[i] >= duration_bounce[i] && i_bounce[i] == 9)
          {
            for(int j = -1; j < 2 ; j++)
            {
              setPix( j + 299 - i_hum_forBounce[1] + i_bounce[1] - 1, leds, 0, 0, 0);
            }
            for(int j = -1; j < 2 ; j++)
            {
              setPix(j + 299 - i_hum_forBounce[2] + i_bounce[2] - 1, leds, 0, 0, 0);
            }
            for(int j = -1; j < 2 ; j++)
            {
              setPix(j + 300 + i_hum_forBounce[3] - i_bounce[3] + 1, leds, 0, 0, 0);
            }
            for(int j = -1; j < 2 ; j++)
            {
              setPix(j + 300 + i_hum_forBounce[4] - i_bounce[4] + 1, leds, 0, 0, 0);
            }
            flag_initBounce[i] = false;
            i_bounce[i] = 10000;
          }else if(!(flag_initBounce[i]))
          {
            i_drops[i] = 0;
            eTime_drops[i] = 0;
            duration_dropsGravity[i] = 0.0;
            flag_drops[i] = true;
            flag_drops_start[i] = true;
            n[i] = 1;
            i_hum_forBounce[i] = 0;
            i_bounce[i] = 0;
            flag_bounce[i] = true;
            flag_initBounce[i] = true;
            flag_bounce_forDrops[i] = true;
            flag_i_hum_forBounce[i] = true;
            duration_bounce[i] = 0;
            duration_speed_bounce[i] = 0.1;
          }
        }
        else
        {
          if(duration_dropsSpeed - duration_dropsGravity[i] > 0)
          {
            double a = sqrt( n[i] );
            double b = sqrt( n[i] - 1 );
            duration_dropsGravity[i] = duration_dropsGravity[i] + ( 8 * a ) - ( 8 * b );
          }
          i_drops[i]++;
          n[i]++;
        }
        eTime_dropsSpeed[i] = 0;
        flag_dropsSpeed[i] = true;
      }
    }
  }
}