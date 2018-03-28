#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Objects/GameObject.h"

#include "Character/FSM.h"
#include "Character/PhysicsSensor.h"

#include "Tools/Line.h"
#include "Motions/Motions.h"

class Character : public GameObject
{
    URHO3D_OBJECT(Character, GameObject);

public:
    Character(Context* context);
    virtual ~Character();

    // virtual
    virtual void Update(float timeStep);
    virtual bool ActionCheck(bool bAttack, bool bDistract, bool bCounter, bool bEvade);
    // virtual т.к. Player тоже имеет эту функцию и вызывает аналогичную у родителя Character::ObjectStart();
    virtual void ObjectStart(); 
    void Kill();
    virtual bool Counter();
    virtual bool Evade();
    virtual bool Redirect();
    virtual bool Distract();
    virtual void ResetObject();
    virtual void SetVelocity(const Vector3& vel);
    virtual bool Attack();
    virtual void CommonStateFinishedOnGroud();
    virtual void DebugDraw(DebugRenderer* debug);
    virtual float GetTargetAngle();
    virtual bool OnDamage(GameObject* attacker, const Vector3& position, const Vector3& direction, int damage, bool weak = false);
    virtual void OnAttackSuccess(Character* object);
    virtual void OnCounterSuccess();
    virtual void SetTarget(Character* t);
    virtual bool CheckFalling();
    virtual bool CheckDocking(float dist = 3);

    Context* GetContext(){return context_;}

    // Component class functions
    void Start();
    void Stop();

    void SetTimeScale(float scale);
    // unsigned int layer = LAYER_MOVE - in Motions.h
    void PlayAnimation(const String& animName, unsigned int layer = 0, bool loop = false, float blendTime = 0.1f, float startTime = 0.0f, float speed = 1.0f);
    String GetDebugText();
    Vector3 GetVelocity();
    void MoveTo(const Vector3& position, float dt);
    void SetHealth(int h);
    bool CanBeAttacked();
    bool CanBeCountered();
    bool CanBeRedirected();
    bool CanAttack();
    void TestAnimation(const Vector<String>& animations);//void TestAnimation(const Array<String>& animations);
    void TestAnimation(const String& animation);
    float GetTargetAngle(Node* _node);
    float GetTargetDistance();
    float GetTargetDistance(Node* _node);
    float ComputeAngleDiff();
    float ComputeAngleDiff(Node* _node);
    int RadialSelectAnimation(int numDirections); 
    int RadialSelectAnimation(Node* _node, int numDirections);
    float GetCharacterAngle();
    String GetName();
    float GetFootFrontDiff();
    void PlayCurrentPose();
    //Node* GetNode();
    void OnDead();
    void MakeMeRagdoll(const Vector3& velocity = Vector3(0, 0, 0), const Vector3& position = Vector3(0, 0, 0));
    void RequestDoNotMove();
    Node* SpawnParticleEffect(const Vector3& position, const String& effectName, float duration, float scale = 1.0f);
    Node* SpawnSound(const Vector3& position, const String& soundName, float duration);

    void SetComponentEnabled(const String& boneName, const String& componentName, bool bEnable);
    void SetNodeEnabled(const String& nodeName, bool bEnable);
    State* GetState();
    bool IsInState(const String& name);
    bool IsInState_Hash(const StringHash& nameHash);
    bool ChangeState(const String& name);
    bool ChangeState_Hash(const StringHash& nameHash);
    void ChangeStateQueue(const StringHash& nameHash);
    State* FindState(const String& name);
    State* FindState_Hash(const StringHash& nameHash);
    void FixedUpdate(float timeStep); // GameObject class
       
    virtual bool IsTargetSightBlocked();
    virtual void CheckCollision();
    void SetPhysics(bool b);
    void PlayRandomSound(int type);
    bool IsVisible();
    virtual void CheckAvoidance(float dt);
    virtual void ClearAvoidance();
    void CheckTargetDistance(Character* t, float dist);
    bool IsInAir();
    void SetHeight(float height);
    void AssignDockLine(Line* l);
    void SetPhysicsType(int type);
    void SetGravity(const Vector3& gravity);
    void HandleAnimationTrigger(StringHash eventType, VariantMap& eventData);

// ========================
    //FSM* stateMachine;
    SharedPtr<FSM> stateMachine;

    WeakPtr<Character> target;

    SharedPtr<Node> sceneNode;
    WeakPtr<Node> renderNode;

    SharedPtr<AnimationController> animCtrl;
    SharedPtr<AnimatedModel> animModel;
    SharedPtr<Animation> ragdollPoseAnim;
    SharedPtr<RigidBody> body;
    SharedPtr<PhysicsSensor> sensor;

    //SharedPtr<Line> dockLine;
    Line* dockLine;
// ========================

    Vector3 startPosition;
    Quaternion startRotation;

    int health;

    float attackRadius;
    int attackDamage;

    int physicsType;

    String lastAnimation;
    String walkAlignAnimation;

    // ==============================================
    //   DYNAMIC VALUES For Motion
    // ==============================================
    Vector3 motion_startPosition;
    float motion_startRotation;

    float motion_deltaRotation;
    Vector3 motion_deltaPosition;
    Vector3 motion_velocity;

    bool motion_translateEnabled;
    bool motion_rotateEnabled;

};