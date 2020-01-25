#include <ArduinoJson.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include <HTTP_Method.h>
#include <time.h>
  #define JST 3600*9
#include <WebServer.h>
#include <WiFi.h>

//===================Configurable Values=====================//

#define NUM_LED 450
#define BRIGHTNESS 64
#define PIN 0
#define Oclock 17
#define Minute 58
int placeData = 1;
const char* ssid = "kouheki";
const char* pass = "kouheki0000";
int color_temp[3] = {65, 100, 220};
int color_hum[3] = {0, 35, 50};
int color_drops[3] = {0, 120, 200};
int duration_drops_forRandom = 3000;
float gravity_speed_first_forDrops = 10.0;
float gravity_magnification_forDrops = 1.0;
float gravity_speed_first_forBounce = 0.01;
float gravity_magnification_forBounce = 3.5;
float magnification_forAirPress[2] = {200, 4};

//===================values=====================//

const String server = "http://opendata.artful.co.jp/get/?output=json";
CRGB leds[NUM_LED];
String envVls;
float temp, hum, air_press;
int temp_int;
int delay_temp;
int hum_int;
int delay_hum;
bool flag_getHttp = true;
int i_forLoop;
bool flag_get_min = true;
float color_temp_magnification_for13[3];
float color_temp_magnification_for2[3];
float color_T_for13[3];
float color_T_for2[3];

int n = 1;
bool flag_temp = true;
int i_temp = 0;
int t_temp = 0;
int i_forTemp;
bool flag_hum = true;
int i_hum = 0;
int t_hum = 0;
int i_forHum;
int duration_drops_min_max[2] = {duration_drops_forRandom, duration_drops_forRandom * 4};
float gravity_forDrops[2] = {gravity_speed_first_forDrops, gravity_speed_first_forDrops};
bool flag_drops[2] = {true, true};
bool flag_dropsSpeed[2] = {true, true};
bool flag_drops_once[2] = {true, true};
bool flag_drops_start[2] = {true, true};
unsigned long time_drops[2];
unsigned long time_dropsSpeed[2];
int duration_drops[2];
int i_temp_forDrops[2];
int i_drops[2] = {0, 0};
int i_bounce[2] = {0, 0};
bool flag_bounce[2] = {true, true};
unsigned long time_bounce[2];
bool flag_drops_forBounce[2] = {false, false};
int eTime_dropsSpeed[2];
int eTime_drops[2];
int i_drops_forBounce[2];
int eTime_bounce[2];
int duration_bounce[2] = {0, 0};
float duration_speed_bounce[2]= {gravity_speed_first_forBounce, gravity_speed_first_forBounce};
int bounce_color[9][3];
float i_dropsGravity[2];
bool flag_bounce_once[2] = {true, true};
int i_hum_forBounce[2];

//===================setup=====================//

void setup()
{
  Serial.begin(115200);
  delay(100);
  delay(100);
  wifi_connect(ssid, pass);
  configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
  FastLED.addLeds<WS2812B, PIN, GRB>(leds, NUM_LED);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.showColor(CRGB::Black);
  Serial.println("init");

  for(int i = 0; i < 9; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      if( i == 0 || i == 8 )
      {
        bounce_color[i][j] = 0;
      }
      else
      {
        switch( j )
        {
          case 0:
            bounce_color[i][j] = color_drops[0] * pow(0.65, i - 1);
            break;
          case 1:
            bounce_color[i][j] = color_drops[1] * pow(0.65, i - 1);
            break;
          case 2:
            bounce_color[i][j] = color_drops[2] * pow(0.65, i - 1);
            break;
        }
      }
    }
  }
}

//===================loop=====================//
bool case_forWifi = true;
bool flag_forLoop = true;
unsigned int count_disconnect = 0;
void loop()
{
  if(WiFi.status() != WL_CONNECTED && flag_forLoop)
  {
    if(case_forWifi)
    {
      case_forWifi = false;
      setPix(count_disconnect, leds, 255, 0, 0);
      setPix(299 - count_disconnect, leds, 255, 0, 0);
      setPix(300 + count_disconnect, leds, 255, 0, 0);
    }
    else
    {
      case_forWifi = true;
      setPix(count_disconnect, leds, 0, 0, 0);
      setPix(299 - count_disconnect, leds, 0, 0, 0);
      setPix(300 + count_disconnect, leds, 0, 0, 0);
    }
    delay(500);
    FastLED.show();
    if(10 <= count_disconnect){
      ESP.restart();
    }
    else{
      count_disconnect++;
    }
  }
  else if(flag_forLoop)
  {
    for(int i = 0; i < NUM_LED ; i++)
    {
      setPix( i, leds, 0, 0, 0 );
    }
    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    flag_forLoop = false;
  }

  if(!flag_forLoop)
  {
    led();
    FastLED.show();
    delay(5);
  }
}

//===================wifi=====================//

void wifi_connect(const char* ssid, const char* password)
{
  WiFi.begin(ssid, password);
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

//===================HOURの取得=====================//

int get_crntTime_hour()
{
  time_t t;
  struct tm *tm;
  t = time(NULL);
  tm = localtime(&t);
  return tm->tm_hour;
}

//===================MINUTEの取得=====================//

int get_crntTime_min()
{
  time_t t;
  struct tm *tm;
  t = time(NULL);
  tm = localtime(&t);
  return tm->tm_min;
}

//===================led=====================//

void led()
{
  if(get_crntTime_hour() == Oclock )
  {
    if(flag_getHttp)
    {
      n = 0;
      envVls = get_http();
      temp = get_envData(envVls, placeData, "temp");
      temp = constrain(temp, -10, 0);
      hum = get_envData(envVls, placeData, "hum");
      hum = constrain(hum, 35, 60);
      Serial.print("気温:           ");
      Serial.println(temp);
      temp = temp * -10 + 0.5;
      temp_int = (int)temp;
      Serial.print("気温のLED数:    ");
      Serial.println(temp_int);
      if(temp_int > 0){
        delay_temp = 3600000 / temp_int;
      }
      else{
        delay_temp = 0;
      }
      Serial.print("気温のDURATION: ");
      Serial.println(delay_temp);
      for(int i = 0; i < 3; i++)
      {
        color_temp_magnification_for13[i] = color_temp[i] / 100;
        color_temp_magnification_for2[i] = color_temp[i] / 50;
      }
      Serial.print("湿度:           ");
      Serial.println(hum);
      hum = ( hum - 34.5 ) * 2;
      hum_int = (int)hum;
      Serial.print("湿度のLED数:    ");
      Serial.println(hum_int);
      if(hum_int > 0){
        delay_hum = 3600000 / hum_int;
      }
      else{
        delay_hum = 0;
      }
      Serial.print("湿度のDURATION: ");
      Serial.println(delay_hum);
      air_press = get_envData(envVls, placeData, "air_press");
      Serial.print("元の気圧:       "); 
      Serial.println(air_press);
      air_press = constrain(air_press, 990, 1020);
      Serial.print("気圧:           ");
      Serial.println(air_press);
      air_press = air_press - 990;
      air_press = air_press * magnification_forAirPress[0];
      duration_drops_forRandom = 2000 + air_press;
      duration_drops_min_max[0] = duration_drops_forRandom;
      duration_drops_min_max[1] = duration_drops_forRandom * magnification_forAirPress[1];
      Serial.print("気圧の頻度min:  ");
      Serial.println(duration_drops_forRandom);
      flag_getHttp = false;
    }
    else if(!flag_getHttp)
    {
      temp_bar(temp_int, delay_temp, color_temp);
      hum_bar(hum_int, delay_hum, color_hum);
    }
  }
  if ( get_crntTime_min() == Minute && flag_get_min && flag_getHttp )
  {
    flag_get_min = false;
    envVls = get_http();
    air_press = get_envData(envVls, 2, "air_press");
    Serial.print("元の気圧(共有):       ");
    Serial.println(air_press);
    air_press = constrain(air_press, 990, 1020);
    Serial.print("気圧(共有):           ");
    Serial.println(air_press);
    air_press = air_press - 990;
    air_press = air_press * magnification_forAirPress[0];
    duration_drops_forRandom = 2000 + air_press;
    duration_drops_min_max[0] = duration_drops_forRandom;
    duration_drops_min_max[1] = duration_drops_forRandom * magnification_forAirPress[1];
    Serial.print("気圧の頻度min(共有):  ");
    Serial.println(duration_drops_forRandom);
    Serial.println(get_crntTime_min());
  }
  if( get_crntTime_min() == Minute + 1 && !flag_get_min && flag_getHttp )
  {
    flag_get_min = true;
    Serial.println(get_crntTime_min());
  }
  for(i_forLoop = 0; i_forLoop < 2; i_forLoop++)
  {
    drops(duration_drops_min_max[0], duration_drops_min_max[1], color_drops);
    bounce();
  }
}

void setPix(int num,  CRGB rin[], int r, int g, int b)
{
  rin[num].r = r;
  rin[num].g = g;
  rin[num].b = b;
}

//===================temp_bar=====================//

void temp_bar(int length_temp, int delay_temp,  int color[3])
{
  if(flag_temp == true)
  {
    i_forTemp = 0;
    flag_temp = false;
    t_temp = millis();
  }
  int e_time_temp = millis() - t_temp;
  if( e_time_temp >= delay_temp && i_temp <= length_temp)
  {
    i_temp++;
    e_time_temp = 0;
    flag_temp = true;
  }
  if(i_forTemp <= i_temp)
  {
    for(int i = 0; i < 3; i++)
    {
      color_T_for13[i] = color_temp_magnification_for13[i] * ( i_temp - i_forTemp );
      color_T_for2[i] = color_temp_magnification_for2[i] * ( i_temp - i_forTemp + 2);
    }
    setPix(149 - i_forTemp, leds, min(color_temp[0], (int)color_T_for13[0]), min(color_temp[1], (int)color_T_for13[1]), min(color_temp[2], (int)color_T_for13[2]));
    setPix(150 + i_forTemp, leds, min(color_temp[0], (int)color_T_for2[0]), min(color_temp[1], (int)color_T_for2[1]), min(color_temp[2], (int)color_T_for2[2]));
    setPix(449 - i_forTemp, leds, min(color_temp[0], (int)color_T_for13[0]), min(color_temp[1], (int)color_T_for13[1]), min(color_temp[2], (int)color_T_for13[2]));
    i_forTemp++;
  } 
  else
  {
    i_forTemp = 0;
  }
}

//===================hum_bar=====================//

void hum_bar(int length_hum, int delay_hum, int color[3])
{
  if(flag_hum == true)
  {
    i_forHum = 0;
    flag_hum = false;
    t_hum = millis();
  }
  int e_time_hum = millis() - t_hum;
  if( e_time_hum >= delay_hum && i_hum <= length_hum)
  {
    i_hum++;
    e_time_hum = 0;
    flag_hum = true;
  }
  if(i_forHum <= i_hum)
  {
    setPix(i_forHum, leds, color[0], color[1], color[2]);
    setPix(299 - i_forHum, leds, color[0], color[1], color[2]);
    setPix(300 + i_forHum, leds, color[0], color[1], color[2]);
    i_forHum++;
  }
  else 
  {
    i_forHum = 0;
  }
}

//===================drops=====================//

void drops(int minDuration, int maxDuration, int color[3])
{
  if(flag_drops[i_forLoop])
  {
    time_drops[i_forLoop] = millis();
    duration_drops[i_forLoop] = random(minDuration, maxDuration);
    flag_drops[i_forLoop] = false;
  }
  if(flag_dropsSpeed[i_forLoop])
  {
    time_dropsSpeed[i_forLoop] = millis();
    flag_dropsSpeed[i_forLoop] = false;
  }
  eTime_dropsSpeed[i_forLoop] = millis() - time_dropsSpeed[i_forLoop];
  eTime_drops[i_forLoop] = millis() - time_drops[i_forLoop];
  if(eTime_drops[i_forLoop] >= duration_drops[i_forLoop])
  {
    if(eTime_dropsSpeed[i_forLoop] >= gravity_forDrops[i_forLoop] )
    {
      eTime_dropsSpeed[i_forLoop] = 0;
      flag_dropsSpeed[i_forLoop] = true;
      if(flag_drops_once[i_forLoop])
      {
        i_temp_forDrops[i_forLoop] = i_temp + 1;
        i_drops[i_forLoop] = 0;
        flag_drops_once[i_forLoop] = false;
      }
      else
      {
        if(gravity_forDrops[i_forLoop] > 0)
        {
          gravity_forDrops[i_forLoop] = gravity_forDrops[i_forLoop] - gravity_magnification_forDrops;
        }
      }
      if(flag_drops_start[i_forLoop] && !flag_drops_once[i_forLoop] )
      {
        if(i_drops[i_forLoop] <= 148 - i_hum - i_temp_forDrops[i_forLoop] + 2 * n)
        {
          setPix(150 - n + i_drops[i_forLoop] + i_temp_forDrops[i_forLoop], leds, color[0], color[1], color[2]);
        }
        if( ( !(i_drops[i_forLoop] == 0) ) && i_drops[i_forLoop] <= 148 - i_hum - i_temp_forDrops[i_forLoop] + 2 * n)
        {
          setPix(150 - n + i_drops[i_forLoop] + i_temp_forDrops[i_forLoop] - 1, leds, 0, 0, 0);
        }
        if( ( !(i_drops[i_forLoop] <= 1) ) && i_drops[i_forLoop] <= 148 - i_hum - i_temp_forDrops[i_forLoop] + 2 * n)
        {
          setPix(150 - n + i_drops[i_forLoop] + i_temp_forDrops[i_forLoop] - 2, leds, 0, 0, 0);
        }
        if( ( !(i_drops[i_forLoop] <= 2) ) && i_drops[i_forLoop] <= 148 - i_hum - i_temp_forDrops[i_forLoop] + 2 * n)
        {
          setPix(150 - n + i_drops[i_forLoop] + i_temp_forDrops[i_forLoop] - 3, leds, 0, 0, 0);
        }
        if( i_drops[i_forLoop] > 148 - i_hum - i_temp_forDrops[i_forLoop] + 2 * n )
        {
          flag_drops_forBounce[i_forLoop] = true;
          flag_drops_start[i_forLoop] = false;
          i_drops[i_forLoop] = 0;
          i_dropsGravity[i_forLoop] = 0;
          eTime_drops[i_forLoop] = 0;
        }
      }
      i_dropsGravity[i_forLoop] = i_dropsGravity[i_forLoop] + 0.02;
      i_drops[i_forLoop] = i_drops[i_forLoop] + 1 + i_dropsGravity[i_forLoop];
    }
  }
}

//===================bounce=====================//

void bounce()
{
  if(flag_drops_forBounce[i_forLoop])
  {
    if(flag_bounce_once[i_forLoop])
    {
      flag_bounce_once[i_forLoop] = false;
      i_hum_forBounce[i_forLoop] = i_hum - n;
    }
    if(flag_bounce[i_forLoop])
    {
      flag_bounce[i_forLoop] = false;
      time_bounce[i_forLoop] = millis();
    }
    eTime_bounce[i_forLoop] = millis() - time_bounce[i_forLoop];
    if( eTime_bounce[i_forLoop] >= duration_bounce[i_forLoop] )
    {
      if( i_bounce[i_forLoop] <= 9 )
      {
        if( !(i_bounce[i_forLoop] == 0) )
        {
          setPix( 299 - i_hum_forBounce[i_forLoop] - i_bounce[i_forLoop] , leds, bounce_color[i_bounce[i_forLoop]][0], bounce_color[i_bounce[i_forLoop]][1], bounce_color[i_bounce[i_forLoop]][2]);
        }
        if( !(i_bounce[i_forLoop] <= 1) )
        {
          setPix(299 - i_hum_forBounce[i_forLoop] - i_bounce[i_forLoop] + 1 , leds, 0, 0, 0);
        }
        if( i_bounce[i_forLoop] == 9 )
        {
          setPix(299 - i_hum_forBounce[i_forLoop] - i_bounce[i_forLoop] , leds, 0, 0, 0);
        }
        flag_bounce[i_forLoop] = true;
        eTime_bounce[i_forLoop] = 0;
        duration_bounce[i_forLoop] = duration_bounce[i_forLoop] + duration_speed_bounce[i_forLoop];
        duration_speed_bounce[i_forLoop] = duration_speed_bounce[i_forLoop] * gravity_magnification_forBounce;
        i_bounce[i_forLoop]++;
      }
      else
      {
        flag_drops_start[i_forLoop] = true;
        flag_drops_start[i_forLoop] = true;
        flag_drops[i_forLoop] = true;
        flag_drops_forBounce[i_forLoop] = false;
        flag_drops_once[i_forLoop] = true;
        i_bounce[i_forLoop] = 0;
        duration_bounce[i_forLoop] = 0;
        duration_speed_bounce[i_forLoop] = gravity_speed_first_forBounce;
        gravity_forDrops[i_forLoop] = gravity_speed_first_forDrops;
        flag_bounce_once[i_forLoop] = true;
      }
    }
  }
}
