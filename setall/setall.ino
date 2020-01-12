#include <FastLED.h>

#define PIN 0
#define NUM_LED 150
#define BRIGHTNESS

CRGB leds[];

int clr[3] = {255, 0, 0};

void setup() {
  FastLED.addLeds<WS2812B, PIN, GRB>(leds, NUM_LED);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.showColor(CRGB::Black);
}

void loop() {
  setall();
  FastLED.show();
}

void setPix(int num,  CRGB rin[], int r, int g, int b)
{
  rin[num].r = r;
  rin[num].g = g;
  rin[num].b = b;
}

void setall(int clr[3]){
  for(int i = 0; i < NUM_LED; i++)
  {
    setPix(i, leds, clr[0], clr[2], clr[2]);
  }  
}
