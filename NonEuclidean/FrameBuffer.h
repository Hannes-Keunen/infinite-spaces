#pragma once

#include "Camera.h"
#include "GameHeader.h"

#include <glad/glad.h>

// Forward declaration
class Portal;

class FrameBuffer
{
public:
    FrameBuffer(int width = GH_FBO_SIZE, int height = GH_FBO_SIZE);

    void Render(const Camera& cam, GLuint curFBO, const Portal* skipPortal);
    void Use();
    void Bind();
    auto TexId() const { return texId; }

private:
    GLuint texId;
    GLuint fbo;
    GLuint renderBuf;
    int width, height;
};
