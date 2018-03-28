#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Character/Enemy.h"
#include "Character/CharacterStates.h"

// STATES ====================================================

class ThugStandState : public MultiAnimationState
{
    URHO3D_OBJECT(ThugStandState, MultiAnimationState);

public:
    ThugStandState(Context* context, Character* c);
    ~ThugStandState();

    void Enter(State* lastState);
    void Update(float dt);
    void OnThinkTimeOut();
    void FixedUpdate(float dt);
    int PickIndex();

    float thinkTime;
    float attackRange;
    bool firstEnter;
};

class ThugStepMoveState : public MultiMotionState
{
    URHO3D_OBJECT(ThugStepMoveState, MultiMotionState);

public:
    ThugStepMoveState(Context* context, Character* c);
    ~ThugStepMoveState();

    void Update(float dt);
    int GetStepMoveIndex();
    void Enter(State* lastState);
    float GetThreatScore();

    float attackRange;
};

class ThugRunState : public SingleMotionState
{
    URHO3D_OBJECT(ThugRunState, SingleMotionState);

public:
    ThugRunState(Context* context, Character* c);
    ~ThugRunState();

    void Update(float dt);
    void Enter(State* lastState);
    void FixedUpdate(float dt);
    float GetThreatScore();

    float turnSpeed;
    float attackRange;
};

class ThugTurnState : public MultiMotionState
{
    URHO3D_OBJECT(ThugTurnState, MultiMotionState);

public:
    ThugTurnState(Context* context, Character* c);
    ~ThugTurnState();

    void Update(float dt);
    void Enter(State* lastState);
    void FixedUpdate(float dt);

    float turnSpeed;
    float endTime;
};

class ThugCounterState : public CharacterCounterState
{
    URHO3D_OBJECT(ThugCounterState, CharacterCounterState);

public:
    ThugCounterState(Context* context, Character* c);
    ~ThugCounterState();

    void OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData);
    void Exit(State* nextState);
};


class ThugAttackState : public CharacterState
{
    URHO3D_OBJECT(ThugAttackState, CharacterState);

public:
    ThugAttackState(Context* context, Character* c);
    ~ThugAttackState();

    void AddAttackMotion(const String& name, int impactFrame, int type, const String& bName);
    void Update(float dt);
    void Enter(State* lastState);
    void Exit(State* nextState);
    void ShowAttackIndicator(bool bshow);
    void OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData);
    void AttackCollisionCheck();
    float GetThreatScore();

    AttackMotion* currentAttack;
    Vector<AttackMotion*> attacks;
    float turnSpeed;
    bool doAttackCheck;
    Node* attackCheckNode;
};

class ThugHitState : public MultiMotionState
{
    URHO3D_OBJECT(ThugHitState, MultiMotionState);

public:
    ThugHitState(Context* context, Character* c);
    ~ThugHitState();

    void Update(float dt);
    void Exit(State* nextState);
    bool CanReEntered();
    void FixedUpdate(float dt);

    float recoverTimer;
};

class ThugGetUpState : public CharacterGetUpState
{
    URHO3D_OBJECT(ThugGetUpState, CharacterGetUpState);

public:
    ThugGetUpState(Context* context, Character* c);
    ~ThugGetUpState();

    void OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData);
    void Enter(State* lastState);
    void Exit(State* nextState);
};

class ThugDeadState : public CharacterState
{
    URHO3D_OBJECT(ThugDeadState, CharacterState);

public:
    ThugDeadState(Context* context, Character* c);
    ~ThugDeadState();

    void Enter(State* lastState);
    void Update(float dt);

    float  duration;
};

class ThugBeatDownHitState : public MultiMotionState
{
    URHO3D_OBJECT(ThugBeatDownHitState, MultiMotionState);

public:
    ThugBeatDownHitState(Context* context, Character* c);
    ~ThugBeatDownHitState();

    bool CanReEntered();
    float GetThreatScore();
    void OnMotionFinished();
};

class ThugBeatDownEndState : public MultiMotionState
{
    URHO3D_OBJECT(ThugBeatDownEndState, MultiMotionState);

public:
    ThugBeatDownEndState(Context* context, Character* c);
    ~ThugBeatDownEndState();

    void Enter(State* lastState);
};

class ThugStunState : public SingleAnimationState
{
    URHO3D_OBJECT(ThugStunState, SingleAnimationState);

public:
    ThugStunState(Context* context, Character* c);
    ~ThugStunState();

};

class ThugPushBackState : public SingleMotionState
{
    URHO3D_OBJECT(ThugPushBackState, SingleMotionState);

public:
    ThugPushBackState(Context* context, Character* c);
    ~ThugPushBackState();

};

// THUG ====================================================

class Thug : public Enemy
{
    URHO3D_OBJECT(Thug, Enemy);

public:
    Thug(Context* context);
    ~Thug();

    void ObjectStart();
    void DebugDraw(DebugRenderer* debug);
    bool CanAttack();
    bool Attack();
    bool Redirect();
    void CommonStateFinishedOnGroud();
    bool OnDamage(GameObject* attacker, const Vector3& position, const Vector3& direction, int damage, bool weak = false);
    void RequestDoNotMove();
    int GetSperateDirection(int& outDir);
    void CheckAvoidance(float dt);
    void ClearAvoidance();
    bool KeepDistanceWithEnemy();
    bool KeepDistanceWithPlayer(float max_dist = -0.25f); // KEEP_DIST_WITH_PLAYER
    void CheckCollision();
    bool Distract();

    float checkAvoidanceTimer;
    float checkAvoidanceTime;
};
























