#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Camera/CameraController.h"

class ThirdPersonCameraController : public CameraController
{
    URHO3D_OBJECT(ThirdPersonCameraController, CameraController);

public:
    ThirdPersonCameraController(Context* context, Node* n, const String& name);
    ~ThirdPersonCameraController();

    void Update(float dt);
    String GetDebugText();
    void Reset();
    void Enter();
    
    float cameraSpeed;
    float cameraDistance;
    float cameraDistSpeed;
    float targetFov;
    float fovSpeed;  
    Vector3 targetOffset;
    bool isScrolling;
};

