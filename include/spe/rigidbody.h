#pragma once

#include "aabb.h"
#include "common.h"
#include "settings.h"

namespace spe
{

struct Node;
struct ContactEdge;
struct JointEdge;

// Children: Polygon, Circle
class RigidBody
{
    friend class World;
    friend class Island;

    friend class Contact;
    friend class ContactSolver;
    friend class BlockSolver;
    friend class PositionSolver;

    friend class Joint;
    friend class GrabJoint;
    friend class RevoluteJoint;
    friend class DistanceJoint;

    friend class AABBTree;
    friend class BroadPhase;
    friend class ContactManager;

public:
    enum Shape : uint8_t
    {
        ShapeCircle,
        ShapePolygon,
        ShapeEdge,
    };

    enum Type : uint8_t
    {
        Static,
        Dynamic,
    };

    RigidBody(RigidBody::Type _type, RigidBody::Shape _shape);
    virtual ~RigidBody() noexcept;

    RigidBody(const RigidBody&) = delete;
    RigidBody& operator=(const RigidBody&) = delete;

    RigidBody(RigidBody&& _other) noexcept;
    RigidBody& operator=(RigidBody&& _other) = delete;

    virtual void SetDensity(float d) = 0;
    virtual void SetMass(float m) = 0;
    virtual float GetArea() const = 0;
    virtual AABB GetAABB() const = 0;

    const Transform& GetTransform() const;
    void SetTransform(const Vec2& pos, float angle);
    const Vec2 GetPosition() const;
    void SetPosition(const Vec2& _pos);
    void SetPosition(float x, float y);
    const Rotation& GetRotation() const;
    void SetRotation(const Rotation& _rotation);
    void SetRotation(float _angle);
    float GetAngle() const;
    void Translate(const Vec2& d);
    void Rotate(float a);

    void Awake();

    float GetDensity() const;
    float GetMass() const;
    float GetInverseMass() const;
    float GetInertia() const;
    float GetInverseInertia() const;
    float GetFriction() const;
    void SetFriction(float _friction);
    float GetRestitution() const;
    void SetRestitution(float _restitution);
    float GetSurfaceSpeed() const;
    void SetSurfaceSpeed(float _surfaceSpeed);
    Vec2 GetLinearVelocity() const;
    void SetLinearVelocity(const Vec2& _linearVelocity);
    float GetAngularVelocity() const;
    void SetAngularVelocity(float _angularVelocity);
    Vec2 GetForce() const;
    void SetForce(const Vec2& _force);
    float GetTorque() const;
    void SetTorque(float _torque);
    Type GetType() const;
    Shape GetShape() const;
    bool IsSleeping() const;

    uint32_t GetID() const;
    uint32_t GetIslandID() const;
    const Node* GetNode() const;
    RigidBody* GetPrev() const;
    RigidBody* GetNext() const;
    World* GetWorld() const;

    // Callbacks
    std::function<void(RigidBody*)> OnDestroy = nullptr;

protected:
    // Center of mass in local space = (0, 0)
    Transform transform;

    Vec2 force{ 0.0f };  // N
    float torque = 0.0f; // N⋅m

    Vec2 linearVelocity{ 0.0f };  // m/s
    float angularVelocity = 0.0f; // rad/s

    float density; // kg/m²
    float mass;    // kg
    float invMass;
    float inertia; // kg⋅m²
    float invInertia;

    float friction = DEFAULT_FRICTION;
    float restitution = DEFAULT_RESTITUTION;
    float surfaceSpeed = DEFAULT_SURFACESPEED; // m/s (Tangential speed)

    Shape shape;
    Type type;

private:
    bool moved = false;

    World* world = nullptr;
    uint32_t id = 0;
    uint32_t islandID = 0;

    ContactEdge* contactList = nullptr;
    JointEdge* jointList = nullptr;

    float resting = 0.0f;
    bool sleeping = false;

    Node* node = nullptr;
    RigidBody* prev = nullptr;
    RigidBody* next = nullptr;
};

inline const Transform& RigidBody::GetTransform() const
{
    return transform;
}

inline void RigidBody::SetTransform(const Vec2& _pos, float _angle)
{
    transform.position = _pos;
    transform.rotation = _angle;
}

inline const Vec2 RigidBody::GetPosition() const
{
    return transform.position;
}

inline void RigidBody::SetPosition(const Vec2& _pos)
{
    transform.position = _pos;
}

inline void RigidBody::SetPosition(float x, float y)
{
    transform.position = Vec2{ x, y };
}

inline const Rotation& RigidBody::GetRotation() const
{
    return transform.rotation;
}

inline void RigidBody::SetRotation(const Rotation& _rotation)
{
    transform.rotation = _rotation;
}

inline void RigidBody::SetRotation(float _angle)
{
    transform.rotation = _angle;
}

inline float RigidBody::GetAngle() const
{
    return transform.rotation.angle;
}

inline void RigidBody::Translate(const Vec2& d)
{
    transform.position += d;
}

inline void RigidBody::Rotate(float a)
{
    transform.rotation += a;
}

inline float RigidBody::GetDensity() const
{
    return density;
}

inline float RigidBody::GetMass() const
{
    return mass;
}

inline float RigidBody::GetInverseMass() const
{
    return invMass;
}

inline float RigidBody::GetInertia() const
{
    return inertia;
}

inline float RigidBody::GetInverseInertia() const
{
    return invInertia;
}

inline const Node* RigidBody::GetNode() const
{
    return node;
}

inline void RigidBody::Awake()
{
    resting = 0.0f;
    sleeping = false;
}

inline float RigidBody::GetFriction() const
{
    return friction;
}

inline void RigidBody::SetFriction(float _friction)
{
    friction = _friction;
}

inline float RigidBody::GetRestitution() const
{
    return restitution;
}

inline void RigidBody::SetRestitution(float _restitution)
{
    restitution = _restitution;
}

inline float RigidBody::GetSurfaceSpeed() const
{
    return surfaceSpeed;
}

inline void RigidBody::SetSurfaceSpeed(float _surfaceSpeed)
{
    surfaceSpeed = _surfaceSpeed;
}

inline Vec2 RigidBody::GetLinearVelocity() const
{
    return linearVelocity;
}

inline void RigidBody::SetLinearVelocity(const Vec2& _linearVelocity)
{
    linearVelocity = _linearVelocity;
}

inline float RigidBody::GetAngularVelocity() const
{
    return angularVelocity;
}

inline void RigidBody::SetAngularVelocity(float _angularVelocity)
{
    angularVelocity = _angularVelocity;
}

inline Vec2 RigidBody::GetForce() const
{
    return force;
}

inline void RigidBody::SetForce(const Vec2& _force)
{
    force = _force;
}

inline float RigidBody::GetTorque() const
{
    return torque;
}

inline void RigidBody::SetTorque(float _torque)
{
    torque = _torque;
}

inline RigidBody::Type RigidBody::GetType() const
{
    return type;
}

inline RigidBody::Shape RigidBody::GetShape() const
{
    return shape;
}

inline bool RigidBody::IsSleeping() const
{
    return sleeping;
}

inline uint32_t RigidBody::GetID() const
{
    return id;
}

inline uint32_t RigidBody::GetIslandID() const
{
    return islandID;
}

inline RigidBody* RigidBody::GetPrev() const
{
    return prev;
}

inline RigidBody* RigidBody::GetNext() const
{
    return next;
}

inline World* RigidBody::GetWorld() const
{
    return world;
}

} // namespace spe