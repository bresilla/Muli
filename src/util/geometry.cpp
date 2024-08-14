#include <algorithm>
#include <list>
#include <random>
#include <unordered_map>

#include "muli/geometry.h"
#include "muli/hash.h"

namespace muli
{

void ComputeConvexHull(const Vec2* vertices, int32 vertexCount, Vec2* outVertices, int32* outVertexCount)
{
    if (vertexCount < 3)
    {
        memcpy(outVertices, vertices, vertexCount * sizeof(Vec2));
        *outVertexCount = vertexCount;
        return;
    }

    // Find the lowest vertex
    int32 index = 0;
    for (int32 i = 1; i < vertexCount; ++i)
    {
        if (vertices[i].y < vertices[index].y)
        {
            index = i;
        }
    }
    Vec2 bottom = vertices[index];

    Vec2* sorted = new Vec2[vertexCount];
    memcpy(sorted, vertices, vertexCount * sizeof(Vec2));
    std::swap(sorted[index], sorted[0]);

    // Sort the vertices based on the angle related to the lowest vertex.
    std::sort(sorted + 1, sorted + vertexCount, [&](const Vec2& a, const Vec2& b) -> bool {
        Vec2 ra = a - bottom;
        Vec2 rb = b - bottom;

        float angleA = Atan2(ra.y, ra.x);
        float angleB = Atan2(rb.y, rb.x);

        if (angleA == angleB)
        {
            return Dist2(bottom, a) < Dist2(bottom, b);
        }
        else
        {
            return angleA < angleB;
        }
    });

    // Discard overlapped bottom vertices
    int32 i = 0;
    while (i < vertexCount)
    {
        Vec2& v0 = sorted[i];
        Vec2& v1 = sorted[(i + 1) % vertexCount];

        if (v0 == v1)
        {
            ++i;
        }
        else
        {
            break;
        }
    }

    // Initialize stack
    int32 sp = 0;
    *outVertexCount = vertexCount - i;
    outVertices[sp++] = sorted[i++];
    outVertices[sp++] = sorted[i++];

    while (i < vertexCount)
    {
        Vec2& v = sorted[i];
        int32 l = sp;

        if (v == outVertices[l - 1])
        {
            ++i;
            continue;
        }

        Vec2 d1 = outVertices[l - 1] - outVertices[l - 2];
        Vec2 d2 = v - outVertices[l - 1];

        if (Cross(d1, d2) <= 0)
        {
            --sp;

            if (l < 3)
            {
                outVertices[sp++] = v;
                break;
            }
        }
        else
        {
            outVertices[sp++] = v;
            ++i;
        }
    }

    delete[] sorted;
}

std::vector<Vec2> ComputeConvexHull(std::span<const Vec2> vertices)
{
    if (vertices.size() < 3)
    {
        return std::vector<Vec2>(vertices.begin(), vertices.end());
    }

    size_t index = 0;
    for (size_t i = 1; i < vertices.size(); ++i)
    {
        if (vertices[i].y < vertices[index].y)
        {
            index = i;
        }
    }
    Vec2 bottom = vertices[index];

    std::vector<Vec2> sorted(vertices.begin(), vertices.end());
    std::swap(sorted[index], sorted[0]);

    std::sort(sorted.begin() + 1, sorted.end(), [&](const Vec2& a, const Vec2& b) -> bool {
        Vec2 ra = a - bottom;
        Vec2 rb = b - bottom;

        float angleA = Atan2(ra.y, ra.x);
        float angleB = Atan2(rb.y, rb.x);

        if (angleA == angleB)
        {
            return Dist2(bottom, a) < Dist2(bottom, b);
        }
        else
        {
            return angleA < angleB;
        }
    });

    // Discard overlapped bottom vertices
    size_t i = 0;
    while (i < sorted.size())
    {
        Vec2& v0 = sorted[i];
        Vec2& v1 = sorted[(i + 1) % sorted.size()];

        if (v0 == v1)
        {
            i++;
        }
        else
        {
            break;
        }
    }

    std::vector<Vec2> s;
    s.push_back(sorted[i++]);
    s.push_back(sorted[i++]);

    while (i < sorted.size())
    {
        Vec2& v = sorted[i];
        size_t l = s.size();

        if (v == s[l - 1])
        {
            ++i;
            continue;
        }

        Vec2 d1 = s[l - 1] - s[l - 2];
        Vec2 d2 = v - s[l - 1];

        if (Cross(d1, d2) <= 0)
        {
            s.pop_back();

            if (l < 3)
            {
                s.push_back(v);
                break;
            }
        }
        else
        {
            s.push_back(v);
            ++i;
        }
    }

    return s;
}

static Circle ComputeCircle2(const Vec2& a, const Vec2& b)
{
    Vec2 center{ (a.x + b.x) / 2, (a.y + b.y) / 2 };
    float radius = Dist(a, b) / 2;

    return Circle{ radius, center };
}

static Circle ComputeCircle3(const Vec2& a, const Vec2& b, const Vec2& c)
{
    // Solve linear equations to find circumcircle
    float d = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));

    float nx =
        ((a.x * a.x + a.y * a.y) * (b.y - c.y) + (b.x * b.x + b.y * b.y) * (c.y - a.y) + (c.x * c.x + c.y * c.y) * (a.y - b.y));
    float ny =
        ((a.x * a.x + a.y * a.y) * (c.x - b.x) + (b.x * b.x + b.y * b.y) * (a.x - c.x) + (c.x * c.x + c.y * c.y) * (b.x - a.x));

    Vec2 center{ nx / d, ny / d };
    float radius = Dist(center, a);

    return Circle{ radius, center };
}

// Welzl algorithm (https://en.wikipedia.org/wiki/Smallest-circle_problem)
// Compute minimum circle containing all points
// P: original all vertices
// R: vertices (n <= 3) to define minimum circle
static Circle Welzl(const std::vector<Vec2>& P, std::vector<Vec2>& R, int32 n)
{
    if (n == 0 || R.size() == 3)
    {
        // Handle trivial cases
        switch (R.size())
        {
        case 0:
            return Circle{ 0, { 0, 0 } };
        case 1:
            return Circle{ 0, R[0] };
        case 2:
            return ComputeCircle2(R[0], R[1]);

        default:
            return ComputeCircle3(R[0], R[1], R[2]);
        }
    }

    // Select random point.
    Vec2 p = P[n - 1];

    // Find minimum circle without current point p.
    Circle D = Welzl(P, R, n - 1);

    // If the returned circle also contains current point,
    // it is the minimum circle for the whole of P and is returned.
    if (D.TestPoint(identity, p))
    {
        return D;
    }
    else
    {
        // Otherwise, current point must lie on the boundary of the resulting circle.
        // It recurses, but with the set R of points known to be on the boundary.

        R.push_back(p);
        Circle newCircle = Welzl(P, R, n - 1);
        R.pop_back();

        return newCircle;
    }
}

Circle ComputeCircle(std::span<Vec2> points)
{
    std::vector<Vec2> R;
    int32 n = int32(points.size());

    std::vector<Vec2> copy(points.begin(), points.end());
    std::shuffle(copy.begin(), copy.end(), std::default_random_engine{});

    return Welzl(copy, R, n);
}

struct TriEdge
{
    Vec2 p0, p1;

    TriEdge() = default;
    TriEdge(const Vec2& p0, const Vec2& p1)
        : p0{ p0 }
        , p1{ p1 }
    {
    }

    Vec2& operator[](int32 index)
    {
        index %= 2;

        switch (index)
        {
        case 0:
            return p0;
        default:
            return p1;
        }
    }

    bool Intersect(const TriEdge& other) const
    {
        Vec2 d = p1 - p0;
        if (Cross(d, other.p0 - p0) * Cross(d, other.p1 - p0) >= 0)
        {
            return false;
        }
        d = other.p1 - other.p0;

        if (Cross(d, p0 - other.p0) * Cross(d, p1 - other.p0) >= 0)
        {
            return false;
        }

        return true;
    }
};

inline bool operator==(const TriEdge& a, const TriEdge& b)
{
    return a.p0 == b.p0 && a.p1 == b.p1;
}

inline TriEdge operator~(const TriEdge& e)
{
    return TriEdge{ e.p1, e.p0 };
}

struct TriEdgeHash
{
    size_t operator()(const TriEdge& e) const
    {
        return Hash(e);
    }
};

struct Tri
{
    Vec2 p0, p1, p2;

    Tri() = default;
    Tri(const Vec2& p0, const Vec2& p1, const Vec2& p2)
        : p0{ p0 }
        , p1{ p1 }
        , p2{ p2 }
    {
    }

    Vec2& operator[](int32 index)
    {
        index %= 3;

        switch (index)
        {
        case 0:
            return p0;
        case 1:
            return p1;
        default:
            return p2;
        }
    }

    const Vec2& operator[](int32 index) const
    {
        index %= 3;

        switch (index)
        {
        case 0:
            return p0;
        case 1:
            return p1;
        default:
            return p2;
        }
    }

    bool HasEdge(const TriEdge& edge) const
    {
        if (edge.p0 == p0 && edge.p1 == p1) return true;
        if (edge.p0 == p1 && edge.p1 == p0) return true;

        if (edge.p0 == p1 && edge.p1 == p2) return true;
        if (edge.p0 == p2 && edge.p1 == p1) return true;

        if (edge.p0 == p2 && edge.p1 == p0) return true;
        if (edge.p0 == p0 && edge.p1 == p2) return true;

        return false;
    }

    bool HasVertex(const Vec2& p) const
    {
        return p0 == p || p1 == p || p2 == p;
    }

    Vec2 GetCenter() const
    {
        return (p0 + p1 + p2) / 3;
    }

    void GetEdges(TriEdge edges[3]) const
    {
        edges[0] = TriEdge{ p0, p1 };
        edges[1] = TriEdge{ p1, p2 };
        edges[2] = TriEdge{ p2, p0 };
    }

    TriEdge GetEdge(int32 index) const
    {
        index %= 3;

        switch (index)
        {
        case 0:
            return TriEdge(p0, p1);
        case 1:
            return TriEdge(p1, p2);
        default:
            return TriEdge(p2, p0);
        }
    }

    int32 GetIndex(const Vec2& p) const
    {
        if (p == p0)
            return 0;
        else if (p == p1)
            return 1;
        else if (p == p2)
            return 2;
        else
            return -1;
    }
};

inline bool operator==(const Tri& a, const Tri& b)
{
    return a.p0 == b.p0 && a.p1 == b.p1 && a.p2 == b.p2;
}

struct TriHash
{
    size_t operator()(const Tri& t) const
    {
        return Hash(t);
    }
};

inline bool RayCastEdge(Vec2 o, Vec2 d, const TriEdge& edge)
{
    Vec2 e = edge.p1 - edge.p0;
    Vec2 o2a = edge.p0 - o;

    float c = Cross(d, e);

    if (c == 0)
    {
        return false;
    }

    float t = Cross(o2a, e) / c;
    float u = Cross(o2a, d) / c;

    return (t >= 0 && u >= 0 && u <= 1);
}

inline bool Contains(std::vector<TriEdge>& edges, const Vec2& p)
{
    int32 count = 0;
    for (TriEdge& e : edges)
    {
        if (RayCastEdge(p, Vec2(0, 1), e))
        {
            ++count;
        }
    }

    return count % 2 == 1;
}

// https://en.wikipedia.org/wiki/Bowyer%E2%80%93Watson_algorithm
std::vector<Polygon> ComputeTriangles(std::span<Vec2> v, std::span<Vec2> constraints, bool removeOutliers)
{
    std::vector<TriEdge> constraintEdges;
    if (constraints.size() > 1)
    {
        size_t i0 = constraints.size() - 1;
        for (size_t i1 = 0; i1 < constraints.size(); ++i1)
        {
            constraintEdges.push_back(TriEdge{ constraints[i0], constraints[i1] });
            i0 = i1;
        }
    }

    std::vector<Vec2> vertices(v.begin(), v.end());
    vertices.insert(vertices.end(), constraints.begin(), constraints.end());

    // Prepare super triangle
    AABB bounds(Vec2{ max_value, max_value }, -Vec2{ max_value, max_value });

    for (const Vec2& p : vertices)
    {
        bounds = AABB::Union(bounds, p);
    }

    Vec2 extents = bounds.GetExtents();

    const float margin = 0.1f;
    Vec2 p0{ bounds.min.x - extents.x - margin, bounds.min.y - margin };
    Vec2 p1{ bounds.max.x + extents.x + margin, bounds.min.y - margin };
    Vec2 p2{ bounds.min.x + extents.x / 2, bounds.max.y + extents.y + margin };

    Tri super{ p0, p1, p2 };

    std::unordered_set<Tri, TriHash> tris;
    tris.insert(super);

    for (const Vec2& p : vertices)
    {
        std::vector<Tri> badTris;

        for (const Tri& t : tris)
        {
            Circle c = ComputeCircle3(t.p0, t.p1, t.p2);
            if (c.TestPoint(identity, p))
            {
                badTris.push_back(t);
            }
        }

        std::vector<TriEdge> poly;

        for (const Tri& t : badTris)
        {
            TriEdge edges[3];
            t.GetEdges(edges);

            for (const TriEdge& e : edges)
            {
                bool hasSharedEdge = false;
                for (const Tri& other : badTris)
                {
                    if (t == other)
                    {
                        continue;
                    }

                    if (other.HasEdge(e))
                    {
                        hasSharedEdge = true;
                    }
                }

                if (!hasSharedEdge)
                {
                    poly.push_back(e);
                }
            }
        }

        for (const Tri& t : badTris)
        {
            tris.erase(t);
        }

        for (const TriEdge& e : poly)
        {
            tris.insert(Tri{ e.p0, e.p1, p });
        }
    }

    std::unordered_map<TriEdge, const Tri*, TriEdgeHash> edge2Tri;

    for (const Tri& t : tris)
    {
        TriEdge triEdges[3];
        t.GetEdges(triEdges);

        for (const TriEdge& e : triEdges)
        {
            muliAssert(!edge2Tri.contains(e));
            edge2Tri.insert({ e, &t });
        }
    }

    for (const TriEdge& ce : constraintEdges)
    {
        if (edge2Tri.contains(ce) || edge2Tri.contains(~ce))
        {
            continue;
        }

        while (true)
        {
            std::vector<TriEdge> badEdges;
            std::unordered_set<TriEdge, TriEdgeHash> resolved;

            // Collect all bad edges
            for (const Tri& t : tris)
            {
                TriEdge triEdges[3];
                t.GetEdges(triEdges);

                for (const TriEdge& e : triEdges)
                {
                    if (e.Intersect(ce))
                    {
                        badEdges.push_back(e);
                    }
                }
            }

            if (badEdges.size() == 0)
            {
                break;
            }

            // Resolve all bad edges
            for (size_t i = 0; i < badEdges.size(); ++i)
            {
                const TriEdge& be = badEdges[i];
                if (resolved.contains(be))
                {
                    continue;
                }

                const Tri* t1 = edge2Tri[be];
                const Tri* t2 = edge2Tri[~be];

                Vec2 p0 = be.p0;
                Vec2 p1 = (*t2)[t2->GetIndex(p0) + 1];
                Vec2 p2 = be.p1;
                Vec2 p3 = (*t1)[t1->GetIndex(p0) - 1];

                // Check convexity
                float s = Cross(p1 - p0, p2 - p1);
                if (Cross(p2 - p1, p3 - p2) * s < 0) continue;
                if (Cross(p3 - p2, p0 - p3) * s < 0) continue;
                if (Cross(p0 - p3, p1 - p0) * s < 0) continue;

                resolved.insert(~be);
                resolved.insert(be);

                // Flip edge
                {
                    TriEdge triEdges[3];
                    t1->GetEdges(triEdges);
                    for (const TriEdge& e : triEdges)
                    {
                        edge2Tri.erase(e);
                    }
                }
                {
                    TriEdge triEdges[3];
                    t2->GetEdges(triEdges);
                    for (const TriEdge& e : triEdges)
                    {
                        edge2Tri.erase(e);
                    }
                }
                tris.erase(*t1);
                tris.erase(*t2);

                t1 = &(*(tris.emplace(p0, p1, p3).first));
                t2 = &(*(tris.emplace(p1, p2, p3).first));
                {
                    TriEdge triEdges[3];
                    t1->GetEdges(triEdges);
                    for (const TriEdge& e : triEdges)
                    {
                        edge2Tri.insert({ e, t1 });
                    }
                }
                {
                    TriEdge triEdges[3];
                    t2->GetEdges(triEdges);
                    for (const TriEdge& e : triEdges)
                    {
                        edge2Tri.insert({ e, t2 });
                    }
                }
            }
        }
    }

    std::vector<Polygon> res;
    for (const Tri& t : tris)
    {
        if (t.HasVertex(super.p0) || t.HasVertex(super.p1) || t.HasVertex(super.p2))
        {
            continue;
        }

        if (!removeOutliers || Contains(constraintEdges, t.GetCenter()))
        {
            res.push_back(Polygon{ t.p0, t.p1, t.p2 });
        }
    }

    return res;
}

} // namespace muli