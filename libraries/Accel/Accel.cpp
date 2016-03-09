#include "Accel.h"

Accel::Accel(uint32_t intervalMS)
{
	_intervalMS = intervalMS;
}

bool Accel::begin()
{
    // Try to initialise and warn if we couldn't detect the chip
    if (!_lsm.begin())
    {
      Serial.println("Oops ... unable to initialize the LSM303. Check your wiring!");
      while (1);
    }
	
	return true;
}

void
Accel::Update()
{
    if ( millis() - _lastUpdateMS > _intervalMS) 
    {
      _lastUpdateMS = millis();
      _lsm.read();
	  
	  // Process the accel / compass data
      Serial.print("Accel X: "); Serial.print((int)_lsm.accelData.x); Serial.print(" ");
      Serial.print("Y: "); Serial.print((int)_lsm.accelData.y);       Serial.print(" ");
      Serial.print("Z: "); Serial.println((int)_lsm.accelData.z);     Serial.print(" ");
      Serial.print("Mag X: "); Serial.print((int)_lsm.magData.x);     Serial.print(" ");
      Serial.print("Y: "); Serial.print((int)_lsm.magData.y);         Serial.print(" ");
      Serial.print("Z: "); Serial.println((int)_lsm.magData.z);       Serial.print(" ");

      float x = _lsm.accelData.x;
      float y = _lsm.accelData.y;
      float z = _lsm.accelData.z;
      float currentAbsAccel = abs(sqrt(x*x + y*y + z*z) - 1000.0);
      _avgAbsAccel = (_avgAbsAccel * 99.0 + currentAbsAccel)/100.0;
      _isDancing = _avgAbsAccel > 100.0;

      //CompassReading will be a number between 0-255, normalized from serial inputs
      float heading = atan2(_lsm.magData.y, _lsm.magData.x);
      // Correct for when signs are reversed.
      if (heading < 0)
      { 
        heading += 2*PI;
      }
      // Check for wrap due to addition of declination.
      if (heading > 2*PI)
      {
        heading -= 2*PI;
      }
      // Convert radians to 256 scale for readability.
      //float headingDegrees = heading * 180/M_PI * (256/360); 
	  heading = ((heading * 180 * 256 ) / 360) / 3.141567;
	  Serial.print("heading256: "); Serial.println(heading);       Serial.print(" ");
      //Convert float to int
      _compassReading = (int)heading;

//      //read the direction, and see if the threshold is in a state for the transition function
//      if ( ( DirectionalThreshold != true ) && ( ( compassReading % 64) < 1 || ( ( compassReading % 64 ) > 62 ) ) )
//         {
//            DirectionalThreshold = true;
//         }
//      else if ( ( DirectionalThreshold == true) && ( ( compassReading % 64 ) > 1 && ( ( compassReading % 64 ) < 62 ) ) ) 
//         {
//            DirectionalThreshold = false;
//         }
    }
}

uint16_t Accel::GetCompassReading()
{
	return _compassReading;
}

bool Accel::isDancing()
{
	return _isDancing;
}
