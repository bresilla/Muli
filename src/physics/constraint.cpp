#include "spe/physics/constraint.h"

namespace spe
{

Constraint::Constraint(RigidBody* _bodyA, RigidBody* _bodyB) :
    bodyA{ _bodyA },
    bodyB{ _bodyB }
{

}

}