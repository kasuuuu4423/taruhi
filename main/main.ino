#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTP_Method.h>

const String server = "http://opendata.artful.co.jp/get/?output=json";

//environment_values
String envVls;

void setup()
{
  Serial.begin(115200);
  delay(100);
  wifi_connect("aterm-358916-g", "simizu7856");
}

void loop()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    envVls = get_http();
    float temp = get_envData(envVls, 2, "temp");
    Serial.println(temp);
    delay(5000);
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
