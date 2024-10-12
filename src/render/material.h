#ifndef MATERIAL_H
#define MATERIAL_H

#include "../obj/hittable.h"
#include "../tool/color.h"
#include "../tool/onb.h"
#include "../tool/PDF.h"
#include "../render/texture.h"

class scatter_record
{
public:
    color attenuation;/* 漫反射颜色 */
    shared_ptr<pdf> pdf_ptr;/* PDF类 */
    bool skip_pdf;/* 对于金属和绝缘体为true，它们遵守反射或折射定律，对于漫反射需要PDF的混合，不用提前跳过 */
    ray skip_pdf_ray;/* 反射光线 */
};

class material
{
public:
    virtual ~material() = default;
    virtual color emitted(
        const ray &r_in, const hit_record &rec, double u, double v, const point3 &p) const
    {
        return color(0, 0, 0);
    }
    virtual bool scatter(/* 根据入射光，交点信息，返回应该返回的颜色和散射向量scattered */
                         const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const
    {
        return false;
    }
    virtual bool scatter(
        const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered, double &pdf) const
    {
        return false;
    }
    virtual bool scatter(const ray &r_in, const hit_record &rec, scatter_record &srec) const
    {
        return false;
    }
    virtual double scattering_pdf(const ray &r_in, const hit_record &rec, const ray &scattered) /* 散射PDF */
        const
    {
        return 0;
    }
};
class lambertian : public material
{
public:
    lambertian(const color &albedo) : tex(make_shared<solid_color>(albedo)) {}
    lambertian(shared_ptr<texture> tex) : tex(tex) {}

    bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered)
        const override
    {
        auto scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction, r_in.time());
        attenuation = tex->value(rec.u, rec.v, rec.p);
        return true;
    }
    bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf
    ) const override {
        onb uvw(rec.normal);/* 法线轴的onb坐标轴 */
        auto scatter_direction = uvw.transform(random_cosine_direction());/* 生成随机余弦分布的向量变换到法线onb坐标下 */

        scattered = ray(rec.p, unit_vector(scatter_direction), r_in.time());/* 作为散射向量 */
        attenuation = tex->value(rec.u, rec.v, rec.p);
        pdf = dot(uvw.w(), scattered.direction()) / pi;/* cos / PI */
        return true;
    }
    bool scatter(const ray &r_in, const hit_record &rec, scatter_record &srec) const override
    {
        srec.attenuation = tex->value(rec.u, rec.v, rec.p);
        srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);/* cos的PDF */
        srec.skip_pdf = false;
        return true;
    }
    double scattering_pdf(const ray &r_in, const hit_record &rec, const ray &scattered)
        const override
    {
        auto cos_theta = dot(rec.normal, unit_vector(scattered.direction()));
        return cos_theta < 0 ? 0 : cos_theta / pi;
    }

private:
    shared_ptr<texture> tex;
};

class metal : public material
{
public:
    metal(const color &albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered)
        const override
    {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected, r_in.time());
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }
     bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());

        srec.attenuation = albedo;
        srec.pdf_ptr = nullptr;
        srec.skip_pdf = true;
        srec.skip_pdf_ray = ray(rec.p, reflected, r_in.time());/* 遵守反射定律 */

        return true;
    }

private:
    color albedo;
    double fuzz;
};
class dielectric : public material
{
public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered)
        const override
    {
        attenuation = color(1.0, 1.0, 1.0);
        /* 如果法线是正面的，那么eta / eta prime = 1 / ref_idx（空气折射率为1），否则法线反面，代表光线从物体内部折射出去，也就是ref_idx / 1 = ref_idx */
        attenuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0 / refraction_index) : refraction_index;

        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);

        scattered = ray(rec.p, direction, r_in.time());
        return true;
    }
    bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
        srec.attenuation = color(1.0, 1.0, 1.0);
        srec.pdf_ptr = nullptr;
        srec.skip_pdf = true;
        double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);

        srec.skip_pdf_ray = ray(rec.p, direction, r_in.time());/* 遵守折射定律 */
        return true;
    }

private:
    double refraction_index;

    static double reflectance(double cosine, double refraction_index)
    {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1 - cosine), 5);
    }
};
class diffuse_light : public material
{
public:
    diffuse_light(shared_ptr<texture> tex) : tex(tex) {}
    diffuse_light(const color &emit) : tex(make_shared<solid_color>(emit)) {}

    color emitted(const ray &r_in, const hit_record &rec, double u, double v, const point3 &p)
        const override
    {
        if (!rec.front_face)
            return color(0, 0, 0);
        return tex->value(u, v, p);
    }

private:
    shared_ptr<texture> tex;
};
class isotropic : public material
{
public:
    isotropic(const color &albedo) : tex(make_shared<solid_color>(albedo)) {}
    isotropic(shared_ptr<texture> tex) : tex(tex) {}

    bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf
    ) const override {
        scattered = ray(rec.p, random_unit_vector(), r_in.time());
        attenuation = tex->value(rec.u, rec.v, rec.p);
        pdf = 1 / (4 * pi);
        return true;
    }
    bool scatter(const ray &r_in, const hit_record &rec, scatter_record &srec) const override
    {
        srec.attenuation = tex->value(rec.u, rec.v, rec.p);
        srec.pdf_ptr = make_shared<sphere_pdf>();/* 均匀PDF */
        srec.skip_pdf = false;/* 比如大理石材质，也属于漫反射 */
        return true;
    }
    double scattering_pdf(const ray &r_in, const hit_record &rec, const ray &scattered)
        const override
    {
        return 1 / (4 * pi);
    }

private:
    shared_ptr<texture> tex;
};
#endif