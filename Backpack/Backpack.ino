#include "FastLED.h"

FASTLED_USING_NAMESPACE


// -Mark Kriegsman, December 2014
// Gradient palette "bhw1_26_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_26.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 52 bytes of program space.

DEFINE_GRADIENT_PALETTE( pastels_gp ) {
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


// Gradient palette "bhw4_044_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw4/tn/bhw4_044.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( fire_bot_gp ) {
    0, 157, 21,  2,
   35, 229,244, 16,
   73, 255, 44,  7,
  107, 142,  7,  1,
  153, 229,244, 16,
  206, 142,  7,  1,
  255, 135, 36,  0};

  
// Gradient palette "bhw3_21_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw3/tn/bhw3_21.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.



DEFINE_GRADIENT_PALETTE( ocean_bot_gp ) {
    0,   1, 40, 98,
   48,   1, 65, 68,
   76,   2,161, 96,
  104,   0, 81, 25,
  130,  65,182, 82,
  153,   0, 86,170,
  181,  17,207,182,
  204,  17,207,182,
  255,   1, 23, 46};



#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    6
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    68
#define SIDE_LENGTH 25
#define BOT_LENGTH 18
#define SPARKING 30
#define COOLING  20
#define BUBBLING 10
#define BUBBLERISE 10
bool gReverseDirection = false;

CRGB leds[NUM_LEDS];

CRGBPalette16 currentPalette;
CRGBPalette16 gPal;
TBlendType    currentBlending;

#define BRIGHTNESS          127
#define FRAMES_PER_SECOND  120

void fire2012WithPalette (uint8_t);
void fire2012WithPaletteB (uint8_t);
void bubbles();

void setup() {
  delay(1000); // 1 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // set master brightness control

  gPal = HeatColors_p;
  
  // These are other ways to set up the color palette for the 'fire'.
  // First, a gradient from black to red to yellow to white -- similar to HeatColors_p
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
  
  // Second, this palette is like the heat colors, but blue/aqua instead of red/yellow
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);
  
  // Third, here's a simpler, three-step gradient, from black to red to white
//     gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);

  
  FastLED.setBrightness(BRIGHTNESS);
  currentPalette = pastels_gp;
  currentBlending = LINEARBLEND;
}
// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { testWave, ocean, rainbow, doubleSinelon, confetti, sineOnSine , fire};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void loop()
{
  random16_add_entropy( random());
  
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 30 ) { nextPattern(); } // change patterns periodically
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
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) 
  {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 8);
  currentPalette = pastels_gp;
  int pos = random16(NUM_LEDS);
  //leds[pos] += CHSV( gHue + random8(64), 200, 127);
  leds[pos] += ColorFromPalette( currentPalette, gHue, 255, currentBlending);
  addGlitter(20);
  //blur1d( leds, NUM_LEDS, 25);
}

void sineOnSine(){
  fadeToBlackBy( leds, NUM_LEDS, 13);
  int pos = beatsin16(3,0,SIDE_LENGTH);
  int pos2 = beatsin16( 40 , pos , (NUM_LEDS-pos));
  
  //leds[pos] += CHSV( gHue, 200, 255);
  for (int i = 0; i < pos ; i++)
  {
    leds[i] = CHSV(gHue+127,130,255); 
    leds[NUM_LEDS-i] = CHSV(gHue+127,130,255);
  }
  leds[pos2] = CHSV(gHue, 255, 255);


//  for (int i = 0; i < pos ; i++)
//  {
//    leds[i] |= CHSV(gHue+127,255,255); 
//    leds[NUM_LEDS-i] |= CHSV(gHue+127,255,127);
//  }
//  leds[pos2] = CHSV(gHue, 255, 127);
  blur1d( leds, NUM_LEDS, 31);
}

void doubleSinelon()  
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 12);
    int pos = beatsin16(14,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 255);
  leds[NUM_LEDS-pos] += CHSV( gHue, 255, 255);

}


void testWave(){
  //fadeToBlackBy ( leds, NUM_LEDS, 30);
  //int amp = beatsin16(14,0,255);
  currentPalette = pastels_gp;
  for (int i = 0; i < NUM_LEDS ; i++) 
  {
    leds[i] = CHSV (gHue + (256*i/NUM_LEDS) , 122 , beatsin16(23,64,255,(((65535/NUM_LEDS)*i)/13)));
  }
}

void ocean()
{
  //water for the bottom LEDs and bubbles up the side

  //bottom LEDS

  fadeToBlackBy ( leds, NUM_LEDS, 20);
  
  currentPalette = ocean_bot_gp;
  doublePaletteCycle();
//  doublePaletteSides( 0 );
//  doublePaletteSides( SIDE_LENGTH + BOT_LENGTH );
//  gReverseDirection = true;
//  bubbles( 0 );
//  
}

void fire()
{
  //flames flickering up the side and bright orange on the bottom

  //bottome LEDS
  fadeToBlackBy ( leds, NUM_LEDS, 20);
  
  currentPalette = fire_bot_gp;
   
  doublePaletteCycle();
  
  gReverseDirection = true;
  fire2012WithPalette ( 0 );
  gReverseDirection = false;
  fire2012WithPaletteB ( SIDE_LENGTH + BOT_LENGTH);
}

void doublePaletteCycle()
{
    for(int i = 0; i < BOT_LENGTH; i++)
  {
    leds[SIDE_LENGTH+i] |= ColorFromPalette( currentPalette, gHue + (i*12), 255, currentBlending);
    leds[NUM_LEDS-i-SIDE_LENGTH] |= ColorFromPalette( currentPalette, gHue + (i*6), 255, currentBlending);
  }
}

void doublePaletteSides( uint8_t starting)
{
    for(int i = 0; i < SIDE_LENGTH; i++)
  {
    leds[i + starting] |= ColorFromPalette( currentPalette, gHue + (i*3), 255, currentBlending);
    leds[SIDE_LENGTH - i + starting] |= ColorFromPalette( currentPalette, gHue + (i*2), 255, currentBlending);
  }
}


void fire2012WithPalette(uint8_t starting)
{
// Array of temperature readings at each simulation cell
  static byte heat[SIDE_LENGTH];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < SIDE_LENGTH; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / SIDE_LENGTH) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= SIDE_LENGTH - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < SIDE_LENGTH; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (SIDE_LENGTH-1) - j + starting;
      } else {
        pixelnumber = j+starting;
      }
      leds[pixelnumber] = color;
    }
}

void fire2012WithPaletteB(uint8_t starting)
{
// Array of temperature readings at each simulation cell
  static byte heatB[SIDE_LENGTH];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < SIDE_LENGTH; i++) {
      heatB[i] = qsub8( heatB[i],  random8(0, ((COOLING * 10) / SIDE_LENGTH) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= SIDE_LENGTH - 1; k >= 2; k--) {
      heatB[k] = (heatB[k - 1] + heatB[k - 2] + heatB[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(4);
      heatB[y] = qadd8( heatB[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < SIDE_LENGTH; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heatB[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (SIDE_LENGTH-1) - j + starting;
      } else {
        pixelnumber = j+starting;
      }
      leds[pixelnumber] = color;
    }
}

void bubbles (uint8_t starting)
{
  //array to store the state of the bubbles
  static byte risingBubbles[SIDE_LENGTH];
  //each cell gets randomized a little, bubble-ize
  
  for( int i = 0; i < SIDE_LENGTH; i++) 
  {
    risingBubbles[i] = qadd8( risingBubbles[i],  random8(0, ((BUBBLERISE * 10) / SIDE_LENGTH) + 2));
  }
 
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= SIDE_LENGTH - 1; k >= 2; k--) 
  {
    risingBubbles[k] = (risingBubbles[k - 1] + risingBubbles[k - 2] + risingBubbles[k - 2] ) / 3;
  }
 
  //randomly make new bubbles
  if( random8() < BUBBLING ) 
  {
    int y = random8(4);
    risingBubbles[y] = qadd8( risingBubbles[y], random8(64,128) );
  }

  // Step 4.  Map from heat cells to LED colors
  for( int j = 0; j < SIDE_LENGTH; j++) 
  {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8( risingBubbles[j], 240);
    CRGB color = CHSV ( 127 , 255, colorindex);
    int pixelnumber;
    if( gReverseDirection ) 
    {
      pixelnumber = (SIDE_LENGTH-1) - j + starting;
    } 
    else 
    {
      pixelnumber = j+starting;
    }
    leds[pixelnumber] = color;
  }
  blur1d( leds, SIDE_LENGTH, 25);
}

