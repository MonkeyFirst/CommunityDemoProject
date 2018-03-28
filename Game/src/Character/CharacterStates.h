#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Character/FSM.h"
#include "Character/Character.h"

const StringHash ATTACK_STATE("AttackState");
const StringHash REDIRECT_STATE("RedirectState");
const StringHash TURN_STATE("TurnState");
const StringHash COUNTER_STATE("CounterState");
const StringHash GETUP_STATE("GetUpState");
const StringHash STEPMOVE_STATE("StepMoveState");
const StringHash RUN_STATE("RunState");
const StringHash HIT_STATE("HitState");
const StringHash STAND_STATE("StandState");
const StringHash BEATHIT_STATE("BeatDownHitState");
const StringHash DEAD_STATE("DeadState");
const StringHash ANIMTEST_STATE("AnimationTestState");
const StringHash ALIGN_STATE("AlignState");

const StringHash ANIMATION_INDEX("AnimationIndex");
const StringHash ATTACK_TYPE("AttackType");
const StringHash TIME_SCALE("TimeScale");
const StringHash DATA("Data");
const StringHash NAME("Name");
const StringHash ANIMATION("Animation");
const StringHash SPEED("Speed");
const StringHash STATE("State");
const StringHash VALUE("Value");
const StringHash COUNTER_CHECK("CounterCheck");
const StringHash ATTACK_CHECK("AttackCheck");
const StringHash BONE("Bone");
const StringHash NODE("Node");
const StringHash RADIUS("Radius");
const StringHash COMBAT_SOUND("CombatSound");
const StringHash COMBAT_SOUND_LARGE("CombatSoundLarge");
const StringHash COMBAT_PARTICLE("CombatParticle");
const StringHash PARTICLE("Particle");
const StringHash DURATION("Duration");
const StringHash READY_TO_FIGHT("ReadyToFight");
const StringHash FOOT_STEP("FootStep");
const StringHash CHANGE_STATE("ChangeState");
const StringHash IMPACT("Impact");
const StringHash HEALTH("Health");
const StringHash SOUND("Sound");
const StringHash RANGE("Range");
const StringHash TAG("Tag");

enum CounterSubState
{
    COUNTER_NONE,
    COUNTER_WAITING,
    COUNTER_ANIMATING,
};

class CharacterState : public State
{
    URHO3D_OBJECT(CharacterState, State);

public:
    CharacterState(Context* context, Character* c);
    ~CharacterState();

    virtual void OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData);
    virtual void OnFootStep(const String& boneName);
    virtual void OnCombatSound(const String& boneName, bool large);
    virtual void OnCombatParticle(const String& boneName, const String& particleName);
    virtual float GetThreatScore();

    virtual void Enter(State* lastState);
    virtual void Exit(State* nextState);
    virtual void Update(float dt);

    SharedPtr<Character> ownner; // получает из конструктора
    int flags;
    float animSpeed;
    float blendTime;
    float startTime;

    bool combatReady;
    bool firstUpdate;

    int lastPhysicsType;
    int physicsType;

};

class SingleAnimationState : public CharacterState
{
    URHO3D_OBJECT(SingleAnimationState, CharacterState);

public:
    SingleAnimationState(Context* context, Character* c);
    ~SingleAnimationState();

    virtual void Update(float dt);
    virtual void Enter(State* lastState);
    virtual void OnMotionFinished();
    virtual void SetMotion(const String& name);

    String animation;
    bool looped;
    float stateTime;

};

class SingleMotionState : public CharacterState
{
    URHO3D_OBJECT(SingleMotionState, CharacterState);

public:
    SingleMotionState(Context* context, Character* c);
    ~SingleMotionState();

    virtual void Update(float dt);
    virtual void Enter(State* lastState);
    virtual void DebugDraw(DebugRenderer* debug);
    virtual void SetMotion(const String& name);
    virtual void OnMotionFinished();

    SharedPtr<Motion> motion;
};

class MultiAnimationState : public CharacterState
{
    URHO3D_OBJECT(MultiAnimationState, CharacterState);

public:
    MultiAnimationState(Context* context, Character* c);
    ~MultiAnimationState();

    virtual void Update(float dt);
    virtual void Enter(State* lastState);
    virtual void OnMotionFinished();
    virtual void AddMotion(const String& name);
    virtual int PickIndex();

    Vector<String> animations; // Array<String> animations;
    bool looped;
    float stateTime;
    int selectIndex;
};

class MultiMotionState : public CharacterState
{
    URHO3D_OBJECT(MultiMotionState, CharacterState);

public:
    MultiMotionState(Context* context, Character* c);
    ~MultiMotionState();

    virtual void Update(float dt);
    virtual void Enter(State* lastState);
    virtual void Start();
    virtual void DebugDraw(DebugRenderer* debug);
    virtual int PickIndex();
    virtual String GetDebugText();
    virtual void AddMotion(const String& name);
    virtual void OnMotionFinished();
    virtual void OnMotionAlignTimeOut();

    //Vector<SharedPtr<Motion> > motions; // Array<Motion*> motions;
    Vector<Motion*> motions;
    int selectIndex;
};

class AnimationTestState : public CharacterState
{
    URHO3D_OBJECT(AnimationTestState, CharacterState);

public:
    AnimationTestState(Context* context, Character* c);
    ~AnimationTestState();

    virtual void Enter(State* lastState);
    virtual void Exit(State* nextState);
    virtual void Process(Vector<String> animations); // void Process(Array<String> animations);
    virtual void Start();
    virtual void Update(float dt);
    virtual void DebugDraw(DebugRenderer* debug);
    virtual String GetDebugText();
    virtual bool CanReEntered();

    Vector<Motion*> testMotions; // Array<Motion*> testMotions;
    //Vector<SharedPtr<Motion> > testMotions;
    Vector<String> testAnimations; // Array<String> testAnimations;
    int currentIndex;
    bool allFinished;
};

class CharacterCounterState : public CharacterState
{
    URHO3D_OBJECT(CharacterCounterState, CharacterState);

public:
    CharacterCounterState(Context* context, Character* c);
    ~CharacterCounterState();

    virtual void Enter(State* lastState);
    virtual void Exit(State* nextState);
    virtual void StartCounterMotion();
    virtual int GetCounterDirection(int attackType, bool isBack);
    // Array<Motion*>* GetCounterMotions(int attackType, bool isBack);
    virtual Vector<Motion*> GetCounterMotions(int attackType, bool isBack);
    // void DumpCounterMotions(Array<Motion*>* motions);
    virtual void DumpCounterMotions(Vector<Motion*> motions);
    virtual void Update(float dt);
    virtual void ChangeSubState(int newState);
    virtual void Dump();
    virtual void SetTargetTransform(const Vector3& pos, float rot);
    virtual void StartAligning();
    virtual String GetDebugText();
    virtual void DebugDraw(DebugRenderer* debug);

    virtual void OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData){};

    virtual void AddMultiCounterMotions(const String& preFix, bool isPlayer);
    virtual void Add_Counter_Animations(const String& preFix, bool isPlayer);

    Vector<Motion*> doubleCounterMotions;
    Vector<Motion*> tripleCounterMotions;

    Vector<Motion*> frontArmMotions;
    Vector<Motion*> frontLegMotions;
    Vector<Motion*> backArmMotions;
    Vector<Motion*> backLegMotions;

    WeakPtr<Motion> currentMotion;

    int sub_state; // sub state
    int type;
    int index;

    float alignTime;
    Vector3 movePerSec;
    float yawPerSec;
    Vector3 targetPosition;
    float targetRotation;
};


class CharacterRagdollState : public CharacterState
{
    URHO3D_OBJECT(CharacterRagdollState, CharacterState);

public:
    CharacterRagdollState(Context* context, Character* c);
    ~CharacterRagdollState();

    void Update(float dt);
    void Enter(State* lastState);
//    void Exit(State* nextState);
};

class CharacterGetUpState : public MultiMotionState
{
    URHO3D_OBJECT(CharacterGetUpState, MultiMotionState);

public:
    CharacterGetUpState(Context* context, Character* c);
    ~CharacterGetUpState();

    virtual void Enter(State* lastState);
    virtual void Update(float dt);
    virtual void OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData){};

    int state;
    float ragdollToAnimTime;
};


class CharacterAlignState : public CharacterState
{
    URHO3D_OBJECT(CharacterAlignState, CharacterState);

public:
    CharacterAlignState(Context* context, Character* c);
    ~CharacterAlignState();

    virtual void Start(StringHash nextState, const Vector3& tPos, float tRot, float duration, int physicsType = 0, const String& anim = "");
    virtual void Update(float dt);
    virtual void DebugDraw(DebugRenderer* debug);
    virtual void OnAlignTimeOut();

    StringHash  nextStateName;
    String alignAnimation;
    Vector3 targetPosition;
    float targetRotation;
    Vector3 movePerSec;
    float rotatePerSec;
    float alignTime;
    
    int memOwnnerPhysics;
};






