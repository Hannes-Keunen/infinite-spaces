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

Engine::Engine(Args args)
    : args(args)
{
    GH_ENGINE = this;
    GH_INPUT = &input;
    isFullscreen = true;

    if (args.enableVr)
    {
        InitVR();
    }

    CreateGLWindow();
    InitGLObjects();
    input.SetupCallbacks(window);

    player.reset(new Player);
    GH_PLAYER = player.get();

    vScenes.push_back(std::make_shared<InfiniteSpace>(args.physicalSize, args.roomSize, args.removalStrategy));

    LoadScene(0);

    sky.reset(new Sky);
}

Engine::~Engine()
{
    glfwDestroyWindow(window);
    glfwTerminate();

    if (args.enableVr)
    {
        vr::VR_Shutdown();
    }
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

        const float near = GH_CLAMP(NearestPortalDist() * 0.5f, GH_NEAR_MIN, GH_NEAR_MAX);

        // 1. Render the screen view and object IDs
        {
            // Setup camera for rendering
            main_cam.worldView = player->WorldToCam();
            main_cam.SetSize(iWidth, iHeight, near, GH_FAR);
            main_cam.UseViewport();

            GH_REC_LEVEL = GH_MAX_RECURSION;
            screenBuffer->Bind();
            Render(main_cam, screenBuffer->Fbo(), nullptr);

            if (args.showMinimap)
            {
                minimap->Render(*player, *curScene);
            }

            // Present
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, iWidth, iHeight);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            screenBuffer->Present();

            if (args.showMinimap)
            {
                minimap->Present();
            }
        }

        // Render VR view
        if (args.enableVr)
        {
            // 2. obtain modelview matrices from the HMD
            auto Hmd34ToMatrix4 = [](const vr::HmdMatrix34_t& mat) {
                Matrix4 result;
                // clang-format off
                result.m[0]  = mat.m[0][0]; result.m[1]  = mat.m[1][0]; result.m[2]  = mat.m[2][0]; result.m[3]  = 0;
                result.m[4]  = mat.m[0][1]; result.m[5]  = mat.m[1][1]; result.m[6]  = mat.m[2][1]; result.m[7]  = 0;
                result.m[8]  = mat.m[0][2]; result.m[9]  = mat.m[1][2]; result.m[10] = mat.m[2][2]; result.m[11] = 0;
                result.m[12] = mat.m[0][3]; result.m[13] = mat.m[1][3]; result.m[14] = mat.m[2][3]; result.m[15] = 1;
                // clang-format on
                return result;
            };

            auto Hmd44ToMatrix4 = [](const vr::HmdMatrix44_t& mat) {
                Matrix4 result;
                // clang-format off
                result.m[0]  = mat.m[0][0]; result.m[1]  = mat.m[1][0]; result.m[2]  = mat.m[2][0]; result.m[3]  = mat.m[3][0];
                result.m[4]  = mat.m[0][1]; result.m[5]  = mat.m[1][1]; result.m[6]  = mat.m[2][1]; result.m[7]  = mat.m[3][1];
                result.m[8]  = mat.m[0][2]; result.m[9]  = mat.m[1][2]; result.m[10] = mat.m[2][2]; result.m[11] = mat.m[3][2];
                result.m[12] = mat.m[0][3]; result.m[13] = mat.m[1][3]; result.m[14] = mat.m[2][3]; result.m[15] = mat.m[3][3];
                // clang-format on
                return result;
            };

            vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
            vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
            if (poses[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
            {
                // head matrix
                auto head = Hmd34ToMatrix4(poses[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking);

                // eye-to-head
                auto transformLeft = Hmd34ToMatrix4(HMD->GetEyeToHeadTransform(vr::Eye_Left));
                auto transformRight = Hmd34ToMatrix4(HMD->GetEyeToHeadTransform(vr::Eye_Right));
                auto viewLeft = transformLeft.Inverse() * head.Inverse();
                auto viewRight = transformRight.Inverse() * head.Inverse();

                // eye projection
                auto projectionLeft = Hmd44ToMatrix4(HMD->GetProjectionMatrix(vr::Eye_Left, near, GH_FAR));
                auto projectionRight = Hmd44ToMatrix4(HMD->GetProjectionMatrix(vr::Eye_Right, near, GH_FAR));

                auto RenderView = [](const std::shared_ptr<FrameBuffer>& view, const Matrix4& eye,
                                     const Matrix4& proj) {
                    Camera cam;
                    cam.worldView = eye;
                    cam.projection = proj;

                    view->Render(cam, 0, nullptr);
                };

                // 3. render views
                RenderView(leftView, viewLeft, projectionLeft);
                RenderView(rightView, viewRight, projectionRight);

                // 4. present to HMD
                // vr::Texture_t leftTexture = {(void*) leftView->TexId(), vr::TextureType_OpenGL,
                // vr::ColorSpace_Gamma}; vr::VRCompositor()->Submit(vr::Eye_Left, &leftTexture); vr::Texture_t
                // rightTexture = {(void*) rightView->TexId(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma};
                // vr::VRCompositor()->Submit(vr::Eye_Right, &rightTexture);
            }
        }

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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Clear buffers
    if (GH_USE_SKY)
    {
        // glClear(GL_DEPTH_BUFFER_BIT);
        sky->Draw(cam);
    }

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
    if (args.enableVr)
    {
        iWidth = hmdWidth;
        iHeight = hmdHeight;
    }
    else
    {
        iWidth = GH_SCREEN_WIDTH;
        iHeight = GH_SCREEN_HEIGHT;
    }

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

void Engine::InitVR()
{
    if (!vr::VR_IsRuntimeInstalled())
    {
        throw std::runtime_error("No VR runtime installed!");
    }

    if (!vr::VR_IsHmdPresent())
    {
        throw std::runtime_error("No HMD present!");
    }

    vr::HmdError error;
    HMD = vr::VR_Init(&error, vr::VRApplication_Scene);
    if (error != vr::VRInitError_None)
    {
        printf("VR init error: %d\n", error);
        throw std::runtime_error("VR init error!");
    }

    if (!vr::VRCompositor())
    {
        throw std::runtime_error("Unable to initialize VR compositor");
    }

    HMD->GetRecommendedRenderTargetSize(&hmdWidth, &hmdHeight);
    printf("HMD dimensions: %dx%d\n", hmdWidth, hmdHeight);
}

void Engine::InitGLObjects()
{
    // glDisable(GL_BLEND);

    // Check GL functionality
    glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &occlusionCullingSupported);

    screenBuffer = std::make_shared<ScreenBuffer>(iWidth, iHeight);
    minimap = std::make_shared<Minimap>();

    if (args.enableVr)
    {
        leftView = std::make_shared<FrameBuffer>(hmdWidth, hmdHeight);
        rightView = std::make_shared<FrameBuffer>(hmdWidth, hmdHeight);
    }
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
