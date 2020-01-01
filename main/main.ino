#include <ArduinoJson.h>
#include <FastLED.h>

///////////Configurable Value///////////
  #define NUM_LED 600
  #define BRIGHTNESS 64
  #define PIN 0
///////////Configurable Value///////////

#include <HTTPClient.h>
#include <HTTP_Method.h>
#include <time.h>
  #define JST 3600*9
#include <WebServer.h>
#include <WiFi.h>

///////////Configurable Value///////////
const char* ssid = "aterm-358916-g";
const char* pass = "simizu7856";
int dulationMin = 60;
int oclock = 17;
///////////Configurable Value///////////

const String server = "http://opendata.artful.co.jp/get/?output=json";
CRGB leds[NUM_LED];
//environment_values
String envVls;
float temp, hum, press;
//sep => separate
float env_sep[2];
float env_length[3];
bool flag_mills = true;
unsigned long sttTime;

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
    led(oclock, dulationMin);
  }
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

//(data you got with http, place{0=>place1, 1=>place2, 2=>place3}, data you want{"hum", "temp", "press"})
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
  else if(data_name == "press")
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

void led(int oclock, int dulationMin)
{
  if(get_crntTime() == oclock)
  {
    envVls = get_http();
    temp = get_envData(envVls, 2, "temp");
    if(temp > 0)
    {
      temp = 0;
    }else if(temp < -10)
    {
      temp = -10;
    }
    hum = get_envData(envVls, 2, "hum");
    press = get_envData(envVls, 2, "press");
    for(int sep_i; sep_i < sizeof(env_sep); sep_i++)
    {
      switch (sep_i)
      {
      case 0:
        env_sep[sep_i] = temp * -1 / dulationMin * 60;
        break;
      case 1:
        env_sep[sep_i] = hum / dulationMin * 60;
        break;
      }
    }
    delay(5000);
    if(flag_mills)
    {
      sttTime = millis();
      //初期化
      env_length[0] = 0;
      env_length[1] = 0;
      flag_mills = false;
    }
    if(millis() - sttTime > 1000)
    {
      for(int length_i; length_i < sizeof(env_sep); length_i++)
      {
        env_length[length_i] += map(env_sep[length_i], 0, 10, 0, 100);
      }
      flag_mills = true;
    }
    temp_bar(env_length[0]);
    hum_bar(env_length[1]);
  }
}

void setPix(int num,  CRGB rin[], int r, int g, int b)
{
  rin[num].r = r;
  rin[num].g = g;
  rin[num].b = b;
}

//===================env=====================//

void temp_bar(int length_temp)
{
  for(int i_temp = 0; i_temp <= length_temp; i_temp++)
  {
    setPix(i_temp, leds, 100, 149, 237);
    setPix(i_temp + 300, leds, 100, 149, 237);
  }
  for(int i_temp = 299; i_temp >= length_temp; i_temp--)
  {
    setPix(i_temp, leds, 100, 149, 237);
    setPix(i_temp + 300, leds, 100, 149, 237);
  }
}

void hum_bar(int length_hum)
{
  for(int i_hum = 149; i_hum >= length_hum; i_hum--)
  {
    setPix(i_hum, leds, 0, 191, 255);
    setPix(i_hum + 300, leds, 0, 191, 255);
  }
  for(int i_hum = 150; i_hum >= length_hum; i_hum++)
  {
    setPix(i_hum, leds, 0, 191, 255);
    setPix(i_hum + 300, leds, 0, 191, 255);
  }
}

//void temp_back(int delay_temp, int max_i_temp)
//{
//  for(int i = 0; i <= i_temp; i++)
//  {
//    setPix(i, leds, 0, 0, 255);
//  }
//  if(flag_temp)
//  {
//    flag_temp = false;
//    t_temp = millis();
//  }
//  int e_time_temp = millis() - t_temp;
//  if( e_time_temp >= delay_temp && i_temp <= max_i_temp)
//  {
//    i_temp++;
//    e_time_temp = 0;
//    flag_temp = true;
//  }
//}
