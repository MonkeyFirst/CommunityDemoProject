#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Camera/CameraController.h"

class DebugFPSCameraController : public CameraController
{
        URHO3D_OBJECT(DebugFPSCameraController, CameraController);

public:
    DebugFPSCameraController(Context* context, Node* n, const String& name);
    ~DebugFPSCameraController();

    void Update(float dt);
    bool IsDebugCamera();

    float yaw;
    float pitch;
};

