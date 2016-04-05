#include "QuaternionInt.h"

int32_t QuaternionInt::m(int16_t a, int16_t b) {
    int32_t ret = a;
    ret *= b;
    ret >>= BITS_TO_SHIFT_AFTER_MULT;
    return ret;
}

// http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/arithmetic/index.htm
// 800B
QuaternionInt & QuaternionInt::operator*=(const QuaternionInt &q) {
    QuaternionInt ret;
    ret.a = m(a, q.a) - m(b, q.b) - m(c, q.c) - m(d, q.d);
    ret.b = m(b, q.a) + m(a, q.b) + m(c, q.d) - m(d, q.c);
    ret.c = m(a, q.c) - m(b, q.d) + m(c, q.a) + m(d, q.b);
    ret.d = m(a, q.d) + m(b, q.c) - m(c, q.b) + m(d, q.a);
    return (*this = ret);
}


//Quaternion & Quaternion::operator+=(const Quaternion &q) {
//    a += q.a;
//    b += q.b;
//    c += q.c;
//    d += q.d;
//    return *this;
//}
//
//Quaternion & Quaternion::operator*=(float scale) {
//    a *= scale;
//    b *= scale;
//    c *= scale;
//    d *= scale;
//    return *this;
//}

const QuaternionInt QuaternionInt::from_euler_rotation_approx(float x, float y, float z) {
    int16_t int_x = x;
    int16_t int_y = y;
    int16_t int_z = z;
    // approximage cos(theta) as 1 - theta^2 / 2
    int16_t c1 = MAX_QUAT_INT_VALUE - (m(int_y, int_y) >> 3);
    int16_t c2 = MAX_QUAT_INT_VALUE - (m(int_z, int_z) >> 3);
    int16_t c3 = MAX_QUAT_INT_VALUE - (m(int_x, int_x) >> 3);

    // appromixate sin(theta) as theta
    int16_t s1 = int_y >> 1;
    int16_t s2 = int_z >> 1;
    int16_t s3 = int_x >> 1;
    QuaternionInt ret;
    ret.a = m(m(c1, c2), c3) - m(m(s1, s2), s3);
    ret.b = m(s1, m(s2, c3)) + m(m(c1, c2), s3);
    ret.c = m(s1, m(c2, c3)) + m(m(c1, s2), s3);
    ret.d = m(s2, m(c1, c3)) - m(s1, m(c2, s3));
    return ret;
}

const QuaternionInt QuaternionInt::conj() const {
    QuaternionInt ret(*this);
    ret.b = -ret.b;
    ret.c = -ret.c;
    ret.d = -ret.d;
    return ret;
}

// This method takes two vectors and computes the rotation vector between them.
// Both the left and right hand sides must be pure vectors (a == 0)
// Both the left and right hand sides must normalized already.
// This computes the rotation that will tranform this to q.
// 500B
const QuaternionInt QuaternionInt::rotation_between_vectors(const QuaternionInt& q) const {
    // http://www.euclideanspace.com/maths/algebra/vectors/angleBetween/
    // We want to compute the below values.
    // w = 1 + v1•v2
    // x = (v1 x v2).x 
    // y = (v1 x v2).y
    // z = (v1 x v2).z

    // Instead of writing the below code direclty, we reduce code size by
    // just using multiplication to implement it.
    //Quaternion ret;
    //ret.a = 1 + b * q.b + c * q.c + d * q.d;
    //ret.b = c * q.d - d * q.c;
    //ret.c = d * q.b - b * q.d;
    //ret.d = b * q.c - c * q.b;
    //ret.normalize();
    //return ret;

    // From wikipedia https://en.wikipedia.org/wiki/Quaternion#Quaternions_and_the_geometry_of_R3
    // The cross product p x q is just the vector part of multiplying p * q
    QuaternionInt ret = (*this) * q;
    ret.normalizeVector(static_cast<float>(MAX_QUAT_INT_VALUE) - ret.a);
    return ret;
}

// This will roate the input vector by this normalized rotation quaternion.
const QuaternionInt QuaternionInt::rotate(const QuaternionInt& q) const {
    return (*this) * q * conj();
}

// This modifies this normalized rotation quaternion and makes it 
// rotate between 0-1 as much as it would normally rotate.
// The math here is pretty sloppy but should work for 
// most cases.
QuaternionInt & QuaternionInt::fractional(int16_t f) {
    a = MAX_QUAT_INT_VALUE - f +  m(f, a);
    b = m(b, f);
    c = m(c, f);
    d = m(d, f);
    normalize();
    return (*this);
}

void QuaternionInt::normalize() {
    normalizeVector(a);
}

void QuaternionInt::normalizeVector(float newA) {
    float norm2 = (m(b, b) + m(c, c) + m(d, d));
    norm2 = norm2 + a * a;
    float factor = MAX_QUAT_INT_VALUE/sqrt(norm2);
    a = factor * newA;
    b = factor * b;
    c = factor * c;
    d = factor * d;
}

