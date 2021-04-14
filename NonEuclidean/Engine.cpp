#include "Engine.h"
#include "InfiniteSpace.h"
#include "Physical.h"

#include <algorithm>
#include <cmath>
#include <iostream>

Engine* GH_ENGINE = nullptr;
Player* GH_PLAYER = nullptr;
const Input* GH_INPUT = nullptr;
int GH_REC_LEVEL = 0;
int64_t GH_FRAME = 0;

Engine::Engine()
{
    GH_ENGINE = this;
    GH_INPUT = &input;
    isFullscreen = true;

    CreateGLWindow();
    InitGLObjects();
    input.SetupCallbacks(window);

    player.reset(new Player);
    GH_PLAYER = player.get();

    vScenes.push_back(std::make_shared<InfiniteSpace>(16, RemovalStrategy::KEEP_ONE));

    LoadScene(0);

    sky.reset(new Sky);
}

Engine::~Engine()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

int Engine::Run()
{
    // Setup the timer
    double cur_time = timer.GetSeconds();
    GH_FRAME = 0;

    // Game loop
    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
    {
        input.Update();

        if (input.key_press['1'])
        {
            LoadScene(0);
        }
        else if (input.key_press['2'])
        {
            LoadScene(1);
        }
        else if (input.key_press['3'])
        {
            LoadScene(2);
        }
        else if (input.key_press['4'])
        {
            LoadScene(3);
        }
        else if (input.key_press['5'])
        {
            LoadScene(4);
        }
        else if (input.key_press['6'])
        {
            LoadScene(5);
        }
        else if (input.key_press['7'])
        {
            LoadScene(6);
        }

        // Used fixed time steps for updates
        const double new_time = timer.GetSeconds();
        for (int i = 0; cur_time < new_time && i < GH_MAX_STEPS; ++i)
        {
            Update();
            cur_time += GH_DT;
            GH_FRAME += 1;
            input.EndFrame();
        }
        cur_time = (cur_time < new_time ? new_time : cur_time);

        // Setup camera for rendering
        const float n = GH_CLAMP(NearestPortalDist() * 0.5f, GH_NEAR_MIN, GH_NEAR_MAX);
        main_cam.worldView = player->WorldToCam();
        main_cam.SetSize(iWidth, iHeight, n, GH_FAR);
        main_cam.UseViewport();

        // Render scene and minimap
        GH_REC_LEVEL = GH_MAX_RECURSION;
        screenBuffer->Bind();
        Render(main_cam, screenBuffer->Fbo(), nullptr);
        minimap->Render(*player, *curScene);

        // Present
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, iWidth, iHeight);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        screenBuffer->Present();
        minimap->Present();

        glfwSwapBuffers(window);
    }

    DestroyGLObjects();
    return 0;
}

void Engine::LoadScene(int ix)
{
    // Clear out old scene
    if (curScene)
    {
        curScene->Unload();
    }
    vObjects.clear();
    vPortals.clear();
    player->Reset();

    // Create new scene
    curScene = vScenes[ix];
    curScene->Load(vObjects, vPortals, *player);
    // player->SetPosition(Vector3(0.0f, GH_PLAYER_HEIGHT, 0.0f));
    vObjects.push_back(player);
}

void Engine::Update()
{
    // Update
    for (size_t i = 0; i < vObjects.size(); ++i)
    {
        assert(vObjects[i].get());
        vObjects[i]->Update();
    }

    // Portals
    for (size_t i = 0; i < vObjects.size(); ++i)
    {
        Physical* physical = vObjects[i]->AsPhysical();
        if (physical)
        {
            for (size_t j = 0; j < vPortals.size(); ++j)
            {
                if (physical->TryPortal(*vPortals[j]))
                {
                    break;
                }
            }
        }
    }

    // Collisions
    // For each physics object
    for (size_t i = 0; i < vObjects.size(); ++i)
    {
        Physical* physical = vObjects[i]->AsPhysical();
        if (!physical)
        {
            continue;
        }
        Matrix4 worldToLocal = physical->WorldToLocal();

        // For each object to collide with
        for (size_t j = 0; j < vObjects.size(); ++j)
        {
            if (i == j)
            {
                continue;
            }
            Object& obj = *vObjects[j];
            if (!obj.mesh)
            {
                continue;
            }

            // For each hit sphere
            for (size_t s = 0; s < physical->hitSpheres.size(); ++s)
            {
                // Brings point from collider's local coordinates to hits's
                // local coordinates.
                const Sphere& sphere = physical->hitSpheres[s];
                Matrix4 worldToUnit = sphere.LocalToUnit() * worldToLocal;
                Matrix4 localToUnit = worldToUnit * obj.LocalToWorld();
                Matrix4 unitToWorld = worldToUnit.Inverse();

                // For each collider
                for (size_t c = 0; c < obj.mesh->colliders.size(); ++c)
                {
                    Vector3 push;
                    const Collider& collider = obj.mesh->colliders[c];
                    if (collider.Collide(localToUnit, push))
                    {
                        // If push is too small, just ignore
                        push = unitToWorld.MulDirection(push);
                        vObjects[j]->OnHit(*physical, push);
                        physical->OnCollide(*vObjects[j], push);

                        worldToLocal = physical->WorldToLocal();
                        worldToUnit = sphere.LocalToUnit() * worldToLocal;
                        localToUnit = worldToUnit * obj.LocalToWorld();
                        unitToWorld = worldToUnit.Inverse();
                    }
                }
            }
        }
    }
}

void Engine::Render(const Camera& cam, GLuint curFBO, const Portal* skipPortal)
{
    // Basic global variables
    glClearColor(0.6f, 0.9f, 1.0f, 1.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    // Clear buffers
    if (GH_USE_SKY)
    {
        // glClear(GL_DEPTH_BUFFER_BIT);
        sky->Draw(cam);
    }
    else
    {
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int clearValue = -1;
    glClearBufferiv(GL_COLOR, 1, &clearValue);

    // Create queries (if applicable)
    GLuint queries[GH_MAX_PORTALS];
    GLuint drawTest[GH_MAX_PORTALS];
    assert(vPortals.size() <= GH_MAX_PORTALS);
    if (occlusionCullingSupported)
    {
        glGenQueries((GLsizei) vPortals.size(), queries);
    }

    // Draw scene
    for (size_t i = 0; i < vObjects.size(); ++i) { vObjects[i]->Draw(cam, curFBO, i); }

    // Draw portals if possible
    if (GH_REC_LEVEL > 0)
    {
        // Draw portals
        GH_REC_LEVEL -= 1;
        if (occlusionCullingSupported && GH_REC_LEVEL > 0)
        {
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);
            for (size_t i = 0; i < vPortals.size(); ++i)
            {
                if (vPortals[i].get() != skipPortal)
                {
                    glBeginQuery(GL_SAMPLES_PASSED, queries[i]);
                    vPortals[i]->DrawPink(cam);
                    glEndQuery(GL_SAMPLES_PASSED);
                }
            }
            for (size_t i = 0; i < vPortals.size(); ++i)
            {
                if (vPortals[i].get() != skipPortal)
                {
                    glGetQueryObjectuiv(queries[i], GL_QUERY_RESULT, &drawTest[i]);
                }
            }
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            glDeleteQueries((GLsizei) vPortals.size(), queries);
        }
        for (size_t i = 0; i < vPortals.size(); ++i)
        {
            if (vPortals[i].get() != skipPortal)
            {
                if (occlusionCullingSupported && (GH_REC_LEVEL > 0) && (drawTest[i] == 0))
                {
                    continue;
                }
                else
                {
                    vPortals[i]->Draw(cam, curFBO, -1);
                }
            }
        }
        GH_REC_LEVEL += 1;
    }

#if 0
  //Debug draw colliders
  for (size_t i = 0; i < vObjects.size(); ++i) {
    vObjects[i]->DebugDraw(cam);
  }
#endif
}

void Engine::PickMouse()
{
    int objId = screenBuffer->ReadObjId(iWidth / 2, iHeight / 2);
    if (objId >= 0)
    {
        auto& obj = vObjects[objId];
        auto door = std::dynamic_pointer_cast<Door>(obj);
        if (door != nullptr)
        {
            curScene->OnDoorClicked(door, vObjects, vPortals, *player);
        }
        else
        {
            auto target = std::dynamic_pointer_cast<Target>(obj);
            if (target != nullptr)
            {
                curScene->OnTargetClicked(target, vObjects, *player);
            }
        }
    }
}

void Engine::CreateGLWindow()
{
    // Always start in windowed mode
    iWidth = GH_SCREEN_WIDTH;
    iHeight = GH_SCREEN_HEIGHT;

    if (!glfwInit())
    {
        throw std::runtime_error("Failed to init GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_MAXIMIZED, GH_START_FULLSCREEN ? GLFW_TRUE : GLFW_FALSE);

    window = glfwCreateWindow(iWidth, iHeight, GH_TITLE, nullptr, nullptr);
    if (window == nullptr)
    {
        throw std::runtime_error("Failed to create a window");
    }

    glfwGetWindowSize(window, &iWidth, &iHeight);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (gladLoadGL() == 0)
    {
        throw std::runtime_error("Failed to load opengl");
    }
}

void Engine::InitGLObjects()
{
    // glDisable(GL_BLEND);

    // Check GL functionality
    glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &occlusionCullingSupported);

    screenBuffer = std::make_shared<ScreenBuffer>(iWidth, iHeight);
    minimap = std::make_shared<Minimap>();
}

void Engine::DestroyGLObjects()
{
    // curScene->Unload();
    vObjects.clear();
    vPortals.clear();
    screenBuffer.reset();
    minimap.reset();
}

float Engine::NearestPortalDist() const
{
    float dist = FLT_MAX;
    for (size_t i = 0; i < vPortals.size(); ++i) { dist = GH_MIN(dist, vPortals[i]->DistTo(player->pos)); }
    return dist;
}

void Engine::OnPlayerEnterRoom(const Vector3& previousPosition)
{
    curScene->OnPlayerEnterRoom(player, previousPosition, vObjects, vPortals);
}

void Engine::ToggleFullscreen()
{
    isFullscreen = !isFullscreen;
    if (isFullscreen)
    {
        // TODO
    }
    else
    {
        // TODO
    }
}
