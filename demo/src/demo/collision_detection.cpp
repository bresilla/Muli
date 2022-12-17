#include "demo.h"
#include "game.h"
#include "window.h"

namespace muli
{

class CollisionDetection : public Demo
{
public:
    CollisionDetection(Game& game)
        : Demo(game)
    {
        options.draw_outline_only = true;
        options.show_contact_normal = true;
        options.show_contact_point = true;
        settings.apply_gravity = false;
        settings.velocity_iterations = 0;
        settings.position_iterations = 0;

        float size = 1.0f;
        float range = size * 0.7f;

        for (int32 i = 0; i < 2; ++i)
        {
            float r = LinearRand(0.0f, 3.0f);
            RigidBody* b;

            if (r < 1.0f)
            {
                b = world->CreateBox(size);
            }
            else if (r < 2.0f)
            {
                b = world->CreateCircle(size / 2.0f);
            }
            else
            {
                b = world->CreateCapsule(size, size / 2.0f);
            }

            b->SetPosition(LinearRand(Vec2{ -range, -range }, Vec2{ range, range }));
            b->SetRotation(LinearRand(0.0f, MULI_PI));
        }

        camera.position = 0.0f;
        camera.scale = 0.5f;
    }

    void UpdateUI() override
    {
        ImGui::SetNextWindowPos({ Window::Get().GetWindowSize().x - 5, 5 }, ImGuiCond_Always, { 1.0f, 0.0f });
        ImGui::Begin("Collision detection", NULL,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
        ImGui::TextColored(ImColor{ 12, 11, 14 }, "Contact normal is pointing from refernce body to incident body");
        ImGui::End();
    }

    ~CollisionDetection()
    {
        options.draw_outline_only = false;
        options.show_contact_normal = false;
        options.show_contact_point = false;
    }

    static Demo* Create(Game& game)
    {
        return new CollisionDetection(game);
    }
};

DemoFrame collision_detection{ "Collision detection", CollisionDetection::Create };

} // namespace muli
