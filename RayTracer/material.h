#pragma once

#include "math.h"
#include "ray.h"
#include "hitRecord.h"

class Material
{
public:
	virtual bool scatter(const Ray& in, const HitRecord& rec, vec3& attn, Ray& scattered) const = 0;
};

class Lambertian : public Material
{
public:
	explicit Lambertian(const vec3 albedo)
		:mAlbedo(albedo) {}

	virtual bool scatter(const Ray& in, const HitRecord& rec, vec3& attn, Ray& scattered) const override
	{
		const vec3 target = rec.intersectionPoint + rec.normal + randomInUnitSphere();
		attn = mAlbedo;
		scattered = Ray(rec.intersectionPoint, target - rec.intersectionPoint);
		return true;
	}

private:
	vec3 mAlbedo;
};

class Metal : public Material
{
public:
	explicit Metal(const vec3 attn, float roughness)
		:mAttn(attn), mRoughness(roughness) {}

	bool scatter(const Ray& in, const HitRecord& rec, vec3& attn, Ray& scattered) const override
	{
		const vec3 reflectDir = reflect(in.direction(), rec.normal) + randomInUnitSphere() * mRoughness;
		scattered = Ray(rec.intersectionPoint, reflectDir);
		attn = mAttn;
		return true;
	}

private:
	vec3 mAttn;
	float mRoughness;
};

class Dielectric : public Material
{
public:
	explicit Dielectric(float refractionIndex)
		:mRefractionIndex(refractionIndex) {}

	bool scatter(const Ray& in, const HitRecord& rec, vec3& attn, Ray& scattered) const override
	{
		attn = vec3{ 1.0f,1.0f,1.0f };
		vec3 refractedDir;
		bool isRefracted = false;
		const vec3& i = in.direction();
		const vec3& n = rec.normal;
		float reflProb = 1.0f;
		float cosine = 0.0f;

		if (dot(n, i) > 0.0f) //ray exiting the dielectric material
		{
			isRefracted = refract(i, -n, mRefractionIndex, 1.0f, refractedDir);
			cosine = mRefractionIndex * dot(i, n) / i.length();
		}
		else //ray entering the dielectric material
		{
			isRefracted = refract(i, n, 1.0f, mRefractionIndex, refractedDir);
			cosine = -dot(i, n) / i.length();
		}
		if (isRefracted)
		{
			reflProb = schlick(cosine, mRefractionIndex);
		}

		scattered = Ray(rec.intersectionPoint, randf() < reflProb ? reflect(i, n) : refractedDir);
		return true;
	}

private:
	static float schlick(float cosine, float ri)
	{
		const float r0 = (1.0f - ri) / (1.0f + ri);
		const float r0sq = r0 * r0;
		return r0sq + (1.0f - r0sq) * pow((1.0f - cosine), 5);
	}

private:
	float mRefractionIndex;
};