#include "spe/polytope.h"

namespace spe
{

ClosestEdgeInfo Polytope::GetClosestEdge() const
{
    uint32_t minIndex = 0;
    float minDistance = FLT_MAX;
    Vec2 minNormal{ 0.0f };

    for (uint32_t i = 0; i < vertices.Count(); i++)
    {
        uint32_t j = (i + 1) % vertices.Count();

        const Vec2& vertexI = vertices[i];
        const Vec2& vertexJ = vertices[j];

        const Vec2 edge = vertexJ - vertexI;

        Vec2 normal = Cross(edge, 1.0f).Normalized();
        float distance = Dot(normal, vertexI);

        if (distance < 0)
        {
            distance *= -1;
            normal *= -1;
        }

        if (distance < minDistance)
        {
            minDistance = distance;
            minNormal = normal;
            minIndex = i;
        }
    }

    return ClosestEdgeInfo{ minIndex, minDistance, minNormal };
}

} // namespace spe