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

    // This is a vector that can be rotated in Quaternion space.
    Quaternion(int x, int y, int z) {a = 0; b = x; c = y; d = z;}

    Quaternion & operator=(const Quaternion &rhs) {
        a = rhs.a;
        b = rhs.b;
        c = rhs.c;
        d = rhs.d;
        return *this;
    }

    // http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/arithmetic/index.htm
    Quaternion & operator*=(const Quaternion &q);
    Quaternion & operator*=(const float &scale);
    const Quaternion operator* (const Quaternion& q) const { return Quaternion(*this) *= q; }
    const Quaternion operator* (const float& scale) const { return Quaternion(*this) *= scale; }
    void normalize();
    const Quaternion conj() const;
    void from_euler_rotation(float x, float y, float z);
    void rotate(float &x, float y, float &z) const;
};

#endif
