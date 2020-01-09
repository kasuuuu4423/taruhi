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

const char* ssid = "くさか";
const char* pass = "aaaabbbb";
float durationMin = 3;
//0=>red, 1=>green, 2=>blue
int color_temp[3] = {0, 149, 237};
int color_hum[3] = {0, 191, 255};
int color_drops[3] = {0, 255, 0};
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
      if(temp > 0)
      {
        temp = 0;
      }
      else if(temp < -10)
      {
        temp = -10;
      }
      hum = get_envData(envVls, 2, "hum");
      if(hum < 35)
      {
        hum = 35;  
      }
      else if(hum > 60)
      {
        hum = 60;  
      }
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
    drops(1000, 2000, 10, color_drops, i_hum_forDrops);
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
    //setPix(i_temp_0 + 300, leds, color[0], color[1], color[2]);
  }
 for(int i_temp_1 = 299; i_temp_1 >= 299 - length_temp; i_temp_1--)
 {
   setPix(i_temp_1, leds, color[0], color[1], color[2]);
   //setPix(i_temp_1 + 300, leds, color[0], color[1], color[2]);
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

bool flag_drops = true;
bool flag_dropsSpeed = true;
bool flag_drops_start = true;
unsigned long time_drops;
unsigned long time_dropsSpeed;
int duration_drops;
int i_temp_forDrops;
int i_drops;
int n = 1;
int i_bounce = 0;
int i_hum_forBounce;
float duration_dropsGravity = 0;
bool flag_bounce = true;
bool flag_initBounce = true;
unsigned long time_bounce;
bool flag_bounce_forDrops = true;
bool flag_i_hum_forBounce = true;



void drops(int minDuration, int maxDuration, float duration_dropsSpeed, int color[3], int i_hum)
{
  if(flag_drops)
  {
    time_drops = millis();
    duration_drops = random(minDuration, maxDuration);
    flag_drops = false;
  }
  if(flag_dropsSpeed)
  {
    time_dropsSpeed = millis();
    flag_dropsSpeed = false;
  }
  int eTime_dropsSpeed = millis() - time_dropsSpeed;
  int eTime_drops = millis() - time_drops;
  if(eTime_drops >= duration_drops)
  {
    if(eTime_dropsSpeed >= duration_dropsSpeed - duration_dropsGravity)
    {
      if(flag_drops_start)
      {
        i_temp_forDrops = (int)env_length[0] + 1;
        i_drops = 0;
        flag_drops_start = false;
      }
      if(flag_bounce_forDrops)
      {
        setPix(i_drops + i_temp_forDrops, leds, color[0], color[1], color[2]);
        setPix(299 - i_temp_forDrops - i_drops, leds, color[0], color[1], color[2]);
        Serial.println("G");
      }
      if(!i_drops == 0)
      {
        setPix(i_drops + i_temp_forDrops - 1, leds, 0, 0, 0);
        setPix(299 - i_temp_forDrops - i_drops + 1, leds, 0, 0, 0);
        
      }
      if(i_drops + i_temp_forDrops + 1 >= i_hum)
      {
        flag_bounce_forDrops = false;
        if( flag_i_hum_forBounce )
        {
          flag_i_hum_forBounce = false;
          i_hum_forBounce = i_hum;
        }
        if(flag_bounce)
        {
          flag_bounce = false;
          time_bounce = millis();
        }
        int eTime_bounce = millis() - time_bounce;
        if( eTime_bounce >= 500 && i_bounce <= 10)
        {
          if(i_bounce == 0)
          {
            //setPix(i_drops + i_temp_forDrops, leds, 0, 0, 0);
            Serial.println("あああ");
          }
          
          if(!i_bounce == 0)
          {
            //setPix(i_hum_forBounce - i_bounce + 1, leds, 0, 0, 0);
            setPix(i_hum_forBounce - i_bounce, leds, 255, 0, 255);
            Serial.println("0以外");
          }
          if(!(i_bounce <= 1))
          {
            // setPix(i_hum_forBounce - i_bounce + 1, leds, 0, 0, 0);
            Serial.println("に");
          }
          flag_bounce = true;
          eTime_bounce = 0;
          Serial.println( i_hum_forBounce - i_bounce - 1);
          Serial.println( i_hum_forBounce - i_bounce );
          Serial.println( i_bounce );
          i_bounce++;
        }else if(i_bounce == 11)
        {
          Serial.println("中間");
          // setPix(i_hum_forBounce - i_bounce + 1, leds, 0, 0, 0);
          // setPix(i_hum_forBounce - i_bounce, leds, 0, 0, 0);
          // setPix(i_hum_forBounce - i_bounce - 1, leds, 0, 0, 0);
          flag_initBounce = false;
          i_bounce = 10000;
        }else if(!(flag_initBounce))
        {
          Serial.println("初期化！！！！");
          i_drops = 0;
          eTime_drops = 0;
          duration_dropsGravity = 0;
          flag_drops = true;
          flag_drops_start = true;
          n = 1;
          i_hum_forBounce = 0;
          i_bounce = 0;
          flag_bounce = true;
          flag_initBounce = true;
          flag_bounce_forDrops = true;
          flag_i_hum_forBounce = true;
        }
      }
      else
      {
        if(duration_dropsSpeed - duration_dropsGravity > 0)
        {
          double a = sqrt( n );
          double b = sqrt( n - 1 );
          duration_dropsGravity = duration_dropsGravity + (8*a) - (8*b);
        }
        i_drops++;
        n++;
      }
      eTime_dropsSpeed = 0;
      flag_dropsSpeed = true;
    }
  }

}