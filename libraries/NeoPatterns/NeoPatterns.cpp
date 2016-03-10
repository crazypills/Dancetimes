#include "NeoPatterns.h"

NeoPatterns::NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
  : Adafruit_NeoPixel(pixels, pin, type)
{
  OnComplete = callback;
}

// Update the pattern
void NeoPatterns::Update()
{
  if ((millis() - lastUpdate) > Interval) // time to update
  {
    lastUpdate = millis();
    switch (ActivePattern)
    {
      case RAINBOW_CYCLE:
        RainbowCycleUpdate();
        break;
      case THEATER_CHASE:
        TheaterChaseUpdate();
        break;
      case COLOR_WIPE:
        ColorWipeUpdate();
        break;
      case SCANNER:
        ScannerUpdate();
        break;
      case FADE:
        FadeUpdate();
        break;
      case COMPASS:
        CompassUpdate();
        break;
	  case DOUBLESCANNER:
		DoubleScannerUpdate();
      	break;
	  case FOLLOWER:
	  	FollowerUpdate();
	  	break;
	  default:
        break;
    }
  }
}

// Increment the Index and reset at the end
void
NeoPatterns::Increment()
{
  if (Direction == FORWARD)
  {
    Index++;
    if (Index >= TotalSteps)
    {
      Index = 0;
      if (OnComplete != NULL)
      {
        OnComplete(); // call the comlpetion callback
      }
    }
  }
  else // Direction == REVERSE
  {
    --Index;
    if (Index <= 0)
    {
      Index = TotalSteps - 1;
      if (OnComplete != NULL)
      {
        OnComplete(); // call the comlpetion callback
      }
    }
  }
}

// Reverse pattern direction
void
NeoPatterns::Reverse()
{
  if (Direction == FORWARD)
  {
    Direction = REVERSE;
    Index = TotalSteps - 1;
  }
  else
  {
    Direction = FORWARD;
    Index = 0;
  }
}

// Initialize for a RainbowCycle
void NeoPatterns::RainbowCycle(uint8_t interval, direction dir)
{
  ActivePattern = RAINBOW_CYCLE;
  Interval = interval;
  TotalSteps = 255;
  Index = 0;
  Direction = dir;
}

// Update the Rainbow Cycle Pattern
void
NeoPatterns::RainbowCycleUpdate()
{
  for (int i = 0; i < numPixels(); i++)
  {
    setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
  }
  show();
  Increment();
}

// Initialize for a Theater Chase
void
NeoPatterns::TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir)
{
  ActivePattern = THEATER_CHASE;
  Interval = interval;
  TotalSteps = numPixels();
  Color1 = color1;
  Color2 = color2;
  Index = 0;
  Direction = dir;
}

// Update the Theater Chase Pattern
void
NeoPatterns::TheaterChaseUpdate()
{
  for (int i = 0; i < numPixels(); i++)
  {
    if ((i + Index) % 3 == 0)
    {
      setPixelColor(i, Color1);
    }
    else
    {
      setPixelColor(i, Color2);
    }
  }
  show();
  Increment();
}

// Initialize for a ColorWipe
void
NeoPatterns::ColorWipe(uint32_t color, uint8_t interval, direction dir)
{
  ActivePattern = COLOR_WIPE;
  Interval = interval;
  TotalSteps = numPixels();
  Color1 = color;
  Index = 0;
  Direction = dir;
}

// Update the Color Wipe Pattern
void
NeoPatterns::ColorWipeUpdate()
{
  setPixelColor(Index, Color1);
  show();
  Increment();
}

// Initialize for a SCANNNER
void
NeoPatterns::Scanner(uint32_t color1, uint8_t interval)
{
  ActivePattern = SCANNER;
  Interval = interval;
  TotalSteps = (numPixels() - 1) * 2;
  Color1 = color1;
  Index = 0;
}

// Update the Scanner Pattern
void
NeoPatterns::ScannerUpdate()
{
  for (int i = 0; i < numPixels(); i++)
  {
    if (i == Index)  // Scan Pixel to the right
    {
      setPixelColor(i, Color1);
    }
    else if (i == TotalSteps - Index) // Scan Pixel to the left
    {
      setPixelColor(i, Color1);
    }
    else // Fading tail
    {
      setPixelColor(i, DimColor(getPixelColor(i)));
    }
  }
  show();
  Increment();
}
// Initialize for a SCANNNER
void
NeoPatterns::DoubleScanner(uint32_t color1, uint8_t interval, uint8_t width, uint8_t followers)
{
  ActivePattern = DOUBLESCANNER;
  Interval = interval;
  TotalSteps = numPixels();
  Color1 = color1;
  Width = width;
  Followers = followers;
  Index = 0;
}

// Update the Scanner Pattern
void
NeoPatterns::DoubleScannerUpdate()
{
  for (int i = 0; i < numPixels(); i++)
  {
    if (i == Index)  // Scan Pixel to the right
    {
      setPixelColor(i, Color1);
    }
    else if (i == TotalSteps - Index) // Scan Pixel to the left
    {
      setPixelColor(i, Color1);
    }
    else // Fading tail
    {
      setPixelColor(i, DimColor(getPixelColor(i)));
    }
  }
  show();
  Increment();
}
void
NeoPatterns::Follower(uint32_t color1, uint8_t interval, uint8_t width, uint8_t followers)
{
  ActivePattern = FOLLOWER;
  Interval = interval;
  TotalSteps = numPixels();
  Color1 = color1;
  Width = width;
  Followers = followers;
  Index = 0;
}

// Update the Scanner Pattern
void
NeoPatterns::FollowerUpdate()
{
  
  for (int i = 0; i < numPixels() - 1 + Width; i++)
  {
    if (i == Index)  // Scan Pixel to the right
    {
	  for (int j = 0; j < Followers; j++)
	  {
	  	if (i + ((TotalSteps / Followers) * j) >= TotalSteps)
	  	{
		  setPixelColor(i + ((TotalSteps / Followers) * j) - TotalSteps , Color1);
	  	}
	  else
	  setPixelColor(i + ((TotalSteps / Followers) * j) , Color1);
  	  }	
	}
    /*
	else if (i == TotalSteps - Index) // Scan Pixel to the left
    {
      setPixelColor(i, Color1);
    }
	*/
    else // Fading tail
    {
      setPixelColor(i, DimColor(getPixelColor(i)));
    }
  }
  show();
  Increment();
}

// Initialize for a Fade
void
NeoPatterns::Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir)
{
  ActivePattern = FADE;
  Interval = interval;
  TotalSteps = steps;
  Color1 = color1;
  Color2 = color2;
  Index = 0;
  Direction = dir;
}

// Update the Fade Pattern
void
NeoPatterns::FadeUpdate()
{
  // Calculate linear interpolation between Color1 and Color2
  // Optimise order of operations to minimize truncation error
  uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
  uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
  uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

  ColorSet(Color(red, green, blue));
  show();
  Increment();
}

// Initialize for a Compass Function
void
NeoPatterns::Compass( uint16_t compassreading, uint16_t steps)
{
  ActivePattern = COMPASS;
  CompassReading = compassreading;
  Index = 0;
  TotalSteps = steps;
}

// Update the Fade Pattern
void
NeoPatterns::CompassUpdate()
{
  // Calculate linear interpolation between Color1 and Color2
  // Optimise order of operations to minimize truncation error
  //uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
  //uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
  //uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;

  if ( CompassReading / 4 == 0 )
  {
    red = 255;
    green = 0;
    blue = 0;

  }
  else if ( CompassReading / 4 == 1 )
  {

    red = 128;
    green = 0;
    blue = 255;
  }
  else if ( CompassReading / 4 == 2 )
  {

    red = 0;
    green = 255;
    blue = 255;
  }
  else if ( CompassReading / 4 == 3 )
  {

    red = 128;
    green = 255;
    blue = 0;
  }
  //calculate based on 0-255 what direction we are facing from compass, brightness 50%-100%
  setBrightness( ( ( (CompassReading % 64) *.5 ) + 32 ) * 4);
  ColorSet(Color(red, green, blue));
  show();
}


// Calculate 50% dimmed version of a color (used by ScannerUpdate)
uint32_t
NeoPatterns::DimColor(uint32_t color)
{
  // Shift R, G and B components one bit to the right
  uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
  return dimColor;
}

// Set all pixels to a color (synchronously)
void
NeoPatterns::ColorSet(uint32_t color)
{
  for (int i = 0; i < numPixels(); i++)
  {
    setPixelColor(i, color);
  }
  show();
}

// Returns the Red component of a 32-bit color
uint8_t
NeoPatterns::Red(uint32_t color)
{
  return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t
NeoPatterns::Green(uint32_t color)
{
  return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t
NeoPatterns::Blue(uint32_t color)
{
  return color & 0xFF;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t
NeoPatterns::Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  else
  {
    WheelPos -= 170;
    return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
