#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Character/EnemyManager.h"

class PlayerAttackState : public CharacterState
{
    URHO3D_OBJECT(PlayerAttackState, CharacterState);

public:
    PlayerAttackState(Context* context, Character* c);
    ~PlayerAttackState();

    // void DumpAttacks(Array<AttackMotion*>* attacks);
    void DumpAttacks(Vector<AttackMotion*>& attacks);
    // float UpdateMaxDist(Array<AttackMotion*>* attacks, float dist);
    float UpdateMaxDist(Vector<AttackMotion*>& attacks, float dist);
    void Dump();
    void ChangeSubState(int newState);
    void Update(float dt);
    void CheckInput(float t);
    // void PickBestMotion(Array<AttackMotion*>* attacks, int dir);
    void PickBestMotion(Vector<AttackMotion*>& attacks, int dir);
    void StartAttack();
    void Enter(State* lastState);
    void Exit(State* nextState);
    void DebugDraw(DebugRenderer* debug);
    String GetDebugText();
    bool CanReEntered();
    void AttackImpact();
    void PostInit(float closeDist = 2.5f);

    Vector<AttackMotion*> forwardAttacks;
    Vector<AttackMotion*> leftAttacks;
    Vector<AttackMotion*> rightAttacks;
    Vector<AttackMotion*> backAttacks;

    WeakPtr<AttackMotion> currentAttack;

//    float max_attack_dist; // MAX_ATTACK_DIST

    int state;
    Vector3 movePerSec;
    Vector3 predictPosition;
    Vector3 motionPosition;

    float alignTime;

    int forwadCloseNum;
    int leftCloseNum;
    int rightCloseNum;
    int backCloseNum;

    int slowMotionFrames;

    int lastAttackDirection;
    int lastAttackIndex;

    bool weakAttack;
    bool slowMotion;
    bool lastKill;
};


class PlayerCounterState : public CharacterCounterState
{
    URHO3D_OBJECT(PlayerCounterState, CharacterCounterState);

public:
    PlayerCounterState(Context* context, Character* c);
    ~PlayerCounterState();

    void Update(float dt);
    void Enter(State* lastState);
    void Exit(State* nextState);
    void StartAnimating();
    void StartCounterMotion();
    
    virtual void OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData);

    bool CanReEntered();

    Enemy* counterEnemy;
    Vector<int> intCache; // Array<int> intCache;
    int lastCounterIndex;
    int lastCounterDirection;

    // MultiCounter only
    void ChooseBestIndices(Motion* alignMotion, int index);
    int GetValidNumOfCounterEnemy();
    int TestTrippleCounterMotions(int i);
    int TestDoubleCounterMotions(int i);
    
    Vector<Enemy*>   counterEnemies; // for MultiCounter

};


class PlayerHitState : public MultiMotionState
{
    URHO3D_OBJECT(PlayerHitState, MultiMotionState);

public:
    PlayerHitState(Context* context, Character* c);
    ~PlayerHitState();
};


class PlayerDeadState : public MultiMotionState
{
    URHO3D_OBJECT(PlayerDeadState, MultiMotionState);

public:
    PlayerDeadState(Context* context, Character* c);
    ~PlayerDeadState();

    void Enter(State* lastState);
    void Update(float dt);

    Vector<String> animations;
    int state;
};

class PlayerBeatDownEndState : public MultiMotionState
{
    URHO3D_OBJECT(PlayerBeatDownEndState, MultiMotionState);

public:
    PlayerBeatDownEndState(Context* context, Character* c);
    ~PlayerBeatDownEndState();

    void Enter(State* lastState);
    void Exit(State* nextState);
    int PickIndex();
    void OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData);
};

class PlayerBeatDownHitState : public MultiMotionState
{
    URHO3D_OBJECT(PlayerBeatDownHitState, MultiMotionState);

public:
    PlayerBeatDownHitState(Context* context, Character* c);
    ~PlayerBeatDownHitState();

    bool CanReEntered();
    bool IsTransitionNeeded(float curDist);
    void Update(float dt);
    void Enter(State* lastState);
    void OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData);
    int PickIndex();
    void DebugDraw(DebugRenderer* debug);
    String GetDebugText();

    int beatIndex;
    int beatNum;
    int maxBeatNum;
    int minBeatNum;
    int beatTotal;
    bool attackPressed;

    Vector3 targetPosition;
    float targetRotation;
};

class PlayerTransitionState : public SingleMotionState
{
    URHO3D_OBJECT(PlayerTransitionState, SingleMotionState);

public:
    PlayerTransitionState(Context* context, Character* c);
    ~PlayerTransitionState();

    void OnMotionFinished();
    void Enter(State* lastState);
    void Exit(State* nextState);
    String GetDebugText();

    String nextStateName;
};






