#pragma once

#include "rigidbody.h"
#include "util.h"

#define MAX_LOCAL_POLYGON_VERTICES 8

namespace muli
{

class Polygon : public RigidBody
{
public:
    Polygon(const Vec2* _vertices,
            int32 _vertexCount,
            RigidBody::Type _type = dynamic_body,
            bool _resetCenter = true,
            float _radius = DEFAULT_RADIUS,
            float _density = DEFAULT_DENSITY);
    virtual ~Polygon() noexcept;

    virtual void SetDensity(float d) override;
    virtual void SetMass(float m) override;

    virtual float GetArea() const override;
    virtual AABB GetAABB() const override;
    virtual ContactPoint Support(const Vec2& localDir) const override;
    virtual Edge GetFeaturedEdge(const Vec2& dir) const override;
    virtual bool TestPoint(const Vec2& p) const override;
    virtual Vec2 GetClosestPoint(const Vec2& p) const override;
    virtual bool RayCast(const RayCastInput& input, RayCastOutput* output) const override;

    const Vec2* GetVertices() const;
    const Vec2* GetNormals() const;
    int32 GetVertexCount() const;

    Edge GetIntersectingEdge(const Vec2& dir) const;

protected:
    Vec2* vertices;
    Vec2* normals;
    int32 vertexCount;
    float area;

private:
    Vec2 localVertices[MAX_LOCAL_POLYGON_VERTICES];
    Vec2 localNormals[MAX_LOCAL_POLYGON_VERTICES];
};

inline float Polygon::GetArea() const
{
    return area;
}

inline const Vec2* Polygon::GetVertices() const
{
    return vertices;
}

inline const Vec2* Polygon::GetNormals() const
{
    return normals;
}

inline int32 Polygon::GetVertexCount() const
{
    return vertexCount;
}

} // namespace muli