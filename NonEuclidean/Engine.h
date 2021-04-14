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

#include <memory>
#include <vector>

class Engine
{
    friend class Input;

public:
    Engine();
    ~Engine();

    int Run();
    void Update();
    void Render(const Camera& cam, GLuint curFBO, const Portal* skipPortal);
    void LoadScene(int ix);
    void PickMouse();

    const Player& GetPlayer() const { return *player; }
    float NearestPortalDist() const;
    void OnPlayerEnterRoom(const Vector3& previousPosition);

private:
    void CreateGLWindow();
    void InitGLObjects();
    void DestroyGLObjects();
    void ToggleFullscreen();

    GLFWwindow* window;

    int iWidth;        // window width
    int iHeight;       // window height
    bool isFullscreen; // fullscreen state
    std::shared_ptr<ScreenBuffer> screenBuffer;
    std::shared_ptr<Minimap> minimap;

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
