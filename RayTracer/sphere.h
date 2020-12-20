#pragma once

#include <memory>

#include "hittable.h"
#include "material.h"
#include "ray.h"
#include "hitRecord.h"
#include "math.h"

class Sphere : public Hittable
{
public:
	Sphere() :mRadius(0.0f) {}

	Sphere(const vec3& center, float radius, std::shared_ptr<Material> material)
		:mCenter(center), mRadius(radius), mMaterial(material) {}

	bool hitTest(const Ray& ray, float tmin, float tmax, HitRecord& hitRecord) const override
	{
        vec3 oc = ray.origin() - mCenter;
        auto a = ray.direction().lengthSquared();
        auto halfB = dot(oc, ray.direction());
        auto c = oc.lengthSquared() - mRadius * mRadius;

        auto discriminant = halfB * halfB - a * c;
        if (discriminant < 0) return false;
        auto sqrtd = sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (-halfB - sqrtd) / a;
        if (root < tmin || tmax < root) {
            root = (-halfB + sqrtd) / a;
            if (root < tmin || tmax < root)
                return false;
        }

        hitRecord.t = root;
        hitRecord.intersectionPoint = ray.pointAt(hitRecord.t);
        vec3 outwardNormal = (hitRecord.intersectionPoint - mCenter) / mRadius;
        hitRecord.setFaceNormal(ray, outwardNormal);
        hitRecord.material = mMaterial;

        return true;
	}

private:
	vec3 mCenter;
	float mRadius;
	std::shared_ptr<Material> mMaterial;
};