#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Camera/CameraController.h"

class CameraManager : public Object
{
    URHO3D_OBJECT(CameraManager, Object);

public:
    CameraManager(Context* context);
    ~CameraManager();

    void Initialize();
    void Finalize();

    CameraController* FindCameraController(const StringHash& nameHash);
    void SetCameraController(const String& name);
    void SetCameraController_Hash(StringHash nameHash);
    void Start(Node* n);
    void Stop();
    void Update(float dt);
    Node* GetCameraNode();
    Camera* GetCamera();
    Vector3 GetCameraForwardDirection();
    float GetCameraAngle();
    void DebugDraw(DebugRenderer* debug);
    void OnCameraEvent(VariantMap& eventData);
    void CheckCameraAnimation(const String& anim);
    String GetDebugText();
    
    Vector<CameraController*> cameraControllers; 
    WeakPtr<CameraController> currentController;
    SharedPtr<Node> cameraNode;
    Vector3 cameraTarget;
    Vector<StringHash> cameraAnimations;
};












