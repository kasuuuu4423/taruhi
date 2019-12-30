class Kouheki
{
  public:
    String server;
    String envVls;
    const char* ssid;
    const char* pass;
    Kouheki()
    {
      server = "http://opendata.artful.co.jp/get/?output=json";
      ssid = "scu-free";
      pass = "scu-free";
    }

//===================wifi=====================//

    void wifi_connenct()
    {
      WiFi.begin(ssid, pass);
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
      }else if(data_name == "temp")
      {
        data = object["temperature"];
      }else if(data_name == "press")
      {
        data = object["air_pressure"];
      }
      return atof(data);
    }
};