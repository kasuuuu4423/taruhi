#include <FastLED.h>

#define PIN 0
#define NUM_LED 600
#define BRIGHTNESS 64

CRGB leds[NUM_LED];

void setup()
{
  Serial.begin(115200);
  delay(100);
  FastLED.addLeds<WS2812B, PIN, GRB>(leds, NUM_LED);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.showColor(CRGB::Black);
  Serial.println("init");
}

void loop() {
  setall();
  FastLED.show();
  delay(5);
}

void setall(){
  for(int i = 0; i < NUM_LED; i++)
  {
    setPix(i, leds, 0, 0, 255);
  }                            
}

void setPix(int num,  CRGB rin[], int r, int g, int b)
{
  rin[num].r = r;
  rin[num].g = g;
  rin[num].b = b;
}
