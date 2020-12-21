#define _CRT_SECURE_NO_WARNINGS

#include "framebuffer.h"
#include "math.h"
#include "ray.h"
#include "hitRecord.h"
#include "camera.h"
#include "material.h"
#include "hittableList.h"
#include "sphere.h"

#include <thread>
#include <functional>

/**
* Cast a ray into the scene and determine color
*/
vec3 color(const Ray& ray, const Hittable& world, int depth)
{
	static constexpr int maxBounces = 50;
	HitRecord hit;

	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return vec3(0.0f, 0.0f, 0.0f);

	if (world.hitTest(ray, 0.001f, INF, hit)) {
		Ray scattered;
		vec3 attenuation;
		if (hit.material->scatter(ray, hit, attenuation, scattered))
			return attenuation * color(scattered, world, depth - 1);
		return vec3(0.0f, 0.0f, 0.0f);
	}

	vec3 unit_direction = normalize(ray.direction());
	auto t = 0.5f * (unit_direction.y() + 1.0f);
	return (1.0f- t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);
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

	auto material3 = std::make_shared<Metal>(vec3(0.7f, 0.6f, 0.5f), 0.0f);
	world.add(std::make_shared<Sphere>(vec3(4.0f, 1.0f, 0.0f), 1.0f, material3));

	return world;
}

void singleThreadLoop(std::vector<vec3>& buffer, const Camera& cam, const HittableList& world,
	const int w, const int h, const int samples, const int bounces, const int numThread, const int threadId)
{
	for (int row = h - threadId - 1; row >=0; row -= numThread)
	{
		for (int col = 0; col < w; ++col)
		{
			vec3 c{ 0.0f, 0.0f, 0.0f };
			for (int s = 0; s < samples; ++s)
			{
				const float u = ((float)col + randf()) / (float)(w);
				const float v = ((float)row + randf()) / (float)(h);
				c += color(cam.getRay(u, v), world, bounces);
			}
			c /= (float)samples;
			buffer[row * w + col] = vec3(
				(255.99f * sqrt(c.x())),
				(255.99f * sqrt(c.y())),
				(255.99f * sqrt(c.z())));
		}
	}
}

void multiThreadLoop(std::vector<vec3>& buffer, const Camera& cam, const HittableList& world,
	const int w, const int h, const int samples, const int bounces)
{
	const int numThreads = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 2);
	std::vector<std::thread> threads;

	std::cerr << "Ray-tracing using " << numThreads << " threads" << std::endl;

	for (int t = 0; t != numThreads; ++t)
	{
		threads.emplace_back([=, &buffer, &cam, &world]()
		{
			singleThreadLoop(buffer, cam, world, w, h, samples, bounces, numThreads, t);
		});
	}

	for (auto& thread : threads)
	{
		thread.join();
	}
}

int main(int argc, char* argv[])
{
	const auto aspectRatio = 16.0f / 9.0f;
	const size_t imageWidth = 1920u;
	const size_t imageHeight = static_cast<size_t>(imageWidth/aspectRatio);
	constexpr size_t kNumSamples = 1024;
	constexpr size_t maxDepth = 16;

	Framebuffer framebuffer{ imageWidth, imageHeight };
	vec3 lookfrom(13.0f, 2.0f, 3.0f);
	vec3 lookat(0.0f, 0.0f, 0.0f);
	vec3 up(0.0f, 1.0f, 0.0f);
	auto distToFocus = 10.0f;
	auto aperture = 0.1f;
	Camera camera{vec3(13.0f,2.0f,3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 20.0f, aspectRatio, aperture, distToFocus};

	// World
	const HittableList world = randomScene();
	
	//Single Thread
	/*for (size_t row = 0u; row < framebuffer.height(); row++)
	{
		for (size_t col = 0u; col < framebuffer.width(); col++)
		{
			vec3 c{ 0.0f, 0.0f, 0.0f };
			for (size_t s = 0u; s < kNumSamples; s++)
			{
				const float u = ((float)col + randf()) / (float)(framebuffer.width()-1);
				const float v = ((float)row + randf()) / (float)(framebuffer.height()-1);
				c += color(camera.getRay(u, v), world, maxDepth);
			}
			c /= (float)kNumSamples;
			framebuffer.setPixel(row, col,
				(uint8_t)(255.99f * sqrt(c.x())),
				(uint8_t)(255.99f * sqrt(c.y())),
				(uint8_t)(255.99f * sqrt(c.z())));
		}
	}
	framebuffer.saveToFile("C:\\dev\\RayTracer\\image.tga");*/
	
	//MultiThread
	std::vector<vec3> buffer(imageWidth * imageHeight);
	try
	{
		multiThreadLoop(buffer, camera, world, imageWidth, imageHeight, kNumSamples, maxDepth);
		for (size_t row = 0u; row < framebuffer.height(); row++)
		{
			for (size_t col = 0u; col < framebuffer.width(); col++)
			{
				const vec3 pixelColor = buffer[row * imageWidth + col];
				framebuffer.setPixel(row, col, (uint8_t)pixelColor.x(), (uint8_t)pixelColor.y(), (uint8_t)pixelColor.z());
			}
		}
		framebuffer.saveToFile("C:\\dev\\RayTracer\\image.tga");
	}
	catch (const std::exception& exception)
	{
		std::cerr << "ERROR: " << exception.what() << std::endl;
	}

	catch (...)
	{
		std::cerr << "ERROR: unhandled exception" << std::endl;
	}

	return 0;
}
