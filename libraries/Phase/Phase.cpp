#include "Phase.h"

#define LOG_OUT 1
#include <FHT.h>

bool Phase::update(float linearAcceleration) {
    int newMillis = millis();
    int elaspedMillis = newMillis - _lastUpdateMS;
    if (elaspedMillis > _intervalMS + 10) {
        Serial.print("E: phase time: "); Serial.println(elaspedMillis);
    }
    if (elaspedMillis < _intervalMS) {
        return false;
    }
    _lastUpdateMS = newMillis;

    // Clear interrupts when we are doing FHT.
    cli();
    {
        computeFht(linearAcceleration, elaspedMillis);
    }
    sei();

    //int lastMillis = millis();
    //Serial.print("Phase time: "); Serial.println(lastMillis - newMillis);
    return true;
}

float normalize_rads(float angle_rad) {
    if (angle_rad < 0) {
        angle_rad += 2 * PI;
    } else if (angle_rad >= 2*PI) {
        angle_rad -= 2 * PI;
    }
    return angle_rad;
}

void Phase::computeFht(float lastValue, int elaspedMillis) {
    lastValue *= 8000;
    lastValue =  max(-32767, min(32767, lastValue));
    for (int i = FHT_N - 2; i >= 0; i--) {
        _old_fht[i+1] = _old_fht[i];
        fht_input[i+1] = _old_fht[i];
    }
    fht_input[0] = (int) lastValue;
    _old_fht[0] = (int) lastValue;
    fht_window();
    fht_reorder();
    fht_run();
    fht_mag_log();

    int maxValue = 0;
    int maxIndex = 0;
    for (int i = 0; i < FHT_N/2; i++) {
        uint8_t val = fht_log_out[i];
        //int val = fht_lin_out[i];
        if (val > maxValue) {
            maxValue = val;
            maxIndex = i;
        }
    }

    int realPlusImg = fht_input[maxIndex];
    int realMinusImg = maxIndex == 0 ? realPlusImg : fht_input[FHT_N - maxIndex];

    float phase = atan2(realPlusImg - realMinusImg, realPlusImg + realMinusImg) + PI;

    // Add the rate to our phase even in the case where we don't update the rate.
    _phase_avg += _phaseRateAverage;

    if (maxIndex == _old_max_index && maxIndex > 1 && maxIndex < FHT_N/2 - 5) {
        float phaseDiff = normalize_rads(phase - _old_phase);
        //Serial.print("PhaseDiff: "); Serial.println(phaseDiff);

        // Only update the rate if we are in the same fht bucket.
        _phaseRateAverage = (((_phaseRateAverage * 9) + phaseDiff ) / 10);

        if (phase + PI < _phase_avg) {
            phase += 2*PI;
        } else if (phase - PI > _phase_avg) {
            phase -= 2*PI;
        }

        _phase_avg = (_phase_avg * 9 + phase) / 10;
        _phase_avg = normalize_rads(_phase_avg);
        //Serial.print("Phase    : "); Serial.println(phase);
        //Serial.print("Phase Avg: "); Serial.println(_phase_avg);
    }
    _old_phase = phase;
    _old_max_index = maxIndex;

    //Serial.print("bpm   avg: "); Serial.println(_phaseRateAverage / (2.0 * PI) * 1000.0 / FHT_INTERVAL_MS * 60);
}

float Phase::getPhase() const {
    return _phase_avg;
}

float Phase::getPhasePercentage() const {
    return _phase_avg/(2*PI);
}

float Phase::getPhaseRatePercentage() const {
    return _phaseRateAverage/(2*PI);
}
