#pragma once

#include <Urho3D/Urho3DAll.h>

const StringHash TARGET_POSITION("TargetPosition");
const StringHash TARGET_ROTATION("TargetRotation");
const StringHash TARGET_CONTROLLER("TargetController");
const StringHash TARGET_FOV("TargetFOV");

//const float BASE_FOV = 45.0f;

class CameraController : public Object
{
    URHO3D_OBJECT(CameraController, Object);

public:
    CameraController(Context* context, Node* n, const String& name);
    virtual ~CameraController();

    virtual void Update(float dt);
    virtual void OnCameraEvent(VariantMap& eventData);
    virtual void Enter();
    virtual void Exit();
    virtual bool IsDebugCamera();
    virtual String GetDebugText();
    virtual void Reset();
  
    virtual void DebugDraw(DebugRenderer* debug);
    virtual void UpdateView(const Vector3& position, const Vector3& lookat, float blend);
    
    StringHash nameHash;
    SharedPtr<Node> cameraNode;
    SharedPtr<Camera> camera;
};






