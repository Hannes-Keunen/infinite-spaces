#pragma once

#include "Resources.h"

#include <glad/glad.h>

class ScreenBuffer
{
public:
    ScreenBuffer(int width, int height);
    ~ScreenBuffer();

    void Bind();
    void Present();
    int ReadObjId(int x, int y);
    auto Fbo() const { return fbo; }

private:
    GLuint texId[2];
    GLuint fbo;
    GLuint renderBuf;

    int width, height;
    std::shared_ptr<Shader> shader;
    GLuint vao, vbo;
};
