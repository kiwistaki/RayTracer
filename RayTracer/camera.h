#pragma once

#include "math.h"
#include "ray.h"

class Camera
{
public:
	Camera(const vec3& lookFrom, const vec3& lookAt, const vec3& up, 
		float vFov, float aspectRatio)
		: mOrigin{ 0.0f, 0.0f, 0.0f }
	{
		auto theta = vFov * PI / 180.0f;
		auto h = tan(theta / 2.0f);
		auto viewportHeight = 2.0f * h;
		auto viewportWidth = aspectRatio * viewportHeight;

		auto w = normalize(lookFrom - lookAt);
		auto u = normalize(cross(up, w));
		auto v = cross(w, u);

		mOrigin = lookFrom;
		mHorizontal = viewportWidth*u;
		mVertical = viewportHeight*v;
		mLowerLeftCorner = mOrigin - mHorizontal / 2.0f - mVertical / 2.0f - w;
	}

	Ray getRay(float s, float t) const
	{
		return Ray(mOrigin, mLowerLeftCorner + s * mHorizontal + t * mVertical - mOrigin);
	}

private:
	vec3 mOrigin;
	vec3 mLowerLeftCorner;
	vec3 mHorizontal;
	vec3 mVertical;
};
