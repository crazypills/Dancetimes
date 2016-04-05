#ifndef QUATERNION_INT_H
#define QUATERNION_INT_H

#include "Arduino.h"

#define MAX_QUAT_INT_VALUE 0x7fff
#define MIN_QUAT_INT_VALUE 0x8000
#define BITS_TO_SHIFT_AFTER_MULT 15

// This class models a Quaternion using 16 bits of precision for each value.
// the allowed values are only between -1 and 1 and these are represented by 
// value between -2^15 and 2^15-1.
// ALL INSTANCES OF THIS CLASS MUST BE NORMALIZED (LENGTH 1) FOR MULTIPLICATIONS TO WORK.
class QuaternionInt {
private:
    static int32_t m(int16_t a, int16_t b);
    void normalize();
    void normalizeVector(float newA);
    QuaternionInt(int x, int y, int z) { a = 0; b = x; c = y; d = z; }
public:
    int16_t a;
    int16_t b;
    int16_t c;
    int16_t d;

    QuaternionInt() {a = MAX_QUAT_INT_VALUE; b = c = d = 0;}


    // This returns a Quaternion that rotates in each given axis in radians.
    // We use standard right hand rule for rotations and coordinates.
    // This behaves like from_euler_rotation but is only appropirate for small 
    // angles (less than 30 deg (abput .5 rad))
    static const QuaternionInt from_euler_rotation_approx(float x, float y, float z);
    static const QuaternionInt create_north_facing() { return QuaternionInt(MAX_QUAT_INT_VALUE, 0, 0); }
    static const QuaternionInt create_up_facing() { return QuaternionInt(0, 0, MAX_QUAT_INT_VALUE); }

    QuaternionInt & operator=(const QuaternionInt &rhs) {
        a = rhs.a;
        b = rhs.b;
        c = rhs.c;
        d = rhs.d;
        return *this;
    }

    // http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/arithmetic/index.htm
    QuaternionInt & operator*=(const QuaternionInt &q);
    const QuaternionInt operator* (const QuaternionInt& q) const { return QuaternionInt(*this) *= q; }
    QuaternionInt & operator+=(const QuaternionInt &q);
    const QuaternionInt operator+(const QuaternionInt& q) const { return QuaternionInt(*this) += q; }
    QuaternionInt & operator*=(int16_t scale);
    const QuaternionInt operator*(int16_t scale) const { return QuaternionInt(*this) *= scale; }
    const QuaternionInt conj() const;
    // This method takes two vectors and computes the rotation vector between them.
    // Both the left and right hand sides must be pure vectors (a == 0)
    // Both the left and right hand sides must normalized already.
    // This computes the rotation that will tranform this to q.
    const QuaternionInt rotation_between_vectors(const QuaternionInt& v) const;

    // Since all these are normalized to length 1, the dot product will be between 0 and 1.
    // We use an into represent the range from -1 to 1.
    int16_t dot_product(const QuaternionInt& q) const;

    // This method takes one vector and rotates it using this Quaternion.
    // The input must be a pure vector (a == 0).
    const QuaternionInt rotate(const QuaternionInt& q) const;
    QuaternionInt & fractional(int16_t f);
};

#endif
