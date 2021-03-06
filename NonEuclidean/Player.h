#pragma once
#include "Physical.h"
#include "Vector.h"

class Player : public Physical
{
public:
    Player();
    virtual ~Player() override {}

    virtual void Reset() override;
    virtual void Update() override;
    virtual void OnCollide(Object& other, const Vector3& push) override;
    virtual bool TryPortal(const Portal& portal) override;

    void Look(float mouseDx, float mouseDy);
    void Move(float moveF, float moveL);

    Matrix4 WorldToCam() const;
    Matrix4 CamToWorld() const;
    Vector3 CamOffset() const;

public:
    Vector3 physicalPos;
    Vector3 virtualOffsets;

private:
    float cam_rx;
    float cam_ry;

    float bob_mag;
    float bob_phi;

    bool onGround;
};
