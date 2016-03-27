#include <NeoPatterns.h>
#include <Wire.h>
#include <Accel.h>


#define ACCEL_INTERVAL_MS 10

// Define some NeoPatterns for the two rings and the stick
//  as well as some completion routines
void StickComplete();
void SingleComplete();

Accel accel(ACCEL_INTERVAL_MS);
NeoPatterns Stick(1, 6, NEO_GRB + NEO_KHZ800, &StickComplete);
NeoPatterns Single(1, 8, NEO_GRB + NEO_KHZ800, &SingleComplete);

bool DirectionalThreshold;  //whether the compass threshold can be used
bool Dance;                 //whether accelerometer is dancing hard enought to be used


// Initialize everything and prepare to start
void setup()
{
  Serial.begin(115200);

   pinMode(10, INPUT_PULLUP);
   pinMode(9, INPUT_PULLUP);
    
    // Initialize all the pixelStrips
   Stick.begin();
   Single.begin();
    //Single.ActivePattern = RAINBOW_CYCLE;
	accel.begin();
    
    //setup the stick with red
    Stick.Scanner(Stick.Color(255,0,0), 200);
    Single.RainbowCycle(ACCEL_INTERVAL_MS);
}

// Main loop
void loop()
{
    // Read the sensors
    
    // Update the rings.
    Single.Update();    
	  accel.Update();

    if (accel.isDancing())
    {
      Single.SetIndex(accel.getPhasePercentage());
      Single.Update();
      //Single.ColorSet(Single.Wheel(random(255)));
      //Stick.ColorSet(Stick.Color(255, 0, 0));
      if(Stick.ActivePattern != FOLLOWER)
      {
        Stick.Follower(Stick.Color(0,255,0),40,2);
      }
    
      Stick.Update();
    }
    else if (digitalRead(9) == LOW)
    {
      if(Stick.ActivePattern != FOLLOWER)
      {
        Stick.Follower(Stick.Color(255,0,255),200,5);
      }
      Stick.Update();
    }
    // Update the rings.
    //Single.ActivePattern = RAINBOW_CYCLE;
    else if (digitalRead(10) == LOW)
    {
      if(Stick.ActivePattern != HALFUPDOWN)
      {
        Stick.HalfUpDown(Stick.Color(0,255,255),500);
      } 
    
      Stick.Update();
    }
    else
    { 
      Single.Update();
      if(Stick.ActivePattern != SCANNER)
      {
        Stick.Scanner(Stick.Color(0,255,0),70);
      }
    //Stick.Interval = 20;
      Stick.Update();
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
      Stick.setBrightness(0);
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
