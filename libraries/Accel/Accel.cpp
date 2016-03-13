#include "Accel.h"
#define LOG_OUT 1
#include <FHT.h>

#define MOVING_AVERAGE_INTERVALS 50
#define ACCEL_THRESHOLD 300
#define COMPASS_AVERAGE_INTERVALS 100

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
    float currentCompass = 0;
    if ( millis() - _lastUpdateMS > _intervalMS * 2) {
        Serial.println("ERROR: We didn't update in time.");
    }

    if ( millis() - _lastUpdateMS > _intervalMS) 
    {
      _lastUpdateMS = millis();
      _lsm.read();
	  
	  // Process the accel / compass data
      

      float x = _lsm.accelData.x;
      float y = _lsm.accelData.y;
      float z = _lsm.accelData.z;
      float magx = _lsm.magData.x;
      float magy = _lsm.magData.y;
	  
      Serial.print("Accel X: "); Serial.print(x); Serial.print(" ");
      Serial.print("Y: "); Serial.print(y);       Serial.print(" ");
      Serial.print("Z: "); Serial.print(z);     Serial.print(" ");
      Serial.print("Mag X: "); Serial.print(magx);     Serial.print(" ");
      Serial.print("Y: "); Serial.println(magy);
	  
      float currentAbsAccel = abs(sqrt(x*x + y*y + z*z) - 1000.0);
      computeFht(currentAbsAccel);

      _avgAbsAccel = (_avgAbsAccel * (MOVING_AVERAGE_INTERVALS-1) + currentAbsAccel)/MOVING_AVERAGE_INTERVALS;
      _isDancing = _avgAbsAccel > ACCEL_THRESHOLD;
	  //Serial.print("avgAbsAccel: "); Serial.println(_avgAbsAccel);       Serial.print(" ");
	  //Serial.print("currentAbsAccel: "); Serial.println(currentAbsAccel);       Serial.print(" ");
      //CompassReading will be a number between 0-255, normalized from serial inputs
      float heading = atan2(magy, magx);
      Serial.print("heading: "); Serial.println(heading);
      Serial.print("orientation: "); Serial.println(_lsm.magData.orientation);

      // Correct for when signs are reversed.
      if (heading < 0) { 
        heading += 2*PI;
      } else if (heading >= 2*PI) {
        // Check for wrap due to addition of declination.
        heading -= 2*PI;
      }

      heading = heading * 180 / PI;
      Serial.print("headingdegrees: "); Serial.println(heading);

      // We adjust the heading before taking the moving average to handle the 360 -> 0 jump
      if (heading > _compassAvg + 180) {
          heading -= 360;
      } else if (heading < _compassAvg - 180) {
          heading += 360;
      }
      _compassAvg = (_compassAvg * (COMPASS_AVERAGE_INTERVALS - 1) + heading) / COMPASS_AVERAGE_INTERVALS;
      if (_compassAvg < 0) {
          _compassAvg += 360;
      } else if (_compassAvg >= 360) {
          _compassAvg -= 360;
      }
      Serial.print("compasAvg: "); Serial.println(_compassAvg);

      //Convert float to int
      _compassReading = (int)_compassAvg;
    }
}

void Accel::computeFht(float lastValue) {
  fht_input[_fht_index++] = lastValue;
  if (_fht_index == FHT_N) {
    _fht_index = 0;
    fht_window();
    fht_reorder();
    fht_run();
    fht_mag_log();
    // Serial.println(fht_log_out, FHT_N/2);
  }
}

long Accel::GetCompassReading()
{
	return _compassReading;
}

bool Accel::isDancing()
{
	return _isDancing;
}
