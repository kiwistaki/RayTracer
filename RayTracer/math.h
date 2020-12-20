#pragma once

#include <cmath>
#include <iostream>
#include <random>

using std::sqrt;

static constexpr float PI = 3.1415926535897932385f;

/**
* Taken from https://www.iquilezles.org/www/articles/sfrand/sfrand.htm
*/
inline float frand(int* seed)
{
    union
    {
        float fres;
        unsigned int ires;
    };

    seed[0] *= 16807;
    ires = ((((unsigned int)seed[0]) >> 9) | 0x3f800000);
    return fres - 1.0f;
}

/**
* Produce a random floating point number between 0.0f and 1.0f
*/
inline float randf()
{
    static int seed = 13575;
    return frand(&seed);
}

inline float randf(float min, float max)
{
    return min + (max - min) * randf();
}

class vec3
{
public:
    vec3() : e{ 0,0,0 } {}
    vec3(float e0, float e1, float e2) : e{ e0, e1, e2 } {}

    float x() const { return e[0]; }
    float y() const { return e[1]; }
    float z() const { return e[2]; }

    vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
    float operator[](int i) const { return e[i]; }
    float& operator[](int i) { return e[i]; }

    vec3& operator+=(const vec3& v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    vec3& operator*=(const float t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    vec3& operator/=(const float t) {
        return *this *= 1 / t;
    }

    float length() const {
        return sqrt(lengthSquared());
    }

    float lengthSquared() const {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    inline static vec3 random()
    {
        return vec3(randf(), randf(), randf());
    }
    inline static vec3 random(float min, float max)
    {
        return vec3(randf(min, max), randf(min, max), randf(min, max));
    }

public:
    float e[3];
};

// vec3 Utility Functions
inline std::ostream& operator<<(std::ostream& out, const vec3& v)
{
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v)
{
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v)
{
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v)
{
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(float t, const vec3& v)
{
    return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3& v, float t)
{
    return t * v;
}

inline vec3 operator/(vec3 v, float t)
{
    return (1 / t) * v;
}

inline float dot(const vec3& u, const vec3& v)
{
    return u.e[0] * v.e[0]
        + u.e[1] * v.e[1]
        + u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v)
{
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
        u.e[2] * v.e[0] - u.e[0] * v.e[2],
        u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 normalize(vec3 v)
{
    return v / v.length();
}

inline vec3 reflect(const vec3 direction, const vec3 normal)
{
    return	direction - 2.0f * dot(direction, normal) * normal;
}

static bool refract(const vec3& i, const vec3& n, float ni, float nt, vec3& refracted)
{
    const vec3 ui = normalize(i);
    const float dt = dot(i, n);
    const float niOverNt = ni / nt;
    float d = 1.0f - niOverNt * niOverNt * (1.0f - dt * dt);
    const bool isRefracted = d > 0.0f;
    if (isRefracted)
        refracted = niOverNt * (ui - n * dt) - n * sqrtf(d);
    return isRefracted;
}

/**
* Produce a random 3D point within a sphere of radius 1
*/
inline vec3 randomInUnitSphere()
{
    vec3 p;
    do 
    {
        p = 2.0f * vec3{ randf(), randf(), randf() } - vec3{ 1.0f,1.0f,1.0f };
    } while (p.lengthSquared() >= 1.0f);
    return p;
}
