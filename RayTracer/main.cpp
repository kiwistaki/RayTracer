#define _CRT_SECURE_NO_WARNINGS

#include "framebuffer.h"
#include "math.h"
#include "ray.h"
#include "hitRecord.h"
#include "camera.h"
#include "material.h"
#include "hittableList.h"
#include "sphere.h"

/**
* Cast a ray into the scene and determine color
*/
vec3 color(const Ray& ray, const Hittable& world, int bounce)
{
	static constexpr int maxBounces = 50;
	HitRecord hit;

	if (bounce < maxBounces && world.hitTest(ray, 0.001f, 1000.0f, hit))
	{
		const vec3 target = hit.intersectionPoint + hit.normal + randomInUnitSphere();
		vec3 attn;
		Ray scattered;
		if (hit.material->scatter(ray, hit, attn, scattered))
		{
			return attn * color(scattered, world, bounce + 1);
		}
		return 0.5f * color(Ray(hit.intersectionPoint, target - hit.intersectionPoint), world, bounce + 1);
	}
	else
	{
		const float t = 0.5f * (ray.direction().y() + 1.0f);
		return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);
	}
}

HittableList randomScene()
{
	HittableList world;

	auto groundMaterial = std::make_shared<Lambertian>(vec3(0.5f, 0.5f, 0.5f));
	world.add(std::make_shared<Sphere>(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, groundMaterial));

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto chooseMat = randf();
			vec3 center(a + 0.9f * randf(), 0.2f, b + 0.9f * randf());

			if ((center - vec3(4.0f, 0.2f, 0.0f)).length() > 0.9f) {
				std::shared_ptr<Material> sphereMaterial;

				if (chooseMat < 0.8f) {
					// diffuse
					auto albedo = vec3::random() * vec3::random();
					sphereMaterial = std::make_shared<Lambertian>(albedo);
					world.add(std::make_shared<Sphere>(center, 0.2f, sphereMaterial));
				}
				else if (chooseMat < 0.95f) {
					// metal
					auto albedo = vec3::random(0.5f, 1.0f);
					auto fuzz = randf(0.0f, 0.5f);
					sphereMaterial = std::make_shared<Metal>(albedo, fuzz);
					world.add(std::make_shared<Sphere>(center, 0.2f, sphereMaterial));
				}
				else {
					// glass
					sphereMaterial = std::make_shared<Dielectric>(1.5f);
					world.add(std::make_shared<Sphere>(center, 0.2f, sphereMaterial));
				}
			}
		}
	}

	auto material1 = std::make_shared<Dielectric>(1.5f);
	world.add(std::make_shared<Sphere>(vec3(0.0f, 1.0f, 0.0f), 1.0f, material1));

	auto material2 = std::make_shared<Lambertian>(vec3(0.4f, 0.2f, 0.1f));
	world.add(std::make_shared<Sphere>(vec3(-4.0f, 1.0f, 0.0f), 1.0f, material2));

	auto material3 = make_shared<Metal>(vec3(0.7f, 0.6f, 0.5f), 0.0f);
	world.add(std::make_shared<Sphere>(vec3(4.0f, 1.0f, 0.0f), 1.0f, material3));

	return world;
}

int main(int argc, char* argv[])
{
	size_t imageWidth = 800u;
	size_t imageHeight = 400u;
	constexpr size_t kNumSamples = 100u;
	Framebuffer framebuffer{ imageWidth, imageHeight };
	Camera camera{vec3(-2.0f,2.0f,1.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f), 90.0f, (float)(imageWidth/imageHeight)};

	// World
	HittableList world;
	auto materialGround = make_shared<Lambertian>(vec3{ 0.8f, 0.8f, 0.0f });
	auto materialCenter = make_shared<Lambertian>(vec3{ 0.1f, 0.2f, 0.5f });
	auto materialRight = make_shared<Metal>(vec3{ 0.8f, 0.6f, 0.2f }, 0.0f);
	auto materialLeft = make_shared<Dielectric>(1.5f);

	world.add(make_shared<Sphere>(vec3(0.0f, -100.5f, -1.0f), 100.0f, materialGround));
	world.add(make_shared<Sphere>(vec3(0.0f, 0.0f, -1.0f), 0.5f, materialCenter));
	world.add(make_shared<Sphere>(vec3(1.0f, 0.0f, -1.0f), 0.5f, materialRight));
	world.add(make_shared<Sphere>(vec3(-1.0f, 0.0f, -1.0f), 0.5f, materialLeft));
	world.add(make_shared<Sphere>(vec3(-1.0f, 0.0f, -1.0f), -0.45f, materialLeft));
	
	for (size_t row = 0u; row < framebuffer.height(); row++)
	{
		for (size_t col = 0u; col < framebuffer.width(); col++)
		{
			vec3 c{ 0.0f, 0.0f, 0.0f };
			for (size_t s = 0u; s < kNumSamples; s++)
			{
				const float u = ((float)col + randf()) / (float)framebuffer.width();
				const float v = ((float)row + randf()) / (float)framebuffer.height();
				c += color(camera.getRay(u, v), world, 0);
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
