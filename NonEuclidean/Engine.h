#pragma once

#include <glad/glad.h>

#include "Camera.h"
#include "GameHeader.h"
#include "InfiniteSpace.h"
#include "Input.h"
#include "Minimap.h"
#include "Object.h"
#include "Player.h"
#include "Portal.h"
#include "ScreenBuffer.h"
#include "Sky.h"
#include "Timer.h"

#include <GLFW/glfw3.h>
#include <openvr.h>

#include <memory>
#include <vector>

class Engine
{
    friend class Input;

public:
    struct Args
    {
        bool enableVr = false;
        bool showMinimap = false;
        int physicalSize = 16;
        int roomSize = 5;
        RemovalStrategy removalStrategy = RemovalStrategy::IMMEDIATE;
    };

    Engine(Args args);
    ~Engine();

    int Run();
    void Update();
    void Render(const Camera& cam, GLuint curFBO, const Portal* skipPortal);
    void LoadScene(int ix);
    void PickMouse();

    const Player& GetPlayer() const { return *player; }
    float NearestPortalDist() const;
    void OnPlayerEnterRoom(const Vector3& previousPosition, const Vector3& currentPosition);

private:
    void CreateGLWindow();
    void InitVR();
    void InitGLObjects();
    void DestroyGLObjects();
    void ToggleFullscreen();
    Matrix4 GetHeadMatrix();
    Matrix4 GetEyeMatrix(vr::Hmd_Eye eye);
    Matrix4 GetProjectionMatrix(vr::Hmd_Eye eye, float fNear, float fFar);

    void ProcessPlayerMotion(const Matrix4& headMatrix);
    bool TryPortals();

public:
    Args args;

    GLFWwindow* window;
    vr::IVRSystem* HMD;

    int iWidth;                   // window width
    int iHeight;                  // window height
    uint32_t hmdWidth, hmdHeight; // HMD render target size
    bool isFullscreen;            // fullscreen state
    std::shared_ptr<ScreenBuffer> screenBuffer;
    std::shared_ptr<Minimap> minimap;
    std::shared_ptr<FrameBuffer> leftView, rightView;

    Matrix4 eyeMatrixLeft, eyeMatrixRight;
    Matrix4 projectionMatrixLeft, projectionMatrixRight;

    Camera main_cam;
    Input input;
    Timer timer;

    std::vector<std::shared_ptr<Object>> vObjects;
    std::vector<std::shared_ptr<Portal>> vPortals;
    std::shared_ptr<Sky> sky;
    std::shared_ptr<Player> player;

    GLint occlusionCullingSupported;

    std::vector<std::shared_ptr<InfiniteSpace>> vScenes;
    std::shared_ptr<InfiniteSpace> curScene;
};
