#include "muli/broad_phase.h"
#include "muli/util.h"
#include "muli/world.h"

namespace muli
{

void BroadPhase::UpdateDynamicTree(float dt)
{
    for (RigidBody* body = world.bodyList; body; body = body->next)
    {
        // Clear island flag
        body->flag &= ~RigidBody::Flag::flag_island;

        if (body->IsSleeping())
        {
            continue;
        }
        if (body->type == RigidBody::Type::static_body)
        {
            body->flag |= RigidBody::Flag::flag_sleeping;
        }

        for (Collider* collider = body->colliderList; collider; collider = collider->next)
        {
            int32 node = collider->node;
            AABB treeAABB = tree.nodes[node].aabb;
            AABB aabb = collider->GetAABB();

            if (ContainsAABB(treeAABB, aabb) && body->resting < world.settings.SLEEPING_TRESHOLD)
            {
                continue;
            }

            Vec2 d = body->linearVelocity * dt * velocityMultiplier;

            if (d.x > 0.0f)
            {
                aabb.max.x += d.x;
            }
            else
            {
                aabb.min.x += d.x;
            }

            if (d.y > 0.0f)
            {
                aabb.max.y += d.y;
            }
            else
            {
                aabb.min.y += d.y;
            }

            aabb.max += aabbMargin;
            aabb.min -= aabbMargin;

            tree.Remove(collider);
            tree.Insert(collider, aabb);
        }
    }
}

void BroadPhase::FindContacts(const std::function<void(Collider*, Collider*)>& callback) const
{
    for (RigidBody* bodyA = world.bodyList; bodyA; bodyA = bodyA->next)
    {
        for (Collider* colliderA = bodyA->colliderList; colliderA; colliderA = colliderA->next)
        {
            Shape::Type typeA = colliderA->GetType();

            tree.Query(tree.nodes[colliderA->node].aabb, [&](Collider* colliderB) -> bool {
                RigidBody* bodyB = colliderB->body;

                if (bodyA == bodyB)
                {
                    return true;
                }

                Shape::Type typeB = colliderB->GetType();
                if (typeA < typeB)
                {
                    return true;
                }
                else if (typeA == typeB)
                {
                    if (colliderA > colliderB)
                    {
                        return true;
                    }
                }

                callback(colliderA, colliderB);

                return true;
            });
        }
    }
}

} // namespace muli