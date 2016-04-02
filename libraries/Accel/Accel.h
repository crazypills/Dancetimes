#ifndef ACCEL_H
#define ACCEL_H

#define FHT_N 64

#include <Adafruit_LSM9DS0.h>
#include "quaternion.h"

#ifndef FHT_INTERVAL_MS // sample rate
#define FHT_INTERVAL_MS 50
#endif

class Accel {
    protected:
        Adafruit_LSM9DS0 _lsm;
        uint32_t _lastUpdateMS;
        uint32_t _intervalMS;
        float _avgAbsAccel;
        void computeFht(float newValue, int elaspedMillis);
        int _old_fht[FHT_N];
        float _old_phase;
        float _phase_avg;
        int _old_max_index;
        float _phaseRateAverage;
        int _count;
        Quaternion _q;
    public:
        Accel(uint32_t intervalMS);
        bool begin();           // Convention for this funciton is lower case...not sure why
        bool Update();
        const bool isDancing() const;
        const float getPhasePercentage() const;
        const float getPhaseRatePercentage() const;
        void getOrientation(float &x, float &y, float &z) const;
};

#endif // ACCEL_H
