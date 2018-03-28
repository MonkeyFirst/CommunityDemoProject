
#include <Urho3D/Urho3DAll.h>

#include "Camera/AnimationCameraController.h"
#include "Camera/CameraManager.h"

#include "Character/CharacterStates.h"
#include "States/GameManager.h"

//#include "radio.h"

AnimationCameraController::AnimationCameraController(Context* context, Node* n, const String& name) : 
    CameraController(context, n, name)
{
    nodeId = M_MAX_UNSIGNED;
    playingIndex = 0;
    cameraSpeed = 5.0f;
}

AnimationCameraController::~AnimationCameraController()
{}

void AnimationCameraController::Enter()
{
    CreateAnimationNode();
}

void AnimationCameraController::Update(float dt)
{
    Player* p = RADIO::g_gm->GetPlayer();
    if (!p)
        return;

    Scene* scn = RADIO::g_gm->GetScene();
    Node* _node = scn->GetNode(nodeId);
    Node* n = _node->GetChild("Camera_ROOT", true);
    Vector3 v = GetTarget();
    UpdateView(n->GetWorldPosition(), v, cameraSpeed * dt);

    AnimationController* ac = _node->GetComponent<AnimationController>();
    if (ac->IsAtEnd(animation))
    {
        // finished.
        // todo.
        RADIO::g_cm->SetCameraController("ThirdPerson");
    }
}

Vector3 AnimationCameraController::GetTarget()
{
    Player* p = RADIO::g_gm->GetPlayer();
    if (!p)
        return Vector3(0, 0, 0);
    Vector3 v = p->GetNode()->GetWorldPosition();
    v.y_ += CHARACTER_HEIGHT;
    return v;
}

Node* AnimationCameraController::CreateAnimationNode()
{
    Scene* scn = RADIO::g_gm->GetScene();

    if (nodeId == M_MAX_UNSIGNED)
    {
        Node* _node = scn->CreateChild("AnimatedCamera");
        nodeId = _node->GetID();

        AnimatedModel* model = _node->CreateComponent<AnimatedModel>();
        AnimationController* ac = _node->CreateComponent<AnimationController>();

        ResourceCache* cache = GetSubsystem<ResourceCache>();

        model->SetModel(cache->GetResource<Model>("Models/Camera_Rig.mdl"));
        model->SetUpdateInvisible(true);
        model->SetViewMask(0);
        return _node;
    }
    else
        return scn->GetNode(nodeId);
}

void AnimationCameraController::PlayCamAnimation(const String& animName)
{
    Scene* scn = RADIO::g_gm->GetScene();

    animation = animName;
    scn->GetNode(nodeId)->SetWorldPosition(GetTarget());

    PlayAnimation(CreateAnimationNode()->GetComponent<AnimationController>(), animName, LAYER_MOVE, false, 0.1f, 0.0f, 1.0f);
}

void AnimationCameraController::PlayAnimation(AnimationController* ctrl, const String& name, unsigned int layer, bool loop, float blendTime, float startTime, float speed)
{
    //Print("PlayAnimation " + name + " loop=" + loop + " blendTime=" + blendTime + " startTime=" + startTime + " speed=" + speed);
    ctrl->StopLayer(layer, blendTime);
    ctrl->PlayExclusive(name, layer, loop, blendTime);
    ctrl->SetTime(name, startTime);
    ctrl->SetSpeed(name, speed);
}

void AnimationCameraController::OnCameraEvent(VariantMap& eventData)
{
    Log::Write(-1," AnimationCameraController: OnCameraEvent\n");
    if (!eventData.Contains(ANIMATION))
        return;
    PlayCamAnimation(eventData[ANIMATION].GetString());
}

void AnimationCameraController::DebugDraw(DebugRenderer* debug)
{
    Scene* scn = RADIO::g_gm->GetScene();

    Node* _node = scn->GetNode(nodeId);
    Node* n = _node->GetChild("Camera_ROOT", true);
    debug->AddNode(_node, 0.5f, false);
    debug->AddNode(n, 0.5f, false);
}




