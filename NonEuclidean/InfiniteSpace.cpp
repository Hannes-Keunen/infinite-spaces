#include "InfiniteSpace.h"

#include "Ground.h"

#include <algorithm>
#include <random>

constexpr float CORRIDOR_WIDTH = 1.0f;
constexpr float CORRIDOR_HEIGHT = 3.0f;
constexpr float PORTAL_WIDTH = 0.5f;

constexpr int ROOMTYPE_START = 0;
constexpr int ROOMTYPE_TARGET = 1;
constexpr int ROOMTYPE_2 = 2;

static auto CreateCorridorMesh(
    const std::vector<Vector3>& points,
    std::shared_ptr<Mesh>& part1,
    std::shared_ptr<Mesh>& part2,
    Side entranceSide,
    Side exitSide,
    Side& connectionSide)
{
    std::vector<float> verts;
    std::vector<float> uvs;
    std::vector<float> normals;
    std::vector<Collider> colliders;

    auto vertex = [&](const Vector3& vert, const Vector3& uv, const Vector3& normal) {
        verts.push_back(vert.x);
        verts.push_back(vert.y);
        verts.push_back(vert.z);

        uvs.push_back(uv.x);
        uvs.push_back(uv.y);

        normals.push_back(normal.x);
        normals.push_back(normal.y);
        normals.push_back(normal.z);
    };

    enum Faces
    {
        None = 0x00,
        Left = 0x01,
        Right = 0x02,
        Front = 0x04,
        Back = 0x08,
    };

    auto segment = [&](const Vector3& center, const Vector3& scale, uint8_t exclude, uint8_t skipColliders = 0) {
        Vector3 v_blb(center.x - scale.x / 2, center.y, center.z - scale.z / 2);           // bottom left back
        Vector3 v_brb(center.x + scale.x / 2, center.y, center.z - scale.z / 2);           // bottom right back
        Vector3 v_blf(center.x - scale.x / 2, center.y, center.z + scale.z / 2);           // bottom left front
        Vector3 v_brf(center.x + scale.x / 2, center.y, center.z + scale.z / 2);           // bottom left front
        Vector3 v_tlb(center.x - scale.x / 2, center.y + scale.y, center.z - scale.z / 2); // top left back
        Vector3 v_trb(center.x + scale.x / 2, center.y + scale.y, center.z - scale.z / 2); // top right back
        Vector3 v_tlf(center.x - scale.x / 2, center.y + scale.y, center.z + scale.z / 2); // top left front
        Vector3 v_trf(center.x + scale.x / 2, center.y + scale.y, center.z + scale.z / 2); // top left front

        Vector3 vt(0, 0, 0);

        // floor
        vertex(v_blf, vt, Vector3(0, 1, 0));
        vertex(v_brb, vt, Vector3(0, 1, 0));
        vertex(v_blb, vt, Vector3(0, 1, 0));
        colliders.emplace_back(v_blb, v_brb, v_blf);
        vertex(v_blf, vt, Vector3(0, 1, 0));
        vertex(v_brf, vt, Vector3(0, 1, 0));
        vertex(v_brb, vt, Vector3(0, 1, 0));
        colliders.emplace_back(v_blf, v_brb, v_brf);

        // ceiling
        vertex(v_tlb, vt, Vector3(0, 1, 0));
        vertex(v_trb, vt, Vector3(0, 1, 0));
        vertex(v_tlf, vt, Vector3(0, 1, 0));
        colliders.emplace_back(v_tlb, v_trb, v_tlf);
        vertex(v_tlf, vt, Vector3(0, 1, 0));
        vertex(v_trb, vt, Vector3(0, 1, 0));
        vertex(v_trf, vt, Vector3(0, 1, 0));
        colliders.emplace_back(v_tlf, v_trb, v_trf);

        if (!(exclude & Left))
        {
            // left
            vertex(v_blf, vt, Vector3(1, 0, 0));
            vertex(v_tlb, vt, Vector3(1, 0, 0));
            vertex(v_tlf, vt, Vector3(1, 0, 0));
            vertex(v_blf, vt, Vector3(1, 0, 0));
            vertex(v_blb, vt, Vector3(1, 0, 0));
            vertex(v_tlb, vt, Vector3(1, 0, 0));

            if (!(skipColliders & Left))
            {
                colliders.emplace_back(v_blf, v_tlb, v_tlf);
                colliders.emplace_back(v_blf, v_blb, v_tlb);
            }
        }

        if (!(exclude & Right))
        {
            // right
            vertex(v_brf, vt, Vector3(1, 0, 0));
            vertex(v_trf, vt, Vector3(1, 0, 0));
            vertex(v_trb, vt, Vector3(1, 0, 0));
            vertex(v_brf, vt, Vector3(1, 0, 0));
            vertex(v_trb, vt, Vector3(1, 0, 0));
            vertex(v_brb, vt, Vector3(1, 0, 0));

            if (!(skipColliders & Right))
            {
                colliders.emplace_back(v_brf, v_trf, v_trb);
                colliders.emplace_back(v_brf, v_trb, v_brb);
            }
        }

        if (!(exclude & Front))
        {
            // front
            vertex(v_blf, vt, Vector3(0, 0, 1));
            vertex(v_tlf, vt, Vector3(0, 0, 1));
            vertex(v_trf, vt, Vector3(0, 0, 1));
            vertex(v_blf, vt, Vector3(0, 0, 1));
            vertex(v_trf, vt, Vector3(0, 0, 1));
            vertex(v_brf, vt, Vector3(0, 0, 1));

            if (!(skipColliders & Front))
            {
                colliders.emplace_back(v_blf, v_trf, v_tlf);
                colliders.emplace_back(v_blf, v_trf, v_brf);
            }
        }

        if (!(exclude & Back))
        {
            // back
            vertex(v_blb, vt, Vector3(0, 0, 1));
            vertex(v_trb, vt, Vector3(0, 0, 1));
            vertex(v_tlb, vt, Vector3(0, 0, 1));
            vertex(v_blb, vt, Vector3(0, 0, 1));
            vertex(v_brb, vt, Vector3(0, 0, 1));
            vertex(v_trb, vt, Vector3(0, 0, 1));

            if (!(skipColliders & Back))
            {
                colliders.emplace_back(v_blb, v_trb, v_tlb);
                colliders.emplace_back(v_blb, v_tlb, v_brb);
            }
        }
    };

    for (int i = 0; i < points.size(); i++)
    {
        const auto& currrent = points[i];
        Faces in = None, out = None;

        if (i > 0)
        {
            const auto& prev = points[i - 1];

            // draw a corridor segment from the previous point
            auto center = (prev + currrent) / 2.0f;

            auto distance = prev - currrent;
            distance.x = std::fabs(distance.x);
            distance.z = std::fabs(distance.z);
            if (distance.x < 0.001 && distance.z > CORRIDOR_WIDTH)
            {
                // vertical segment
                float length = distance.z - CORRIDOR_WIDTH;
                segment(center, Vector3(CORRIDOR_WIDTH, CORRIDOR_HEIGHT, length), Front | Back);
            }
            else if (distance.z < 0.001 && distance.x > CORRIDOR_WIDTH)
            {
                // horizontal segment
                float length = distance.x - CORRIDOR_WIDTH;
                segment(center, Vector3(length, CORRIDOR_HEIGHT, CORRIDOR_WIDTH), Left | Right);
            }

            // find incoming side
            auto d_in = currrent - prev;
            if (d_in.x == 0)
            {
                in = d_in.z < 0 ? Front : Back;
            }
            else if (d_in.z == 0)
            {
                in = d_in.x < 0 ? Right : Left;
            }
        }

        Side outSide;
        if (i < points.size() - 1)
        {
            const auto& next = points[i + 1];

            // find outgoing face
            auto d_out = next - currrent;
            if (d_out.x == 0)
            {
                out = d_out.z < 0 ? Back : Front;
                outSide = d_out.z < 0 ? Side::North : Side::South;
            }
            else if (d_out.z == 0)
            {
                out = d_out.x < 0 ? Left : Right;
                outSide = d_out.x < 0 ? Side::West : Side::East;
            }
        }

        // corner segment
        auto sideToFaces = [](Side side) {
            switch (side)
            {
                case Side::North: return Front;
                case Side::East: return Left;
                case Side::South: return Back;
                case Side::West: return Right;
                default: return (Faces) 0;
            }
        };

        Faces skipColliders = (Faces) 0;
        if (i == 0)
        {
            skipColliders = sideToFaces(entranceSide);
        }
        if (i == points.size() - 1)
        {
            skipColliders = sideToFaces(exitSide);
        }
        segment(points[i], Vector3(CORRIDOR_WIDTH, CORRIDOR_HEIGHT, CORRIDOR_WIDTH), in | out, skipColliders);

        if (i == points.size() / 2)
        {
            // cut corridor in two parts so that it may overlap with itself
            part1 = std::make_shared<Mesh>(verts, uvs, normals, colliders);
            verts.clear();
            uvs.clear();
            normals.clear();
            colliders.clear();
            connectionSide = outSide;
        }
    }

    part2 = std::make_shared<Mesh>(verts, uvs, normals, colliders);
}

Corridor::Corridor(
    const std::shared_ptr<Room>& entranceRoom,
    int entranceRoomIndex,
    Side entranceSide,
    const std::shared_ptr<Room>& exitRoom,
    int exitRoomIndex,
    Side exitSide,
    int physicalSize)
    : entranceRoomIndex(entranceRoomIndex)
    , exitRoomIndex(exitRoomIndex)
    , entranceSide(entranceSide)
    , exitSide(exitSide)
{
    auto S = entranceRoom->GetDoorPhysicalPos(entranceSide);
    // generate corridor points based on door positions
    std::random_device r;
    std::mt19937 e(r());
    auto dist = std::uniform_real_distribution<>();

    Vector3 E = exitRoom->GetDoorPhysicalPos(exitSide);

    // pick first intermediate point I
    Vector3 I(0, 0, 0);
    switch (entranceSide)
    {
        case Side::North:
        {
            double rangeX = physicalSize - CORRIDOR_WIDTH * 4;
            I.x = dist(e) * rangeX - rangeX / 2; // random
            I.x += (I.x < S.x ? -CORRIDOR_WIDTH : CORRIDOR_WIDTH);

            double min = S.z - CORRIDOR_WIDTH;
            double max = -(physicalSize - CORRIDOR_WIDTH) / 2.0f;
            double range = min - max;
            I.z = min - dist(e) * range; // random -z
            break;
        }

        case Side::East:
        {
            double wall = (physicalSize - CORRIDOR_WIDTH) / 2;
            double min = S.x + CORRIDOR_WIDTH;
            double range = wall - min;
            I.x = min + dist(e) * range; // random -z

            double rangeZ = physicalSize - CORRIDOR_WIDTH * 4;
            I.z = dist(e) * rangeZ - rangeZ / 2; // random
            I.z += (I.z < S.z ? -CORRIDOR_WIDTH : CORRIDOR_WIDTH);
            break;
        }

        case Side::South:
        {
            double rangeX = physicalSize - CORRIDOR_WIDTH * 4;
            I.x = dist(e) * rangeX - rangeX / 2; // random
            I.x += (I.x < S.x ? -CORRIDOR_WIDTH : CORRIDOR_WIDTH);

            double min = S.z + CORRIDOR_WIDTH;
            double max = (physicalSize - CORRIDOR_WIDTH) / 2;
            double range = max - min;
            I.z = min + dist(e) * range; // random +z
            break;
        }

        case Side::West:
        {
            double min = S.x - CORRIDOR_WIDTH;
            double max = -(physicalSize - CORRIDOR_WIDTH) / 2;
            double range = min - max;
            I.x = min - dist(e) * range; // random -z

            double rangeZ = physicalSize - CORRIDOR_WIDTH * 4;
            I.z = dist(e) * rangeZ - rangeZ / 2; // random
            I.z += (I.z < S.z ? -CORRIDOR_WIDTH : CORRIDOR_WIDTH);
            break;
        }
    }

    // pick additional points a
    // first point
    Vector3 a1 = S;
    auto random = dist(e);
    if (random < 0.5)
    {
        // first Z
        a1.z = I.z;
    }
    else
    {
        // first X
        a1.x = I.x;
    }

    // second point
    Vector3 a2 = I;
    random = dist(e);
    if ((a1.z == I.z && ((a1.x < I.x && E.x < I.x) || (a1.x > I.x && E.x > I.x)))     // can't walk along X first
        || !(a1.x == I.x && ((a1.z < I.z && E.z < I.z) || (a1.z > I.z && E.z > I.z))) // can walk along Z first
               && random < 0.5)
    {
        // first Z
        a2.z = E.z;
    }
    else
    {
        // first X
        a2.x = E.x;
    }

    // generate corridor mesh
    points = {S, a1, I, a2, E};
    part1 = std::make_shared<Object>();
    part2 = std::make_shared<Object>();
    CreateCorridorMesh(points, part1->mesh, part2->mesh, entranceSide, exitSide, connectionSide);
    part1->texture = AquireTexture("three_room.bmp");
    part2->texture = AquireTexture("three_room.bmp");
    part1->shader = AquireShader("texture");
    part2->shader = AquireShader("texture");
    this->part1 = part1;
    this->part2 = part2;

    // connect parts
    p1 = std::make_shared<Portal>();
    p2 = std::make_shared<Portal>();
}

void Corridor::SetEntrancePortal(std::shared_ptr<Portal>& portal, Side side)
{
    entrancePortal = portal;
    portal->pos = part1->pos + points.front();
    portal->pos.y = 1.0f;
    portal->scale.x = PORTAL_WIDTH;
    switch (side)
    {
        case Side::North: portal->pos.z += (CORRIDOR_WIDTH / 2 - 0.001f); break;
        case Side::East:
            portal->pos.x -= (CORRIDOR_WIDTH / 2 - 0.001f);
            portal->euler.y = -GH_PI / 2;
            break;
        case Side::South:
            portal->pos.z -= (CORRIDOR_WIDTH / 2 - 0.001f);
            portal->euler.y = GH_PI;
            break;
        case Side::West:
            portal->pos.x += (CORRIDOR_WIDTH / 2 - 0.001f);
            portal->euler.y = GH_PI / 2;
            break;
    }
}

void Corridor::SetExitPortal(std::shared_ptr<Portal>& portal, Side side)
{
    exitPortal = portal;
    portal->pos = part2->pos + points.back();
    portal->pos.y = 1.0f;
    portal->scale.x = PORTAL_WIDTH;
    switch (side)
    {
        case Side::North: portal->pos.z += (CORRIDOR_WIDTH / 2 - 0.001f); break;
        case Side::East:
            portal->pos.x -= (CORRIDOR_WIDTH / 2 - 0.001f);
            portal->euler.y = -GH_PI / 2;
            break;
        case Side::South:
            portal->pos.z -= (CORRIDOR_WIDTH / 2 - 0.001f);
            portal->euler.y = GH_PI;
            break;
        case Side::West:
            portal->pos.x += (CORRIDOR_WIDTH / 2 - 0.001f);
            portal->euler.y = GH_PI / 2;
            break;
    }
}

void Corridor::ConnectMiddlePortals()
{
    p1->pos = part1->pos + points[points.size() / 2];
    p1->pos.y = 1.5f;
    p1->scale.x = PORTAL_WIDTH;
    p1->scale.y = CORRIDOR_HEIGHT / 2.0f;

    p2->pos = part2->pos + points[points.size() / 2];
    p2->pos.y = 1.5f;
    p2->scale.x = PORTAL_WIDTH;
    p2->scale.y = CORRIDOR_HEIGHT / 2.0f;

    switch (connectionSide)
    {
        case Side::North:
            p1->pos.z -= CORRIDOR_WIDTH / 2.0f;
            p2->pos.z -= CORRIDOR_WIDTH / 2.0f;
            break;
        case Side::East:
            p1->pos.x += CORRIDOR_WIDTH / 2.0f;
            p2->pos.x += CORRIDOR_WIDTH / 2.0f;
            p1->euler.y = -GH_PI / 2.0f;
            p2->euler.y = -GH_PI / 2.0f;
            break;
        case Side::South:
            p1->pos.z += CORRIDOR_WIDTH / 2.0f;
            p2->pos.z += CORRIDOR_WIDTH / 2.0f;
            break;
        case Side::West:
            p1->pos.x -= CORRIDOR_WIDTH / 2.0f;
            p2->pos.x -= CORRIDOR_WIDTH / 2.0f;
            p1->euler.y = GH_PI / 2.0f;
            p2->euler.y = GH_PI / 2.0f;
            break;
    }

    Portal::Connect(p1, p2);
}

InfiniteSpace::InfiniteSpace(int physicalSize, int roomSize, RemovalStrategy removalStrategy)
    : physicalSize(physicalSize)
    , roomSize(roomSize)
    , removalStrategy(removalStrategy)
{
    roomTypes.push_back({.texture = AquireTexture("stonetiles.bmp"), .hasTarget = false});
    roomTypes.push_back({.texture = AquireTexture("ParchmentWallpaper.bmp"), .hasTarget = true});
    roomTypes.push_back({.texture = AquireTexture("ParchmentWallpaper.bmp"), .hasTarget = false});
}

void Room::PlaceDoor(std::shared_ptr<Door>& door, Side side)
{
    doors[(int) side] = door;
    door->pos = pos;
    switch (side)
    {
        case Side::North: door->pos.z -= size / 2.0f; break;
        case Side::East:
            door->pos.x += size / 2.0f;
            door->euler.y -= GH_PI / 2.0f;
            break;
        case Side::South:
            door->pos.z += size / 2.0f;
            door->euler.y += GH_PI;
            break;
        case Side::West:
            door->pos.x -= size / 2.0f;
            door->euler.y += GH_PI / 2.0f;
            break;
    }
}

void Room::PlacePortal(std::shared_ptr<Portal>& portal, Side side)
{
    activePortals[(int) side] = portal;
    portal->scale.x = PORTAL_WIDTH;
    switch (side)
    {
        case Side::North: portal->pos = pos + Vector3(0, 0, -size / 2.0f + 0.001f); break;
        case Side::East:
            portal->pos = pos + Vector3(size / 2.0f - 0.001f, 0, 0);
            portal->euler.y = -GH_PI / 2.0f;
            break;
        case Side::South:
            portal->pos = pos + Vector3(0, 0, size / 2.0f - 0.001f);
            portal->euler.y = GH_PI;
            break;
        case Side::West:
            portal->pos = pos + Vector3(-size / 2.0f + 0.001f, 0, 0);
            portal->euler.y = GH_PI / 2.0f;
            break;
    }
    portal->pos.y = 1.0f;
}

auto Room::GetDoorPhysicalPos(Side side) const -> Vector3
{
    switch (side)
    {
        case Side::North: return physicalPos + Vector3(0, 0, -size / 2.0f - CORRIDOR_WIDTH / 2.0f);
        case Side::East: return physicalPos + Vector3(size / 2.0f + CORRIDOR_WIDTH / 2.0f, 0, 0);
        case Side::South: return physicalPos + Vector3(0, 0, size / 2.0f + CORRIDOR_WIDTH / 2.0f);
        case Side::West: return physicalPos + Vector3(-size / 2.0f - CORRIDOR_WIDTH / 2.0f, 0, 0);
        default: return Vector3(0, 0, 0);
    }
}

void InfiniteSpace::Load(PObjectVec& objs, PPortalVec& portals, Player& player)
{
    GenerateNode(ROOMTYPE_START, 0);
    PlaceRoom(objs, 0);

    player.pos = Vector3(0, GH_PLAYER_HEIGHT, 0);
}

void InfiniteSpace::OnDoorClicked(std::shared_ptr<Door>& door, PObjectVec& objs, PPortalVec& portals, Player& player)
{
    // Which room are we in? => player position
    // the player must be in a room; rooms are placed along the z axis.
    assert(player.pos.x > -physicalSize / 2 && player.pos.x < physicalSize / 2);
    int currentRoomIndex = std::round(player.pos.z / physicalSize);
    auto currentRoom = nodes[currentRoomIndex].room;
    assert(currentRoom != nullptr);

    // close any corridors that are currently open in this room
    for (auto& corridor : activeCorridors)
    {
        if (corridor != nullptr
            && (corridor->entranceRoomIndex == currentRoomIndex && corridor->entranceSide != (Side) door->side
                || corridor->exitRoomIndex == currentRoomIndex && corridor->exitSide != (Side) door->side))
        {
            CloseCorridor(currentRoomIndex, corridor, objs, portals);
        }
    }

    // if targetRoom == -1, the corridor is already placed so the door only has to be removed
    if (door->targetRoom >= 0)
    {
        // Place the next corridor and room
        // To which room do we want to go? => in Door struct
        // other room incoming direction? => same as exit direction; in door struct
        // place the room we want to go to.
        int type = rand() % (roomTypes.size() - 1) + 1;
        GenerateNode(type, door->targetRoom, door->side, currentRoomIndex);
        assert(nodes[door->targetRoom].connections[door->side] == currentRoomIndex);
        auto nextRoom = PlaceRoom(objs, door->targetRoom, door->side);

        // add corridor
        auto corridor = std::shared_ptr<Corridor>(new Corridor(
            currentRoom, currentRoomIndex, (Side) door->side, nextRoom, door->targetRoom, (Side) door->side,
            physicalSize));
        PlaceCorridor(corridor);
        corridor->ConnectMiddlePortals();
        objs.push_back(corridor->part1);
        objs.push_back(corridor->part2);
        portals.push_back(corridor->p1);
        portals.push_back(corridor->p2);

        // entrance portal
        auto p1 = std::make_shared<Portal>();
        auto p2 = std::make_shared<Portal>();
        currentRoom->PlacePortal(p1, (Side) door->side);
        corridor->SetEntrancePortal(p2, (Side) door->side);
        portals.push_back(p1);
        portals.push_back(p2);
        Portal::Connect(p1, p2);

        // exit portal
        auto p3 = std::make_shared<Portal>();
        auto p4 = std::make_shared<Portal>();
        corridor->SetExitPortal(p3, corridor->exitSide);
        nextRoom->PlacePortal(p4, corridor->exitSide);
        portals.push_back(p3);
        portals.push_back(p4);
        Portal::Connect(p3, p4);
    }

    // remove door
    objs.erase(std::remove_if(objs.begin(), objs.end(), [&](auto& it) { return it == door; }), objs.end());
    currentRoom->doors[door->side] = nullptr;
}

void InfiniteSpace::OnTargetClicked(std::shared_ptr<Target>& target, PObjectVec& objs, Player& player)
{
    objs.erase(std::remove(objs.begin(), objs.end(), target));

    auto& currentNode = nodes[std::round(player.pos.z / physicalSize)];
    currentNode.hasTarget = false;
    currentNode.room->target = nullptr;
}

void InfiniteSpace::OnPlayerEnterRoom(
    const std::shared_ptr<Player>& player, const Vector3& previousPosition, PObjectVec& objs, PPortalVec& portals)
{
    // did we just enter a room?
    if (player->pos.x > -physicalSize / 2 && player->pos.x < physicalSize / 2)
    {
        // Which room are we in? => player position
        // Which corridor do we come from? => previous position
        // rooms are placed along the z=0 axis, corridors along the z=8 axis.
        int currentRoomIndex = std::round(player->pos.z / physicalSize);
        auto& currentCorridor = activeCorridors[std::round(previousPosition.z / physicalSize)];
        assert(currentCorridor != nullptr);

        if (removalStrategy == RemovalStrategy::IMMEDIATE)
        {
            CloseCorridor(currentRoomIndex, currentCorridor, objs, portals);
        }
        else if (removalStrategy == RemovalStrategy::KEEP_ONE)
        {
            // find the last room
            int lastRoomIndex;
            Side side;
            if (currentRoomIndex == currentCorridor->exitRoomIndex)
            {
                lastRoomIndex = currentCorridor->entranceRoomIndex;
                side = currentCorridor->exitSide;
            }
            else if (currentRoomIndex == currentCorridor->entranceRoomIndex)
            {
                lastRoomIndex = currentCorridor->exitRoomIndex;
                side = currentCorridor->entranceSide;
            }

            // in the last room, close all doors except the one leading to the current room
            for (auto& corridor : activeCorridors)
            {
                if (corridor != currentCorridor)
                {
                    CloseCorridor(lastRoomIndex, corridor, objs, portals);
                }
            }

            // Place a door
            auto door = std::make_shared<Door>(-1, (int) side, ROOM_COLORS[lastRoomIndex % ROOM_COLORS.size()]);
            nodes[currentRoomIndex].room->PlaceDoor(door, side);
            objs.push_back(door);
        }
    }
}

Vector3 InfiniteSpace::GetPhysicalPos(const Vector3& pos) const
{
    auto result = pos;
    while (result.x > physicalSize / 2.0f) result.x -= physicalSize;
    while (result.z > physicalSize / 2.0f) result.z -= physicalSize;
    result.z = -result.z;
    return result;
}

void InfiniteSpace::CreateFloorplanVertices(const Player& player, std::vector<float>& vertices) const
{
    for (const auto& node : nodes)
    {
        const auto& room = node.room;
        if (room != nullptr)
        {
            vertices.push_back(room->physicalPos.x - node.size / 2.0f);
            vertices.push_back(-room->physicalPos.z - node.size / 2.0f);
            vertices.push_back(room->physicalPos.x + node.size / 2.0f);
            vertices.push_back(-room->physicalPos.z - node.size / 2.0f);

            vertices.push_back(room->physicalPos.x + node.size / 2.0f);
            vertices.push_back(-room->physicalPos.z - node.size / 2.0f);
            vertices.push_back(room->physicalPos.x + node.size / 2.0f);
            vertices.push_back(-room->physicalPos.z + node.size / 2.0f);

            vertices.push_back(room->physicalPos.x + node.size / 2.0f);
            vertices.push_back(-room->physicalPos.z + node.size / 2.0f);
            vertices.push_back(room->physicalPos.x - node.size / 2.0f);
            vertices.push_back(-room->physicalPos.z + node.size / 2.0f);

            vertices.push_back(room->physicalPos.x - node.size / 2.0f);
            vertices.push_back(-room->physicalPos.z + node.size / 2.0f);
            vertices.push_back(room->physicalPos.x - node.size / 2.0f);
            vertices.push_back(-room->physicalPos.z - node.size / 2.0f);
        }
    }

    for (const auto& corridor : activeCorridors)
    {
        if (corridor != nullptr)
        {
            auto entrancePos = GetPhysicalPos(corridor->entrancePortal->pos);
            auto exitPos = GetPhysicalPos(corridor->exitPortal->pos);

            vertices.push_back(entrancePos.x);
            vertices.push_back(entrancePos.z);
            vertices.push_back(corridor->points.front().x);
            vertices.push_back(-corridor->points.front().z);

            for (int i = 0; i < corridor->points.size() - 1; i++)
            {
                vertices.push_back(corridor->points[i].x);
                vertices.push_back(-corridor->points[i].z);
                vertices.push_back(corridor->points[i + 1].x);
                vertices.push_back(-corridor->points[i + 1].z);
            }

            vertices.push_back(corridor->points.back().x);
            vertices.push_back(-corridor->points.back().z);
            vertices.push_back(exitPos.x);
            vertices.push_back(exitPos.z);
        }
    }

    for (auto& vertex : vertices) { vertex = vertex / ((float) physicalSize / 2.0f); }
}

auto InfiniteSpace::PlaceRoom(PObjectVec& objs, int nodeIndex, int entranceSide) -> std::shared_ptr<Room>
{
    assert(IsValidNode(nodeIndex));

    // create room object
    auto room = std::make_shared<Room>(nodes[nodeIndex].size);
    room->texture = roomTypes[nodes[nodeIndex].roomType].texture;
    room->pos.z = nodeIndex * physicalSize;
    objs.push_back(room);
    nodes[nodeIndex].room = room;

    // pick a random position in the environment
    if (nodeIndex != 0) // room 0 is always at (0, 0)
    {
        // random number between +- physicalSize / 2.0f + 1.0f
        int a = physicalSize - nodes[nodeIndex].size - 2;
        room->physicalPos.x = (rand() % a) - (a / 2.0f);
        room->physicalPos.z = (rand() % a) - (a / 2.0f);
        room->pos += room->physicalPos;
    }

    // place doors
    for (int side = 0; side < nodes[nodeIndex].connections.size(); side++)
    {
        int connection = nodes[nodeIndex].connections[side];
        if (side != entranceSide && connection >= 0)
        {
            // create door object
            auto door = std::make_shared<Door>(connection, side, ROOM_COLORS[connection % ROOM_COLORS.size()]);
            room->PlaceDoor(door, (Side) side);
            objs.push_back(door);
        }
    }

    // place colored object
    auto pillar = std::make_shared<Pillar>(nodeIndex);
    pillar->pos.x = room->pos.x - 2.0f;
    pillar->pos.z = room->pos.z - 2.0f;
    objs.push_back(pillar);
    room->pillar = pillar;

    // place target if this is a target room
    if (nodes[nodeIndex].hasTarget)
    {
        auto target = std::make_shared<Target>();
        target->pos.x = room->pos.x + 2.0f;
        target->pos.z = room->pos.z - 2.0f;
        objs.push_back(target);
        room->target = target;
    }

    return room;
}

void InfiniteSpace::PlaceCorridor(std::shared_ptr<Corridor>& corridor)
{
    // look for an empty slot
    int i = 0;
    for (; i < activeCorridors.size(); i++)
    {
        if (activeCorridors[i] == nullptr)
        {
            activeCorridors[i] = corridor;
            break;
        }
    }

    // no empty slot found, add a new one
    if (i == activeCorridors.size())
    {
        activeCorridors.push_back(corridor);
    }

    corridor->part1->pos.x = physicalSize;
    corridor->part1->pos.z = physicalSize * i;
    corridor->part2->pos.x = physicalSize * 2;
    corridor->part2->pos.z = physicalSize * i;
}

bool InfiniteSpace::IsValidNode(int nodeIndex)
{
    return nodes.size() > nodeIndex && nodes[nodeIndex].size != 0;
}

void InfiniteSpace::GenerateNode(int type, int nodeIndex, int entranceSide, int entranceNode)
{
    if (nodes.size() <= nodeIndex)
    {
        nodes.resize(nodeIndex + 1);
    }

    auto& node = nodes[nodeIndex];
    if (node.size != 0)
    {
        // node is already generated; nothing to do!
        return;
    }

    node.size = roomSize; // TODO: random size
    node.roomType = type;

    std::vector<int> sides = {0, 1, 2, 3};
    node.connections = {-1, -1, -1, -1};
    if (entranceSide >= 0)
    {
        node.connections[entranceSide] = entranceNode;
        sides.erase(sides.begin() + entranceSide);
    }

    // add connections
    int numConnections = rand() % 3 + 1;
    for (int i = 0; i < numConnections; i++)
    {
        int index = rand() % sides.size();
        int side = sides[index];
        sides.erase(sides.begin() + index);
        node.connections[side] = nextNode++;
    }

    node.hasTarget = roomTypes[type].hasTarget;
}

void InfiniteSpace::RemoveRoom(int index, PObjectVec& objs, PPortalVec& portals)
{
    const auto& room = nodes[index].room;

    // 1. remove door, room and target objects
    objs.erase(
        std::remove_if(
            objs.begin(), objs.end(),
            [&](auto& it) {
                return it == room->doors[0] || it == room->doors[1] || it == room->doors[2] || it == room->doors[3]
                       || it == room || it == room->target || it == room->pillar;
            }),
        objs.end());

    // 2. remove portals
    portals.erase(
        std::remove_if(
            portals.begin(), portals.end(),
            [&](auto& it) {
                return it == room->activePortals[0] || it == room->activePortals[1] || it == room->activePortals[2]
                       || it == room->activePortals[3];
            }),
        portals.end());

    // 3. remove room from node
    nodes[index].room = nullptr;
}

void InfiniteSpace::RemoveCorridor(std::shared_ptr<Corridor>& corridor, PObjectVec& objs, PPortalVec& portals)
{
    // 1. remove corridor object
    objs.erase(
        std::remove_if(
            objs.begin(), objs.end(), [&](auto& it) { return it == corridor->part1 || it == corridor->part2; }),
        objs.end());
    // 2. remove portals
    portals.erase(
        std::remove_if(
            portals.begin(), portals.end(),
            [&](auto& it) {
                return it == corridor->entrancePortal || it == corridor->exitPortal || it == corridor->p1
                       || it == corridor->p2;
            }),
        portals.end());
}

void InfiniteSpace::CloseCorridor(
    int currentRoomIndex, std::shared_ptr<Corridor>& corridor, PObjectVec& objs, PPortalVec& portals)
{
    int roomToDelete;
    Side side;
    if (currentRoomIndex == corridor->exitRoomIndex)
    {
        roomToDelete = corridor->entranceRoomIndex;
        side = corridor->exitSide;
    }
    else if (currentRoomIndex == corridor->entranceRoomIndex)
    {
        roomToDelete = corridor->exitRoomIndex;
        side = corridor->entranceSide;
    }

    auto currentRoom = nodes[currentRoomIndex].room;
    assert(currentRoom != nullptr);

    // we entered the new room
    // 1. remove the last room
    RemoveRoom(roomToDelete, objs, portals);
    // 2. remove the corridor
    RemoveCorridor(corridor, objs, portals);
    // 3. remove the exit portal in the current room
    portals.erase(std::remove(portals.begin(), portals.end(), currentRoom->activePortals[(int) side]), portals.end());
    // 4. place a door
    if (currentRoom->doors[(int) side] == nullptr)
    {
        auto door = std::make_shared<Door>(roomToDelete, (int) side, ROOM_COLORS[roomToDelete % ROOM_COLORS.size()]);
        currentRoom->PlaceDoor(door, side);
        objs.push_back(door);
    }
    else
    {
        // with the KEEP_ONE strategy, the door already exists but has no target room set
        currentRoom->doors[(int) side]->targetRoom = roomToDelete;
    }
    // 5. delete the corridor
    corridor = nullptr;
}
