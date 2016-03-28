#ifndef ACCEL_H
#define ACCEL_H

#include <Adafruit_LSM9DS0.h>
#include "quaternion.h"

class Accel
{
protected:
		Adafruit_LSM9DS0 _lsm;
		uint32_t _lastUpdateMS;
		uint32_t _intervalMS;
		float _compassAvg;
		long _compassReading;
		float _accelreadingX;
		float _accelreadingY;
		float _accelreadingZ;
		float _avgAbsAccel;
		bool _isDancing;
		void computeFht(float newValue);
		int _old_fht[128];
		float _old_phase;
		float _phase_avg;
		float _old_max_index;
		float _phaseRateAverage;
                Quaternion _q;
	public:
		Accel(uint32_t intervalMS);
		bool begin();		// Convention for this funciton is lower case...not sure why
		bool Update();
		long GetCompassReading();
		bool isDancing();
		float getPhase();
		float getPhasePercentage();
		bool directionalThreshold();
		float getPhaseRatePercentage();
};

#endif // ACCEL_H
