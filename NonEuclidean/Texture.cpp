#include "Texture.h"

#include <stb_image.h>

#include <cassert>
#include <fstream>

Texture::Texture(const char* fname, int rows, int cols)
{
    // Check if this is a 3D texture
    assert(rows >= 1 && cols >= 1);
    is3D = (rows > 1 || cols > 1);

    auto file = std::string("NonEuclidean/Textures/") + fname;
    int width, height;
    auto data = stbi_load(file.c_str(), &width, &height, nullptr, 3);
    assert(data);

    // Load texture into video memory
    glGenTextures(1, &texId);
    if (is3D)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texId);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_TRUE);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexImage3D(
            GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, width / rows, height / cols, rows * cols, 0, GL_RGB, GL_UNSIGNED_BYTE,
            data);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }

    // Clenup
    stbi_image_free(data);
}

void Texture::Use()
{
    if (is3D)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texId);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, texId);
    }
}