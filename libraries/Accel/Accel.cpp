#include "Accel.h"

#define LIN_OUT 0
#define LOG_OUT 1
#define FHT_N 128
#include <FHT.h>

#define MOVING_AVERAGE_INTERVALS 50
#define ACCEL_THRESHOLD 1
#define COMPASS_AVERAGE_INTERVALS 100
#define ACCELEROMETER_CALIBRATE 0

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
    _lsm.setupAccel(_lsm.LSM9DS0_ACCELRANGE_4G);
    _lsm.setupMag(_lsm.LSM9DS0_MAGGAIN_4GAUSS);
	
    return true;
}

void
Accel::Update()
{
    float currentCompass = 0;
    if ( millis() - _lastUpdateMS > _intervalMS * 2) {
        Serial.println("ERROR: We didn't update in time.");
    }
    if ( millis() - _lastUpdateMS < _intervalMS) {
        return;
    }
    _lastUpdateMS = millis();

    sensors_event_t accelEvent;
    sensors_event_t magEvent;
    sensors_event_t gyroEvent;
    sensors_event_t tempEvent;
    _lsm.getEvent(&accelEvent, &magEvent, &gyroEvent, &tempEvent);

    float x = accelEvent.acceleration.x;
    float y = accelEvent.acceleration.y;
    float z = accelEvent.acceleration.z;
    float magx = magEvent.magnetic.x;
    float magy = magEvent.magnetic.y;
    float magz = magEvent.magnetic.z;
    //Serial.print("Accel X: "); Serial.print(accelEvent.acceleration.x);     Serial.print(" ");
    //Serial.print("Y: "); Serial.print(accelEvent.acceleration.y);  Serial.print(" ");
    //Serial.print("Z: "); Serial.println(accelEvent.acceleration.z);
    //Serial.print("Mag X: "); Serial.print(magx);     Serial.print(" ");
    //Serial.print("Y: "); Serial.print(magy);  Serial.print(" ");
    //Serial.print("Z: "); Serial.println(magz);
    //Serial.print("Gyro X: "); Serial.print(gyroEvent.gyro.x);     Serial.print(" ");
    //Serial.print("Y: "); Serial.print(gyroEvent.gyro.y);  Serial.print(" ");
    //Serial.print("Z: "); Serial.println(gyroEvent.gyro.z);
    float currentAccel = sqrt(x*x + y*y + z*z) - SENSORS_GRAVITY_EARTH + ACCELEROMETER_CALIBRATE;
    float currentAbsAccel = abs(currentAccel);
    computeFht(currentAccel);

    _avgAbsAccel = (_avgAbsAccel * (MOVING_AVERAGE_INTERVALS-1) + currentAbsAccel)/MOVING_AVERAGE_INTERVALS;
    _isDancing = _avgAbsAccel > ACCEL_THRESHOLD;
    // Serial.print("avgAbsAccel: "); Serial.println(_avgAbsAccel);       Serial.print(" ");
    // Serial.print("currentAccel: "); Serial.println(currentAccel);
    // Serial.print("temp (C): "); Serial.println(tempEvent.temperature);       Serial.print(" ");

    float heading = atan2(magy, magx);

    //Serial.print("heading (rad): "); Serial.println(heading);
    //Serial.print("orientation: "); Serial.println(_lsm.magData.orientation);

    // Correct for when signs are reversed.
    if (heading < 0) { 
        heading += 2*PI;
    } else if (heading >= 2*PI) {
        // Check for wrap due to addition of declination.
        heading -= 2*PI;
    }

    heading = heading * 180 / PI;
    // Serial.print("headingdegrees: "); Serial.println(heading);

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
    // Serial.print("compasAvg: "); Serial.println(_compassAvg);

    //Convert float to int
    _compassReading = (int)_compassAvg;
}

void Accel::computeFht(float lastValue) {
    lastValue *= 4000;
    lastValue =  max(-32767, min(32767, lastValue));
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
    // Serial.print("Index: "); Serial.print(maxIndex); Serial.print("  val: "); Serial.println(max);
    int k = maxIndex;
    int negK = FHT_N - 1 - k;
    int realPlusImg = fht_input[k];
    int realMinusImg = fht_input[negK];

    //float phase = atan2((fht_input[k]	 - fht_input[negK]), (fht_input[k] + fht_input[negK]));
    float phase = atan2(realPlusImg - realMinusImg, realPlusImg + realMinusImg);
    // Serial.print("Phase: "); Serial.println(phase);
    if (maxIndex == _old_max_index && maxIndex != 0) {
        float _phaseDiff = phase - _old_phase;
        if (_phaseDiff < 0) {
            _phaseDiff += 2 * PI;
        } else if (_phaseDiff >= 2*PI) {
            _phaseDiff -= 2 * PI;
        }

        // Only update the rate if we are in the same fht bucket.
        _phaseRateAverage = (((_phaseRateAverage * 9) + _phaseDiff ) / 10);
    }
    _old_phase = phase;
    _old_max_index = maxIndex;

    _phase_avg += _phaseRateAverage;
    if (phase + PI < _phase_avg) {
        phase += 2*PI;
    } else if (phase - PI > _phase_avg) {
        phase -= 2*PI;
    }

    _phase_avg = (_phase_avg * 19 + phase) / 20;
    if (_phase_avg < -PI) {
        _phase_avg += 2*PI;
    } else if (_phase_avg >= PI) {
        _phase_avg -= 2*PI;
    }

    // Serial.print("Phase Rate Avg: "); Serial.println(_phaseRateAverage);
    // Serial.print("Phase Avg: "); Serial.println(_phase_avg);
    //for (int i = 0; i < FHT_N; i++) {
    //	  fht_input[i] = (int) old_fht[0];
    //}
}

float Accel::getPhase() {
    return _phase_avg;
}

float Accel::getPhasePercentage() {
    return (_phase_avg + PI)/(2*PI);
}

long Accel::GetCompassReading()
{
    return _compassReading;
}

bool Accel::isDancing()
{
    return _isDancing;
}

float Accel::getPhaseRatePercentage()
{
    return _phaseRateAverage/(2*PI);
}
