#pragma once

#include <Urho3D/Urho3DAll.h>

class PhysicsSensor : public Object
{
    URHO3D_OBJECT(PhysicsSensor, Object);

public:
    PhysicsSensor(Context* context, Node* n);
    ~PhysicsSensor();

    void Update(float dt);
    void DebugDraw(DebugRenderer* debug);
    Vector3 GetGround(const Vector3& pos);
    int DetectWallBlockingFoot(float dist = 1.5f);

    bool grounded;
    Node* sceneNode;
    Node* sensorNode;
    CollisionShape* shape;
    CollisionShape* verticalShape;
    CollisionShape* horinzontalShape;
    Vector3 start;
    Vector3 end;
    float inAirHeight = 0.0f;
    int inAirFrames = 0;
    float halfHeight;
};





