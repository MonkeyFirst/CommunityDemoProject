#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Camera/CameraController.h"

class TransitionCameraController : public CameraController
{
    URHO3D_OBJECT(TransitionCameraController, CameraController);

public:
    TransitionCameraController(Context* context, Node* n, const String& name);
    ~TransitionCameraController();
    
    void Update(float dt);
    void Enter();
    void OnCameraEvent(VariantMap& eventData);

    Vector3 targetPosition;
    Quaternion targetRotation;
    StringHash targetController;
    float duration;
    float curTime;
};

