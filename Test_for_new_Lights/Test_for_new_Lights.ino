#include <NeoPatterns.h>
#include <Wire.h>
#include <Accel.h>


#define ACCEL_INTERVAL_MS 10

// Define some NeoPatterns for the two rings and the stick
//  as well as some completion routines
void StickComplete();
void SingleComplete();
void RingComplete();


Accel accel(ACCEL_INTERVAL_MS);
NeoPatterns Stick(30, 6, NEO_GRB + NEO_KHZ800, &StickComplete);
NeoPatterns Single(1, 8, NEO_GRB + NEO_KHZ800, &SingleComplete);
NeoPatterns Ring(24, 12, NEO_GRB + NEO_KHZ800, &RingComplete);

bool DirectionalThreshold;  //whether the compass threshold can be used
bool Dance;                 //whether accelerometer is dancing hard enought to be used
bool Lighton;
unsigned long cycletime = 400;
unsigned long offtime = 30;
unsigned long lastCycle;


// Initialize everything and prepare to start
void setup()
{
  Serial.begin(115200);

   pinMode(10, INPUT_PULLUP);
   pinMode(9, INPUT_PULLUP);
    
    // Initialize all the pixelStrips
   Stick.begin();
   Single.begin();
   Ring.begin();
    //Single.ActivePattern = RAINBOW_CYCLE;
    Single.RainbowCycle(240, FORWARD);
    Single.setBrightness(50);
	  accel.begin();
    Ring.setBrightness(127);
     Stick.setBrightness(255);


    lastCycle = millis();
    //setup the stick with red
    //Stick.Scanner(Stick.Color(255,0,0), 200);
    Stick.ColorSet(Stick.Color(255,0,0));
    Ring.ColorSet(Ring.Color(0,255,0));
}

// Main loop
void loop()
{
    // Read the sensors

    // Update the rings.
    //Single.Update();    
	  accel.Update();
    //Stick.SetIndex((accel.getPhase()+PI)/(2*PI));
    //Stick.Interval=((int)((accel.getPhaseRate()+PI)/(2*PI) * 150));

    if ( millis() - lastCycle > cycletime)
    {
      lastCycle = millis();
      Ring.SetBlank(true);
      Stick.SetBlank(true);
    }
    if (millis() - lastCycle > offtime)
    {
      Ring.SetBlank(false);
      Stick.SetBlank(false);
    }
    /*
    if (accel.getPhase() > 0)
    {
      Stick.ColorSet(Stick.Color(255,0,0));
    }
    else
    {
      Stick.ColorSet(Stick.Color(0,255,255));
    }
    */
    Stick.floatIndexRate = 1.0 / Stick.TotalSteps;
    Ring.floatIndexRate = 1.0/ Ring.TotalSteps;
    if (accel.isDancing())
    {
      Single.ColorSet(Single.Wheel(random(255)));
      //Stick.ColorSet(Stick.Wheel(accel.GetCompassReading()));
      Stick.ColorSet(Stick.Color(255, 0, 0));
      if(Stick.ActivePattern != HALFUPDOWN)
      {
        Stick.HalfUpDown(Stick.Color(0,255,0),10);
        Stick.SetIndex((accel.getPhase()+PI)/(2*PI));
      }
      Stick.floatIndexRate = accel.getPhaseRatePercentage();
      Stick.Update();
    }
    else if (digitalRead(9) == LOW)
    {
      //Stick.SetBlank(true);
      if(Ring.ActivePattern != THEATER_CHASE)
      {
        Ring.TheaterChase(Ring.Color(255,0,255),Ring.Color(0,255,255),80,FORWARD);
      }
      
      if(Stick.ActivePattern != FOLLOWER)
      {
        Stick.Follower(Stick.Color(255,0,255),50,5);
      }
      Ring.Update();
      Stick.Update();
    }
    // Update the rings.
    //Single.ActivePattern = RAINBOW_CYCLE;
    else if (digitalRead(10) == LOW)
    {
      //Stick.SetBlank(false);
      if(Ring.ActivePattern != HALFUPDOWN)
      {
        Ring.HalfUpDown(random(255),150);
      }
      if(Stick.ActivePattern != DOUBLESCANNER)
      {
        Stick.DoubleScanner(random(255),30);
      }
      Ring.Update();
      Stick.Update();
    }
    else
    { 
//      Stick.SetBlank(false);
      Single.Update();
      if(Stick.ActivePattern != RUNNINGRAINBOW)
      {
        //Stick.Fade(Stick.Color(255,0,0),Stick.Color(0,255,255),120,50,FORWARD);
        Stick.RunningRainbow(Stick.Wheel(random(255)),150);
        //Stick.TheaterChase(Stick.Color(0,255,255),Stick.Color(255,0,255), 250 , FORWARD);
        //Stick.HalfUpDown(Stick.Color(0,255,255),40);
        //Stick.ColorWipe(Stick.Color(0,255,255),20,FORWARD);
        //Stick.Scanner(Stick.Color(0,255,255),40);
      }
      if(Ring.ActivePattern != RAINBOW_CYCLE)
      {
        Ring.RainbowCycle(20, FORWARD);
      }
    //Stick.Interval = 20;
      Stick.Update();
      Ring.Update();
    }
    /*
    // Switch patterns on a button press:
    if (digitalRead(8) == LOW) // Button #1 pressed
    {
        // Switch Ring1 to FASE pattern
        //Ring1.ActivePattern = FADE;
        //Ring1.Interval = 20;
        // Speed up the rainbow on Ring2
        //Ring2.Interval = 0;
        // Set stick to all red
        Stick.ColorSet(Stick.Color(255, 0, 0));
    }
    else if (digitalRead(9) == LOW) // Button #2 pressed
    {
        // Switch to alternating color wipes on Rings1 and 2
        //Ring1.ActivePattern = COLOR_WIPE;
        //Ring2.ActivePattern = COLOR_WIPE;
        //Ring2.TotalSteps = Ring2.numPixels();
        // And update tbe stick
        switch(Stick.ActivePattern)
            {
                case RAINBOW_CYCLE:
                    Stick.ActivePattern = THEATER_CHASE;
                    break;
                case THEATER_CHASE:
                    Stick.ActivePattern = COLOR_WIPE;
                    break;
                case COLOR_WIPE:
                    Stick.ActivePattern = SCANNER;
                    break;
                case SCANNER:
                    Stick.ActivePattern = FADE;
                    break;
                case FADE:
                    Stick.ActivePattern = COMPASS;
                    break;
                case COMPASS:
                    Stick.ActivePattern = RAINBOW_CYCLE;
                    break;
                default:
                    break;
            }
            //Stick.ActivePattern = Stick.ActivePattern + 1;
        Stick.Update();
    }
    else // Back to normal operation
    {
        Stick.Update();
    }    
    */
}

//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

// Stick Completion Callback
void StickComplete()
{
    // Random color change for next scan
    if (Stick.ActivePattern == HALFUPDOWN)
    {
      Stick.Color1 = Stick.Wheel(random(255));
      Stick.clear();
      Stick.show();
    }
    else
    {
      Stick.Color1 = Stick.Wheel(random(255));
    }
}

void SingleComplete()
{
    // Random color change for next scan
    //Stick.Color1 = Stick.Wheel(random(255));
}
void RingComplete()
{
  if (Stick.ActivePattern == HALFUPDOWN)
    {
      Stick.Color1 = Stick.Wheel(random(255));
      Stick.clear();
      Stick.show();
    }
    Ring.Color1 = Stick.Wheel(random(255));
}


