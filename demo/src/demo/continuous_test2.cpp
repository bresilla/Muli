#include "demo.h"
#include "game.h"
#include "window.h"

namespace muli
{

static bool drawTrajectory = true;
static int selection = 1;
const char* items[] = { "Circle", "Box", "Capsule", "Rounded polygon", "Random" };

class ContinuousTest2 : public Demo
{
public:
    RigidBody* target;

    ContinuousTest2(Game& game)
        : Demo(game)
    {
        RigidBody* ground = world->CreateBox(100.0f, 0.4f, RigidBody::Type::static_body);

        float start = 0.5f;
        float size = 0.3f;
        float gap = 0.25f;

        switch (selection)
        {
        case 0:
            target = world->CreateCircle(0.2f);
            break;
        case 1:
            target = world->CreateBox(0.4f);
            break;
        case 2:
            target = world->CreateCapsule(1.2f, 0.05f);
            break;
        case 3:
            target = world->CreateRandomConvexPolygon(0.28f, 6, RigidBody::Type::dynamic_body, 0.05f);
            target->UserFlag |= UserFlag::render_polygon_radius;
            break;
        case 4:
            target = world->CreateRandomConvexPolygon(0.28f, LinearRand(6, 8));
            break;

        default:
            muliAssert(false);
            break;
        }

        target->SetRotation(LinearRand(-MULI_PI / 2.0f, MULI_PI / 2.0f));
        target->SetPosition(-1.0f, 3.0f);
        target->SetContinuous(true);
        target->SetLinearVelocity(100.0f, 0.0f);

        world->CreateCapsule(Vec2{ 3.6f, 6.0f }, Vec2{ 3.6f, 0.0f }, 0.05f, RigidBody::Type::static_body);
        world->CreateCapsule(Vec2{ 3.6f, 6.0f }, Vec2{ -1.0f, 6.0f }, 0.05f, RigidBody::Type::static_body);

        settings.continuous = true;
    }

    void Render() override
    {
        if (drawTrajectory)
        {
            Transform t = target->GetTransform();
            t.rotation = t.rotation.GetAngle() + target->GetAngularVelocity() * settings.dt;
            t.position += target->GetLinearVelocity() * settings.dt;

            game.GetRigidBodyRenderer().Render(target->GetColliderList(), t);
        }
    }

    void UpdateUI() override
    {
        ImGui::SetNextWindowPos({ Window::Get().GetWindowSize().x - 5, 5 }, ImGuiCond_Once, { 1.0f, 0.0f });
        ImGui::SetNextWindowSize({ 200, 180 }, ImGuiCond_Once);

        if (ImGui::Begin("Control"))
        {
            ImGui::Text("Shapes");
            ImGui::PushID(0);
            if (ImGui::ListBox("", &selection, items, IM_ARRAYSIZE(items)))
            {
                game.RestartDemo();
            }
            ImGui::PopID();

            ImGui::Checkbox("Draw trajectory", &drawTrajectory);
        }
        ImGui::End();
    }

    ~ContinuousTest2() {}

    static Demo* Create(Game& game)
    {
        return new ContinuousTest2(game);
    }
};

DemoFrame continuous_test2{ "Continuous test 2", ContinuousTest2::Create };

} // namespace muli