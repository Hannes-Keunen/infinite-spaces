#pragma once

#include "FrameBuffer.h"
#include "InfiniteSpace.h"
#include "Shader.h"

class Minimap
{
public:
    Minimap();
    ~Minimap();
    void Render(const Player& player, const InfiniteSpace& space);
    void Present();

private:
    FrameBuffer fbo;
    std::shared_ptr<Shader> presentShader;
    std::shared_ptr<Shader> playerShader;
    std::shared_ptr<Shader> linesShader;
    std::vector<float> lineVertices;
    GLuint vao, vbo, linesVao, linesVbo;
    GLuint playerPositionId;
    Matrix4 mvp;
    int lineBufferSize;
};
