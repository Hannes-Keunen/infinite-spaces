#pragma once

#include "Scene.h"

#include <array>

enum class Side
{
    North,
    East,
    South,
    West,
};

// clang-format off
const std::vector<Vector3> ROOM_COLORS = {
    Vector3(1.0, 0.0, 0.0),
    Vector3(0.0, 1.0, 0.0),
    Vector3(0.0, 0.0, 1.0),
    Vector3(1.0, 1.0, 0.0),
    Vector3(1.0, 0.0, 1.0),
    Vector3(1.0, 1.0, 1.0),
};
// clang-format on

class Door : public Object
{
public:
    Door(int targetRoom, int side, const Vector3& doorColor)
        : targetRoom(targetRoom)
        , side(side)
    {
        mesh = AquireMesh("door.obj");
        shader = AquireShader("color");
        color = doorColor;
        scale = Vector3(0.32);
        euler = Vector3(0, GH_PI / 2.0f, 0);
    }

    int targetRoom;
    int side;
};

struct Target : public Object
{
public:
    Target()
    {
        mesh = AquireMesh("bunny.obj");
        shader = AquireShader("texture");
        texture = AquireTexture("gold.bmp");
        scale = Vector3(6);
        pos.y = -0.2;
    }
    virtual ~Target() {}
};

class Pillar : public Object
{
public:
    Pillar(int room)
    {
        mesh = AquireMesh("pillar.obj");
        shader = AquireShader("color");
        color = ROOM_COLORS[room % ROOM_COLORS.size()];
        scale = Vector3(0.2, 0.094, 0.2);
    }
};

class Room : public Object
{
public:
    Room(int size)
        : size(size)
    {
        mesh = AquireMesh("box.obj");
        shader = AquireShader("texture");
        texture = AquireTexture("ParchmentWallpaper.png");
        scale = Vector3(size, 1, size);
    }

    void PlaceDoor(std::shared_ptr<Door>& door, Side side);
    void PlacePortal(std::shared_ptr<Portal>& portal, Side side);
    auto GetDoorPhysicalPos(Side side) const -> Vector3;

    std::array<std::shared_ptr<Door>, 4> doors = {nullptr, nullptr, nullptr, nullptr};
    std::array<std::shared_ptr<Portal>, 4> activePortals = {nullptr, nullptr, nullptr, nullptr};
    std::shared_ptr<Target> target;
    std::shared_ptr<Pillar> pillar;
    int size;
    Vector3 physicalPos;
};

struct Corridor
{
    Corridor(
        const std::shared_ptr<Room>& entranceRoom,
        int entranceRoomIndex,
        Side entranceSide,
        const std::shared_ptr<Room>& exitRoom,
        int exitRoomIndex,
        Side exitSide,
        int physicalSize);
    void SetEntrancePortal(std::shared_ptr<Portal>& portal, Side side);
    void SetExitPortal(std::shared_ptr<Portal>& portal, Side side);
    void ConnectMiddlePortals();

    std::shared_ptr<Object> part1;
    std::shared_ptr<Object> part2;
    std::shared_ptr<Portal> p1;
    std::shared_ptr<Portal> p2;
    Side connectionSide;

    std::vector<Vector3> points;
    std::shared_ptr<Portal> entrancePortal;
    std::shared_ptr<Portal> exitPortal;
    int entranceRoomIndex;
    int exitRoomIndex;
    Side entranceSide;
    Side exitSide;
};

struct RoomType
{
    std::shared_ptr<Texture> texture;
    bool hasTarget;
};

struct Node
{
    int size;
    int roomType;
    std::array<int, 4> connections;
    bool hasTarget;

    std::shared_ptr<Room> room;
};

enum class RemovalStrategy
{
    IMMEDIATE,
    KEEP_ONE,
};

class InfiniteSpace : public Scene
{
public:
    InfiniteSpace(int physicalSize, int roomSize, RemovalStrategy removalStrategy = RemovalStrategy::IMMEDIATE);
    virtual void Load(PObjectVec& objs, PPortalVec& portals, Player& player) override;

    void OnDoorClicked(std::shared_ptr<Door>& door, PObjectVec& objs, PPortalVec& portals, Player& player);
    void OnTargetClicked(std::shared_ptr<Target>& target, PObjectVec& objs, Player& player);
    void OnPlayerEnterRoom(
        const std::shared_ptr<Player>& player, const Vector3& previousPosition, PObjectVec& objs, PPortalVec& portals);

    Vector3 GetPhysicalPos(const Vector3& pos) const;
    int GetPhysicalSize() const { return physicalSize; };
    void CreateFloorplanVertices(const Player& player, std::vector<float>& vertices) const;

private:
    /**
     * Places a room node in virtual space.
     * The node must have been generated first.
     */
    auto PlaceRoom(PObjectVec& objs, int nodeIndex, int entranceSide = -1) -> std::shared_ptr<Room>;
    void PlaceCorridor(std::shared_ptr<Corridor>& corridor);
    bool IsValidNode(int nodeIndex);
    void GenerateNode(int type, int index, int entranceSide = -1, int entranceNode = -1);

    /** Removes a room, including it's portals and doors. */
    void RemoveRoom(int index, PObjectVec& objs, PPortalVec& portals);
    /** Removes a corridor, including it's entrance and exit portals. */
    void RemoveCorridor(std::shared_ptr<Corridor>& corridor, PObjectVec& objs, PPortalVec& portals);
    /** Removes a corridor and the room it leads to */
    void
    CloseCorridor(int currentRoomIndex, std::shared_ptr<Corridor>& corridor, PObjectVec& objs, PPortalVec& portals);

private:
    int physicalSize;
    int roomSize;
    int nextNode = 1;
    RemovalStrategy removalStrategy;
    std::vector<RoomType> roomTypes;
    std::vector<Node> nodes;
    std::vector<std::shared_ptr<Corridor>> activeCorridors;
};
