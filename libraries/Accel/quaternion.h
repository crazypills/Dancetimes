#ifndef QUATERNION_H
#define QUATERNION_H

class Quaternion {
protected:
    float a;
    float b;
    float c;
    float d;

public:
    Quaternion() {a = 1; b = c = d = 0;}

    Quaternion & operator=(const Quaternion &rhs) {
        a = rhs.a;
        b = rhs.b;
        c = rhs.c;
        d = rhs.d;
        return *this;
    }

    // http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/arithmetic/index.htm
    Quaternion & operator*=(const Quaternion &q);
    const Quaternion operator* (const Quaternion& q) const { return Quaternion(*this) *= q; }
    void normalize();
    Quaternion conj() const;
    void from_euler_rotation(float x, float y, float z);
};

#endif
