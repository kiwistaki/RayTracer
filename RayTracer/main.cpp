#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>
#include "math.h"

class Framebuffer
{
public:
	static constexpr size_t kBytesPerPixels = 3;

	Framebuffer(size_t width, size_t height)
		:mWidth(width), mHeight(height),
		mData((uint8_t*)malloc(width* height* kBytesPerPixels))
	{}

	~Framebuffer() { free(mData); }

	void setPixel(size_t row, size_t col, uint8_t r, uint8_t g, uint8_t b)
	{
		const size_t index = kBytesPerPixels * (row * mWidth + col);
		mData[index + 0] = b;
		mData[index + 1] = g;
		mData[index + 2] = r;
	}

	/**
	* Save framebuffer to a TGA file
	*/
	void saveToFile(const char* filepath)
	{
		FILE* file = fopen(filepath, "wb");
		assert(file);
		putc(0, file);
		putc(0, file);
		putc(2, file); /* uncompressed RGB */
		putc(0, file); putc(0, file);
		putc(0, file); putc(0, file);
		putc(0, file);
		putc(0, file); putc(0, file); /* X origin */
		putc(0, file); putc(0, file); /* y origin */
		putc((mWidth & 0x00FF), file);
		putc((mWidth & 0xFF00) / 256, file);
		putc((mHeight & 0x00FF), file);
		putc((mHeight & 0xFF00) / 256, file);
		putc(24, file); /* 24 bit bitmap */
		putc(0, file);
		fwrite(mData, kBytesPerPixels, mWidth * mHeight, file);
		fclose(file);
	}

	size_t width() const { return mWidth; }
	size_t height() const { return mHeight; }

private:
	uint8_t*	mData;
	size_t		mWidth;
	size_t		mHeight;
};

class Ray
{
public:
	Ray() = default;

	Ray(const vec3 &o, const vec3 &d)
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

class Camera
{
public:
	Camera(float aspectH, float aspectV)
		:mAspectH(aspectH), mAspectV(aspectV), mOrigin{0.0f, 0.0f, 0.0f}
	{}

	Ray getRay(float u, float v)
	{
		const vec3 lowerLeft{ -mAspectH / 2.0f, -mAspectV / 2.0f, -1.0f };
		return Ray(mOrigin, lowerLeft + vec3{ u * mAspectH, v * mAspectV, 0.0f });
	}

private:
	float mAspectH, mAspectV;
	vec3 mOrigin;
};

class Material;

/**
* Information about a point where a ray intersects geometry
*/
struct HitRecord
{
	vec3 normal; // Surface normal at intersection
	float t; // Value of t (ray parameter) of the intersection
	vec3 intersectionPoint;
	const Material* material; // Material of the geometry intersecting ray
};

class Material
{
public:
	virtual bool scatter(const Ray& in, const HitRecord& rec, vec3& attn, Ray& scattered) const = 0;

	vec3 reflect(const vec3 direction, const vec3 normal) const
	{
		return	direction - 2.0f * dot(direction, normal) * normal;
	}

	static bool refract(const vec3& i, const vec3& n, float ni, float nt, vec3& refracted)
	{
		const vec3 ui = normalize(i);
		const float dt = dot(i, n);
		const float niOverNt = ni / nt;
		float d = 1.0f - niOverNt * niOverNt * (1.0f - dt * dt);
		const bool isRefracted = d > 0.0f;
		if (isRefracted)
			refracted = niOverNt * (ui - n * dt) - n * sqrtf(d);
		return isRefracted;
	}
};

class Lambertian : public Material
{
public:
	explicit Lambertian(const vec3 albedo)
		:mAlbedo(albedo) {}

	bool scatter(const Ray& in, const HitRecord& rec, vec3& attn, Ray& scattered) const override
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
		const vec3 reflectDir = reflect(in.direction(), rec.normal) + randomInUnitSphere()*mRoughness;
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
		const float r0sq = r0*r0;
		return r0sq + (1.0f - r0sq) * pow((1.0f - cosine), 5);
	}

private: 
	float mRefractionIndex;
};

/**
* Any geometry that may be intersect by rays
*/
class Hitable
{
public:
	virtual bool hitTest(const Ray& r, float tmin, float tmax, HitRecord& hitRecord) const = 0;
};

class Sphere : public Hitable
{
public:
	Sphere(const vec3& center, float radius, const Material* material)
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
			hitRecord.normal = (hitRecord.intersectionPoint - mCenter)/mRadius;
			hitRecord.material = mMaterial;
		}
		return (discriminant > 0.0f);
	}

private:
	vec3 mCenter;
	float mRadius;
	const Material* mMaterial;
};

class SphereList : Hitable
{
public:
	template<class ...Args>
	SphereList(Args... args)
		:mList{ std::forward<Args>(args) ... } {}

	bool hitTest(const Ray& ray, float tmin, float tmax, HitRecord& hitRecord) const override
	{
		bool anyHit = false;
		float closestHit = tmax;
		for (const Sphere& s : mList)
		{
			bool hasHit = s.hitTest(ray, tmin, closestHit, hitRecord);
			if (hasHit) closestHit = hitRecord.t;
			anyHit |= hasHit;
		}
		return anyHit;
	}
private:
	std::vector<Sphere> mList;
};

/**
* Cast a ray into the scene and determine color
*/
vec3 color(const Ray& ray, int bounce)
{
	static constexpr int maxBounces = 50;
	static Lambertian diffuseYellow{ vec3{0.8f, 0.8f, 0.0f} };
	static Lambertian diffuseRed{ vec3{0.8f, 0.3f, 0.3f} };
	static Lambertian diffuseBlue{ vec3{0.1f, 0.2f, 0.5f} };
	static Metal gold_metallic{ vec3{0.8f, 0.6f, 0.2f}, 0.0f };
	static Metal silver_metallic{ vec3{0.8f, 0.8f, 0.8f}, 0.3f };
	static Dielectric glass{ 1.5f };
	static SphereList scene {
		Sphere{vec3(0.0f, -100.5f, -1.0f), 100.0f, &diffuseYellow}, //Ground
		Sphere{vec3(0.0f, 0.0f, -1.0f), 0.5f, &diffuseBlue}, //Sphere Diffuse
		Sphere{vec3(1.0f, 0.0f, -1.0f), 0.5f, &gold_metallic}, //Sphere Gold Metal
		Sphere{vec3(-1.0f, 0.0f, -1.0f), 0.5f, &glass}, //Sphere outer glass
		Sphere{vec3(-1.0f, 0.0f, -1.0f), -0.4f, &glass}, //Sphere inner glass
	};
	HitRecord hit;
	if (bounce < maxBounces && scene.hitTest(ray, 0.001f, 1000.0f, hit))
	{
		const vec3 target = hit.intersectionPoint + hit.normal + randomInUnitSphere();
		vec3 attn;
		Ray scattered;
		if (hit.material->scatter(ray, hit, attn, scattered))
		{
			return attn * color(scattered, bounce + 1);
		}
		return 0.5f * color(Ray(hit.intersectionPoint, target - hit.intersectionPoint), bounce + 1);
	}
	else
	{
		const float t = 0.5f * (ray.direction().y() + 1.0f);
		return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);
	}
}

int main(int argc, char* argv[])
{
	size_t imageWidth = 800u;
	size_t imageHeight = 400u;
	constexpr size_t kNumSamples = 100u;
	Framebuffer framebuffer{ imageWidth, imageHeight };
	Camera camera{ 4.0f, 2.0f };

	for (size_t row = 0u; row < framebuffer.height(); row++)
	{
		for (size_t col = 0u; col < framebuffer.width(); col++)
		{
			vec3 c{ 0.0f, 0.0f, 0.0f };
			for (size_t s = 0u; s < kNumSamples; s++)
			{
				const float u = ((float)col + randf()) / (float)framebuffer.width();
				const float v = ((float)row + randf()) / (float)framebuffer.height();
				c += color(camera.getRay(u, v), 0);
			}
			c /= (float)kNumSamples;
			framebuffer.setPixel(row, col,
				(uint8_t)(255.99f * sqrt(c.x())),
				(uint8_t)(255.99f * sqrt(c.y())),
				(uint8_t)(255.99f * sqrt(c.z())));
		}
	}
	framebuffer.saveToFile("C:\\dev\\RayTracer\\image.tga");
	return 0;
}
