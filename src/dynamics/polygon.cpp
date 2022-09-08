#include "spe/polygon.h"

namespace spe
{

Polygon::Polygon(std::vector<Vec2> _vertices, Type _type, bool _resetPosition, float _density)
    : RigidBody(std::move(_type), RigidBody::Shape::ShapePolygon)
    , vertices{ std::move(_vertices) }
{
    Vec2 centerOfMass{ 0.0f };
    size_t count = vertices.size();

    for (size_t i = 0; i < count; i++)
    {
        centerOfMass += vertices[i];
    }

    centerOfMass *= 1.0f / count;

    float _area = 0;

    vertices[0] -= centerOfMass;
    radius = spe::length(vertices[0]);

    for (uint32_t i = 1; i < count; i++)
    {
        vertices[i] -= centerOfMass;
        radius = spe::max(radius, spe::length(vertices[i]));
        _area += cross(vertices[i - 1], vertices[i]);
    }
    _area += cross(vertices[count - 1], vertices[0]);

    area = spe::abs(_area) / 2.0f;

    if (type == Dynamic)
    {
        assert(_density > 0);

        density = _density;
        mass = _density * area;
        invMass = 1.0f / mass;
        inertia = compute_convex_polygon_inertia(vertices, mass);
        invInertia = 1.0f / inertia;
    }

    if (!_resetPosition)
    {
        Translate(centerOfMass);
    }
}

void Polygon::SetMass(float _mass)
{
    assert(_mass > 0);

    density = _mass / area;
    mass = _mass;
    invMass = 1.0f / mass;
    inertia = compute_convex_polygon_inertia(vertices, mass);
    invInertia = 1.0f / inertia;
}

void Polygon::SetDensity(float _density)
{
    assert(_density > 0);

    density = _density;
    mass = _density * area;
    invMass = 1.0f / mass;
    inertia = compute_convex_polygon_inertia(vertices, mass);
    invInertia = 1.0f / inertia;
}

} // namespace spe