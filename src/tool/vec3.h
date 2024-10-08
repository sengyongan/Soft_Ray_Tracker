#ifndef VEC3_H
#define VEC3_H

#include "rtweekend.h"
#include <cmath>
#include <iostream>

class vec3
{
public:
    double e[3];

    vec3() : e{0, 0, 0} {}
    vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}
    /* 默认的拷贝构造 */
    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }

    vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
    double operator[](int i) const { return e[i]; }
    double &operator[](int i) { return e[i]; }

    vec3 &operator+=(const vec3 &v)
    {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    vec3 &operator*=(double t)
    {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    vec3 &operator/=(double t)
    {
        return *this *= 1 / t;
    }

    double length() const /* 向量长度 */
    {
        return std::sqrt(length_squared());
    }

    double length_squared() const /* 向量长度的平方 */
    {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }
    bool near_zero() const
    {
        // Return true if the vector is close to zero in all dimensions.如果向量在所有维度上都接近于零，则返回true。
        auto s = 1e-8;
        return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s); /* std::fabs浮点数的绝对值 */
    }
    static vec3 random()
    {
        return vec3(random_double(), random_double(), random_double());
    }

    static vec3 random(double min, double max)
    {
        return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
    }
};

// point3 is just an alias for vec3, but useful for geometric clarity in the code.
using point3 = vec3;

// Vector Utility Functions

inline std::ostream &operator<<(std::ostream &out, const vec3 &v)
{
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3 &u, const vec3 &v)
{
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3 &u, const vec3 &v)
{
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3 &u, const vec3 &v)
{
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(double t, const vec3 &v)
{
    return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3 &v, double t)
{
    return t * v;
}

inline vec3 operator/(const vec3 &v, double t)
{
    return (1 / t) * v;
}

inline double dot(const vec3 &u, const vec3 &v)
{
    return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2];
}

inline vec3 cross(const vec3 &u, const vec3 &v)
{
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(const vec3 &v)
{
    return v / v.length();
}
inline vec3 random_in_unit_disk() {/* 生成一个随机的终点在单位球面内的向量 */
    while (true) {
        auto p = vec3(random_double(-1,1), random_double(-1,1), 0);
        if (p.length_squared() < 1)
            return p;
    }
}

inline vec3 random_unit_vector()
{ /* 生成一个随机的终点在单位球体内的向量 */
    while (true)
    {
        auto p = vec3::random(-1, 1);
        auto lensq = p.length_squared();  /* 计算长度的平方通常比计算实际长度要快,sqrt函数比较昂贵，所以我们仅在if成立再执行sqrt */
        if (1e-160 < lensq && lensq <= 1) /* 1e-160是一个非常小的数，接近于零但不等于零 */
            return p / sqrt(lensq);       /* 返回单位向量 */
    }
}
inline vec3 random_on_hemisphere(const vec3 &normal)
{ /* 生成一个随机的终点在单位半球体内的向量 */
    vec3 on_unit_sphere = random_unit_vector();
    /* 如果法线为正面不用反转，否则反转到正半球内 */
    if (dot(on_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
        return on_unit_sphere;
    else
        return -on_unit_sphere;
}
inline vec3 reflect(const vec3 &v, const vec3 &n)
{ /* 反射向量 */
    return v - 2 * dot(v, n) * n;
}
inline vec3 refract(const vec3 &uv, const vec3 &n, double etai_over_etat)
{ /* 折射向量 */
    auto cos_theta = std::fmin(dot(-uv, n), 1.0);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}
#endif