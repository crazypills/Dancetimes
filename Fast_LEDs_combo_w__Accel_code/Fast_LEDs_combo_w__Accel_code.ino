#include <Wire.h>
#include <Accel.h>
#include <Phase.h>
#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define ACCEL_INTERVAL_MS 50
#define PHASE_INTERVAL_MS 50
Accel accel(ACCEL_INTERVAL_MS);
Phase phase(PHASE_INTERVAL_MS);
#if FASTLED_VERSION < 3001000

#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    6
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    30
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          127
#define FRAMES_PER_SECOND  120
bool DirectionalThreshold;  //whether the compass threshold can be used
bool Dance;                 //whether accelerometer is dancing hard enought to be used
unsigned long Interval;
unsigned long LastUpdate;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
   accel.begin();
   LastUpdate = millis();
   Interval = (1000/FRAMES_PER_SECOND);
   pinMode(10, INPUT_PULLUP);
   pinMode(9, INPUT_PULLUP);
     // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  //FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  
}
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void loop() {
  // put your main code here, to run repeatedly:

  
   bool didUpdate = accel.Update();
    phase.update(accel.getLinearAcceleration());

    EVERY_N_MILLISECONDS ( 30 )
    {
      gHue++;
    }
    if (millis() - LastUpdate > Interval)
    {
        LastUpdate = millis();
        FastLED.show();
        if (accel.isDancing()) {

            computed_bpm(phase.getPhasePercentage() * 256);
         }
        
         else if (digitalRead(9) == LOW) {

            sinelon();
            
        }

        else if (digitalRead(10) == LOW)
        {
            confetti();
        }
        else
        { 
          //Quaternion device = accel.getDeviceOrientation(Quaternion(1, 0, 0));
          Quaternion device = accel.getAbsoluteOrientation(Quaternion(1, 0, 0));
          float x = device.b < 0 ? 0 : device.b;
          float y = device.c < 0 ? 0 : device.c;
          float z = device.d < 0 ? 0 : device.d;

            rgb(x*64, y*64, z*64);
        }
    }
}




#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  //fill_rainbow( leds, NUM_LEDS, gHue, 7);
  fill_gradient( leds, NUM_LEDS, CHSV (gHue,255,127), CHSV(gHue + 128,255,127));
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()  
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 10);
    int pos = beatsin16(15,0,NUM_LEDS);
    //int pos = beat16(13);
  leds[pos] += CHSV( gHue, 255, 192);
  leds[NUM_LEDS-pos] += CHSV( gHue, 255, 192);
  
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void computed_bpm(uint8_t beat)
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  //CRGBPalette16 palette = PartyColors_p;
  CRGBPalette16 palette = CloudColors_p;
  //uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void rgb(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB (r, g, b);
  }
}

void juggle() {
//  // eight colored dots, weaving in and out of sync with each other
//  fadeToBlackBy( leds, NUM_LEDS, 20);
//  byte dothue = 0;
//  for( int i = 0; i < 8; i++) {
//    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
//    dothue += 32;
//  }
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CHSV (gHue, 255,127);
  }
}

