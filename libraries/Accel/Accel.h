#ifndef ACCEL_H
#define ACCEL_H

#include <Adafruit_LSM303.h>

class Accel
{
protected:
		Adafruit_LSM303 _lsm;
		uint32_t _lastUpdateMS;
		uint32_t _intervalMS;
		uint16_t _compassReading;
		
	public:
		Accel(uint32_t intervalMS);
		bool begin();		// Convention for this funciton is lower case...not sure why
		void Update();
		uint16_t GetCompassReading();
};

#endif // ACCEL_H