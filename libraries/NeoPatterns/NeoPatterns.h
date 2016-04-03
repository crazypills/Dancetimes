	#ifndef ADAFRUIT_NEOPATTERN_H
#define ADAFRUIT_NEOPATTERN_H
#define ENDPAUSE 5000

#include <Adafruit_NeoPixel.h>

// Pattern types supported:

enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE , COMPASS, DOUBLESCANNER, FOLLOWER, HALFUPDOWN, TWINKLE, RUNNING, RUNNINGRAINBOW};
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

//class sensorReader : public 
//
//  For Flora input pins, 8 is for built in neoxpixel on board
//  SCL and SDA, probably used for sensors, 
//  D9,D10 to be used for neopixels, serial out
//  D6, D12 to be used as toggle inputs for testing
//
//
// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
    public:
    // Member Variables:  
    pattern ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
   
    unsigned long Interval;   // milliseconds between updates
    unsigned long LastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
	uint8_t Followers;  //how many followers are in each pass
    uint16_t CompassReading;  //reading from the compass function
	//uint8_t Flicker = 3;
	//uint8_t Brightness;
	uint8_t Sparkles;
	uint32_t *LastColors;
	bool BlankState;
	
	float floatIndex;
	float floatIndexRate;
	
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)());
    // Update the pattern
    void Update();
	// Blank turns off the lights and prevents them from making progress
	void SetBlank(bool blank);
    // Increment the Index and reset at the end
    void Increment(); 
    // Reverse pattern direction
    void Reverse();
    // Initialize for a RainbowCycle
    void RainbowCycle(uint32_t interval, direction dir = FORWARD);
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate();
    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint32_t interval, direction dir = FORWARD);
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate();
    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint32_t interval, direction dir = FORWARD);
    // Update the Color Wipe Pattern
    void ColorWipeUpdate();
    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint32_t interval);
    // Update the Scanner Pattern
    void ScannerUpdate();
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint32_t interval, direction dir = FORWARD);
    // Update the Fade Pattern
    void FadeUpdate();
	// Initialize for a DoubleScanner
	void DoubleScanner(uint32_t color1, uint32_t interval);
	// Update the Double Scanner Pattern
	void DoubleScannerUpdate();
	// Initialize for a Follower
	void Follower(uint32_t color1, uint32_t interval, uint8_t followers);
	// Update the Follower pattern
	void FollowerUpdate();
	// Initialize a HalfUpDown 
	void HalfUpDown(uint32_t color1, uint32_t interval);
	// Update the HalfUpDown Pattern
	void HalfUpDownUpdate();
	// Initialize a Running
	void Running(uint32_t color1, uint32_t interval);
	// Update the Running Pattern
	void RunningUpdate();
	// Initialize a Running Rainbow
	void RunningRainbow(uint32_t color1, uint32_t interval);
	// Update the Running Rainbow
	void RunningRainbowUpdate();
	// Initialize a Twinkle
	void Twinkle(uint8_t sparkles, uint32_t interval);
	// Update the Twinkle Pattern
	void TwinkleUpdate();
    // Initialize for a Compass Function
	void Compass(uint16_t compassReading, uint16_t steps);
    void SetCompassReading( uint16_t compassReading);
    // Update the Fade Pattern
    void CompassUpdate();
	// this is between 0 and 1
	void SetIndex(float percentage, float percentageRate);
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color);
    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color);
    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color);
	// Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color);
    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color);
	uint32_t setRed(uint8_t red);
	uint32_t setGreen(uint8_t green);
	uint32_t setBlue(uint8_t blue);
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos);
	// Store the state of the LED strip
	void StoreLights();
	// Return the lights to the previous state
	void RestoreLights();
	
	void setPixelColor(uint16_t n, uint32_t c);
    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
	
};

#endif // ADAFRUIT_NEOPATTERN_H
