#pragma once

#include "ray.h"
#include "hitRecord.h"

/**
* Any geometry that may be intersect by rays
*/
class Hittable
{
public:
	virtual bool hitTest(const Ray& r, float tmin, float tmax, HitRecord& hitRecord) const = 0;
};
