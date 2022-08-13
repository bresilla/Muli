#include "game.h"
#include "application.h"
#include "spe/detection.h"
#include "demo.h"

namespace spe
{

Game::Game(Application& _app) :
    app{ _app }
{
    glm::vec2 windowSize = Window::Get().GetWindowSize();

    UpdateProjectionMatrix();
    Window::Get().SetFramebufferSizeChangeCallback
    (
        [&](int width, int height) -> void
        {
            glViewport(0, 0, width, height);
            UpdateProjectionMatrix();
        }
    );

    settings.INV_DT = static_cast<float>(Window::Get().GetRefreshRate());
    settings.DT = 1.0f / settings.INV_DT;

    world = std::unique_ptr<World>(new World(settings));

    demos = get_demos();
    InitSimulation(0);
}

Game::~Game() noexcept
{
}

void Game::Update(float dt)
{
    time += dt;

    HandleInput();

    if (pause)
    {
        if (step)
        {
            step = false;
            world->Update(dt);
        }
    }
    else
    {
        world->Update(dt);
    }
}

void Game::HandleInput()
{
    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
    {
        mpos = rRenderer.Pick(Input::GetMousePosition());

        if (Input::IsKeyPressed(GLFW_KEY_R))
        {
            InitSimulation(currentDemo);
        }

        if (Input::IsKeyPressed(GLFW_KEY_SPACE))
        {
            pause = !pause;
        }

        if (Input::IsKeyDown(GLFW_KEY_RIGHT))
        {
            step = true;
        }

        if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            auto q = world->QueryPoint(mpos);

            if (q.size() != 0)
            {
                gj = world->CreateGrabJoint(q[0], mpos, mpos, 2.0f);
                gj->OnDestroy = [&](Joint* me) -> void
                {
                    gj = nullptr;
                };
            }
            else
            {
                RigidBody* b = world->CreateBox(0.5f);
                b->position = mpos;
                rRenderer.Register(b);

                b->OnDestroy = [&](RigidBody* me) -> void
                {
                    rRenderer.Unregister(me);
                };
            }
        }

        if (gj != nullptr)
        {
            gj->SetTarget(mpos);
        }

        if (Input::IsMouseReleased(GLFW_MOUSE_BUTTON_LEFT))
        {
            if (gj != nullptr)
            {
                world->Remove(gj);
                gj = nullptr;
            }
        }

        if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_RIGHT))
        {
            std::vector<RigidBody*> q = world->QueryPoint(mpos);

            world->Remove(q);
        }

        if (Input::GetMouseScroll().y != 0)
        {
            camera.scale *= Input::GetMouseScroll().y < 0 ? 1.1 : 1.0f / 1.1f;
            camera.scale = glm::clamp(camera.scale, glm::vec2{ 0.1f }, glm::vec2{ FLT_MAX });
        }

        // Camera moving
        {
            static bool cameraMove = false;
            static glm::vec2 cursorStart;
            static glm::vec2 cameraPosStart;

            if (!cameraMove && Input::IsMousePressed(GLFW_MOUSE_BUTTON_RIGHT))
            {
                cameraMove = true;
                cursorStart = Input::GetMousePosition();
                cameraPosStart = camera.position;
            }
            else if (Input::IsMouseReleased(GLFW_MOUSE_BUTTON_RIGHT))
            {
                cameraMove = false;
            }

            if (cameraMove)
            {
                glm::vec2 dist = Input::GetMousePosition() - cursorStart;
                dist.x *= 0.01f * -camera.scale.x;
                dist.y *= 0.01f * camera.scale.y;
                camera.position = cameraPosStart + dist;
            }
        }
    }

    // ImGui::ShowDemoWindow();

    // ImGui Windows
    ImGui::SetNextWindowPos({ 10, 10 }, ImGuiCond_Once);
    ImGui::SetNextWindowSize({ 400, 360 }, ImGuiCond_Once);

    if (ImGui::Begin("Control Panel"))
    {
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_AutoSelectNewTabs;
        if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Control"))
            {
                // Simulation buttons
                {
                    ImGui::BeginDisabled(pause);
                    if (ImGui::Button("Pause")) pause = true;
                    ImGui::EndDisabled();

                    ImGui::SameLine();

                    ImGui::BeginDisabled(!pause);
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::Button("Step")) step = true;
                    ImGui::PopButtonRepeat();
                    ImGui::EndDisabled();

                    ImGui::SameLine();

                    ImGui::BeginDisabled(!pause);
                    if (ImGui::Button("Start")) pause = false;
                    ImGui::EndDisabled();
                }

                static int f = Window::Get().GetRefreshRate();
                if (ImGui::SliderInt("Frame rate", &f, 10, 300))
                {
                    app.SetFrameRate(f);
                }
                ImGui::Separator();

                ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

                ImGui::Separator();

                ImGui::ColorEdit4("Background color", glm::value_ptr(app.clearColor));

                ImGui::Separator();

                ImGui::Checkbox("Camera reset", &resetCamera);
                ImGui::Checkbox("Draw outline only", &drawOutlineOnly);
                ImGui::Checkbox("Show BVH", &showBVH);
                ImGui::Checkbox("Show Contact point", &showCP);
                ImGui::Separator();
                if (ImGui::Checkbox("Apply gravity", &settings.APPLY_GRAVITY)) world->Awake();
                ImGui::SliderInt("Solve iteration", &settings.SOLVE_ITERATION, 1, 50);

                ImGui::Separator();
                ImGui::Text(demos[currentDemo].first.data());
                ImGui::Text("Bodies: %d", world->GetBodies().size());
                ImGui::Text("Sleeping dynamic bodies: %d", world->GetSleepingBodyCount());
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Demos"))
            {
                if (ImGui::BeginListBox("##listbox 2", ImVec2(-FLT_MIN, 15 * ImGui::GetTextLineHeightWithSpacing())))
                {
                    for (size_t i = 0; i < demos.size(); i++)
                    {
                        const bool selected = (currentDemo == i);

                        if (ImGui::Selectable(demos[i].first.data(), selected))
                        {
                            InitSimulation(i);
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
        ImGui::End();
    }

    // ImGuiWindowFlags window_flags = 0;
    // // window_flags |= ImGuiWindowFlags_NoBackground;
    // window_flags |= ImGuiWindowFlags_NoTitleBar;
    // window_flags |= ImGuiWindowFlags_NoResize;
    // // etc.
    // bool open_ptr = true;
    // if (ImGui::Begin("", &open_ptr, window_flags))
    // {
    //     ImGui::Text("Bodies: %d", bodies.size());

    //     ImGui::End();
    // }
}

void Game::Render()
{
    rRenderer.SetViewMatrix(camera.CameraTransform());
    rRenderer.SetDrawOutlined(drawOutlineOnly);
    rRenderer.Render();

    points.clear();
    lines.clear();

    const std::vector<Joint*>& joints = world->GetJoints();
    for (size_t i = 0; i < joints.size(); i++)
    {
        Joint* j = joints[i];

        if (typeid(*j) == typeid(GrabJoint))
        {
            RigidBody* b = j->GetBodyA();
            GrabJoint* gj = static_cast<GrabJoint*>(j);

            const glm::vec2& anchor = b->LocalToGlobal() * gj->GetLocalAnchor();
            points.push_back(anchor);
            points.push_back(gj->GetTarget());

            lines.push_back(anchor);
            lines.push_back(gj->GetTarget());
        }
        else if (typeid(*j) == typeid(RevoluteJoint))
        {
            RigidBody* ba = j->GetBodyA();
            RigidBody* bb = j->GetBodyB();
            RevoluteJoint* rj = static_cast<RevoluteJoint*>(j);

            const glm::vec2& anchorA = ba->LocalToGlobal() * rj->GetLocalAnchorA();
            const glm::vec2& anchorB = bb->LocalToGlobal() * rj->GetLocalAnchorB();

            points.push_back(anchorA);
            points.push_back(anchorB);

            lines.push_back(anchorA);
            lines.push_back(ba->position);
            lines.push_back(anchorB);
            lines.push_back(bb->position);
        }
        else if (typeid(*j) == typeid(DistanceJoint))
        {
            RigidBody* ba = j->GetBodyA();
            RigidBody* bb = j->GetBodyB();
            DistanceJoint* dj = static_cast<DistanceJoint*>(j);

            const glm::vec2& anchorA = ba->LocalToGlobal() * dj->GetLocalAnchorA();
            const glm::vec2& anchorB = bb->LocalToGlobal() * dj->GetLocalAnchorB();

            points.push_back(anchorA);
            points.push_back(anchorB);

            lines.push_back(anchorA);
            lines.push_back(anchorB);
        }
    }

    if (showBVH)
    {
        const AABBTree& tree = world->GetBVH();
        tree.Traverse([&](const Node* n)->void
            {
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

    if (showCP)
    {
        const std::vector<ContactConstraint>& cc = world->GetContactConstraints();

        for (size_t i = 0; i < cc.size(); i++)
        {
            auto ci = cc[i].GetContactInfo();
            for (size_t j = 0; j < ci.numContacts; j++)
            {
                points.push_back(ci.contactPoints[j].point);
            }
        }
    }

    dRenderer.SetViewMatrix(camera.CameraTransform());
    glPointSize(5.0f);
    dRenderer.Draw(points, GL_POINTS);
    glLineWidth(1.0f);
    dRenderer.Draw(lines, GL_LINES);
}

void Game::UpdateProjectionMatrix()
{
    glm::vec2 windowSize = Window::Get().GetWindowSize();
    windowSize /= 100.0f;

    glm::mat4 projMatrix = glm::ortho(-windowSize.x / 2.0f, windowSize.x / 2.0f, -windowSize.y / 2.0f, windowSize.y / 2.0f, 0.0f, 1.0f);
    rRenderer.SetProjectionMatrix(projMatrix);
    dRenderer.SetProjectionMatrix(projMatrix);
}

void Game::InitSimulation(size_t demo)
{
    time = 0;
    if (resetCamera)
    {
        camera.position = glm::vec2{ 0, 3.6 };
        camera.scale = glm::vec2{ 1, 1 };
    }

    Reset();

    currentDemo = demo;
    demoTitle = demos[currentDemo].first;
    demos[currentDemo].second(*world, settings);

    for (RigidBody* b : world->GetBodies())
    {
        rRenderer.Register(b);

        b->OnDestroy = [&](RigidBody* me)-> void
        {
            rRenderer.Unregister(me);
        };
    }
}

void Game::Reset()
{
    world->Reset();
    rRenderer.Reset();
}

}