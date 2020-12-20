#pragma once
#include "math.h"
#include <memory>

class Material;

/**
* Information about a point where a ray intersects geometry
*/
struct HitRecord
{
	vec3 normal; // Surface normal at intersection
	float t; // Value of t (ray parameter) of the intersection
	vec3 intersectionPoint;
	std::shared_ptr<Material> material; // Material of the geometry intersecting ray
};
