#include "mesh.h"

namespace muli
{

Mesh::Mesh(const std::vector<Vec3>& _vertices, const std::vector<Vec2>& _texCoords, const std::vector<uint32>& _indices)
    : vertices{ _vertices }
    , texCoords{ _texCoords }
    , indices{ _indices }
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBOv);
    glGenBuffers(1, &VBOt);
    glGenBuffers(1, &EBOt);
    glGenBuffers(1, &EBOl);

    // For better performance, merge vertices and texCoords vectors and define a shader vertex layout with only one VBO.
    glBindVertexArray(VAO);
    {
        // vertices
        glBindBuffer(GL_ARRAY_BUFFER, VBOv);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size() * 3, vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
        glEnableVertexAttribArray(0);

        // tex coords
        glBindBuffer(GL_ARRAY_BUFFER, VBOt);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * texCoords.size() * 2, texCoords.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    // indices for triangle
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOt);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * indices.size(), indices.data(), GL_STATIC_DRAW);

    std::vector<uint32> indices_l(vertices.size());
    std::iota(indices_l.begin(), indices_l.end(), 0);

    // indices for outline
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOl);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * indices_l.size(), indices_l.data(), GL_STATIC_DRAW);
}

Mesh::~Mesh()
{
    if (moved) return;

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBOv);
    glDeleteBuffers(1, &VBOt);
    glDeleteBuffers(1, &EBOt);
    glDeleteBuffers(1, &EBOl);
}

Mesh::Mesh(Mesh&& _m) noexcept
{
    _m.moved = true;

    vertices = std::move(_m.vertices);
    texCoords = std::move(_m.texCoords);
    indices = std::move(_m.indices);

    VAO = _m.VAO;
    VBOv = _m.VBOv;
    VBOt = _m.VBOt;
    EBOt = _m.EBOt;
    EBOl = _m.EBOl;
}

void Mesh::Draw(GLenum drawMode)
{
    glBindVertexArray(VAO);

    switch (drawMode)
    {
    case GL_TRIANGLES:
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOt);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        break;

    case GL_LINE_LOOP:
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOl);
        glDrawElements(GL_LINE_LOOP, static_cast<GLsizei>(vertices.size()), GL_UNSIGNED_INT, 0);
        break;

    case GL_POINTS:
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOl);
        glDrawElements(GL_POINTS, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        break;

    default:
        SPDLOG_ERROR("Not a support draw mode");
        break;
    }

    glBindVertexArray(0);
}

} // namespace muli