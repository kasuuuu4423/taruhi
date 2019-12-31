#include <FastLED.h>
#define LED_PIN     0
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define NUM_LEDS    150
#define BRIGHTNESS  128
#define FRAMES_PER_SECOND 60

bool gReverseDirection = false;

CRGB leds[NUM_LEDS];

bool flag_temp = true;
bool flag_drops = true;
bool flag_hum = true;
bool flag_speed_drops = true;
bool flag_delay_drops = true;
bool flag_start_drops = true;
int i_temp = 0;
int i_temp_for_drops;
int i_drops = 0;
int i_hum = 149;
int n = 1; 
float delay_gravity_drops = 0;
long delay_drops;

unsigned long t_temp;
unsigned long t_drops;
unsigned long t_hum;
unsigned long t_speed_drops;

void loop()
{
  temp_bar(1000, 100);
  drops(1000, 2000, 100);
  hum_bar(1000, 120);
  FastLED.show();
  delay(5);
}



void setall(CRGB rin[], int r, int g, int b)
{
  for(int i = 0;i<NUM_LEDS;i++)
  {
    setPix(i, rin, r, g, b);
  }
}


void temp_bar( int delay_temp, int max_i_temp )
{
  for(int i = 0; i <= i_temp; i++)
    {
      setPix(i, leds, 0, 0, 255);
    }
  if(flag_temp)
  {
    flag_temp = false;
    t_temp = millis();
  }
  int e_time_temp = millis() - t_temp;
  if( e_time_temp >= delay_temp && i_temp <= max_i_temp)
  {
    i_temp++;
    e_time_temp = 0;
    flag_temp = true;
  }
}

void hum_bar( int delay_hum, int max_i_hum )
{
  for(int i = 149; i >= i_hum; i--)
    {
      setPix(i, leds, 255, 0, 0);
    }
  if(flag_hum == true)
  {
    flag_hum = false;
    t_hum = millis();
  }
  int e_time_hum = millis() - t_hum;
  if( e_time_hum >= delay_hum && i_hum >= max_i_hum)
  {
    i_hum--;
    e_time_hum = 0;
    flag_hum = true;
  }
}


void drops( int min_delay_drops, int max_delay_drops, float delay_speed_drops)
{
  if(flag_drops == true)
  {
    flag_drops = false;
    t_drops = millis();
    delay_drops = random(min_delay_drops, max_delay_drops);
  }

  if(flag_speed_drops == true)
  {
    flag_speed_drops = false;
    t_speed_drops = millis();
  }

  int e_time_speed_drops = millis() - t_speed_drops;
  int e_time_drops = millis() - t_drops;
  if( e_time_drops >= delay_drops )
  {
    if( e_time_speed_drops >= delay_speed_drops - delay_gravity_drops)
    {
      if(flag_start_drops)
      {
        flag_start_drops = false;
        i_temp_for_drops = i_temp + 1;
      }
      setPix(i_drops + i_temp_for_drops, leds, 0, 255, 0);
      if(!i_drops == 0)
      {
        setPix(i_drops + i_temp_for_drops - 1, leds, 0, 0, 0);
      }
      if(i_drops + i_temp_for_drops - 1 >= i_hum)
      {
        setPix(i_drops + i_temp_for_drops, leds, 0, 0, 0);
        i_drops = 0;
        delay_gravity_drops = 0;
        e_time_drops = 0;
        flag_drops = true;
        flag_start_drops = true;
        n = 1;
      }else{
        if(delay_speed_drops - delay_gravity_drops > 0){
          double a = sqrt( n );
          double b = sqrt( n - 1 );
          delay_gravity_drops = delay_gravity_drops + (8*a) - (8*b);
          Serial.println(delay_speed_drops - delay_gravity_drops);
        }
        i_drops++;
        n++;
      }
      e_time_speed_drops = 0;
      flag_speed_drops = true;
    }
  }
}



void setPix(int num,  CRGB rin[], int r, int g, int b){
  rin[num].r = r;
  rin[num].g = g;
  rin[num].b = b;
}

void setup()
{
  delay(3000); // sanity delay
  Serial.begin(115200);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  setall(leds, 0, 0, 0);
}
