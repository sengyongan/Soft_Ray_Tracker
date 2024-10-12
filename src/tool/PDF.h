#ifndef PDF_H
#define PDF_H

#include "onb.h"
#include "../obj/hittable_list.h"

class pdf
{
public:
    virtual ~pdf() {}

    virtual double value(const vec3 &direction) const = 0; /* 此方向PDF的值 */
    virtual vec3 generate() const = 0;                     /* 反射方向 */
};
class sphere_pdf : public pdf /* 均等PDF */
{
public:
    sphere_pdf() {}

    double value(const vec3 &direction) const override
    {
        return 1 / (4 * pi); /* 每个方向概率均等 */
    }

    vec3 generate() const override
    {
        return random_unit_vector(); /* 单位球体方向 */
    }
};
class cosine_pdf : public pdf /* 余弦PDF */
{
public:
    cosine_pdf(const vec3 &w) : uvw(w) {}

    double value(const vec3 &direction) const override /* 反射方向的概率：costheta / PI */
    {
        auto cosine_theta = dot(unit_vector(direction), uvw.w());
        return std::fmax(0, cosine_theta / pi);
    }

    vec3 generate() const override
    {
        return uvw.transform(random_cosine_direction()); /* 余弦定理，靠近n法线方向（通过乘以onb正交基） */
    }

private:
    onb uvw;
};
class hittable_pdf : public pdf
{
public:
    hittable_pdf(const hittable &objects, const point3 &origin) /* 应传入光物体，交点 */
        : objects(objects), origin(origin)
    {
    }

    double value(const vec3 &direction) const override /* 被quad实现 */
    {
        return objects.pdf_value(origin, direction); /* p(direction) = distance(p,q)^2 / (cosα * A) */
    }

    vec3 generate() const override
    {
        return objects.random(origin); /* 大体反射方向是：交点-》光源，有random变化 */
    }

private:
    const hittable &objects;
    point3 origin;
};
class mixture_pdf : public pdf
{
public:
    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1)
    {
        p[0] = p0;
        p[1] = p1;
    }

    double value(const vec3 &direction) const override
    {
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    vec3 generate() const override
    {
        if (random_double() < 0.5)
            return p[0]->generate();
        else
            return p[1]->generate();
    }

private:
    shared_ptr<pdf> p[2];
};
#endif