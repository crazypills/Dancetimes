#ifndef ACCEL_H
#define ACCEL_H

#include <Adafruit_LSM303.h>

class Accel
{
protected:
		Adafruit_LSM303 _lsm;
		uint32_t _lastUpdateMS;
		uint32_t _intervalMS;
		float _compassAvg;
		long _compassReading;
		float _accelreadingX;
		float _accelreadingY;
		float _accelreadingZ;
		float _avgAbsAccel;
		bool _isDancing;
                int _fht_index;
		void computeFht(float newValue);
		
	public:
		Accel(uint32_t intervalMS);
		bool begin();		// Convention for this funciton is lower case...not sure why
		void Update();
		long GetCompassReading();
		bool isDancing();
		bool directionalThreshold();
};

#endif // ACCEL_H
