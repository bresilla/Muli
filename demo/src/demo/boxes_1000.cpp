#include "demo.h"

namespace muli
{

class Boxes1000 : public Demo
{
public:
    Boxes1000(Game& game)
        : Demo(game)
    {
        float size = 15.0f;
        float halfSize = size / 2.0f;
        float wallWidth = 0.4f;
        float wallRadius = wallWidth / 2.0f;

        world->CreateCapsule(Vec2{ -halfSize, -halfSize }, Vec2{ halfSize, -halfSize }, wallRadius, RigidBody::Type::static_body);
        world->CreateCapsule(Vec2{ halfSize, -halfSize }, Vec2{ halfSize, halfSize }, wallRadius, RigidBody::Type::static_body);
        world->CreateCapsule(Vec2{ halfSize, halfSize }, Vec2{ -halfSize, halfSize }, wallRadius, RigidBody::Type::static_body);
        world->CreateCapsule(Vec2{ -halfSize, halfSize }, Vec2{ -halfSize, -halfSize }, wallRadius, RigidBody::Type::static_body);

        float r = 0.38f;

        for (int32 i = 0; i < 1000; ++i)
        {
            RigidBody* b = world->CreateBox(r);
            b->SetPosition(RandRange(0.0f, size - wallWidth) - (size - wallWidth) / 2.0f,
                           RandRange(0.0f, size - wallWidth) - (size - wallWidth) / 2.0f);
        }

        camera.position = { 0.0f, 0.0f };
        camera.scale = { 3.f, 3.f };
    }

    static Demo* Create(Game& game)
    {
        return new Boxes1000(game);
    }
};

DemoFrame boxes_1000{ "1000 Boxes", Boxes1000::Create };

} // namespace muli
