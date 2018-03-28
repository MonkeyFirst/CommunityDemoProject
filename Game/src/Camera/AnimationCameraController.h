#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Camera/CameraController.h"
#include "Motions/Motions.h"

#include "radio.h" // for LAYER_MOVE

class AnimationCameraController : public CameraController
{
    URHO3D_OBJECT(AnimationCameraController, CameraController);

public:
    AnimationCameraController(Context* context, Node* n, const String& name);
    ~AnimationCameraController();

    void Enter();
    void Update(float dt);
    void OnCameraEvent(VariantMap& eventData);
    void DebugDraw(DebugRenderer* debug);
    
    Vector3 GetTarget();
    Node* CreateAnimationNode();
    void PlayCamAnimation(const String& animName);

    // PlayAnimation in 534 revision not exist but script call this function.
    void PlayAnimation(AnimationController* ctrl, const String& name, unsigned int layer = LAYER_MOVE, bool loop = false, float blendTime = 0.1f, float startTime = 0.0f, float speed = 1.0f);

    unsigned int nodeId;
    int playingIndex;
    String animation;
    float cameraSpeed;
};

