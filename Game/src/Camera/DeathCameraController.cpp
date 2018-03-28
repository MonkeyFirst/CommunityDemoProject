#include <Urho3D/Urho3DAll.h>

#include "Camera/DeathCameraController.h"
#include "Camera/CameraManager.h"

#include "Character/CharacterStates.h"
#include "States/GameManager.h"

#include "radio.h"

DeathCameraController::DeathCameraController(Context* context, Node* n, const String& name) : 
    CameraController(context, n, name)
{
    nodeId = M_MAX_UNSIGNED;
    cameraSpeed = 3.5f;
    cameraDist = 12.5f;
    cameraHeight = 0.5f;
    sideAngle = 0.0;
    timeInState = 0.0f;
}

DeathCameraController::~DeathCameraController()
{

}

void DeathCameraController::Exit()
{
    nodeId = M_MAX_UNSIGNED;
    timeInState = 0;
}

void DeathCameraController::Update(float dt)
{
    timeInState += dt;
    Node* _node = cameraNode->GetScene()->GetNode(nodeId);

    if (!_node || timeInState > 7.5f)
    {
        RADIO::g_cm->SetCameraController("ThirdPerson");
        return;
    }

    Node* playerNode = RADIO::g_gm->GetPlayer()->GetNode();

    Vector3 dir = _node->GetWorldPosition() - playerNode->GetWorldPosition();
    float angle = Atan2(dir.x_, dir.z_) + sideAngle;

    Vector3 v1(Sin(angle) * cameraDist, cameraHeight, Cos(angle) * cameraDist);
    v1 += _node->GetWorldPosition();
    Vector3 v2 = _node->GetWorldPosition() + playerNode->GetWorldPosition();
    v2 /= 2;
    v2.y_ += CHARACTER_HEIGHT;
    UpdateView(v1, v2, dt * cameraSpeed);
}

void DeathCameraController::OnCameraEvent(VariantMap& eventData)
{
    Log::Write(-1," DeathCameraController: OnCameraEvent\n");
    if (!eventData.Contains(NODE))
        return;
    nodeId = eventData[NODE].GetUInt();
    Node* _node = cameraNode->GetScene()->GetNode(nodeId);
    if (!_node)
    {
        RADIO::g_cm->SetCameraController("ThirdPerson");
        return;
    }
    Node* playerNode = RADIO::g_gm->GetPlayer()->GetNode();
    Vector3 dir = _node->GetWorldPosition() - playerNode->GetWorldPosition();
    float angle = Atan2(dir.x_, dir.z_);
    float angle_1 = RADIO::AngleDiff(angle - 90);
    float angle_2 = RADIO::AngleDiff(angle + 90);
    float cur_angle = RADIO::g_cm->GetCameraAngle();
    if (Abs(cur_angle - angle_1) > Abs(cur_angle - angle_2))
        sideAngle = -90.0f;
    else
        sideAngle = +90.0f;

    Log::Write(-1," DeathCamera sideAngle = " + String(sideAngle) + "\n");
}



