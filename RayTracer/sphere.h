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
		const vec3 oc = ray.origin() - mCenter;
		const float a = dot(ray.direction(), ray.direction());
		const float b = 2.0f * dot(oc, ray.direction());
		const float c = dot(oc, oc) - mRadius * mRadius;
		const float discriminant = b * b - 4.0f * a * c;
		if (discriminant > 0.0f)
		{
			const float t1 = (-b - sqrtf(discriminant)) / (2.0f * a);
			const float t2 = (-b + sqrtf(discriminant)) / (2.0f * a);
			if (t1 > tmin && t1 < tmax)
				hitRecord.t = t1;
			else if (t2 > tmin && t2 < tmax)
				hitRecord.t = t2;
			else
				return false; // no acceptable solution
			hitRecord.intersectionPoint = ray.pointAt(hitRecord.t);
			hitRecord.normal = (hitRecord.intersectionPoint - mCenter) / mRadius;
			hitRecord.material = mMaterial;
		}
		return (discriminant > 0.0f);
	}

private:
	vec3 mCenter;
	float mRadius;
	std::shared_ptr<Material> mMaterial;
};