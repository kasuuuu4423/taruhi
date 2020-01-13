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
int temp_int;
int delay_temp;
int hum_int;
int delay_hum;
bool flag_mills = true;
bool flag_getHttp = true;
unsigned long sttTime;
int i_hum_forDrops;
int i;
int duration_bounce[4] = {0, 0, 0, 0};
float duration_speed_bounce [4]= {0.01, 0.01, 0.01, 0.01};
int bounce_color[8][3];
int bounce_color_mag = 1;

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
   for(int i = 0; i < 8; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      if( i == 0 || i == 7 )
      {
        bounce_color[i][j] = 0;
      }
      else
      {
        switch( j )
        {
          case 0:
            bounce_color[i][j] = 0;
            break;
          case 1:
            bounce_color[i][j] = 191 * pow(0.75, i - 1);
            break;
          case 2:
            bounce_color[i][j] = 255 * pow(0.75, i - 1);
            break;
        }
      }
    }
  }

  for(int i = 0; i < 8; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      if(j < 2)
      {
        Serial.print(bounce_color[i][j]);
        Serial.print(":");
      }
      else
        Serial.println(bounce_color[i][j]); 
    }
  }
}

void loop()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    led(Oclock);
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

void led(int oclock)
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
      // ↑ constrainで処理

      Serial.println(temp);
      temp = temp * -10 + 0.5;
      temp_int = (int)temp;
      Serial.println(temp_int);
      delay_temp = 3600000 / temp_int;
      Serial.println(delay_temp);
      Serial.println(hum);
      hum = ( hum - 34.5 ) * 2;
      hum_int = (int)hum;
      Serial.println(hum_int);
      delay_hum = 3600000 / hum_int;
      Serial.println(delay_hum);
    }
    flag_getHttp = false;
  }
    // if(flag_mills)
    // {
    //   sttTime = millis();
    //   flag_mills = false;
    // }
    // int eTime_led = millis() - sttTime;
    // if(eTime_led > 1000)
    // {
    //   for(int length_i = 0; length_i < 2; length_i++)
    //   {
    //     switch(length_i)
    //     {
    //       case 0:
    //         if(env_length[0] / 10 <= temp * -1)
    //         {
    //           env_length[0] += env_sep[0];
    //         }
    //         break;
    //       case 1:
    //         if(env_length[1] <= env_sep[1] * 60 * durationMin)
    //         {
    //           env_length[length_i] += env_sep[length_i];
    //         }
    //         break;
    //     }
    //   }
    //   flag_mills = true;
    // }
  temp_bar(temp_int, 5000, color_temp);
  hum_bar(hum_int, 5000, color_hum);
  for(i = 0; i < 4; i++)
  {
    drops(1000, 10000, 0, color_drops);
    bounce(color_drops);
  }
  
  //drops(1000, 2000, 10, color_drops, i_hum_forDrops, 3);
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

bool flag_temp = true;
int i_temp = 0;
int t_temp = 0;
int i_forTemp;

bool flag_hum = true;
int i_hum = 0;
int t_hum = 0;
int i_forHum;


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
    Serial.println("temp増");
    i_temp++;
    e_time_temp = 0;
    flag_temp = true;
  }
  if(i_forTemp <= i_temp)
  {
    setPix(i_forTemp, leds, color[0], color[1], color[2]);
    setPix(299 - i_forTemp, leds, color[0], color[1], color[2]);
    setPix(300 + i_forTemp, leds, color[0], color[1], color[2]);
    setPix(599 - i_forTemp, leds, color[0], color[1], color[2]);
    i_forTemp++;
  } 
  else
  {
    i_forTemp = 0;
  }
}

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
    setPix(149 - i_forHum, leds, color[0], color[1], color[2]);
    setPix(150 + i_forHum, leds, color[0], color[1], color[2]);
    setPix(449 - i_forHum, leds, color[0], color[1], color[2]);
    setPix(450 + i_forHum, leds, color[0], color[1], color[2]);
    i_forHum++;
  } 
  else 
  {
    i_forHum = 0;
  }
}

bool flag_drops[4] = {true, true, true, true};
bool flag_dropsSpeed[4] = {true, true, true, true};
bool flag_drops_start[4] = {true, true, true, true};
unsigned long time_drops[4];
unsigned long time_dropsSpeed[4];
int duration_drops[4];
int i_temp_forDrops[4];
int i_drops[4] = {0, 0, 0, 0};
int n[4] = {1, 1, 1, 1};
int i_bounce[4] = {0, 0, 0, 0};
int i_hum_forBounce[4];
float duration_dropsGravity[4] = {0.0, 0.0, 0.0, 0.0};
bool flag_bounce[4] = {true, true, true, true};
bool flag_initBounce[4] = {true, true, true, true};
unsigned long time_bounce[4];
bool flag_bounce_forDrops[4] = {true, true, true, true};
bool flag_i_hum_forBounce[4] = {true, true, true, true};
int color_drops_forBounce[3];

bool flag_drops_forBounce[4] = {false, false, false, false};
int eTime_dropsSpeed[4];
int eTime_drops[4];
int i_drops_forBounce[4];


void drops(int minDuration, int maxDuration, float duration_dropsSpeed, int color[3])
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
    eTime_dropsSpeed[i] = millis() - time_dropsSpeed[i];
    eTime_drops[i] = millis() - time_drops[i];
    if(eTime_drops[i] >= duration_drops[i])
    {
    //   Serial.print("あああ");
    //   Serial.println(i);
      if(eTime_dropsSpeed[i] >= duration_dropsSpeed )
      {
        eTime_dropsSpeed[i] = 0;
        flag_dropsSpeed[i] = true;
        if(flag_drops_start[i])
        {
          i_temp_forDrops[i] = i_temp + 1;
          i_drops[i] = 0;
          flag_drops_start[i] = false;
        }
        else 
        {
          i_drops[i]++;
          n[i]++;
        }
        if(!flag_drops_start[i])
        {
          if( ( i == 0 || i == 1 ) && ( 150 <= 299 - i_drops[i] - i_temp_forDrops[i] ) && ( 299 - i_drops[i] - i_temp_forDrops[i] <= 299 ) )
          {
            setPix(299 - i_drops[i] - i_temp_forDrops[i], leds, color[0], color[1], color[2]);
          //setPix(299 - i_drops[1] - i_temp_forDrops, leds, color[0], color[1], color[2]);
          //setPix(300 + i_drops[2] + i_temp_forDrops, leds, color[0], color[1], color[2]);
          //setPix(300 + i_drops[3] + i_temp_forDrops, leds, color[0], color[1], color[2]);
          //setPix(i_drops[i] + i_temp_forDrops, leds, color[0], color[1], color[2]);
          }
          else if( ( i == 2 || i == 3 ) && ( 300 <= 300 + i_drops[i] + i_temp_forDrops[i] ) && ( 300 + i_drops[i] + i_temp_forDrops[i] <= 449 ) )
          {
            setPix(300 + i_drops[i] + i_temp_forDrops[i], leds, color[0], color[1], color[2]);
          }

          if( !i_drops[i] == 0 )
          {
            if( ( i == 0 || i == 1 ) && ( 150 <= 299 - i_drops[0] - i_temp_forDrops[i] + 1 ) && ( 299 - i_drops[0] - i_temp_forDrops[i] + 1 <= 299 ) )
            {
              setPix(299 - i_drops[i] - i_temp_forDrops[i] + 1, leds, 0, 0, 0);
          //setPix(299 - i_drops[0] - i_temp_forDrops + 1, leds, 0, 0, 0);
          //setPix(299 - i_drops[1] - i_temp_forDrops + 1, leds, 0, 0, 0);
          //setPix(i_drops[2] + i_temp_forDrops + 299, leds, 0, 0, 0);
          //setPix(i_drops[3] + i_temp_forDrops + 299, leds, 0, 0, 0);
            }
            else if( ( i == 2 || i == 3 ) && ( 150 <= 299 - i_drops[0] - i_temp_forDrops[i] + 1 ) && ( 299 - i_drops[0] - i_temp_forDrops[i] + 1 <= 299 ) )
            {
              setPix(i_drops[i] + i_temp_forDrops[i] + 299, leds, 0, 0, 0);
            }
          }
          if( i_drops[i] > 148 - i_hum - i_temp_forDrops[i])
          {
          

          // flag_drops_start[i] = true;
          // flag_drops[i] = true;
          // flag_drops_forBounce[i] = true;

          
          if( i == 0 || i == 1 )
          {
            i_drops_forBounce[i] = 299 - i_drops[i] - i_temp_forDrops[i];
          } else if(i == 2 || i == 3 )
          {
            i_drops_forBounce[i] = 300 + i_drops[i] + i_temp_forDrops[i];
          }

          Serial.print(i);
          Serial.print("水滴目のバウンスは");
          Serial.println(i_drops_forBounce[i]);
          i_drops[i] = 0;
          n[i] = 0;
          eTime_drops[i] = 0;
          }
        
          // if(duration_dropsSpeed - duration_dropsGravity[i] > 0)
          // {
          //   double a = sqrt( n[i] );
          //   double b = sqrt( n[i] - 1 );
          //   duration_dropsGravity[i] = duration_dropsGravity[i] + ( 8 * a ) - ( 8 * b );
          // }

          // i_drops[i]++;
          // Serial.println(i_drops[i]);
        }
      }
    }
  
}

int eTime_bounce[4];
int i_forBounce[4] = {0, 0, 0, 0};

void bounce( int color[3] )
{
  if(flag_drops_forBounce[i])
  {
    if(flag_bounce[i])
    {
      flag_bounce[i] = false;
      time_bounce[i] = millis();
    }
    eTime_bounce[i] = millis() - time_bounce[i];
    if( eTime_bounce[i] >= duration_bounce[i] )
    {
      if( i_forBounce[i] <= 8 )
      {
        Serial.print(i);
        Serial.print("バウンス");
        if( !(i_forBounce[i] == 0) )
        {
          if( i == 0 || i == 1 )
          {
            setPix(i_drops_forBounce[i] + i_forBounce[i] , leds, color[0], color[1], color[2]);
          }
          else if( i == 2 || i == 3 )
          {
            setPix(i_drops_forBounce[i] - i_forBounce[i] , leds, color[0], color[1], color[2]);
          }
        }
        if( !(i_forBounce[i] <= 1) )
        {
          if( i == 0 || i == 1 )
          {
            setPix(i_drops_forBounce[i] + i_forBounce[i] - 1 , leds, 0, 0, 0);
          }
          else if( i == 2 || i == 3 )
          {
            setPix(i_drops_forBounce[i] - i_forBounce[i] + 1, leds, 0, 0, 0);
          }
        }
        if( i_forBounce[i] == 8 )
        {
          if( i == 0 || i == 1 )
          {
            setPix(i_drops_forBounce[i] + i_forBounce[i] , leds, 0, 0, 0);
          }
          else if( i == 2 || i == 3 )
          {
            setPix(i_drops_forBounce[i] - i_forBounce[i], leds, 0, 0, 0);
          }
        }
        flag_bounce[i] = true;
        eTime_bounce[i] = 0;
        duration_bounce[i] = duration_bounce[i] + duration_speed_bounce[i];
        duration_speed_bounce[i] = duration_speed_bounce[i] * 1.0;
        i_forBounce[i]++;
      } 
      else
      {
        flag_drops_forBounce[i] = false;
        Serial.println("抜け出した！！");
        i_forBounce[i] = 0;
        duration_bounce[i] = 0;
        duration_speed_bounce[i] = 0.01;
      }
    }
  }
  
}