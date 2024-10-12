#ifndef ONB_H
#define ONB_H

#include "vec3.h"
class onb
{
public:
  onb(const vec3 &n)
  {                           /* 创建以法线为轴的obn坐标轴，标准化 */
    axis[2] = unit_vector(n); /* n/w */
    vec3 a = (std::fabs(axis[2].x()) > 0.9) ? vec3(0, 1, 0) : vec3(1, 0, 0);
    axis[1] = unit_vector(cross(axis[2], a)); /* t/u */
    axis[0] = cross(axis[2], axis[1]);        /* s/v */
  }

  const vec3 &u() const { return axis[0]; }
  const vec3 &v() const { return axis[1]; }
  const vec3 &w() const { return axis[2]; }

  vec3 transform(const vec3 &v) const
  { /* 向量变换到onb坐标下，向量 * 单位向量 == | 向量 | * costheta  */
    // Transform from basis coordinates to local space.从世界坐标变换到局部onb空间。
    return (v[0] * axis[0]) + (v[1] * axis[1]) + (v[2] * axis[2]);
  }

private:
  vec3 axis[3];
};

#endif