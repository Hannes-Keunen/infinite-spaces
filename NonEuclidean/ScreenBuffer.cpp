#include "ScreenBuffer.h"

// clang-format off
constexpr static float vertices[] = {
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, 1.0f,   1.0f, 1.0f,
    -1.0f, 1.0f,  0.0f, 1.0f,

    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
    1.0f, 1.0f,   1.0f, 1.0f,
};
// clang-format on

void CheckError()
{
    auto error = glGetError();
    while (error != GL_NO_ERROR)
    {
        printf("gl error 0x%x\n", error);
        error = glGetError();
    }
}

ScreenBuffer::ScreenBuffer(int width, int height)
    : width(width)
    , height(height)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenTextures(2, texId);
    // color attachment
    glBindTexture(GL_TEXTURE_2D, texId[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId[0], 0);
    // object id attachment
    glBindTexture(GL_TEXTURE_2D, texId[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texId[1], 0);
    // depth attachment
    glGenRenderbuffers(1, &renderBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuf);

    // Does the GPU support current FBO configuration?
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        return;
    }

    // Unbind so future rendering can proceed normally
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shader = AquireShader("present");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*) (2 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

ScreenBuffer::~ScreenBuffer()
{
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void ScreenBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLenum buffers[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
    };
    glDrawBuffers(2, buffers);

    glViewport(0, 0, width, height);
}

void ScreenBuffer::Present()
{
    shader->Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texId[0]);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

int ScreenBuffer::ReadObjId(int x, int y)
{
    // int currentFbo;
    // glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &currentFbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    int value;
    glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &value);
    CheckError();

    // glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);
    return value;
}
