#include <Urho3D/Urho3DAll.h>

#include "Camera/ThirdPersonCameraController.h"
#include "States/GameManager.h"
#include "Objects/InputManager.h"

#include "radio.h"

ThirdPersonCameraController::ThirdPersonCameraController(Context* context, Node* n, const String& name) : 
    CameraController(context, n, name)
{
    cameraSpeed = 4.5f;
    cameraDistance = 14.0f;
    cameraDistSpeed = 100.0f;
    targetFov = RADIO::BASE_FOV;
    fovSpeed = 1.5f;
    targetOffset = Vector3(2.0f, 3.5f, 0);
    isScrolling = false;
}

ThirdPersonCameraController::~ThirdPersonCameraController()
{}

void ThirdPersonCameraController::Update(float dt)
{
//    Log::Write(-1," ThirdPersonCameraController: Update\n");
/*
    if(RADIO::tmpdebugvalue <= 3)
    {
        Log::Write(-1," ThirdPersonCameraController: Update " + String(RADIO::tmpdebugvalue) + " (3)\n");
    }
*/
    Player* p = RADIO::g_gm->GetPlayer();
    if (!p)
        return;
    Node* _node = p->GetNode();

    bool blockView = false;
    Vector3 target_pos = _node->GetWorldPosition();
    if (p->target)
    {
        if (!p->target->IsVisible())
        {
            //target_pos += p.target.GetNode().worldPosition;
            //target_pos /= 2.0f;
            //blockView = true;
        }
    }

//    targetCameraDistance = p->HasFlag(FLAGS_RUN) ? 15 : 7.5;
//    cameraDistance += (targetCameraDistance - cameraDistance) * dt * cameraDistSpeed;

    Vector3 offset = cameraNode->GetWorldRotation() * targetOffset;
    target_pos += offset;

    Vector3 v = RADIO::g_im->GetRightAxis();
    float pitch = v.y_;
    float yaw = v.x_;
    pitch = Clamp(pitch, -10.0f, 60.0f);

    float dist = cameraDistance;
    Quaternion q(pitch, yaw, 0);
    Vector3 pos = q * Vector3(0, 0, -dist) + target_pos;
    UpdateView(pos, target_pos, dt * cameraSpeed);

    int mmw = GetSubsystem<Input>()->GetMouseMoveWheel();
    if (mmw != 0)
    {
        unsigned int t = Time::GetSystemTime();
        unsigned int t_diff = t - RADIO::g_im->lastMiddlePressedTime;
        // Print("lastMiddlePressedTime diff = " + t_diff);
        if (t_diff > 500)
            cameraDistance +=  float(mmw) * dt * -cameraDistSpeed;
    }
    cameraDistance = Clamp(cameraDistance, 9.0f, 50.0f);

    float fv = camera->GetFov();
    float diff = targetFov - fv;
    camera->SetFov(fv + (diff * dt * fovSpeed));//.fov += diff * dt * fovSpeed;
    
}

String ThirdPersonCameraController::GetDebugText()
{
    return "camera fov=" + String(camera->GetFov()) + " position=" + cameraNode->GetWorldPosition().ToString()  + " distance=" + String(cameraDistance) + " targetOffset=" + targetOffset.ToString() + " targetFov=" + String(targetFov) + "\n";
}

void ThirdPersonCameraController::Reset()
{
    InputManager* gInput = RADIO::g_im;

    Player* p = RADIO::g_gm->GetPlayer();
    Quaternion q = cameraNode->GetWorldRotation();
    Vector3 offset = q * targetOffset;
    Vector3 target_pos = p->GetNode()->GetWorldPosition() + offset;
    Vector3 dir = target_pos - cameraNode->GetWorldPosition();
    //cameraNode->LookAt(target_pos);
    cameraDistance = dir.Length();

    float h = Abs(dir.y_);
    gInput->m_rightStickY = Asin(h/cameraDistance);
    gInput->m_rightStickX = Asin(dir.x_/cameraDistance);
    gInput->m_rightStickMagnitude = gInput->m_rightStickX * gInput->m_rightStickX + gInput->m_rightStickY * gInput->m_rightStickY;
}

void ThirdPersonCameraController::Enter()
{
    Log::Write(-1," ThirdPersonCameraController: Enter\n");
    targetFov = RADIO::BASE_FOV;
    //Reset();
}



