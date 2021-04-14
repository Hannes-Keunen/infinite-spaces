#pragma once

#include "Vector.h"

#include <glad/glad.h>
#include <string>
#include <vector>

class Shader
{
public:
    Shader(const char* name);
    ~Shader();

    void Use();
    void SetMVP(const float* mvp, const float* mv);
    void SetObjId(int objId);
    void SetColor(const Vector3& color);
    GLuint GlId() const { return progId; }
    GLint GetUniformLocation(const char* name);

private:
    GLuint LoadShader(const char* fname, GLenum type);

    std::vector<std::string> attribs;
    GLuint vertId;
    GLuint fragId;
    GLuint progId;
    GLuint mvpId;
    GLuint mvId;
    GLuint objIdId;
    GLuint colorId;
};
