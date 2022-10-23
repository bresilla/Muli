#include "muli/motor_joint.h"
#include "muli/world.h"

namespace muli
{

MotorJoint::MotorJoint(RigidBody* _bodyA,
                       RigidBody* _bodyB,
                       const Vec2& _anchor,
                       const WorldSettings& _settings,
                       float _maxForce,
                       float _maxTorque,
                       float _frequency,
                       float _dampingRatio,
                       float _jointMass)
    : Joint(Joint::Type::JointMotor, _bodyA, _bodyB, _settings, _frequency, _dampingRatio, _jointMass)
{
    localAnchorA = MulT(bodyA->GetTransform(), _anchor);
    localAnchorB = MulT(bodyB->GetTransform(), _anchor);
    angleOffset = bodyB->GetAngle() - bodyA->GetAngle();

    linearOffset.SetZero();
    angularOffset = 0.0f;

    maxForce = _maxForce < 0 ? FLT_MAX : Clamp<float>(_maxForce, 0.0f, FLT_MAX);
    maxTorque = _maxTorque < 0 ? FLT_MAX : Clamp<float>(_maxTorque, 0.0f, FLT_MAX);
}

void MotorJoint::Prepare()
{
    // Calculate Jacobian J and effective mass M
    // J = [-I, -skew(ra), I, skew(rb)] // Revolute
    //     [ 0,        -1, 0,        1] // Angle
    // M = (J · M^-1 · J^t)^-1

    ra = bodyA->GetRotation() * localAnchorA;
    rb = bodyB->GetRotation() * localAnchorB;

    Mat2 k0;

    k0[0][0] = bodyA->invMass + bodyB->invMass + bodyA->invInertia * ra.y * ra.y + bodyB->invInertia * rb.y * rb.y;
    k0[1][0] = -bodyA->invInertia * ra.y * ra.x - bodyB->invInertia * rb.y * rb.x;
    k0[0][1] = k0[1][0];
    k0[1][1] = bodyA->invMass + bodyB->invMass + bodyA->invInertia * ra.x * ra.x + bodyB->invInertia * rb.x * rb.x;

    k0[0][0] += gamma;
    k0[1][1] += gamma;

    float k1 = bodyA->invInertia + bodyB->invInertia + gamma;

    m0 = k0.GetInverse();
    m1 = 1.0f / k1;

    Vec2 pa = bodyA->GetPosition() + ra;
    Vec2 pb = bodyB->GetPosition() + rb;

    bias0 = pb - pa + linearOffset;
    bias1 = bodyB->GetAngle() - bodyA->GetAngle() - angleOffset - angularOffset;

    bias0 *= beta * settings.INV_DT;
    bias1 *= beta * settings.INV_DT;

    if (settings.WARM_STARTING)
    {
        ApplyImpulse(linearImpulseSum, angularImpulseSum);
    }
}

void MotorJoint::SolveVelocityConstraint()
{
    // Calculate corrective impulse: Pc
    // Pc = J^t * λ (λ: lagrangian multiplier)
    // λ = (J · M^-1 · J^t)^-1 ⋅ -(J·v+b)

    Vec2 jv0 =
        (bodyB->linearVelocity + Cross(bodyB->angularVelocity, rb)) - (bodyA->linearVelocity + Cross(bodyA->angularVelocity, ra));
    float jv1 = bodyB->angularVelocity - bodyA->angularVelocity;

    Vec2 lambda0 = m0 * -(jv0 + bias0 + linearImpulseSum * gamma);
    float lambda1 = m1 * -(jv1 + bias1 + angularImpulseSum * gamma);

    // Clamp linear impulse
    {
        float maxLinearImpulse = maxForce * settings.DT;
        Vec2 oldLinearImpulse = linearImpulseSum;
        linearImpulseSum += lambda0;

        if (linearImpulseSum.Length2() > maxLinearImpulse * maxLinearImpulse)
        {
            linearImpulseSum.Normalize();
            linearImpulseSum *= maxLinearImpulse;
        }

        lambda0 = linearImpulseSum - oldLinearImpulse;
    }

    // Clamp angular impulse
    {
        float maxAngularImpulse = maxTorque * settings.DT;
        float oldAngularImpulse = angularImpulseSum;
        angularImpulseSum += lambda1;

        angularImpulseSum = Clamp<float>(angularImpulseSum, -maxAngularImpulse, maxAngularImpulse);

        lambda1 = angularImpulseSum - oldAngularImpulse;
    }

    ApplyImpulse(lambda0, lambda1);
}

void MotorJoint::ApplyImpulse(const Vec2& lambda0, float lambda1)
{
    // V2 = V2' + M^-1 ⋅ Pc
    // Pc = J^t ⋅ λ

#if 1
    bodyA->linearVelocity -= bodyA->invMass * lambda0;
    bodyA->angularVelocity -= bodyA->invInertia * (Cross(ra, lambda0) + lambda1);
    bodyB->linearVelocity += bodyB->invMass * lambda0;
    bodyB->angularVelocity += bodyB->invInertia * (Cross(rb, lambda0) + lambda1);
#else
    // Solve for point-to-point constraint
    bodyA->linearVelocity -= lambda0 * bodyA->invMass;
    bodyA->angularVelocity -= bodyA->invInertia * Cross(ra, lambda0);
    bodyB->linearVelocity += lambda0 * bodyB->invMass;
    bodyB->angularVelocity += bodyB->invInertia * Cross(rb, lambda0);

    // Solve for angle constraint
    bodyA->angularVelocity -= lambda1 * bodyA->invInertia;
    bodyB->angularVelocity -= lambda1 * bodyB->invInertia;
#endif
}

} // namespace muli