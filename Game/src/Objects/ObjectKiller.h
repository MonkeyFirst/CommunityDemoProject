#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Objects/GameObject.h"

class ObjectKiller : public GameObject
{
    URHO3D_OBJECT(ObjectKiller, GameObject);

public:
    ObjectKiller(Context* context);
    ~ObjectKiller();


};