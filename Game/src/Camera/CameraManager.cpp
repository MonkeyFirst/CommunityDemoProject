#include <Urho3D/Urho3DAll.h>

#include "Camera/CameraManager.h"
#include "Camera/DebugFPSCameraController.h"
#include "Camera/ThirdPersonCameraController.h"
#include "Camera/TransitionCameraController.h"
#include "Camera/DeathCameraController.h"
#include "Camera/AnimationCameraController.h"

#include "Motions/Motions.h"
//#include "Character/Player.h"
#include "Character/CharacterStates.h"

#include "radio.h"

CameraManager::CameraManager(Context* context) :
    Object(context)
{
    Log::Write(-1," CameraManager: Constructor\n");
}

CameraManager::~CameraManager()
{
    Log::Write(-1," CameraManager: Destructor\n");
}

void CameraManager::Initialize()
{
    Log::Write(-1," CameraManager: Initialize\n");
}

void CameraManager::Finalize()
{
    Log::Write(-1," CameraManager: Finalize\n");
}

CameraController* CameraManager::FindCameraController(const StringHash& nameHash)
{
    for (unsigned int i = 0; i < cameraControllers.Size(); ++i)
    {
        if (cameraControllers[i]->nameHash == nameHash)
            return cameraControllers[i];
    }
    Log::Write(-1," ERROR: CameraManager::FindCameraController " + nameHash.ToString());
    return NULL;
}

void CameraManager::SetCameraController(const String& name)
{
    SetCameraController_Hash(StringHash(name));
    Log::Write(-1," CameraManager: Set current " + name + " Controller\n");
}

void CameraManager::SetCameraController_Hash(StringHash nameHash)
{
    CameraController* cc = FindCameraController(nameHash);
    if (currentController == cc)
        return;

    if (currentController)
        currentController->Exit();
    currentController = cc;
    if (currentController)
        currentController->Enter();
}

void CameraManager::Start(Node* n)
{
//    Log::Write(-1," CameraManager::Start 1\n");

    cameraNode = n;
    cameraControllers.Push(new DebugFPSCameraController(context_, n, "Debug"));
    cameraControllers.Push(new ThirdPersonCameraController(context_, n, "ThirdPerson"));
    cameraControllers.Push(new TransitionCameraController(context_, n, "Transition"));
    cameraControllers.Push(new DeathCameraController(context_, n, "Death"));
    cameraControllers.Push(new AnimationCameraController(context_, n, "Animation"));

    Log::Write(-1," CameraManager: Add " + String(cameraControllers.Size()) + " Controllers\n");

    /*
    cameraAnimations.Push(StringHash("Counter_Arm_Back_05"));
    cameraAnimations.Push(StringHash("Counter_Arm_Back_06"));
    cameraAnimations.Push(StringHash("Counter_Arm_Front_07"));
    cameraAnimations.Push(StringHash("Counter_Arm_Front_09"));
    cameraAnimations.Push(StringHash("Counter_Arm_Front_13"));
    cameraAnimations.Push(StringHash("Counter_Arm_Front_14"));
    cameraAnimations.Push(StringHash("Counter_Leg_Back_04"));
    cameraAnimations.Push(StringHash("Counter_Leg_Front_07"));
    cameraAnimations.Push(StringHash("Double_Counter_2ThugsA"));
    cameraAnimations.Push(StringHash("Double_Counter_2ThugsB"));
    cameraAnimations.Push(StringHash("Double_Counter_2ThugsG"));
    cameraAnimations.Push(StringHash("Double_Counter_2ThugsH"));
    cameraAnimations.Push(StringHash("Double_Counter_3ThugsB"));
    */
}

void CameraManager::Stop()
{
    Log::Write(-1," CameraManager: Stop\n");

    if (currentController)
    {
        currentController->Exit();
        currentController = NULL;
    }
    if (cameraNode)
    {
        cameraNode.Reset();
        cameraNode = NULL;
    }
    cameraAnimations.Clear();
    cameraControllers.Clear();
}

void CameraManager::Update(float dt)
{
//    Log::Write(-1," CameraManager: Update 1\n"); 
    if(RADIO::tmpdebugvalue <= 3)
    {
        Log::Write(-1," CameraManager: Update " + String(RADIO::tmpdebugvalue) + " (3)\n");
    }
//    Log::Write(-1," CameraManager: Update 2\n");     
    if (currentController)
        currentController->Update(dt);
//    Log::Write(-1," CameraManager: Update 3\n"); 
}

Node* CameraManager::GetCameraNode()
{
    return cameraNode;
}

Camera* CameraManager::GetCamera()
{
    if (!cameraNode)
        return NULL;
    return cameraNode->GetComponent<Camera>();
}

Vector3 CameraManager::GetCameraForwardDirection()
{
    return cameraNode->GetWorldRotation() * Vector3(0, 0, 1);
}

float CameraManager::GetCameraAngle()
{
    if (currentController)
    {
        if (currentController->IsDebugCamera())
            return 0.0f;
    }

    Vector3 dir = GetCameraForwardDirection();
    return Atan2(dir.x_, dir.z_);
}

void CameraManager::DebugDraw(DebugRenderer* debug)
{
    //debug.AddCross(cameraTarget, 1.0f, RED, false);
    if (currentController)
        currentController->DebugDraw(debug);
}

void CameraManager::OnCameraEvent(VariantMap& eventData)
{
    Log::Write(-1," CameraManager: OnCameraEvent 1\n");
    StringHash name = eventData[NAME].GetStringHash();
    if (name == CHANGE_STATE)
        SetCameraController_Hash(eventData[VALUE].GetStringHash());
    Log::Write(-1," CameraManager: OnCameraEvent 2\n");
    if (currentController)
        currentController->OnCameraEvent(eventData);
}

void CameraManager::CheckCameraAnimation(const String& anim)
{
    unsigned int pos = anim.FindLast('/');
    String name = anim.Substring(pos + 1);
    //URHO3D_LOGINFO("CheckCameraAnimation, name=" + name);
    StringHash nameHash(name);
    int k = -1;
    for (unsigned int i = 0; i < cameraAnimations.Size(); ++i)
    {
        if (nameHash == cameraAnimations[i])
        {
            k = int(i);
            break;
        }
    }

    if (k < 0)
        return;
// return "Animations/" + name + ".ani"; in DoItBro
    //String camAnim = GetAnimationName("BM_Combat_Cameras/" + name);
    String camAnim = "Animations/BM_Combat_Cameras/" + name + ".ani"; // временно вместо глобальной функции

    VariantMap eventData;
    eventData[NAME] = CHANGE_STATE;
    eventData[VALUE] = StringHash("Animation");
    eventData[ANIMATION] = camAnim;

    Log::Write(-1," camAnim = " + camAnim + "\n");

    OnCameraEvent(eventData);
}

String CameraManager::GetDebugText()
{
    return (currentController != NULL) ? currentController->GetDebugText() : "";
}





