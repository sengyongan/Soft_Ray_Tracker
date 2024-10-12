#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "../render/ray.h"
#include "../tool/interval.h"
#include "../tool/onb.h"
class sphere : public hittable
{
public:
    sphere(const point3 &static_center, double radius, shared_ptr<material> mat) : vray(static_center, vec3(0, 0, 0)), radius(std::fmax(0, radius)), mat(mat)
    {
        auto rvec = vec3(radius, radius, radius);
        bbox = aabb(static_center - rvec, static_center + rvec);
    }
    // Moving Sphere
    sphere(const point3 &center1, const point3 &center2, double radius,
           shared_ptr<material> mat)
        : vray(center1, center2 - center1), radius(std::fmax(0, radius)), mat(mat)
    {
        auto rvec = vec3(radius, radius, radius);
        aabb box1(vray.at(0) - rvec, vray.at(0) + rvec); /* 球体运动的总范围 */
        aabb box2(vray.at(1) - rvec, vray.at(1) + rvec);
        bbox = aabb(box1, box2);
    }
    bool hit(const ray &r, interval ray_t, hit_record &rec) const override
    {
        /* 是否有交点 */

        point3 current_center = vray.at(r.time()); /* 新的物体位置 */
        vec3 oc = current_center - r.origin();

        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - radius * radius;

        auto discriminant = h * h - a * c;
        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);
        /* 深度测试 */
        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a; /* 求根公式解t */
        if (!ray_t.surrounds(root))
        {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }
        /* 都通过更新交点信息 */
        rec.t = root;
        rec.p = r.at(rec.t);
        rec.normal = (rec.p - current_center) / radius;
        vec3 outward_normal;
        outward_normal = (rec.p - current_center) / radius;
        rec.set_face_normal(r, outward_normal);
        get_sphere_uv(outward_normal, rec.u, rec.v);
        rec.mat = mat;

        return true;
    }
    aabb bounding_box() const override { return bbox; }
    static void get_sphere_uv(const point3 &p, double &u, double &v) /* 从p交点-》极坐标-》uv对应值 */
    {
        // p: a given point on the sphere of radius one, centered at the origin.
        // u: returned value [0,1] of angle around the Y axis from X=-1.
        // v: returned value [0,1] of angle from Y=-1 to Y=+1.
        //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
        //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
        //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

        auto theta = std::acos(-p.y());
        auto phi = std::atan2(-p.z(), p.x()) + pi;

        u = phi / (2 * pi);
        v = theta / pi;
    }
    double pdf_value(const point3 &origin, const vec3 &direction) const override /* 和quad一样，向球体和光源发射光线 */
    {
        // This method only works for stationary spheres.

        hit_record rec;
        if (!this->hit(ray(origin, direction), interval(0.001, infinity), rec))
            return 0;

        auto dist_squared = (vray.at(0) - origin).length_squared();
        auto cos_theta_max = std::sqrt(1 - radius * radius / dist_squared);
        auto solid_angle = 2 * pi * (1 - cos_theta_max);

        return 1 / solid_angle;
    }

    vec3 random(const point3 &origin) const override
    {
        vec3 direction = vray.at(0) - origin;
        auto distance_squared = direction.length_squared();
        onb uvw(direction);
        return uvw.transform(random_to_sphere(radius, distance_squared));
    }
    static vec3 random_to_sphere(double radius, double distance_squared) /* 均匀随机方向 */
    {
        auto r1 = random_double();
        auto r2 = random_double();
        auto z = 1 + r2 * (std::sqrt(1 - radius * radius / distance_squared) - 1);

        auto phi = 2 * pi * r1;
        auto x = std::cos(phi) * std::sqrt(1 - z * z);
        auto y = std::sin(phi) * std::sqrt(1 - z * z);

        return vec3(x, y, z);
    }

private:
    ray vray; /* center2 - center1，物体移动方向 */

    double radius;
    shared_ptr<material> mat;
    aabb bbox;
};

#endif