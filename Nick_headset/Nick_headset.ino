#include "FastLED.h"

FASTLED_USING_NAMESPACE


// -Mark Kriegsman, December 2014
// Gradient palette "bhw1_26_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_26.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 52 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw1_26_gp ) {
    0, 107,  1,205,
   35, 255,255,255,
   73, 107,  1,205,
  107,  10,149,210,
  130, 255,255,255,
  153,  10,149,210,
  170,  27,175,119,
  198,  53,203, 56,
  207, 132,229,135,
  219, 255,255,255,
  231, 132,229,135,
  252,  53,203, 56,
  255,  53,203, 56};

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    6
//#define CLK_PIN   4
#define DATA_PIN2   12
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    20
#define NUM_LEDS2   19
CRGB leds[NUM_LEDS];
CRGB shortsLeds[NUM_LEDS2];
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

#define BRIGHTNESS          127
#define FRAMES_PER_SECOND  120

void setup() {
  delay(1000); // 1 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA_PIN2,COLOR_ORDER>(shortsLeds, NUM_LEDS2).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  currentPalette = bhw1_26_gp;
  currentBlending = LINEARBLEND;
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { testWave, doubleSinelon, rainbow, confetti, sineOnSine };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 60 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
    fill_gradient( leds, NUM_LEDS, CHSV (gHue,255,255), CHSV(gHue + 128,255,255));
    fill_gradient( shortsLeds, NUM_LEDS2, CHSV (gHue,255,255), CHSV(gHue + 128,255,255));
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(20);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
    shortsLeds[random16(NUM_LEDS2) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  fadeToBlackBy( shortsLeds, NUM_LEDS2, 10);
  int pos = random16(NUM_LEDS);
  int pos2 = random16(NUM_LEDS2);
  //leds[pos] += CHSV( gHue + random8(64), 200, 127);
  leds[pos] += ColorFromPalette( currentPalette, gHue, 63, currentBlending);
  shortsLeds[pos2] += ColorFromPalette( currentPalette, gHue, 63, currentBlending);
  addGlitter(20);
  //blur1d( leds, NUM_LEDS, 25);
}

void sineOnSine(){
  fadeToBlackBy( leds, NUM_LEDS, 10);
  fadeToBlackBy( shortsLeds, NUM_LEDS2, 7);
  int pos = beatsin16(3,0,NUM_LEDS/2);
  int pos2 = beatsin16( 40 , pos , (NUM_LEDS-pos));
  int pos3 = beatsin16(3,0,NUM_LEDS2/2);
  int pos4 = beatsin16( 40 , pos3 , (NUM_LEDS2-pos3));
  //leds[pos] += CHSV( gHue, 200, 255);
  for (int i = 0; i < pos ; i++)
  {
    leds[i] |= CHSV(gHue+127,255,255); 
    leds[NUM_LEDS-i] |= CHSV(gHue+127,255,255);
  }
//  for (int i = 0; i < pos3 ; i++)
//  {
//    shortsLeds[i] |= CHSV(gHue+127,255,255); 
//    shortsLeds[NUM_LEDS2-i] |= CHSV(gHue+127,255,255);
//  }
  leds[pos2] = CHSV(gHue, 255, 255);
//  shortsLeds[pos4] = CHSV(gHue, 255, 255);
    int pos5 = beatsin16(12,0,NUM_LEDS2);
  shortsLeds[pos5] += CHSV( gHue, 255, 255);
  shortsLeds[NUM_LEDS2-pos5] += CHSV( gHue, 255, 255);
  blur1d( leds, NUM_LEDS, 31);
//  blur1d( shortsLeds, NUM_LEDS2, 31);
}

void doubleSinelon()  
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 7);
  fadeToBlackBy( shortsLeds, NUM_LEDS2, 7);
    int pos = beatsin16(15,0,NUM_LEDS);
    int pos2 = beatsin16(12,0,NUM_LEDS2);
    //int pos = beat16(13);
  leds[pos] += CHSV( gHue, 255, 255);
  leds[NUM_LEDS-pos] += CHSV( gHue, 255, 255);
  shortsLeds[pos2] += CHSV( gHue, 255, 255);
  shortsLeds[NUM_LEDS2-pos2] += CHSV( gHue, 255, 255);
 
}


void testWave(){
  //fadeToBlackBy ( leds, NUM_LEDS, 30);
  //int amp = beatsin16(14,0,255);
  for (int i = 0; i < NUM_LEDS ; i++) {
    leds[i] = CHSV (gHue + (256*i/NUM_LEDS) , 122 , beatsin16(30,64,255,(((65535/NUM_LEDS)*i)/18)));
  }
   for (int j = 0; j < NUM_LEDS2 ; j++) {
    shortsLeds[j] = CHSV (gHue + (256*j/NUM_LEDS2) , 122 , beatsin16(50,64,255,(((65535/NUM_LEDS2)*j)/18)));
  }
}

