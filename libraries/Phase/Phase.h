#ifndef PHASE_H
#define PHASE_H

#define FHT_N 64

#include "Arduino.h"

class Phase {
    protected:
        uint32_t _lastUpdateMS;
        uint32_t _intervalMS;
        bool computeFht(float newValue, int elaspedMillis);
        int _old_fht[FHT_N];
        float _old_phase;
        float _phase_avg;
        int _old_max_index;
        float _phaseRateAverage;
    public:
        Phase(uint32_t intervalMS) { _intervalMS = intervalMS; }

        // The return value here is if this phase looped around from 2*PI to 0
        bool update(float linearAcceleration);

        // This is between 0 and 1
        float getPhasePercentage() const;

        // This is between 0 and 2*PI
        float getPhase() const;
        float getPhaseRatePercentage() const;
};

#endif // PHASE_H
