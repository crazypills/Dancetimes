#include "Accel.h"

#define LOG_OUT 1
#include <FHT.h>

#define MOVING_AVERAGE_INTERVALS 20
#define ACCEL_THRESHOLD 1
#define COMPASS_AVERAGE_INTERVALS 10
#define ACCELEROMETER_CALIBRATE 0
#define MAG_CALIBRATE_X 0.7
#define MAG_CALIBRATE_Y -0.5

Accel::Accel(uint32_t intervalMS)
{
	_intervalMS = intervalMS;
}

bool Accel::begin()
{
    // Try to initialise and warn if we couldn't detect the chip
    if (!_lsm.begin())
    {
      Serial.println("Oops ... unable to initialize the LSM. Check your wiring!");
      while (1);
    }
    _lsm.setupAccel(_lsm.LSM9DS0_ACCELRANGE_4G);
    _lsm.setupMag(_lsm.LSM9DS0_MAGGAIN_4GAUSS);
    _lsm.setupGyro(_lsm.LSM9DS0_GYROSCALE_500DPS);
	
    return true;
}

bool
Accel::Update()
{
    float currentCompass = 0;
    if ( millis() - _lastUpdateMS > _intervalMS + 10) {
        Serial.println("ERROR: We didn't update accel in time.");
    }
    if ( millis() - _lastUpdateMS < _intervalMS) {
        return false;
    }
    _lastUpdateMS = millis();

    sensors_event_t accelEvent;
    sensors_event_t magEvent;
    sensors_event_t gyroEvent;
    //sensors_event_t tempEvent;
    _lsm.getEvent(&accelEvent, &magEvent, &gyroEvent, NULL);

    float x = accelEvent.acceleration.x;
    float y = accelEvent.acceleration.y;
    float z = accelEvent.acceleration.z;
    float magx = magEvent.magnetic.x + MAG_CALIBRATE_X;
    float magy = magEvent.magnetic.y + MAG_CALIBRATE_Y;
    float magz = magEvent.magnetic.z;
    float gyrox = gyroEvent.gyro.x * PI/180.0 * ACCEL_INTERVAL_MS / 1000;
    float gyroy = gyroEvent.gyro.y * PI/180.0 * ACCEL_INTERVAL_MS / 1000;
    float gyroz = gyroEvent.gyro.z * PI/180.0 * ACCEL_INTERVAL_MS / 1000;
    // Serial.print("Accel X: "); Serial.print(accelEvent.acceleration.x);     Serial.print(" ");
    // Serial.print("Y: "); Serial.print(accelEvent.acceleration.y);  Serial.print(" ");
    // Serial.print("Z: "); Serial.println(accelEvent.acceleration.z);
    // Serial.print("Mag X: "); Serial.print(magx);     Serial.print(" ");
    // Serial.print("Y: "); Serial.print(magy);  Serial.print(" ");
    // Serial.print("Z: "); Serial.println(magz);
    // Serial.print("Gyro X: "); Serial.print(gyrox);     Serial.print(" ");
    // Serial.print("Y: "); Serial.print(gyroy);  Serial.print(" ");
    // Serial.print("Z: "); Serial.println(gyroz);

    // Quaternion rotate;
    // rotate.from_euler_rotation(gyrox, gyroy, gyroz);
    _q = Quaternion().from_euler_rotation(gyrox, gyroy, gyroz).conj() * _q;
    Quaternion gravity(x, y, z);
    gravity.normalize();

    // Serial.print("_q W: "); Serial.print(_q.a);
    // Serial.print(" X: "); Serial.print(_q.b);
    // Serial.print(" Y: "); Serial.print(_q.c);
    // Serial.print(" Z: "); Serial.println(_q.d);

    Quaternion expected_gravity(0, 0, 1);
    expected_gravity = _q.rotate(expected_gravity);
    Serial.print("expected W: "); Serial.print(expected_gravity.a);
    Serial.print(" X: "); Serial.print(expected_gravity.b);
    Serial.print(" Y: "); Serial.print(expected_gravity.c);
    Serial.print(" Z: "); Serial.println(expected_gravity.d);

    Serial.print("gravity W: "); Serial.print(gravity.a);
    Serial.print(" X: "); Serial.print(gravity.b);
    Serial.print(" Y: "); Serial.print(gravity.c);
    Serial.print(" Z: "); Serial.println(gravity.d);

    Quaternion toRotate = expected_gravity.rotation_between_vectors(gravity);
    // Serial.print("toRot W: "); Serial.print(toRotate.a);
    // Serial.print(" X: "); Serial.print(toRotate.b);
    // Serial.print(" Y: "); Serial.print(toRotate.c);
    // Serial.print(" Z: "); Serial.println(toRotate.d);

    //expected_gravity = toRotate.frational(0.01).rotate(expected_gravity);
    //expected_gravity = (toRotate.frational(0.01) * _q).rotate(Quaternion(0, 0, 1));

    // Serial.print("afterRot W: "); Serial.print(expected_gravity.a);
    // Serial.print(" X: "); Serial.print(expected_gravity.b);
    // Serial.print(" Y: "); Serial.print(expected_gravity.c);
    // Serial.print(" Z: "); Serial.println(expected_gravity.d);

    //_q = toRotate.frational(0.1) * _q;

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
    //Serial.print("compasAvg: "); Serial.println(_compassAvg);

    //Convert float to int
    _compassReading = (int)_compassAvg;

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
        float _phaseDiff = normalize_rads(phase - _old_phase);

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
    _phase_avg = normalize_rads(_phase_avg + PI) - PI;

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
