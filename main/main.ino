#include <ArduinoJson.h>
#include <FastLED.h>
  #define NUM_LED 600
  #define BRIGHTNESS 64
  #define PIN 0
#include <HTTPClient.h>
#include <HTTP_Method.h>
#include <time.h>
  #define JST 3600*9
#include <WebServer.h>
#include <WiFi.h>

const String server = "http://opendata.artful.co.jp/get/?output=json";

CRGB leds[NUM_LED];

//environment_values
String envVls;
float temp, hum, press;
int env_sep[3];
int env_length[3];
bool flag_mills = true;

unsigned long sttTime;

void setup()
{
  Serial.begin(115200);
  delay(100);
  wifi_connect("aterm-358916-g", "simizu7856");
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
    led();
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
  tm = licaltime(&t);
  return tm->tm_hour;
}

//===================led=====================//

void led()
{
  if(get_crntTime() == 17)
    {
      envVls = get_http();
      temp = get_envData(envVls, 2, "temp");
      hum = get_envData(envVls, 2, "hum");
      press = get_envData(envVls, 2, "press");
      for(int sep_i; sep_i > sizeof(env_sep); sep_i++)
      {
        switch (sep_i)
        {
        case 0:
          env_sep[sep_i] = temp / 3600;
          break;
        case 1:
          env_sep[sep_i] = hum / 3600;
          break;
        case 2:
          env_sep[sep_i] = press / 3600;
          break;
        }
      }
      delay(5000);
      if(flag_mills)
      {
        sttTime = mills();
        //初期化
        length_temp = 0;
        length_hum = 0;
        length_press = 0;
        flag_mills = false;
      }
      if(mills() - sttTime > 1000)
      {
        for(int length_i; length_i > sizeof(env_sep); length_i++)
        {
          switch (length_i)
          {
          case 0:
            env_length[length_i] += sep_temp;
            break;
          case 1:
            env_length[length_i] += sep_hum;
            break;
          case 2:
            env_length[length_i] += sep_press;
            break;
          }
        }
        length_temp += sep_temp;
        length_hum += sep_hum;
        length_press += sep_press;
      }
    }
}