#include "muli/pulley_joint.h"
#include "muli/world.h"

namespace muli
{

PulleyJoint::PulleyJoint(RigidBody* _bodyA,
                         RigidBody* _bodyB,
                         const Vec2& _anchorA,
                         const Vec2& _anchorB,
                         const Vec2& _groundAnchorA,
                         const Vec2& _groundAnchorB,
                         float _ratio,
                         float _frequency,
                         float _dampingRatio,
                         float _jointMass)
    : Joint(pulley_joint, _bodyA, _bodyB, _frequency, _dampingRatio, _jointMass)
    , impulseSum{ 0.0f }
{
    localAnchorA = MulT(bodyA->GetTransform(), _anchorA);
    localAnchorB = MulT(bodyB->GetTransform(), _anchorB);
    groundAnchorA = _groundAnchorA;
    groundAnchorB = _groundAnchorB;

    ratio = _ratio;
    length = Dist(_anchorA, _groundAnchorA) + Dist(_anchorB, _groundAnchorB);
}

void PulleyJoint::Prepare()
{
    ComputeBetaAndGamma();

    // Compute Jacobian J and effective mass M
    // J = -[ua, ra×ua, r*ub, r*rb×ub]
    // K = (J · M^-1 · J^t)
    //   = iMa + iIa * (ra×ua)^2 + ratio*(iMb + iIb * (rb×ub)^2)
    // M = K^-1

    ra = Mul(bodyA->GetRotation(), localAnchorA - bodyA->sweep.localCenter);
    rb = Mul(bodyB->GetRotation(), localAnchorB - bodyB->sweep.localCenter);

    ua = (bodyA->sweep.c + ra) - groundAnchorA;
    ub = (bodyB->sweep.c + rb) - groundAnchorB;

    float lengthA = ua.Length();
    float lengthB = ub.Length();

    if (lengthA > linear_slop)
    {
        ua *= 1.0f / lengthA;
    }
    else
    {
        ua.SetZero();
    }

    if (lengthB > linear_slop)
    {
        ub *= 1.0f / lengthB;
    }
    else
    {
        ub.SetZero();
    }

    float rua = Cross(ra, ua);
    float rub = Cross(rb, ub);

    // clang-format off
    float k = bodyA->invMass + bodyA->invInertia * rua * rua
            + (bodyB->invMass + bodyB->invInertia * rub * rub) * ratio * ratio
            + gamma;
    // clang-format on

    if (k != 0.0f)
    {
        m = 1.0f / k;
    }

    bias = length - (lengthA + lengthB);
    bias *= beta * settings.inv_dt;

    if (settings.warm_starting)
    {
        ApplyImpulse(impulseSum);
    }
}

void PulleyJoint::SolveVelocityConstraints()
{
    // Compute corrective impulse: Pc
    // Pc = J^t · λ (λ: lagrangian multiplier)
    // λ = (J · M^-1 · J^t)^-1 ⋅ -(J·v+b)

    float jv = -(ratio * (Dot(ub, bodyB->linearVelocity + Cross(bodyB->angularVelocity, rb))) +
                 Dot(ua, bodyA->linearVelocity + Cross(bodyA->angularVelocity, ra)));

    float lambda = m * -(jv + bias + impulseSum * gamma);

    ApplyImpulse(lambda);
    impulseSum += lambda;
}

void PulleyJoint::ApplyImpulse(float lambda)
{
    // V2 = V2' + M^-1 ⋅ Pc
    // Pc = J^t ⋅ λ

    Vec2 pa = -lambda * ua;
    Vec2 pb = -ratio * lambda * ub;

    bodyA->linearVelocity += pa * bodyA->invMass;
    bodyA->angularVelocity += Cross(ra, pa) * bodyA->invInertia;
    bodyB->linearVelocity += pb * bodyB->invMass;
    bodyB->angularVelocity += Cross(rb, pb) * bodyB->invInertia;
}

} // namespace muli