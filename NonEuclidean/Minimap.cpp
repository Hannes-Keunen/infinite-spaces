#include "Minimap.h"
#include "GameHeader.h"
#include "Resources.h"

// clang-format off
constexpr static float vertices[] = {
    1080.0f, 520.0f,    0.0f, 0.0f,
    1280.0f, 720.0f,    1.0f, 1.0f,
    1080.0f, 720.0f,    0.0f, 1.0f,

    1080.0f, 520.0f,    0.0f, 0.0f,
    1280.0f, 520.0f,    1.0f, 0.0f,
    1280.0f, 720.0f,    1.0f, 1.0f,
};
// clang-format on

constexpr static int LINE_BUFFER_SIZE = 28 * 2 * sizeof(float);

Minimap::Minimap()
    : fbo(GH_MINIMAP_SIZE, GH_MINIMAP_SIZE)
    , lineBufferSize(LINE_BUFFER_SIZE)
{
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

    glGenVertexArrays(1, &linesVao);
    glBindVertexArray(linesVao);

    glGenBuffers(1, &linesVbo);
    glBindBuffer(GL_ARRAY_BUFFER, linesVbo);
    glBufferData(GL_ARRAY_BUFFER, lineBufferSize, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    presentShader = AquireShader("minimap_present");
    playerShader = AquireShader("minimap_player");
    playerPositionId = playerShader->GetUniformLocation("position");
    linesShader = AquireShader("minimap_lines");

    mvp = Matrix4::Trans(Vector3(-1.0f, -1.0f, 0.0f)) * Matrix4::Scale(Vector3(1.0f / 640.0f, 1.0f / 360.0f, 1.0f))
          * Matrix4::Identity();
}

Minimap::~Minimap()
{
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &linesVbo);
    glDeleteVertexArrays(1, &linesVao);
}

void Minimap::Render(const Player& player, const InfiniteSpace& space)
{
    fbo.Bind();
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    linesShader->Use();
    lineVertices.clear();
    space.CreateFloorplanVertices(player, lineVertices);
    glBindBuffer(GL_ARRAY_BUFFER, linesVbo);

    if (lineVertices.size() * sizeof(lineVertices[0]) > lineBufferSize)
    {
        lineBufferSize = lineVertices.size() * sizeof(lineVertices[0]);
        glBufferData(GL_ARRAY_BUFFER, lineBufferSize, lineVertices.data(), GL_DYNAMIC_DRAW);
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, lineVertices.size() * sizeof(lineVertices[0]), lineVertices.data());
    }

    glBindVertexArray(linesVao);
    glLineWidth(3.0f);
    glDrawArrays(GL_LINES, 0, lineVertices.size() / 2);

    auto pos = space.GetPhysicalPos(player.pos) / (space.GetPhysicalSize() / 2.0f);
    playerShader->Use();
    glUniform2f(playerPositionId, pos.x, pos.z);
    glPointSize(5.0f);
    glDrawArrays(GL_POINTS, 0, 1);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Minimap::Present()
{
    presentShader->Use();
    presentShader->SetMVP(mvp.m, nullptr);
    glActiveTexture(GL_TEXTURE0);
    fbo.Use();
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
