#include "Accel.h"

#define MOVING_AVERAGE_INTERVALS 20
#define ACCEL_THRESHOLD 1
#define COMPASS_ROTATE_FRACTION 0.05
#define ACCELEROMETER_FRACTION 0.1
#define ACCELEROMETER_CALIBRATE 0
#define MAG_CALIBRATE_X 0.7
#define MAG_CALIBRATE_Y -0.5
#define MAG_INVERT_Z -1.0

#define USE_ACCELEROMETER_THRESHOLD_M_PER_S2 1
#define USE_ACCELEROMETER_THRESHOLD_DEG_PER_S 90

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
    int elaspedMillis = newMillis - _lastUpdateMS;
    if ( elaspedMillis > _intervalMS + 10) {
        Serial.print("E: accel time: "); Serial.println(elaspedMillis);
    }
    if (elaspedMillis < _intervalMS) {
        return false;
    }
    _lastUpdateMS = newMillis;
    _count++;

    sensors_event_t accelEvent;
    sensors_event_t magEvent;
    sensors_event_t gyroEvent;
    //sensors_event_t tempEvent;
    _lsm.getEvent(&accelEvent, &magEvent, &gyroEvent, NULL);

    float accelx = accelEvent.acceleration.x;
    float accely = accelEvent.acceleration.y;
    float accelz = accelEvent.acceleration.z;

    float magx = magEvent.magnetic.x + MAG_CALIBRATE_X;
    float magy = magEvent.magnetic.y + MAG_CALIBRATE_Y;
    float magz = magEvent.magnetic.z * MAG_INVERT_Z; // For some reason z points the opposite of north.

    // Gyro is in degrees per second, so we change to rads and mult by dt to get rotation.
    float degPerSecSquared = gyroEvent.gyro.x * gyroEvent.gyro.x + gyroEvent.gyro.y * gyroEvent.gyro.y + gyroEvent.gyro.z * gyroEvent.gyro.z;
    float degPerSecToRads = PI/180.0 * elaspedMillis / 1000.0;
    float gyrox = gyroEvent.gyro.x * degPerSecToRads;
    float gyroy = gyroEvent.gyro.y * degPerSecToRads;
    float gyroz = gyroEvent.gyro.z * degPerSecToRads;

    // Rotate by the gyro.
    _q *= QuaternionInt::from_euler_rotation_approx(gyrox, gyroy, gyroz);
    //Serial.print("_q: "); Serial.print(_q.a);
    //Serial.print(" X: "); Serial.print(_q.b);
    //Serial.print(" Y: "); Serial.print(_q.c);
    //Serial.print(" Z: "); Serial.println(_q.d);
    if (_count % 100 == 0) {
        _q.normalize();
        //Serial.print("_q: "); Serial.print(_q.a);
        //Serial.print(" X: "); Serial.print(_q.b);
        //Serial.print(" Y: "); Serial.print(_q.c);
        //Serial.print(" Z: "); Serial.println(_q.d);
    }


    float accelNorm = sqrt(accelx * accelx + accely * accely + accelz * accelz);
    _currentAccel = accelNorm - SENSORS_GRAVITY_EARTH + ACCELEROMETER_CALIBRATE;
    float currentAbsAccel = abs(_currentAccel);
    _avgAbsAccel = (_avgAbsAccel * (MOVING_AVERAGE_INTERVALS - 1) + currentAbsAccel)/MOVING_AVERAGE_INTERVALS;

    // Ignore gravity if it isn't around G.  We only want to update based on the accelrometer if we aren't bouncing.
    // We also want to ignore gravity if the gyro is moving a lot.
    // TODO: carrino: make these defines at the top
    if (degPerSecSquared < USE_ACCELEROMETER_THRESHOLD_DEG_PER_S * USE_ACCELEROMETER_THRESHOLD_DEG_PER_S 
                && currentAbsAccel < USE_ACCELEROMETER_THRESHOLD_M_PER_S2) {
        //// cal expected gravity takes 1ms
        QuaternionInt expected_gravity = _q.conj().rotate(QuaternionInt::create_up_facing());
        if (_count % 2 == 0) {
            // This chunk of code takes 3ms
            QuaternionInt gravity = QuaternionInt::from_vector(accelx / accelNorm * MAX_QUAT_INT_VALUE,
                                                               accely / accelNorm * MAX_QUAT_INT_VALUE,
                                                               accelz / accelNorm * MAX_QUAT_INT_VALUE);
            QuaternionInt toRotateG = gravity.rotation_between_vectors(expected_gravity);

            _q = _q * toRotateG.fractional(ACCELEROMETER_FRACTION);
        } else {
            // This code path of code takes 5ms

            // We want to subtract gravity from the magnetic reading.
            // mag readings point into the earth quite a bit, but gravity is handled by accelerometer ok.
            // We just want to use mag for rotation around the gravity axis.
            // https://en.wikipedia.org/wiki/Earth%27s_magnetic_field#Inclination
            //QuaternionInt expected_north = _q.conj().rotate(QuaternionInt::create_north_facing());
            //expected_north += expected_gravity * (-expected_gravity.dot_product(expected_north));
            //expected_north.normalize();

            //QuaternionInt mag(magx, magy, magz);
            //mag += expected_gravity * (-expected_gravity.dot_product(mag));
            //mag.normalize();

            //QuaternionInt toRotateMag = mag.rotation_between_vectors(expected_north);
            //_q = _q * toRotateMag.fractional(COMPASS_ROTATE_FRACTION);
        }
    }

    int lastMillis = millis();
    Serial.print("Accel time: "); Serial.println(lastMillis - newMillis);
    return true;
}

const QuaternionInt Accel::getDeviceOrientation(const QuaternionInt &absolutePosition) const {
    return _q.conj().rotate(absolutePosition);
}

const QuaternionInt Accel::getAbsoluteOrientation(const QuaternionInt &deviceVector) const {
    return _q.rotate(deviceVector);
}

float Accel::getLinearAcceleration() const {
    return _currentAccel;
}

bool Accel::isDancing() const {
    return  _avgAbsAccel > ACCEL_THRESHOLD;
}

