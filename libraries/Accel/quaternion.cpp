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

void Quaternion::normalize() {
    float norm2 = a*a + b*b + c*c + d*d;
    float norm = sqrt(norm2);
    a /= norm;
    b /= norm;
    c /= norm;
    d /= norm;
}

void Quaternion::from_euler_rotation(float x, float y, float z) {
    a = cos(x/2) * cos(y/2) * cos(z/2) + sin(x/2) * sin(y/2) * sin(z/2);
    b = sin(x/2) * cos(y/2) * cos(z/2) - cos(x/2) * sin(y/2) * sin(z/2);
    c = cos(x/2) * sin(y/2) * cos(z/2) + sin(x/2) * cos(y/2) * sin(z/2);
    d = cos(x/2) * cos(y/2) * sin(z/2) - sin(x/2) * sin(y/2) * cos(z/2);
    normalize();
}

const Quaternion conj() {
    Quaternion ret;
}

