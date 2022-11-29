#pragma once

#include "shape.h"
#include "util.h"

namespace muli
{

class CircleShape : public Shape
{
public:
    CircleShape(float radius, const Vec2& center = Vec2{ 0.0f });
    ~CircleShape() = default;

    virtual void ComputeMass(float density, MassData* outMassData) const override;
    virtual ContactPoint Support(const Vec2& localDir) const override;
    virtual Edge GetFeaturedEdge(const Transform& transform, const Vec2& dir) const override;
    virtual void ComputeAABB(const Transform& transform, AABB* outAABB) const override;
    virtual bool TestPoint(const Transform& transform, const Vec2& q) const override;
    virtual Vec2 GetClosestPoint(const Transform& transform, const Vec2& q) const override;
    virtual bool RayCast(const Transform& transform, const RayCastInput& input, RayCastOutput* output) const override;

protected:
    virtual Shape* Clone(PredefinedBlockAllocator* allocator) const override;
};

inline Shape* CircleShape::Clone(PredefinedBlockAllocator* allocator) const
{
    void* mem = allocator->Allocate(sizeof(CircleShape));
    CircleShape* shape = new (mem) CircleShape(*this);
    return shape;
}

inline Edge CircleShape::GetFeaturedEdge(const Transform& transform, const Vec2& dir) const
{
    return Edge{ transform * center, transform * center };
}

inline ContactPoint CircleShape::Support(const Vec2& localDir) const
{
    return ContactPoint{ Vec2{ 0.0f, 0.0f }, -1 };
}

inline void CircleShape::ComputeMass(float density, MassData* outMassData) const
{
    outMassData->mass = density * area;
    outMassData->inertia = outMassData->mass * (0.5f * radius * radius + Length2(center));
    outMassData->centerOfMass = center;
}

inline void CircleShape::ComputeAABB(const Transform& transform, AABB* outAABB) const
{
    Vec2 p = transform * center;

    outAABB->min = Vec2{ p.x - radius, p.y - radius };
    outAABB->max = Vec2{ p.x + radius, p.y + radius };
}

inline bool CircleShape::TestPoint(const Transform& transform, const Vec2& q) const
{
    Vec2 localQ = MulT(transform, q);
    Vec2 d = center - localQ;

    return Dot(d, d) <= radius * radius;
}

} // namespace muli