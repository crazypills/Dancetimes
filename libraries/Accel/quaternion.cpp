#include "quaternion.h"
#include "Arduino.h"

// http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/arithmetic/index.htm
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

Quaternion & Quaternion::operator-=(const Quaternion &q) {
    a -= q.a;
    b -= q.b;
    c -= q.c;
    d -= q.d;
    return *this;
}

Quaternion & Quaternion::operator*=(const float &scale) {
    a *= scale;
    b *= scale;
    c *= scale;
    d *= scale;
    return *this;
}

Quaternion & Quaternion::normalize() {
    float norm2 = a*a + b*b + c*c + d*d;
    float norm = sqrt(norm2);
    a /= norm;
    b /= norm;
    c /= norm;
    d /= norm;
    return *this;
}

Quaternion & Quaternion::from_euler_rotation(float x, float y, float z) {
    a = cos(x/2) * cos(y/2) * cos(z/2) + sin(x/2) * sin(y/2) * sin(z/2);
    b = sin(x/2) * cos(y/2) * cos(z/2) - cos(x/2) * sin(y/2) * sin(z/2);
    c = cos(x/2) * sin(y/2) * cos(z/2) + sin(x/2) * cos(y/2) * sin(z/2);
    d = cos(x/2) * cos(y/2) * sin(z/2) - sin(x/2) * sin(y/2) * cos(z/2);
    normalize();
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

//const float Quaternion::dot_product(const Quaternion& q) const {
//    return a * q.a + b * q.b + c * q.c + d * q.d;
//}

const Quaternion Quaternion::rotate(const Quaternion& q) const {
    return (*this) * q * conj();
}


