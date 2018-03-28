#include <Urho3D/Urho3DAll.h>

#include "Camera/TransitionCameraController.h"
#include "Camera/CameraManager.h"
#include "radio.h"
#include "Character/CharacterStates.h"

TransitionCameraController::TransitionCameraController(Context* context, Node* n, const String& name) : 
    CameraController(context, n, name)
{

}

TransitionCameraController::~TransitionCameraController()
{}

void TransitionCameraController::Update(float dt)
{
    curTime += dt;
    Vector3 curPos = cameraNode->GetWorldPosition();
    Quaternion curRot = cameraNode->GetWorldRotation();
    cameraNode->SetWorldPosition(curPos.Lerp(targetPosition, curTime/dt)); 
    cameraNode->SetWorldRotation(curRot.Slerp(targetRotation, curTime/dt)); 
    if (curTime >= dt)
        RADIO::g_cm->SetCameraController_Hash(targetController);
}

void TransitionCameraController::Enter()
{
    curTime = 0.0f;
}

void TransitionCameraController::OnCameraEvent(VariantMap& eventData)
{
    Log::Write(-1," TransitionCameraController: OnCameraEvent\n");
    if (!eventData.Contains(DURATION))
        return;
    duration = eventData[DURATION].GetFloat();
    targetPosition = eventData[TARGET_POSITION].GetVector3();
    targetRotation = eventData[TARGET_ROTATION].GetQuaternion();
    targetController = eventData[TARGET_CONTROLLER].GetStringHash();
}




