#include "demo.h"

namespace muli
{

class SingleBox : public Demo
{
public:
    SingleBox(Game& game)
        : Demo(game)
    {
        RigidBody* ground = world->CreateBox(100.0f, 0.4f, RigidBody::Type::static_body);

        RigidBody* box = world->CreateBox(0.4f);
        box->SetPosition(0.0f, 5.0f);
        box->SetAngularVelocity(LinearRand(-12.0f, 12.0f));
    }

    static Demo* Create(Game& game)
    {
        return new SingleBox(game);
    }
};

DemoFrame single_box{ "Single Box", SingleBox::Create };

} // namespace muli
