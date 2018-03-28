#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Camera/CameraController.h"


class DeathCameraController : public CameraController
{
    URHO3D_OBJECT(DeathCameraController, CameraController);

public:
    DeathCameraController(Context* context, Node* n, const String& name);
    ~DeathCameraController();

    void Exit();
    void Update(float dt);
    void OnCameraEvent(VariantMap& eventData);

    unsigned int nodeId;
    float cameraSpeed;
    float cameraDist;
    float cameraHeight;
    float sideAngle;
    float timeInState;
};


