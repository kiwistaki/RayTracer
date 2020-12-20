#pragma once
#include "math.h"

class Ray
{
public:
	Ray() = default;

	Ray(const vec3& o, const vec3& d)
		:mOrigin(o), mDirection(normalize(d)) {}

	~Ray() {}

	const vec3& origin() const { return mOrigin; }
	const vec3& direction() const { return mDirection; }

	vec3 pointAt(float t) const
	{
		return mOrigin + mDirection * t;
	}

private:
	vec3 mOrigin;
	vec3 mDirection;
};
