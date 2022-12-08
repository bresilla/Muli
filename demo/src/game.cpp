#include "game.h"
#include "application.h"

namespace muli
{

Game::Game(Application& _app)
    : app{ _app }
{
    UpdateProjectionMatrix();
    Window::Get().SetFramebufferSizeChangeCallback([&](int width, int height) -> void {
        glViewport(0, 0, width, height);
        UpdateProjectionMatrix();
    });

    InitDemo(demo_count - 1);
}

Game::~Game() noexcept
{
    delete demo;
}

void Game::Update(float dt)
{
    time += dt;

    UpdateInput();
    demo->Step();
    UpdateUI();
}

void Game::UpdateInput()
{
    if (Input::IsKeyPressed(GLFW_KEY_R))
    {
        InitDemo(demoIndex);
    }

    demo->UpdateInput();
}

void Game::UpdateUI()
{
    // ImGui::ShowDemoWindow();

    // ImGui Windows
    ImGui::SetNextWindowPos({ 5, 5 }, ImGuiCond_Once, { 0.0f, 0.0f });
    ImGui::SetNextWindowSize({ 240, 505 }, ImGuiCond_Once);

    if (ImGui::Begin("Control Panel"))
    {
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_AutoSelectNewTabs;
        if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Control"))
            {
                // Simulation buttons
                {
                    ImGui::BeginDisabled(options.pause);
                    if (ImGui::Button("Pause")) options.pause = true;
                    ImGui::EndDisabled();

                    ImGui::SameLine();

                    ImGui::BeginDisabled(!options.pause);
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::Button("Step")) options.step = true;
                    ImGui::PopButtonRepeat();
                    ImGui::EndDisabled();

                    ImGui::SameLine();

                    ImGui::BeginDisabled(!options.pause);
                    if (ImGui::Button("Start")) options.pause = false;
                    ImGui::EndDisabled();

                    ImGui::SameLine();

                    if (ImGui::Button("Restart")) InitDemo(demoIndex);
                }

                static int f = Window::Get().GetRefreshRate();
                ImGui::SetNextItemWidth(120);
                if (ImGui::SliderInt("Frame rate", &f, 30, 300))
                {
                    app.SetFrameRate(f);
                }
                ImGui::Separator();

                ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

                ImGui::Separator();

                // ImGui::ColorEdit4("Background color", &app.clearColor.x);
                // ImGui::Separator();
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::CollapsingHeader("Debug options"))
                {
                    ImGui::Checkbox("Camera reset", &options.reset_camera);
                    ImGui::Checkbox("Draw outline only", &options.draw_outline_only);
                    ImGui::Checkbox("Show BVH", &options.show_bvh);
                    ImGui::Checkbox("Show AABB", &options.show_aabb);
                    ImGui::Checkbox("Show contact point", &options.show_contact_point);
                    ImGui::Checkbox("Show contact normal", &options.show_contact_normal);
                }

                if (ImGui::CollapsingHeader("Simulation settings"))
                {
                    WorldSettings& settings = demo->GetWorldSettings();

                    if (ImGui::Checkbox("Apply gravity", &settings.apply_gravity)) demo->GetWorld().Awake();
                    ImGui::Text("Constraint solve iterations");
                    {
                        ImGui::SetNextItemWidth(120);
                        int velIterations = settings.velocity_iterations;
                        ImGui::SliderInt("Velocity", &velIterations, 0, 50);
                        settings.velocity_iterations = static_cast<uint32>(velIterations);

                        ImGui::SetNextItemWidth(120);
                        int posIterations = settings.position_iterations;
                        ImGui::SliderInt("Position", &posIterations, 0, 50);
                        settings.position_iterations = static_cast<uint32>(posIterations);
                    }
                    ImGui::Checkbox("Contact block solve", &settings.block_solve);
                    ImGui::Checkbox("Warm starting", &settings.warm_starting);
                    ImGui::Checkbox("Sleeping", &settings.sleeping);
                }

                World& world = demo->GetWorld();

                ImGui::Separator();
                ImGui::Text("%s", demos[demoIndex].name);
                ImGui::Text("Bodies: %d", world.GetBodyCount());
                ImGui::Text("Sleeping dynamic bodies: %d", world.GetSleepingBodyCount());
                // ImGui::Text("Awake island count: %d", world.GetIslandCount());
                ImGui::Text("Broad phase contacts: %d", world.GetContactCount());

                ImGui::Separator();

                RigidBody* t = demo->GetTargetBody();

                if (t)
                {
                    ImGui::Text("Mass: %.4f", t->GetMass());
                    ImGui::Text("Inertia: %.4f", t->GetInertia());
                    ImGui::Text("Pos: %.4f, %.4f", t->GetPosition().x, t->GetPosition().y);
                    ImGui::Text("Rot: %.4f", t->GetAngle());
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Demos"))
            {
                if (ImGui::BeginListBox("##listbox 2", ImVec2{ -FLT_MIN, 26 * ImGui::GetTextLineHeightWithSpacing() }))
                {
                    for (uint32 i = 0; i < demo_count; ++i)
                    {
                        const bool selected = (demoIndex == i);

                        if (ImGui::Selectable(demos[i].name, selected))
                        {
                            InitDemo(i);
                        }

                        if (selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndListBox();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();

    demo->UpdateUI();
}

void Game::Render()
{
    Camera& camera = demo->GetCamera();
    rRenderer.SetViewMatrix(camera.GetCameraMatrix());
    rRenderer.SetDrawOutlined(options.draw_outline_only);
    rRenderer.Render();

    for (Joint* j = demo->GetWorld().GetJoints(); j; j = j->GetNext())
    {
        Joint::Type type = j->GetType();

        switch (type)
        {
        case Joint::Type::grab_joint:
        {
            RigidBody* b = j->GetBodyA();
            GrabJoint* gj = static_cast<GrabJoint*>(j);

            const Vec2& anchor = b->GetTransform() * gj->GetLocalAnchor();
            points.push_back(anchor);
            points.push_back(gj->GetTarget());

            lines.push_back(anchor);
            lines.push_back(gj->GetTarget());
        }
        break;
        case Joint::Type::revolute_joint:
        {

            RigidBody* ba = j->GetBodyA();
            RigidBody* bb = j->GetBodyB();
            RevoluteJoint* rj = static_cast<RevoluteJoint*>(j);

            const Vec2& anchorA = ba->GetTransform() * rj->GetLocalAnchorA();
            const Vec2& anchorB = bb->GetTransform() * rj->GetLocalAnchorB();

            points.push_back(anchorA);
            points.push_back(anchorB);

            lines.push_back(anchorA);
            lines.push_back(ba->GetPosition());
            lines.push_back(anchorB);
            lines.push_back(bb->GetPosition());
        }
        break;
        case Joint::Type::distance_joint:
        {
            RigidBody* ba = j->GetBodyA();
            RigidBody* bb = j->GetBodyB();
            DistanceJoint* dj = static_cast<DistanceJoint*>(j);

            const Vec2& anchorA = ba->GetTransform() * dj->GetLocalAnchorA();
            const Vec2& anchorB = bb->GetTransform() * dj->GetLocalAnchorB();

            points.push_back(anchorA);
            points.push_back(anchorB);

            lines.push_back(anchorA);
            lines.push_back(anchorB);
        }
        break;
        case Joint::Type::line_joint:
        {
            RigidBody* ba = j->GetBodyA();
            RigidBody* bb = j->GetBodyB();
            LineJoint* lj = static_cast<LineJoint*>(j);

            const Vec2& anchorA = ba->GetTransform() * lj->GetLocalAnchorA();
            const Vec2& anchorB = bb->GetTransform() * lj->GetLocalAnchorB();

            points.push_back(anchorA);
            points.push_back(anchorB);

            lines.push_back(anchorA);
            lines.push_back(anchorB);
        }
        case Joint::Type::prismatic_joint:
        {
            RigidBody* ba = j->GetBodyA();
            RigidBody* bb = j->GetBodyB();
            PrismaticJoint* pj = static_cast<PrismaticJoint*>(j);

            const Vec2& anchorA = ba->GetTransform() * pj->GetLocalAnchorA();
            const Vec2& anchorB = bb->GetTransform() * pj->GetLocalAnchorB();

            points.push_back(anchorA);
            points.push_back(anchorB);

            lines.push_back(anchorA);
            lines.push_back(anchorB);
        }
        break;
        case Joint::Type::pulley_joint:
        {
            RigidBody* ba = j->GetBodyA();
            RigidBody* bb = j->GetBodyB();
            PulleyJoint* pj = static_cast<PulleyJoint*>(j);

            const Vec2& anchorA = ba->GetTransform() * pj->GetLocalAnchorA();
            const Vec2& anchorB = bb->GetTransform() * pj->GetLocalAnchorB();
            const Vec2& groundAnchorA = pj->GetGroundAnchorA();
            const Vec2& groundAnchorB = pj->GetGroundAnchorB();

            points.push_back(anchorA);
            points.push_back(groundAnchorA);
            points.push_back(anchorB);
            points.push_back(groundAnchorB);

            lines.push_back(anchorA);
            lines.push_back(groundAnchorA);
            lines.push_back(anchorB);
            lines.push_back(groundAnchorB);
            lines.push_back(groundAnchorA);
            lines.push_back(groundAnchorB);
        }
        break;
        case Joint::Type::motor_joint:
        {
            RigidBody* ba = j->GetBodyA();
            RigidBody* bb = j->GetBodyB();
            MotorJoint* pj = static_cast<MotorJoint*>(j);

            const Vec2& anchorA = ba->GetTransform() * pj->GetLocalAnchorA();
            const Vec2& anchorB = bb->GetTransform() * pj->GetLocalAnchorB();

            points.push_back(anchorA);
            points.push_back(anchorB);
        }
        break;
        default:
            break;
        }
    }

    if (options.show_bvh || options.show_aabb)
    {
        const AABBTree& tree = demo->GetWorld().GetBVH();
        tree.Traverse([&](const Node* n) -> void {
            if (!options.show_bvh && !n->isLeaf) return;
            lines.push_back(n->aabb.min);
            lines.push_back({ n->aabb.max.x, n->aabb.min.y });
            lines.push_back({ n->aabb.max.x, n->aabb.min.y });
            lines.push_back(n->aabb.max);
            lines.push_back(n->aabb.max);
            lines.push_back({ n->aabb.min.x, n->aabb.max.y });
            lines.push_back({ n->aabb.min.x, n->aabb.max.y });
            lines.push_back(n->aabb.min);
        });
    }

    if (options.show_contact_point || options.show_contact_normal)
    {
        const Contact* c = demo->GetWorld().GetContacts();

        while (c)
        {
            if (c->IsTouching() == false)
            {
                c = c->GetNext();
                continue;
            }

            const ContactManifold& m = c->GetContactManifold();

            for (int32 j = 0; j < m.numContacts; ++j)
            {
                const Vec2& cp = m.contactPoints[j].position;

                if (options.show_contact_point)
                {
                    points.push_back(cp);
                }
                if (options.show_contact_normal)
                {
                    lines.push_back(cp);
                    lines.push_back(cp + m.contactNormal * 0.15f);

                    lines.push_back(cp + m.contactNormal * 0.15f);
                    lines.push_back(cp + m.contactNormal * 0.13f + m.contactTangent * 0.02f);

                    lines.push_back(cp + m.contactNormal * 0.15f);
                    lines.push_back(cp + m.contactNormal * 0.13f - m.contactTangent * 0.02f);
                }
            }

            c = c->GetNext();
        }
    }

    dRenderer.SetViewMatrix(camera.GetCameraMatrix());

    demo->Render();

    glPointSize(5.0f);
    dRenderer.Draw(points, GL_POINTS);
    glLineWidth(1.0f);
    dRenderer.Draw(lines, GL_LINES);

    points.clear();
    lines.clear();
}

void Game::UpdateProjectionMatrix()
{
    Vec2 windowSize = Window::Get().GetWindowSize();
    windowSize /= 100.0f;

    Mat4 projMatrix = Orth(-windowSize.x / 2.0f, windowSize.x / 2.0f, -windowSize.y / 2.0f, windowSize.y / 2.0f, 0.0f, 1.0f);
    rRenderer.SetProjectionMatrix(projMatrix);
    dRenderer.SetProjectionMatrix(projMatrix);
}

void Game::InitDemo(uint32 index)
{
    if (index >= demo_count)
    {
        return;
    }

    bool restoreCameraPosition = false;
    bool restoreSettings = (demoIndex == index);
    Camera prevCamera;
    WorldSettings prevSettings;

    if (restoreSettings)
    {
        prevSettings = demo->GetWorldSettings();
    }

    if (demo)
    {
        restoreCameraPosition = !options.reset_camera;
        prevCamera = demo->GetCamera();
        delete demo;
    }

    time = 0;
    rRenderer.Reset();

    demoIndex = index;
    demo = demos[demoIndex].createFunction(*this);

    if (restoreSettings)
    {
        demo->GetWorldSettings() = prevSettings;
    }

    if (restoreCameraPosition)
    {
        demo->GetCamera() = prevCamera;
    }

    for (RigidBody* b = demo->GetWorld().GetBodyList(); b; b = b->GetNext())
    {
        RegisterRenderBody(b);
    }
}

} // namespace muli