#include "FrameBuffer.h"
#include "Engine.h"

#include <iostream>

FrameBuffer::FrameBuffer(int width, int height)
    : width(width)
    , height(height)
{
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    //-------------------------
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
    //-------------------------
    glGenRenderbuffers(1, &renderBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, height, height);
    //-------------------------
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuf);
    //-------------------------

    // Does the GPU support current FBO configuration?
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        return;
    }

    // Unbind so future rendering can proceed normally
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Use()
{
    glBindTexture(GL_TEXTURE_2D, texId);
}

void FrameBuffer::Render(const Camera& cam, GLuint curFBO, const Portal* skipPortal)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, GH_FBO_SIZE, GH_FBO_SIZE);
    GH_ENGINE->Render(cam, fbo, skipPortal);
    glBindFramebuffer(GL_FRAMEBUFFER, curFBO);
}

void FrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
}
