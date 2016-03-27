#include "Accel.h"


#define LIN_OUT 0
#define LOG_OUT 1
#define FHT_N 256
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
        //Serial.println("ERROR: We didn't update in time.");
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
	  float magz = _lsm.magData.z;
	  /*
      Serial.print("Accel X: "); Serial.print(x); Serial.print(" ");
      Serial.print("Y: "); Serial.print(y);       Serial.print(" ");
      Serial.print("Z: "); Serial.print(z);     Serial.print(" ");
      Serial.print("Mag X: "); Serial.print(magx);     Serial.print(" ");
      Serial.print("Y: "); Serial.print(magy);  Serial.print(" ");
	  Serial.print("Z: "); Serial.println(magz);
	  */
	  float currentAccel = sqrt(x*x + y*y + z*z) - 1000.0;
      float currentAbsAccel = abs(currentAccel);
      computeFht(currentAccel);

      _avgAbsAccel = (_avgAbsAccel * (MOVING_AVERAGE_INTERVALS-1) + currentAbsAccel)/MOVING_AVERAGE_INTERVALS;
      _isDancing = _avgAbsAccel > ACCEL_THRESHOLD;
	  //Serial.print("avgAbsAccel: "); Serial.println(_avgAbsAccel);       Serial.print(" ");
	  //Serial.print("currentAbsAccel: "); Serial.println(currentAbsAccel);       Serial.print(" ");
      //CompassReading will be a number between 0-255, normalized from serial inputs
	  //Serial.print("Gyro X: "); Serial.print(_lsm.gyroData.x);   Serial.print(" ");
	  //Serial.print("Y: "); Serial.print(_lsm.gyroData.y);        Serial.print(" ");
	  //Serial.print("Z: "); Serial.println(_lsm.gyroData.z);      Serial.println(" ");
      float heading = atan2(_lsm.magData.y, _lsm.magData.x);
      
	  
	  //Serial.print("heading: "); Serial.println(heading);
      //Serial.print("orientation: "); Serial.println(_lsm.magData.orientation);

      // Correct for when signs are reversed.
      if (heading < 0) { 
        heading += 2*PI;
      } else if (heading >= 2*PI) {
        // Check for wrap due to addition of declination.
        heading -= 2*PI;
      }

      heading = heading * 180 / PI;
      //Serial.print("headingdegrees: "); Serial.println(heading);

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
      //Serial.print("compasAvg: "); Serial.println(_compassAvg);

      //Convert float to int
      _compassReading = (int)_compassAvg;
	  Serial.print("Compass Reading:  "); Serial.println(_compassReading);
    }
}

void Accel::computeFht(float lastValue) {
  for (int i = 0; i < FHT_N - 1; i++) {
    _old_fht[i] = _old_fht[i + 1];
	fht_input[i] = _old_fht[i];
  }
  fht_input[FHT_N - 1] = (int) lastValue;
  _old_fht[FHT_N - 1] = (int) lastValue;
  fht_window();
  fht_reorder();
  fht_run();
  fht_mag_log();
  //fht_mag_lin();
      int max = 0;
      int maxIndex = 0;
      for (int i = 0; i < FHT_N/2; i++)
      {
		  uint8_t val = fht_log_out[i];
		  //uint16_t val = fht_lin_out[i];
        //Serial.print("Index: "); Serial.print(i); Serial.print("  val: "); Serial.println(val);
      	if (val > max)
      	{
      		max = val;
      		maxIndex = i;
      	}
      }
	  int k = maxIndex;
	  int negK = k == 0 ? 0 : FHT_N - k;
      //Serial.println(maxIndex);
	  //float phase = atan2((fht_input[k]	 - fht_input[negK]), (fht_input[k] + fht_input[negK]));
	  float phase = atan2((fht_input[negK]), (fht_input[k]));
	  //Serial.println(phase);
	  float _phaseDiff = phase - _phase;
	  if (_phaseDiff < -PI) {
		  _phaseDiff += 2 * PI;
	  } else if (_phaseDiff > PI) {
		  _phaseDiff -= 2 * PI;
	  }
	  _phaseRateAverage = (((_phaseRateAverage * 19) + _phaseDiff ) / 20);
	  _phase = phase;
      //for (int i = 0; i < FHT_N; i++) {
	//	  fht_input[i] = (int) old_fht[0];
	  //}
}

float Accel::getPhase()
{
	return _phase;
}

long Accel::GetCompassReading()
{
	return _compassReading;
}

bool Accel::isDancing()
{
	//return _isDancing;
	return false;
}

float Accel::getPhaseRate()
{
	return _phaseRateAverage;
}
