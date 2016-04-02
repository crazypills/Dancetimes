#include "Accel.h"

#define LOG_OUT 1
#define LIN_OUT 0
#include <FHT.h>

#define MOVING_AVERAGE_INTERVALS 20
#define ACCEL_THRESHOLD 1
#define COMPASS_AVERAGE_INTERVALS 10
#define ACCELEROMETER_CALIBRATE 0
#define MAG_CALIBRATE_X 0.7
#define MAG_CALIBRATE_Y -0.5
#define MAG_INVERT_Z -1.0

bool Accel::begin() {
    // Try to initialise and warn if we couldn't detect the chip
    if (!_lsm.begin()) {
      Serial.println("LSM!");
      while (1);
    }
    // _lsm.setupAccel(_lsm.LSM9DS0_ACCELRANGE_4G);
    // _lsm.setupMag(_lsm.LSM9DS0_MAGGAIN_2GAUSS);
    _lsm.setupGyro(_lsm.LSM9DS0_GYROSCALE_500DPS);
        
    return true;
}

bool Accel::Update() {
    int newMillis = millis();
    if ( newMillis - _lastUpdateMS > _intervalMS + 10) {
        Serial.println("E: accel time");
    }
    if (newMillis - _lastUpdateMS < _intervalMS) {
        return false;
    }
    int elaspedMillis = newMillis - _lastUpdateMS;
    _lastUpdateMS = newMillis;
    _count++;

    sensors_event_t accelEvent;
    sensors_event_t magEvent;
    sensors_event_t gyroEvent;
    //sensors_event_t tempEvent;
    _lsm.getEvent(&accelEvent, &magEvent, &gyroEvent, NULL);

    float magx = magEvent.magnetic.x + MAG_CALIBRATE_X;
    float magy = magEvent.magnetic.y + MAG_CALIBRATE_Y;
    float magz = magEvent.magnetic.z * MAG_INVERT_Z; // For some reason z points the opposite of north.

    // Gyro is in degrees per second, so we change to rads and mult by dt to get rotation.
    float degPerSec = Quaternion(gyroEvent.gyro.x, gyroEvent.gyro.y, gyroEvent.gyro.z).norm();
    float degPerSecToRads = PI/180.0 * elaspedMillis / 1000.0;
    float gyrox = gyroEvent.gyro.x * degPerSecToRads;
    float gyroy = gyroEvent.gyro.y * degPerSecToRads;
    float gyroz = gyroEvent.gyro.z * degPerSecToRads;

    // Rotate by the gyro.
    Quaternion gyroRotation = Quaternion().from_euler_rotation(gyrox, gyroy, gyroz);
    _q *= gyroRotation;
    _q.normalize();

    //if (_count % 10 == 0) {
    //Serial.print("_q W: "); Serial.print(_q.a);
    //Serial.print(" X: "); Serial.print(_q.b);
    //Serial.print(" Y: "); Serial.print(_q.c);
    //Serial.print(" Z: "); Serial.print(_q.d);
    //Serial.print(" deg: "); Serial.print(2.0*acos(_q.a)*180/PI);
    //Serial.print(" norm: "); Serial.println(_q.norm());
    //}

    Quaternion gravity(accelEvent.acceleration.x, accelEvent.acceleration.y, accelEvent.acceleration.z);

    // Get the size of the vector before normalizing it.
    float currentAccel = gravity.norm() - SENSORS_GRAVITY_EARTH + ACCELEROMETER_CALIBRATE;
    float currentAbsAccel = abs(currentAccel);
    _avgAbsAccel = (_avgAbsAccel * (MOVING_AVERAGE_INTERVALS-1) + currentAbsAccel)/MOVING_AVERAGE_INTERVALS;
    if (elaspedMillis >= FHT_INTERVAL_MS) {
        // clear interrupts when we are doing FHT
        cli();
        computeFht(currentAccel, elaspedMillis);
        sei();
    }

    Quaternion expected_gravity = _q.conj().rotate(Quaternion(0, 0, 1));

    // Ignore gravity if it isn't around G.  We only want to update based on the accelrometer if we aren't bouncing.
    // We also want to ignore gravity if the gyro is moving a lot.
    // TODO: carrino: make these defines at the top
    if (degPerSec < 90 && currentAbsAccel < 1) {
        gravity.normalize();
        Quaternion toRotateG = gravity.rotation_between_vectors(expected_gravity);

        // We want to subtract gravity from the magnetic reading.
        // mag readings point into the earth quite a bit, but we don't really care about that.
        // We just want to use mag for rotation around the gravity axis.
        // https://en.wikipedia.org/wiki/Earth%27s_magnetic_field#Inclination
        Quaternion expected_north = _q.conj().rotate(Quaternion(1, 0, 0));
        expected_north += expected_gravity * (-expected_gravity.dot_product(expected_north));
        expected_north.normalize();

        Quaternion mag(magx, magy, magz);
        mag += expected_gravity * (-expected_gravity.dot_product(mag));
        mag.normalize();

        Quaternion toRotateMag = mag.rotation_between_vectors(expected_north);

        _q = _q * toRotateG.fractional(0.1);
        _q = _q * toRotateMag.fractional(0.05);

        // Serial.print("magRot W: "); Serial.print(toRotateMag.a);
        // Serial.print(" X: "); Serial.print(toRotateMag.b);
        // Serial.print(" Y: "); Serial.print(toRotateMag.c);
        // Serial.print(" Z: "); Serial.print(toRotateMag.d);
        // Serial.print(" deg: "); Serial.print(2.0*acos(toRotateMag.a)*180/PI);
        // Serial.print(" norm: "); Serial.println(toRotateMag.norm());

        //if (_count % 10 == 0) {
        //Serial.print("gyroRot W: "); Serial.print(gyroRotation.a);
        //Serial.print(" X: "); Serial.print(gyroRotation.b);
        //Serial.print(" Y: "); Serial.print(gyroRotation.c);
        //Serial.print(" Z: "); Serial.print(gyroRotation.d);
        //Serial.print(" deg: "); Serial.print(2.0*acos(gyroRotation.a)*180/PI);
        //Serial.print(" norm: "); Serial.println(gyroRotation.norm());

        //Serial.print("G W: "); Serial.print(gravity.a);
        //Serial.print(" X: "); Serial.print(gravity.b);
        //Serial.print(" Y: "); Serial.print(gravity.c);
        //Serial.print(" Z: "); Serial.print(gravity.d);
        //Serial.print(" norm: "); Serial.println(gravity.norm());

        //Serial.print("expectG W: "); Serial.print(expected_gravity.a);
        //Serial.print(" X: "); Serial.print(expected_gravity.b);
        //Serial.print(" Y: "); Serial.print(expected_gravity.c);
        //Serial.print(" Z: "); Serial.print(expected_gravity.d);
        //Serial.print(" norm: "); Serial.println(expected_gravity.norm());

        // Serial.print("MagNorth W: "); Serial.print(mag.a);
        // Serial.print(" X: "); Serial.print(mag.b);
        // Serial.print(" Y: "); Serial.print(mag.c);
        // Serial.print(" Z: "); Serial.print(mag.d);
        // Serial.print(" norm: "); Serial.println(mag.norm());

        // Serial.print("expectNorth W: "); Serial.print(expected_north.a);
        // Serial.print(" X: "); Serial.print(expected_north.b);
        // Serial.print(" Y: "); Serial.print(expected_north.c);
        // Serial.print(" Z: "); Serial.print(expected_north.d);
        // Serial.print(" norm: "); Serial.println(expected_north.norm());

        //Serial.print("toRotG W: "); Serial.print(toRotateG.a);
        //Serial.print(" X: "); Serial.print(toRotateG.b);
        //Serial.print(" Y: "); Serial.print(toRotateG.c);
        //Serial.print(" Z: "); Serial.print(toRotateG.d);
        //Serial.print(" deg: "); Serial.print(2.0*acos(toRotateG.a)*180/PI);
        //Serial.print(" norm: "); Serial.println(toRotateG.norm());

        //expected_gravity = (_q * toRotateG).conj().rotate(Quaternion(0, 0, 1));

        //toRotateG = gravity.rotation_between_vectors(expected_gravity);
        //Serial.print("toRotG2 W: "); Serial.print(toRotateG.a);
        //Serial.print(" X: "); Serial.print(toRotateG.b);
        //Serial.print(" Y: "); Serial.print(toRotateG.c);
        //Serial.print(" Z: "); Serial.print(toRotateG.d);
        //Serial.print(" deg: "); Serial.print(2.0*acos(toRotateG.a)*180/PI);
        //Serial.print(" norm: "); Serial.println(toRotateG.norm());

        //}

    }

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

void Accel::computeFht(float lastValue, int elaspedMillis) {
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
    //fht_mag_lin();

    //Serial.write(255); // send a start byte
    //Serial.write(fht_log_out, FHT_N/2); // send out the data

    int maxValue = 0;
    int maxIndex = 0;
    for (int i = 0; i < FHT_N/2; i++) {
        uint8_t val = fht_log_out[i];
        //int val = fht_lin_out[i];
        if (val > maxValue) {
            maxValue = val;
            maxIndex = i;
        }
        //Serial.print("index: "); Serial.print(i);
        //Serial.print(" lin: "); Serial.print(fht_lin_out[i]);

        //float realPlusImg = fht_input[i];
        //float realMinusImg = i == 0 ? realPlusImg : fht_input[FHT_N - i];
        //Serial.print(" compute: "); Serial.println(sqrt(realPlusImg * realPlusImg + realMinusImg * realMinusImg));
        //Serial.print(" val: "); Serial.print(fht_input[i]);
        //Serial.print(" val2: "); Serial.println(fht_input[FHT_N - 1 - i]);
    }

    int realPlusImg = fht_input[maxIndex];
    int realMinusImg = maxIndex == 0 ? realPlusImg : fht_input[FHT_N - maxIndex];

    float phase = atan2(realPlusImg - realMinusImg, realPlusImg + realMinusImg) + PI;

    // Add the rate to our phase even in the case where we don't update the rate.
    _phase_avg += _phaseRateAverage;

    if (maxIndex == _old_max_index && maxIndex > 1) {
        float phaseDiff = normalize_rads(phase - _old_phase);
        //Serial.print("maxIndex: "); Serial.print(maxIndex);
        //Serial.print("PhaseDiff: "); Serial.println(phaseDiff);
        //Serial.print("PhaseDBPM: "); Serial.println(phaseDiff/(2*PI) * 1000 / elaspedMillis * 60);
        //Serial.print("Phase    : "); Serial.println(phase);

        // Only update the rate if we are in the same fht bucket.
        _phaseRateAverage = (((_phaseRateAverage * 19) + phaseDiff ) / 20);

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

    // Serial.print("Phase Rate Avg: "); Serial.println(_phaseRateAverage);
    //Serial.print("Phase    : "); Serial.println(phase);
    //Serial.print("Phase Avg: "); Serial.println(_phase_avg);
    //Serial.print("rate  Avg: "); Serial.println(_phaseRateAverage);
    //Serial.print("hz    avg: "); Serial.println(_phaseRateAverage / (2.0 * PI) * 1000.0 / FHT_INTERVAL_MS);
    //Serial.print("bpm   avg: "); Serial.println(_phaseRateAverage / (2.0 * PI) * 1000.0 / FHT_INTERVAL_MS * 60);
}

void Accel::getOrientation(float &x, float &y, float &z) const {
    Quaternion vec(1, 0, 0);
    vec = _q.conj().rotate(vec);
    x = vec.b;
    y = vec.c;
    z = vec.d;
}

const float Accel::getPhasePercentage() const {
    return _phase_avg/(2*PI);
}

const bool Accel::isDancing() const {
    return  _avgAbsAccel > ACCEL_THRESHOLD;
}

const float Accel::getPhaseRatePercentage() const {
    return _phaseRateAverage/(2*PI);
}
