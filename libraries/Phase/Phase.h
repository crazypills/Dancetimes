#ifndef PHASE_H
#define PHASE_H

#define FHT_N 64

#include "Arduino.h"

class Phase {
    protected:
        uint32_t _lastUpdateMS;
        uint32_t _intervalMS;
        void computeFht(float newValue, int elaspedMillis);
        int _old_fht[FHT_N];
        float _old_phase;
        float _phase_avg;
        int _old_max_index;
        float _phaseRateAverage;
    public:
        Phase(uint32_t intervalMS) { _intervalMS = intervalMS; }
        bool update(float linearAcceleration);
        float getPhasePercentage() const;
        float getPhaseRatePercentage() const;
};

#endif // PHASE_H
