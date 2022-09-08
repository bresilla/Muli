#pragma once

#include "dynamic_shader.h"
#include "renderer.h"

namespace spe
{

// Dynamic batch renderer
class DynamicRenderer final : public Renderer
{
    friend class DynamicShader;

public:
    DynamicRenderer();
    virtual ~DynamicRenderer();

    void SetProjectionMatrix(Mat4 _projMatrix);
    void SetViewMatrix(Mat4 _viewMatrix);

    virtual void Render() override;
    void DynamicRenderer::Draw(const std::vector<Vec2>& vertices, GLenum drawMode = GL_LINES, Vec3 color = { 0.0f, 0.0f, 0.0f });

private:
    const uint32_t maxVertexCount = 1024; // must be a even number

    // All registered rigid bodies
    std::unique_ptr<DynamicShader> shader{};

    Mat4 viewMatrix;
    Mat4 projMatrix;

    uint32_t VAO;
    uint32_t VBO;
};

inline void DynamicRenderer::Render()
{
}

inline void DynamicRenderer::SetProjectionMatrix(Mat4 _projMatrix)
{
    shader->Use();
    shader->SetProjectionMatrix(std::move(_projMatrix));
}

inline void DynamicRenderer::SetViewMatrix(Mat4 _viewMatrix)
{
    shader->Use();
    shader->SetViewMatrix(std::move(_viewMatrix));
}

} // namespace spe