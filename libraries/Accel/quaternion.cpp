#include "quaternion.h"
#include "Arduino.h"

// http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/arithmetic/index.htm
// 800B
Quaternion & Quaternion::operator*=(const Quaternion &q) {
    float e = q.a;
    float f = q.b;
    float g = q.c;
    float h = q.d;

    Quaternion ret;
    ret.a = a*e - b*f - c*g - d*h;
    ret.b = b*e + a*f + c*h - d*g;
    ret.c = a*g - b*h + c*e + d*f;
    ret.d = a*h + b*g - c*f + d*e;
    return (*this = ret);
}

Quaternion & Quaternion::operator+=(const Quaternion &q) {
    a += q.a;
    b += q.b;
    c += q.c;
    d += q.d;
    return *this;
}

Quaternion & Quaternion::operator*=(const float &scale) {
    a *= scale;
    b *= scale;
    c *= scale;
    d *= scale;
    return *this;
}

const float Quaternion::norm() const {
    float norm2 = a*a + b*b + c*c + d*d;
    return sqrt(norm2);
}

// 400B
Quaternion & Quaternion::normalize() {
    float n = norm();
    a /= n;
    b /= n;
    c /= n;
    d /= n;
    return *this;
}

// This method takes an euler rotation in rad and converts it to an equivilent 
// Quaternion rotation.
// 800B
Quaternion & Quaternion::from_euler_rotation(float x, float y, float z) {
    // We write cos here as sin(90 - theta) for prog memory
    float c1 = cos(y/2);
    float c2 = cos(z/2);
    float c3 = cos(x/2);

    float s1 = sin(y/2);
    float s2 = sin(z/2);
    float s3 = sin(x/2);
    a = c1 * c2 * c3 - s1 * s2 * s3;
    b = s1 * s2 * c3 + c1 * c2 * s3;
    c = s1 * c2 * c3 + c1 * s2 * s3;
    d = c1 * s2 * c3 - s1 * c2 * s3;
    return *this;
}

const Quaternion Quaternion::conj() const {
    Quaternion ret(*this);
    ret.b *= -1;
    ret.c *= -1;
    ret.d *= -1;
    return ret;
}

// http://www.euclideanspace.com/maths/algebra/vectors/angleBetween/
// This method takes two vectors and computes the rotation vector between them.
// Both the left and right hand sides must be pure vectors (a == 0)
// Both the left and right hand sides must normalized already.
// This computes the rotation that will tranform this to q.
// 500B
const Quaternion Quaternion::rotation_between_vectors(const Quaternion& q) const {
    // w = 1 + v1•v2
    // x = (v1 x v2).x 
    // y = (v1 x v2).y
    // z = (v1 x v2).z
    Quaternion ret;
    ret.a = 1 + b * q.b + c * q.c + d * q.d;
    ret.b = c * q.d - d * q.c;
    ret.c = d * q.b - b * q.d;
    ret.d = b * q.c - c * q.b;
    ret.normalize();
    return ret;
}

const float Quaternion::dot_product(const Quaternion& q) const {
    return a * q.a + b * q.b + c * q.c + d * q.d;
}

// This will roate the input vector by this normalized rotation quaternion.
const Quaternion Quaternion::rotate(const Quaternion& q) const {
    return (*this) * q * conj();
}

// This modifies this normalized rotation quaternion and makes it 
// rotate between 0-1 as much as it would normally rotate.
// The math here is pretty sloppy but should work for 
// most cases.
Quaternion & Quaternion::fractional(const float& f) {
    a = 1-f + f*a;
    b *= f;
    c *= f;
    d *= f;
    return normalize();
}

