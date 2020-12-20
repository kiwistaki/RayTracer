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
		auto target = rec.normal + randomInUnitSphere();
		if (target.nearZero())
			target = rec.normal;

		scattered = Ray(rec.intersectionPoint, target);
		attn = mAlbedo;
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
		const vec3 reflectDir = reflect(normalize(in.direction()), rec.normal);
		scattered = Ray(rec.intersectionPoint, reflectDir + randomInUnitSphere() * mRoughness);
		attn = mAttn;
		return (dot(scattered.direction(), rec.normal) > 0);
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
		float refractionRatio = rec.frontFace ? (1.0f / mRefractionIndex) : mRefractionIndex;

		vec3 unitDirection = normalize(in.direction());
		float cosTheta = fmin(dot(-unitDirection, rec.normal), 1.0f);
		float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

		bool cannotRefract = refractionRatio * sinTheta > 1.0f;
		vec3 direction;

		if (cannotRefract || schlick(cosTheta, refractionRatio) > randf())
			direction = reflect(unitDirection, rec.normal);
		else
			direction = refract(unitDirection, rec.normal, refractionRatio);

		scattered = Ray(rec.intersectionPoint, direction);
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