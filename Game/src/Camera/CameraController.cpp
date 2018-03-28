
#include <Urho3D/Urho3DAll.h>

#include "Camera/CameraController.h"
#include "Camera/CameraManager.h"

#include "radio.h"

CameraController::CameraController(Context* context, Node* n, const String& name) :
    Object(context)
{
    cameraNode = n;
    camera = cameraNode->GetComponent<Camera>(); // ("Camera");
    nameHash = StringHash(name);
}

CameraController::~CameraController()
{}

void CameraController::Update(float dt)
{}

void CameraController::OnCameraEvent(VariantMap& eventData)
{
    Log::Write(-1," CameraController: OnCameraEvent EMPTY\n");
}

void CameraController::Enter()
{
//    Log::Write(-1," CameraController: Enter\n");
}

void CameraController::Exit()
{
//    Log::Write(-1," CameraController: Exit\n");
}

bool CameraController::IsDebugCamera()
{
    return false;
}

void CameraController::UpdateView(const Vector3& position, const Vector3& lookat, float blend)
{
    CameraManager* cm = RADIO::g_cm;

    Vector3 cameraPos = cameraNode->GetWorldPosition(); // worldPosition;
    Vector3 diff = position - cameraPos;
    cameraNode->SetWorldPosition(cameraPos + diff * blend);
    Vector3 target = cm->cameraTarget;
    diff = lookat - target;
    target += diff * blend;
    cameraNode->LookAt(target);
    cm->cameraTarget = target;
}

String CameraController::GetDebugText()
{
    return "camera fov=" + String(camera->GetFov()) + " position=" + cameraNode->GetWorldPosition().ToString() + "\n";
}

void CameraController::Reset()
{}

void CameraController::DebugDraw(DebugRenderer* debug)
{
    Graphics* graphics = GetSubsystem<Graphics>();
    
    float w = float(graphics->GetWidth()); // .width);
    float h = float(graphics->GetHeight()); // .height);
    float w1 = w;
    float h1 = h;
    float gap = 1.0f;
    w -= gap * 2;
    h -= gap * 2;
    // draw horizontal lines
    float depth = 25.0f;
    Color c(0, 1, 0);
    float y = gap;
    float step = h/3;
    for (int i = 0; i < 4; ++i)
    {
        debug->AddLine(camera->ScreenToWorldPoint(Vector3(gap/w1, y/h1, depth)), camera->ScreenToWorldPoint(Vector3((w + gap)/w1, y/h1, depth)), c, false);
        y += step;
    }
    // draw vertical lines
    float x = gap;
    step = w/3;
    for (int i = 0; i < 4; ++i)
    {
        debug->AddLine(camera->ScreenToWorldPoint(Vector3(x/w1, gap/h1, depth)), camera->ScreenToWorldPoint(Vector3(x/w1, (h + gap)/h1, depth)), c, false);
        x += step;
    }
}





