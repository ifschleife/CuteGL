#pragma once

#include <memory>

#include <QMatrix4x4>
#include <QOpenGLFunctions_4_5_Core>

#include "mesh.h"
#include "shader.h"
#include "util.h"


class Texture;

class RenderObject : public QOpenGLFunctions_4_5_Core
{
public:
    RenderObject();

    void initGL();
    void animate();
    void render(const QMatrix4x4& pv);

    void rotate(float angle);
    void translate(float x, float y, float z);

    void setAnimRotation(float angle);

    void setFragmentShader(const QString&& filename);
    void setVertexShader(const QString&& filename);

    void setCullFaceMode(bool mode);
    void setMesh(std::unique_ptr<Mesh> mesh);
    void setTexture(std::unique_ptr<Texture> texture);
    void setWireframeMode(bool mode);

private:
    std::unique_ptr<Mesh> m_mesh;
    Shader m_shader;
    QMatrix4x4 m_model_matrix;
    std::unique_ptr<Texture> m_texture;

    bool m_show_wireframe{false};
    bool m_cull_faces{false};
    float m_anim_rotation{0.0f};
};
