#pragma once

#include "hittable.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

class HittableList : public Hittable {
public:
    HittableList() {}
    HittableList(shared_ptr<Hittable> object) { add(object); }

    void clear() { objects.clear(); }
    void add(shared_ptr<Hittable> object) { objects.push_back(object); }

    bool hitTest(const Ray& r, float tmin, float tmax, HitRecord& hitRecord) const override
    {
        HitRecord tempRec;
        bool hitAnything = false;
        auto closestSoFar = tmax;

        for (const auto& object : objects)
        {
            if (object->hitTest(r, tmin, closestSoFar, tempRec))
            {
                hitAnything = true;
                closestSoFar = tempRec.t;
                hitRecord = tempRec;
            }
        }

        return hitAnything;
    }

public:
    std::vector<shared_ptr<Hittable>> objects;
};
