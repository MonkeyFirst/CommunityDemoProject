#pragma once

#include <Urho3D/Urho3DAll.h>

class Follow : public LogicComponent 
{
    URHO3D_OBJECT(Follow, LogicComponent);

public:
    Follow(Context* context);
    ~Follow();

    void Update(float dt); // LogicComponent

    unsigned int toFollow;
    float speed;
    Vector3 offset;
};