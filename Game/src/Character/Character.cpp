#include <Urho3D/Urho3DAll.h>

#include "Character/Character.h"
#include "Character/CharacterStates.h"
#include "Character/Ragdoll.h"

#include "Motions/MotionManager.h"
#include "States/GameManager.h"
#include "Camera/CameraManager.h"

#include "Objects/ObjectKiller.h"

#include "radio.h"

// CHARACTER

Character::Character(Context* context):
    GameObject(context)
{
    stateMachine = new FSM(context);
    health = INITIAL_HEALTH;
    attackRadius = 0.15f;
    attackDamage = 10;
    motion_translateEnabled = true;
    motion_rotateEnabled = true;
    
//    Log::Write(-1," Character Constructor\n");
}

Character::~Character()
{
//    Log::Write(-1," Character destructor\n");
    if(!finalized)
        Stop();
}


void Character::ObjectStart()
{
    Log::Write(-1," Character: ObjectStart\n");

    sceneNode = Component::GetNode(); 
    renderNode = sceneNode->GetChild("RenderNode", false); 
    animCtrl = renderNode->GetComponent<AnimationController>(); 
    animModel = renderNode->GetComponent<AnimatedModel>();

    Log::Write(-1," sceneNode NAME: " + sceneNode->GetName() + "\n");
    Log::Write(-1," renderNode NAME: " + renderNode->GetName() + "\n");
    Log::Write(-1," sceneNode POS: " + sceneNode->GetWorldPosition().ToString() + "\n");
    Log::Write(-1," renderNode POS: " + renderNode->GetWorldPosition().ToString() + "\n");
    Log::Write(-1," BPos POS: " + renderNode->GetChild("BPos")->GetWorldPosition().ToString() + "\n");

    startPosition = sceneNode->GetWorldPosition();
    startRotation = sceneNode->GetWorldRotation();
    sceneNode->SetVar(TIME_SCALE, 1.0f);

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    String name = sceneNode->GetName() + "_Ragdoll_Pose";

    ragdollPoseAnim = cache->GetResource<Animation>(name);
    if (!ragdollPoseAnim)
    {
        Log::Write(-1," Create animation for " + name  + "\n");
        ragdollPoseAnim = new Animation(context_);
        ragdollPoseAnim->SetName(name);
        ragdollPoseAnim->SetAnimationName(name); 
        cache->AddManualResource(ragdollPoseAnim);
    }

    if (RADIO::collision_type == 1)
    {
        body = sceneNode->CreateComponent<RigidBody>();
        body->SetCollisionLayer(COLLISION_LAYER_CHARACTER);
        body->SetCollisionMask(COLLISION_LAYER_LANDSCAPE | COLLISION_LAYER_PROP);
        body->SetMass(1.0f);
        body->SetAngularFactor(Vector3(0.0f, 0.0f, 0.0f));
        body->SetCollisionEventMode(COLLISION_ALWAYS);
        CollisionShape* shape = sceneNode->CreateComponent<CollisionShape>();
        shape->SetCapsule(COLLISION_RADIUS * 2, CHARACTER_HEIGHT, Vector3(0.0f, CHARACTER_HEIGHT / 2, 0.0f));
        physicsType = 1;
        SetGravity(Vector3(0, -20, 0));

        Log::Write(-1," Character: Created Physics Capsule for " + sceneNode->GetName()  + " node\n");
    }

    SetHealth(INITIAL_HEALTH);
    SubscribeToEvent(renderNode, E_ANIMATIONTRIGGER, URHO3D_HANDLER(Character, HandleAnimationTrigger));

}

// Start() Called when the component is added to a scene node. Other components may not yet exist.
void Character::Start()
{
    //URHO3D_LOGINFO("============================== begin Object Start ==============================");
//    unsigned int startTime = GetSubsystem<Time>()->GetSystemTime(); // time.systemTime;
    ObjectStart();
    //URHO3D_LOGINFO(sceneNode.name + " ObjectStart time-cost=" + String(time.systemTime - startTime) + " ms");
    //URHO3D_LOGINFO("============================== end Object Start ==============================");
}

// Called when the component is detached from a scene node, usually on destruction. 
// Note that you will no longer have access to the node and scene at that point.
void Character::Stop()
{
    //URHO3D_LOGINFO("Character::Stop " + sceneNode.name);
    //Log::Write(-1," Character: Stop\n");
    if(stateMachine)
    {
        // ReleaseRef Decrement reference count and delete self if no more references. 
        // Can also be called outside of a SharedPtr for traditional reference counting.
        stateMachine->ReleaseRef(); // Urho3D объект
        stateMachine = NULL;
    }

    if(sensor)
        sensor.Reset();
    
    if(body)
        body.Reset();
    
    if(ragdollPoseAnim)
        ragdollPoseAnim.Reset();
    
    if(animCtrl)
        animCtrl.Reset();

    if(animModel)
        animModel.Reset();

    if(target)
        target.Reset();

    if(dockLine)
    {
        delete dockLine;
        dockLine = NULL;
    }
    
    renderNode = NULL;
    
    if(sceneNode)
        sceneNode.Reset();

    finalized = true;
}

void Character::Kill() // Remove()
{
    Log::Write(-1," Character: " + GetName() + " Kill\n");
    Stop();
    GameObject::Kill();
}

void Character::SetTimeScale(float scale)
{
    if (timeScale == scale)
        return;
    GameObject::SetTimeScale(scale);
    unsigned int num = animModel->GetNumAnimationStates();
    for (unsigned int i = 0; i < num; ++i)
    {
        AnimationState* state = animModel->GetAnimationState(i);
        //if (d_log)
            //URHO3D_LOGINFO("SetSpeed " + state.animation.name + " scale " + scale);
        animCtrl->SetSpeed(state->GetAnimation()->GetAnimationName(), scale);
    }
    if (body)
        body->SetLinearVelocity(body->GetLinearVelocity() * scale);

    sceneNode->SetVar(TIME_SCALE, scale);
}
// unsigned int layer = LAYER_MOVE - in Motions.h
void Character::PlayAnimation(const String& animName, unsigned int layer, bool loop, float blendTime, float startTime, float speed)
{
//    if (d_log)
//        URHO3D_LOGINFO(GetName() + " PlayAnimation " + animName + " loop=" + loop + " blendTime=" + blendTime + " startTime=" + startTime + " speed=" + speed);

    if (layer == LAYER_MOVE && lastAnimation == animName && loop)
        return;

    lastAnimation = animName;
    AnimationController* ctrl = animCtrl;
    ctrl->StopLayer(layer, blendTime);
    ctrl->PlayExclusive(animName, layer, loop, blendTime);
    ctrl->SetSpeed(animName, speed * timeScale);
    ctrl->SetTime(animName, (speed < 0) ? ctrl->GetLength(animName) : startTime);
}

String Character::GetDebugText()
{
    String debugText = stateMachine->GetDebugText();
    debugText += "NODE NAME: " + sceneNode->GetName() + "\n";
    debugText += "POS X: " + String(sceneNode->GetWorldPosition().x_) + "\n";//.ToString()
    debugText += "POS Y: " + String(sceneNode->GetWorldPosition().y_) + "\n";//.ToString()
    debugText += "POS Z: " + String(sceneNode->GetWorldPosition().z_) + "\n";//.ToString()
//        debugText += "TimeScale: " + timeScale + " Health: " + health + "\n";
    if (animModel->GetNumAnimationStates() > 0)
    {
//            debugText += "Debug-Animations:\n";
        for (unsigned int i = 0; i < animModel->GetNumAnimationStates(); ++i)
        {
            AnimationState* state = animModel->GetAnimationState(i);
            if (state->GetWeight() > 0.0f && state->IsEnabled())
                debugText += "ANI: " + state->GetAnimation()->GetAnimationName() + " time=" + String(state->GetTime()) + " weight=" + String(state->GetWeight()) + "\n";
        }
    }
    return debugText;
}

void Character::SetVelocity(const Vector3& vel)
{
    // URHO3D_LOGINFO("body.linearVelocity = " + vel.ToString());
    //Log::Write(-1," Character: SetVelocity" + sceneNode->GetName() + "\n");

    if (body)
        body->SetLinearVelocity(vel); // .linearVelocity = vel;
//    else
//        Log::Write(-1," Character: SetVelocity NO_BODY " + sceneNode->GetName() + "\n");
}

Vector3 Character::GetVelocity()
{
    return (body != NULL) ? body->GetLinearVelocity() : Vector3(0, 0, 0);
}

void Character::MoveTo(const Vector3& position, float dt)
{
    //Log::Write(-1," Character: MoveTo " + String(position) + "\n");
    sceneNode->SetWorldPosition(RADIO::FilterPosition(position)); 
}

bool Character::Attack()
{
    return false;
}

bool Character::Counter()
{
    return false;
}

bool Character::Evade()
{
    return false;
}

bool Character::Redirect()
{
    ChangeState("RedirectState");
    return false;
}

bool Character::Distract()
{
    return false;
}

void Character::CommonStateFinishedOnGroud()
{
    ChangeState("StandState");
}

void Character::ResetObject() // Reset()
{
    flags = FLAGS_ATTACK;
    sceneNode->SetWorldPosition(startPosition);
    sceneNode->SetWorldRotation(startRotation);
    SetHealth(INITIAL_HEALTH);
    SetTimeScale(1.0f);
    ChangeState("StandState");
}

void Character::SetHealth(int h)
{
    health = h;
}

bool Character::CanBeAttacked()
{
    if (HasFlag(FLAGS_INVINCIBLE))
        return false;
    return HasFlag(FLAGS_ATTACK);
}

bool Character::CanBeCountered()
{
    return HasFlag(FLAGS_COUNTER);
}

bool Character::CanBeRedirected()
{
    return HasFlag(FLAGS_REDIRECTED);
}

bool Character::CanAttack()
{
    return false;
}

void Character::DebugDraw(DebugRenderer* debug)
{
    stateMachine->DebugDraw(debug);
    debug->AddNode(sceneNode, 0.5f, false);
}

void Character::TestAnimation(const Vector<String>& animations)
{
    AnimationTestState* state = static_cast<AnimationTestState*>(stateMachine->FindState("AnimationTestState"));
    if (!state)
        return;
    state->Process(animations);
    ChangeState("AnimationTestState");
}

void Character::TestAnimation(const String& animation)
{
    Vector<String> animations = { animation };
    TestAnimation(animations);
}

float Character::GetTargetAngle()
{
    return (target != NULL) ? GetTargetAngle(target->GetNode()) : 0.0f;
}

float Character::GetTargetDistance()
{
    return (target != NULL) ? GetTargetDistance(target->GetNode()) : 0.0f;
}

float Character::ComputeAngleDiff()
{
    return RADIO::AngleDiff(GetTargetAngle() - GetCharacterAngle());
}

int Character::RadialSelectAnimation(int numDirections)
{
    return RADIO::DirectionMapToIndex(ComputeAngleDiff(), numDirections);
}

float Character::GetTargetAngle(Node* _node)
{
    Vector3 targetPos = _node->GetWorldPosition();
    Vector3 myPos = sceneNode->GetWorldPosition();
    Vector3 diff = targetPos - myPos;
    return Atan2(diff.x_, diff.z_);
}

float Character::GetTargetDistance(Node* _node)
{
    Vector3 targetPos = _node->GetWorldPosition();
    Vector3 myPos = sceneNode->GetWorldPosition();
    Vector3 diff = targetPos - myPos;
    return diff.Length();
}

float Character::ComputeAngleDiff(Node* _node)
{
    return RADIO::AngleDiff(GetTargetAngle(_node) - GetCharacterAngle());
}

int Character::RadialSelectAnimation(Node* _node, int numDirections)
{
    return RADIO::DirectionMapToIndex(ComputeAngleDiff(_node), numDirections);
}

float Character::GetCharacterAngle()
{
    Vector3 characterDir = sceneNode->GetWorldRotation() * Vector3(0, 0, 1);
    return Atan2(characterDir.x_, characterDir.z_);
}

String Character::GetName()
{
    return sceneNode->GetName();
}

float Character::GetFootFrontDiff()
{
    Vector3 fwd_dir = renderNode->GetWorldRotation() * Vector3(0, 0, 1);
    Vector3 pt_lf = renderNode->GetChild("Bip01_L_Foot")->GetWorldPosition() - renderNode->GetWorldPosition();
    Vector3 pt_rf = renderNode->GetChild("Bip01_R_Foot")->GetWorldPosition() - renderNode->GetWorldPosition();
    float dot_lf = pt_lf.DotProduct(fwd_dir);
    float dot_rf = pt_rf.DotProduct(fwd_dir);
//    Log::Write(-1," " + sceneNode->GetName() + " dot_lf=" + String(dot_lf) + " dot_rf=" + String(dot_rf) + " diff=" + String(dot_lf - dot_rf) + "\n");
    return dot_lf - dot_rf;
}

void Character::PlayCurrentPose()
{
    RADIO::FillAnimationWithCurrentPose(ragdollPoseAnim, renderNode); // g_mm->
    AnimationState* state = animModel->AddAnimationState(ragdollPoseAnim);
    state->SetWeight(1.0f);
    animCtrl->PlayExclusive(ragdollPoseAnim->GetName(), LAYER_MOVE, false, 0.0f);
}

bool Character::OnDamage(GameObject* attacker, const Vector3& position, const Vector3& direction, int damage, bool weak)
{
    Log::Write(-1," Character: " + GetName() +  " OnDamage !!!\n");
    ChangeState("HitState");
    return true;
}
/*
Node* Character::GetNode()
{
    return sceneNode;
}
*/
void Character::OnDead()
{
    Log::Write(-1," Character: " + GetName() +  " OnDead !!!\n");
    ChangeState("DeadState");
}

void Character::MakeMeRagdoll(const Vector3& velocity, const Vector3& position)
{
    //URHO3D_LOGINFO("MakeMeRagdoll -- velocity=" + velocity.ToString() + " position=" + position.ToString());
    Log::Write(-1," Character: " + GetName() +  " MakeMeRagdoll\n");

    using namespace AnimationTrigger;

    VariantMap anim_data;
    anim_data[NAME] = RAGDOLL_START; // StringHash RAGDOLL_START("Ragdoll_Start") in Ragdoll.h
    anim_data[VELOCITY] = velocity; // StringHash VELOCITY("Velocity") in Ragdoll.h
    anim_data[POSITION] = position; // StringHash POSITION("Position") in Ragdoll.h

    VariantMap data;
    data[P_DATA] = anim_data; // User-defined data type

    renderNode->SendEvent(E_ANIMATIONTRIGGER, data);
}

void Character::OnAttackSuccess(Character* object)
{

}

void Character::OnCounterSuccess()
{

}

void Character::RequestDoNotMove()
{
    AddFlag(FLAGS_NO_MOVE);
}

Node* Character::SpawnParticleEffect(const Vector3& position, const String& effectName, float duration, float scale)
{
    Log::Write(-1," Character: " + GetName() + " SpawnParticleEffect " + effectName + " duration = " + String(duration) + "\n");

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Node* newNode = sceneNode->GetScene()->CreateChild("ParticleEffect");
    newNode->SetPosition(position);
    newNode->SetScale(Vector3(scale, scale, scale));

    // Create the particle emitter
    ParticleEmitter* emitter = newNode->CreateComponent<ParticleEmitter>();
    emitter->SetEffect(cache->GetResource<ParticleEffect>(effectName));

    // Create a GameObject for managing the effect lifetime. 
    // This is always local, so for server-controlled effects it
    // exists only on the server
    
    //GameObject* object = cast<GameObject>(newNode.CreateScriptObject(scriptFile, "GameObject", LOCAL));
    //GameObject* object = static_cast<GameObject*>(newNode->CreateComponent<ObjectKiller>(LOCAL));
    ObjectKiller* object = newNode->CreateComponent<ObjectKiller>(LOCAL);
    object->duration = duration;

    // URHO3D_LOGINFO(GetName() + " SpawnParticleEffect pos=" + position.ToString() + " effectName=" + effectName + " duration=" + duration);

//    return newNode;
}

Node* Character::SpawnSound(const Vector3& position, const String& soundName, float duration)
{
    Log::Write(-1," Character: " + GetName() + " SpawnSound " + soundName + " duration = " + String(duration) + "\n");

    Node* newNode = sceneNode->GetScene()->CreateChild(); // ("SpawnSound")
    newNode->SetPosition(position);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    
    // Create the sound source
    SoundSource3D* source = newNode->CreateComponent<SoundSource3D>();
    Sound* sound = cache->GetResource<Sound>(soundName);
    source->SetDistanceAttenuation(200, 5000, 1);
    source->Play(sound);

    // Create a GameObject for managing the sound lifetime

    //GameObject* object = cast<GameObject>(newNode.CreateScriptObject(scriptFile, "GameObject", LOCAL));
    //GameObject* object = static_cast<GameObject*>(newNode->CreateComponent<ObjectKiller>(LOCAL));
    ObjectKiller* object = newNode->CreateComponent<ObjectKiller>(LOCAL);

    object->duration = duration;

    return newNode;
}

void Character::SetComponentEnabled(const String& boneName, const String& componentName, bool bEnable)
{
    Node* _node = sceneNode->GetChild(boneName, true);
    if (!_node)
        return;
    Component* comp = _node->GetComponent(StringHash(componentName)); // INFO: StringHash добавил.
    if (!comp)
        return;
    comp->SetEnabled(bEnable);
}

void Character::SetNodeEnabled(const String& nodeName, bool bEnable)
{
    Node* n = sceneNode->GetChild(nodeName, true);
    if (n)
        n->SetEnabled(bEnable);
}

State* Character::GetState()
{
    return stateMachine->currentState;
}

bool Character::IsInState(const String& name)
{
    return IsInState_Hash(StringHash(name));
}

bool Character::IsInState_Hash(const StringHash& nameHash)
{
    State* state = stateMachine->currentState;
    if (!state)
        return false;

    if(state->nameHash == nameHash)
        return true;
    else
        return false;
}

bool Character::ChangeState(const String& name)
{
    String oldStateName = "NULL";
    if (stateMachine->currentState)
    {
        oldStateName = stateMachine->currentState->name;
    }
    if (RADIO::d_log)
    {
        Log::Write(-1,"\n xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
        Log::Write(-1," " + GetName() + " Character: ChangeState from " + oldStateName + " to " + name + "\n");
        Log::Write(-1," xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    }
    
    bool ret = stateMachine->ChangeState(name);
    
    State* s = GetState();
    if (!s)
    {
        Log::Write(-1," ERROR: Character: "+ GetName() +" stateMachine->ChangeState(" + name + ")\n");
        return false;
    }
    sceneNode->SetVar(STATE, s->nameHash);
    return ret;
}

bool Character::ChangeState_Hash(const StringHash& nameHash)
{
    String oldStateName = (stateMachine->currentState != NULL) ? stateMachine->currentState->name : "NULL";
    bool ret = stateMachine->ChangeState_Hash(nameHash);
    String newStateName = (stateMachine->currentState != NULL) ? stateMachine->currentState->name : "NULL";

    if (RADIO::d_log)
    {
        Log::Write(-1,"\n xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
        Log::Write(-1," Character: " + GetName() + " ChangeState_Hash from " + oldStateName + " to " + newStateName + "\n");
        Log::Write(-1," xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    }

    State* s = GetState();
    if (!s)
    {
        Log::Write(-1," ERROR: Character: "+ GetName() +" stateMachine->ChangeState_Hash(" + newStateName + ")\n");
        return false;
    }

    sceneNode->SetVar(STATE, s->nameHash);
    return ret;
}

void Character::ChangeStateQueue(const StringHash& nameHash)
{
    stateMachine->ChangeStateQueue(nameHash);
}

State* Character::FindState(const String& name)
{
    return stateMachine->FindState(name);
}

State* Character::FindState_Hash(const StringHash& nameHash)
{
    return stateMachine->FindState_Hash(nameHash);
}

void Character::FixedUpdate(float timeStep) // запускается Component классом
{
    timeStep *= timeScale;

    if (stateMachine)
        stateMachine->FixedUpdate(timeStep);

    CheckDuration(timeStep);
}

void Character::Update(float timeStep) // запускается Component классом
{
    timeStep *= timeScale;

    if (stateMachine)
        stateMachine->Update(timeStep);
}

bool Character::IsTargetSightBlocked()
{
    return false;
}

void Character::CheckCollision()
{

}

void Character::SetTarget(Character* t)
{
    if (t == target)
        return;
    target = t;
//    Log::Write(-1," Character: "+GetName()+" SetTarget="+ ((t) ? t->GetName() : "NULL")+"\n");
}

void Character::SetPhysics(bool b)
{
    if (body)
        body->SetEnabled(b);
    SetNodeEnabled("Collision", b);
}

void Character::PlayRandomSound(int type)
{
    if (type == 0)
        PlaySound("Sfx/impact_" + String(Random(RADIO::num_of_sounds) + 1) + ".ogg");
    else if (type == 1)
        PlaySound("Sfx/big_" + String(Random(RADIO::num_of_big_sounds) + 1) + ".ogg");
}

bool Character::ActionCheck(bool bAttack, bool bDistract, bool bCounter, bool bEvade)
{
    return false;
}

bool Character::IsVisible()
{
    return animModel->IsInView(RADIO::g_cm->GetCamera());
}

void Character::CheckAvoidance(float dt)
{

}

void Character::ClearAvoidance()
{

}

void Character::CheckTargetDistance(Character* t, float dist)
{
    if (!t)
        return;
    if (motion_translateEnabled && GetTargetDistance(t->GetNode()) < dist)
    {
        //URHO3D_LOGINFO(GetName() + " is too close to " + t.GetName() + " set translateEnabled to false");
        motion_translateEnabled = false;
    }
}

bool Character::IsInAir()
{
    Vector3 lf_pos = renderNode->GetChild(L_FOOT, true)->GetWorldPosition();
    Vector3 rf_pos = renderNode->GetChild(R_FOOT, true)->GetWorldPosition();
    Vector3 myPos = sceneNode->GetWorldPosition();
    float lf_to_ground = (lf_pos.y_ - myPos.y_);
    float rf_to_graound = (rf_pos.y_ - myPos.y_);

    if(lf_to_ground > 1.0f && rf_to_graound > 1.0f)
        return true;
    else
        return false;
//    return lf_to_ground > 1.0f && rf_to_graound > 1.0f;
}

void Character::SetHeight(float height)
{
    CollisionShape* shape = sceneNode->GetComponent<CollisionShape>();
    if (shape)
    {
        shape->SetSize(Vector3(COLLISION_RADIUS * 2, height, 0));
        shape->SetTransform(Vector3(0.0f, height/2, 0.0f), Quaternion());
    }
}

bool Character::CheckFalling()
{
    return false;
}

bool Character::CheckDocking(float dist)
{
    return false;
}

void Character::AssignDockLine(Line* l)
{
//    SharedPtr<Line> sharedL(l);
//    dockLine = sharedL;
    dockLine = l;
}

void Character::SetPhysicsType(int type)
{
    if (physicsType == type)
        return;
    physicsType = type;
    if (body)
    {
        if(physicsType == 1)
            body->SetEnabled(true); // .enabled = (physicsType == 1);
        else
            body->SetEnabled(false);
            
        body->SetPosition(sceneNode->GetWorldPosition());
    }
}

void Character::SetGravity(const Vector3& gravity)
{
    if (body)
        body->SetGravityOverride(gravity);
}

    // ===============================================================================================
    //  EVENT HANDLERS
    // ===============================================================================================
void Character::HandleAnimationTrigger(StringHash eventType, VariantMap& eventData)
{
/*
URHO3D_EVENT(E_ANIMATIONTRIGGER, AnimationTrigger)
{
    URHO3D_PARAM(P_NODE, Node);                    // Node pointer
    URHO3D_PARAM(P_ANIMATION, Animation);          // Animation pointer
    URHO3D_PARAM(P_NAME, Name);                    // String
    URHO3D_PARAM(P_TIME, Time);                    // Float
    URHO3D_PARAM(P_DATA, Data);                    // User-defined data type
}
*/
    using namespace AnimationTrigger;

    // Отправит уже изъятые из P_DATA данные.

    VariantMap xdata = eventData[P_DATA].GetVariantMap();

    //AnimationState@ state = animModel.animationStates[eventData[NAME].GetString()];
    AnimationState* state = animModel->GetAnimationState(eventData[P_NAME].GetString());

    if(RADIO::d_log)
        Log::Write(-1, " Character: " + GetName() + " HandleAnimationTrigger P_NAME = " + eventData[P_NAME].GetString() + "\n");

    State* st = stateMachine->currentState;
    CharacterState* cs = static_cast<CharacterState*>(st);
    if (cs)
        cs->OnAnimationTrigger(state, xdata);
}











