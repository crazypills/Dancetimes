#include <NeoPatterns.h>
#include <Wire.h>
#include <Accel.h>


#define ACCEL_INTERVAL_MS 100

// Define some NeoPatterns for the two rings and the stick
//  as well as some completion routines
void StickComplete();
void SingleComplete();

Accel accel(ACCEL_INTERVAL_MS);
NeoPatterns Stick(30, 6, NEO_GRB + NEO_KHZ800, &StickComplete);
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
      Single.ColorSet(Single.Wheel(random(255)));
      //Stick.ColorSet(Stick.Color(255, 0, 0));
<<<<<<< HEAD
      if(Stick.ActivePattern != FOLLOWER)
      {
        Stick.Follower(Stick.Color(0,255,0),40,2,2);
      }
    
      Stick.Update();
    }
    else if (digitalRead(9) == LOW)
    {
      if(Stick.ActivePattern != FOLLOWER)
      {
        Stick.Follower(Stick.Color(255,0,255),200,1,5);
      }
=======
      if (Stick.ActivePattern != FOLLOWER)
      {
        Stick.Follower(Stick.Color(0,255,0), 50, 1, 4);
      }
      Stick.Interval = 50;
>>>>>>> origin/carrino
      Stick.Update();
    }
    // Update the rings.
    //Single.ActivePattern = RAINBOW_CYCLE;
    else if (digitalRead(10) == LOW)
    {
<<<<<<< HEAD
      if(Stick.ActivePattern != HALFUPDOWN)
      {
        Stick.HalfUpDown(Stick.Color(0,255,255),2000);
      }
    
=======
      Single.ColorSet(Single.Color(0,255,255));
      if (Stick.ActivePattern != FOLLOWER)
      {
        Stick.Follower(Stick.Color(255,0,0), 100, 3, 2);
      }
>>>>>>> origin/carrino
      Stick.Update();
    }
    else
    { 
<<<<<<< HEAD
      Single.Update();
      if(Stick.ActivePattern != SCANNER)
      {
        Stick.Scanner(Stick.Color(0,255,0),70);
      }
    //Stick.Interval = 20;
=======
      Single.ColorSet(Single.Wheel(accel.GetCompassReading()));
      //Serial.print("Compass Reading: "); Serial.println(accel.GetCompassReading());       Serial.print(" ");
      if(Stick.ActivePattern != THEATER_CHASE)
      {
        Stick.TheaterChase(Stick.Color(255,0,0),Stick.Color(0,0,255), 500, FORWARD);
      }
>>>>>>> origin/carrino
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
    Stick.Color1 = Stick.Wheel(random(255));
}

void SingleComplete()
{
    // Random color change for next scan
    //Stick.Color1 = Stick.Wheel(random(255));
}
