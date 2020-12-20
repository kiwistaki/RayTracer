#pragma once

#include "math.h"
#include "ray.h"

class Camera
{
public:
	Camera(const vec3& lookFrom, const vec3& lookAt, const vec3& up, 
		float vFov, float aspectRatio, float aperture, float focusDistance,
		float time0 = 0, float time1 = 0)
	{
		auto theta = degreesToRadians(vFov);
		auto h = tan(theta / 2.0f);
		auto viewportHeight = 2.0f * h;
		auto viewportWidth = aspectRatio * viewportHeight;

		w = normalize(lookFrom - lookAt);
		u = normalize(cross(up, w));
		v = cross(w, u);

		mOrigin = lookFrom;
		mHorizontal = focusDistance*viewportWidth*u;
		mVertical = focusDistance*viewportHeight*v;
		mLowerLeftCorner = mOrigin - mHorizontal / 2.0f - mVertical / 2.0f - focusDistance*w;

		mLensRadius = aperture / 2.0f;
		mTime0 = time0;
		mTime1 = time1;
	}

	Ray getRay(float s, float t) const
	{
		vec3 rd = mLensRadius * randomInUnitDisk();
		vec3 offset = u * rd.x() + v * rd.y();
		return Ray(mOrigin + offset, 
			mLowerLeftCorner + s * mHorizontal + t * mVertical - mOrigin - offset,
			randf(mTime0, mTime1));
	}

private:
	vec3 mOrigin;
	vec3 mLowerLeftCorner;
	vec3 mHorizontal;
	vec3 mVertical;
	vec3 u, v, w;
	float mLensRadius;
	float mTime0, mTime1;
};
