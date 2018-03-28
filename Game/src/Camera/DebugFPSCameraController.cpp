
#include <Urho3D/Urho3DAll.h>

#include "Camera/DebugFPSCameraController.h"

#include "radio.h"

const float MOVE_SPEED = 20.0f;
const float MOUSE_SENSITIVITY = 0.1f;

DebugFPSCameraController::DebugFPSCameraController(Context* context, Node* n, const String& name) : 
    CameraController(context, n, name)
{
    yaw = n->GetWorldRotation().EulerAngles().y_;
    pitch = n->GetWorldRotation().EulerAngles().x_;
}

DebugFPSCameraController::~DebugFPSCameraController()
{}

void DebugFPSCameraController::Update(float dt)
{
    if(RADIO::tmpdebugvalue <= 3)
    {
        Log::Write(-1," DebugFPSCameraController: Update " + String(RADIO::tmpdebugvalue) + " (3)\n");
    }

    Input* input = GetSubsystem<Input>();
    UI* ui = GetSubsystem<UI>();

    if (ui->GetFocusElement())
        return;

    float speed = MOVE_SPEED;
    if (input->GetKeyDown(KEY_LSHIFT))
        speed *= 2;

    IntVector2 mouseMove = input->GetMouseMove();
    yaw += MOUSE_SENSITIVITY * mouseMove.x_;
    pitch += MOUSE_SENSITIVITY * mouseMove.y_;
    pitch = Clamp(pitch, -90.0f, 90.0f);

    cameraNode->SetRotation(Quaternion(pitch, yaw, 0.0f));

    if (input->GetKeyDown(KEY_UP)) // KEY_UP KEY_W
        cameraNode->Translate(Vector3(0.0f, 0.0f, 1.0f) * speed * dt);
    if (input->GetKeyDown(KEY_DOWN)) // KEY_DOWN KEY_S
        cameraNode->Translate(Vector3(0.0f, 0.0f, -1.0f) * speed * dt);
    if (input->GetKeyDown(KEY_LEFT)) // KEY_LEFT KEY_A
        cameraNode->Translate(Vector3(-1.0f, 0.0f, 0.0f) * speed * dt);
    if (input->GetKeyDown(KEY_RIGHT)) // KEY_RIGHT KEY_D
        cameraNode->Translate(Vector3(1.0f, 0.0f, 0.0f) * speed * dt);
}

bool DebugFPSCameraController::IsDebugCamera()
{
    return true;
}


