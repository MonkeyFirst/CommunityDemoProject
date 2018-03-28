#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Character/Character.h"
#include "Character/CharacterStates.h"

class PlayerStandState : public MultiAnimationState
{
    URHO3D_OBJECT(PlayerStandState, MultiAnimationState);

public:
    PlayerStandState(Context* context, Character* c);
    ~PlayerStandState();

    void Enter(State* lastState);
    void Update(float dt);
    int PickIndex();
};

class PlayerEvadeState : public MultiMotionState
{
    URHO3D_OBJECT(PlayerEvadeState, MultiMotionState);

public:
    PlayerEvadeState(Context* context, Character* c);
    ~PlayerEvadeState();
    
    void Enter(State* lastState);
};

class PlayerTurnState : public MultiMotionState
{
    URHO3D_OBJECT(PlayerTurnState, MultiMotionState);

public:
    PlayerTurnState(Context* context, Character* c);
    ~PlayerTurnState();

    void Update(float dt);
    void Enter(State* lastState);
    void OnMotionFinished();
    void CaculateTargetRotation();
    void DebugDraw(DebugRenderer* debug);

    float turnSpeed;
    float targetRotation;
    float dockDist;
};

class PlayerStandToWalkState : public PlayerTurnState
{
    URHO3D_OBJECT(PlayerStandToWalkState, PlayerTurnState);

public:
    PlayerStandToWalkState(Context* context, Character* c);
    ~PlayerStandToWalkState();
};

class PlayerStandToRunState : public PlayerTurnState
{
    URHO3D_OBJECT(PlayerStandToRunState, PlayerTurnState);

public:
    PlayerStandToRunState(Context* context, Character* c);
    ~PlayerStandToRunState();
};

class PlayerMoveForwardState : public SingleMotionState
{
    URHO3D_OBJECT(PlayerMoveForwardState, SingleMotionState);

public:
    PlayerMoveForwardState(Context* context, Character* c);
    ~PlayerMoveForwardState();

    virtual void Enter(State* lastState);
    virtual void OnStop();
    virtual void OnTurn180();
    virtual void Update(float dt);

    float turnSpeed;
    float dockDist;
};

class PlayerWalkState : public PlayerMoveForwardState
{
    URHO3D_OBJECT(PlayerWalkState, PlayerMoveForwardState);

public:
    PlayerWalkState(Context* context, Character* c);
    ~PlayerWalkState();

    void OnStop();
    void OnTurn180();
    void Update(float dt);

    int runHoldingFrames;
};

class PlayerRunState : public PlayerMoveForwardState
{
    URHO3D_OBJECT(PlayerRunState, PlayerMoveForwardState);

public:
    PlayerRunState(Context* context, Character* c);
    ~PlayerRunState();

    void OnStop();
    void OnTurn180();
    void Update(float dt);

    int walkHoldingFrames;
    int maxWalkHoldFrames;
};

class PlayerRunToStandState : public SingleMotionState
{
    URHO3D_OBJECT(PlayerRunToStandState, SingleMotionState);

public:
    PlayerRunToStandState(Context* context, Character* c);
    ~PlayerRunToStandState();

};

class PlayerRunTurn180State : public SingleMotionState
{
    URHO3D_OBJECT(PlayerRunTurn180State, SingleMotionState);

public:
    PlayerRunTurn180State(Context* context, Character* c);
    ~PlayerRunTurn180State();

    void Enter(State* lastState);
    void Update(float dt);
    void DebugDraw(DebugRenderer* debug);

    Vector3   targetPos;
    float     targetAngle;
    float     yawPerSec;
    int       state;
};

class PlayerSlideInState : public SingleMotionState
{
    URHO3D_OBJECT(PlayerSlideInState, SingleMotionState);

public:
    PlayerSlideInState(Context* context, Character* c);
    ~PlayerSlideInState();

    void OnMotionFinished();
    void Update(float dt);
    void Enter(State* lastState);
    void Exit(State* nextState);

    int state;
    float slideTimer;
    Vector3 idleVelocity;
};

class PlayerSlideOutState : public MultiMotionState
{
    URHO3D_OBJECT(PlayerSlideOutState, MultiMotionState);

public:
    PlayerSlideOutState(Context* context, Character* c);
    ~PlayerSlideOutState();

    void OnMotionFinished();
};

class PlayerCrouchState : public SingleAnimationState
{
    URHO3D_OBJECT(PlayerCrouchState, SingleAnimationState);

public:
    PlayerCrouchState(Context* context, Character* c);
    ~PlayerCrouchState();

    void Enter(State* lastState);
    void Exit(State* nextState);
    void Update(float dt);
};

class PlayerCrouchTurnState : public PlayerTurnState
{
    URHO3D_OBJECT(PlayerCrouchTurnState, PlayerTurnState);

public:
    PlayerCrouchTurnState(Context* context, Character* c);
    ~PlayerCrouchTurnState();

    void Enter(State* lastState);
    void Exit(State* nextState);
};

class PlayerCrouchMoveState : public PlayerMoveForwardState
{
    URHO3D_OBJECT(PlayerCrouchMoveState, PlayerMoveForwardState);

public:
    PlayerCrouchMoveState(Context* context, Character* c);
    ~PlayerCrouchMoveState();

    void OnTurn180();
    void OnStop();
    void Enter(State* lastState);
    void Exit(State* nextState);
};

class PlayerFallState : public SingleAnimationState
{
    URHO3D_OBJECT(PlayerFallState, SingleAnimationState);

public:
    PlayerFallState(Context* context, Character* c);
    ~PlayerFallState();

    void Update(float dt);
    void OnMotionFinished();
};

class PlayerLandState : public SingleAnimationState
{
    URHO3D_OBJECT(PlayerLandState, SingleAnimationState);

public:
    PlayerLandState(Context* context, Character* c);
    ~PlayerLandState();

    void Enter(State* lastState);
};

class PlayerCoverState : public SingleAnimationState
{
    URHO3D_OBJECT(PlayerCoverState, SingleAnimationState);

public:
    PlayerCoverState(Context* context, Character* c);
    ~PlayerCoverState();

    void Enter(State* lastState);
    void Exit(State* nextState);
    void DebugDraw(DebugRenderer* debug);
    void Update(float dt);

    int state;
    float alignTime;
    float yawPerSec;
    float startYaw;
    float dockDirection;
    Vector3 dockPosition;
    float yawAdjustSpeed;
};

class PlayerCoverRunState : public SingleMotionState
{
    URHO3D_OBJECT(PlayerCoverRunState, SingleMotionState);

public:
    PlayerCoverRunState(Context* context, Character* c);
    ~PlayerCoverRunState();

    void Update(float dt);
    void DebugDraw(DebugRenderer* debug);

    Vector3 dockPosition;
};

class PlayerCoverTransitionState : public SingleMotionState
{
    URHO3D_OBJECT(PlayerCoverTransitionState, SingleMotionState);

public:
    PlayerCoverTransitionState(Context* context, Character* c);
    ~PlayerCoverTransitionState();

    void Update(float dt);
    void Enter(State* lastState);
    void OnMotionFinished();

    float yawPerSec;
};

class PlayerDockAlignState : public MultiMotionState
{
    URHO3D_OBJECT(PlayerDockAlignState, MultiMotionState);

public:
    PlayerDockAlignState(Context* context, Character* c);
    ~PlayerDockAlignState();

    void Update(float dt);
    void OnMotionAlignTimeOut();
    int PickTargetMotionByHeight(State* lastState, int numOfStandAnimations = 3);
    Vector3 PickDockOutTarget();
    Vector3 PickDockInTarget();
    float PickDockInRotation();
    float PickDockOutRotation();
    void DebugDraw(DebugRenderer* debug);
    void Enter(State* lastState);
    void Exit(State* nextState);

    Vector<Vector3> targetOffsets;
    int motionFlagAfterAlign;
    int motionFlagBeforeAlign;
    float alignTime;
    float turnSpeed;
    Vector3 motionPositon;
    Vector3 targetPosition;
    int dockBlendingMethod;
    float motionRotation;
    float targetRotation;
    float climbBaseHeight;
    float dockInTargetBound;
    bool dockInCheckThinWall;
    bool debug;
};

class PlayerClimbOverState : public PlayerDockAlignState
{
    URHO3D_OBJECT(PlayerClimbOverState, PlayerDockAlignState);

public:
    PlayerClimbOverState(Context* context, Character* c);
    ~PlayerClimbOverState();

    void Enter(State* lastState);
    Vector3 PickDockOutTarget();
    float PickDockOutRotation();
    void DebugDraw(DebugRenderer* debug);
    void OnMotionFinished();
    void OnMotionAlignTimeOut();

    Vector3 groundPos;
    Vector3 down128Pos;
    Vector3 startPos;
    Line* downLine;
};

class PlayerClimbUpState : public PlayerDockAlignState
{
    URHO3D_OBJECT(PlayerClimbUpState, PlayerDockAlignState);

public:
    PlayerClimbUpState(Context* context, Character* c);
    ~PlayerClimbUpState();

    void Enter(State* lastState);
};

class PlayerRailUpState : public PlayerDockAlignState
{
    URHO3D_OBJECT(PlayerRailUpState, PlayerDockAlignState);

public:
    PlayerRailUpState(Context* context, Character* c);
    ~PlayerRailUpState();

    void Enter(State* lastState);
    void OnMotionFinished();
};

class PlayerRailIdleState : public SingleAnimationState
{
    URHO3D_OBJECT(PlayerRailIdleState, SingleAnimationState);

public:
    PlayerRailIdleState(Context* context, Character* c);
    ~PlayerRailIdleState();

    void Enter(State* lastState);
    void Update(float dt);
};

class PlayerRailTurnState : public PlayerTurnState
{
    URHO3D_OBJECT(PlayerRailTurnState, PlayerTurnState);

public:
    PlayerRailTurnState(Context* context, Character* c);
    ~PlayerRailTurnState();

    void CaculateTargetRotation();
    void OnMotionFinished();

    float       turnAngle;
    StringHash  nextStateName;
};

class PlayerRailFwdIdleState : public SingleAnimationState
{
    URHO3D_OBJECT(PlayerRailFwdIdleState, SingleAnimationState);

public:
    PlayerRailFwdIdleState(Context* context, Character* c);
    ~PlayerRailFwdIdleState();

    void Update(float dt);
};

class PlayerRailDownState : public PlayerDockAlignState
{
    URHO3D_OBJECT(PlayerRailDownState, PlayerDockAlignState);

public:
    PlayerRailDownState(Context* context, Character* c);
    ~PlayerRailDownState();

    Vector3 PickDockInTarget();
    void Enter(State* lastState);
    void OnMotionFinished();
    void DebugDraw(DebugRenderer* debug);
    
    Vector3 groundPos;
};

class PlayerRailRunForwardState : public SingleMotionState
{
    URHO3D_OBJECT(PlayerRailRunForwardState, SingleMotionState);

public:
    PlayerRailRunForwardState(Context* context, Character* c);
    ~PlayerRailRunForwardState();

    void Update(float dt);
};

class PlayerRailTurn180State : public SingleMotionState
{
    URHO3D_OBJECT(PlayerRailTurn180State, SingleMotionState);

public:
    PlayerRailTurn180State(Context* context, Character* c);
    ~PlayerRailTurn180State();

    void Enter(State* lastState);
    void Update(float dt);
    void OnMotionFinished();
    void DebugDraw(DebugRenderer* debug);

    Vector3 targetPosition;
    float targetRotation;
    float turnSpeed;
};

class PlayerHangUpState : public PlayerDockAlignState
{
    URHO3D_OBJECT(PlayerHangUpState, PlayerDockAlignState);

public:
    PlayerHangUpState(Context* context, Character* c);
    ~PlayerHangUpState();

    void Enter(State* lastState);
    void OnMotionFinished();
};

class PlayerHangMoveState;

class PlayerHangIdleState : public MultiMotionState
{
    URHO3D_OBJECT(PlayerHangIdleState, MultiMotionState);

public:
    PlayerHangIdleState(Context* context, Character* c);
    ~PlayerHangIdleState();

    void OnMotionFinished();
    void DebugDraw(DebugRenderer* debug);
    void Enter(State* lastState);
    bool CheckFootBlocking();
    void Update(float dt);
    bool VerticalMove();
    PlayerHangMoveState* GetMoveState(Line* l);
    bool TryToMoveToLinePoint(bool left);
    bool HorizontalMove(bool left);

    String idleAnim;
    StringHash overStateName;
    StringHash moveStateName;
    float moveToLinePtDist;
    float turnSpeed;
    float alignTime;
    float inputCheckTime;
    float motionRotation;
    float targetRotation;
    Vector3 targetPosition;
    Vector3 motionPositon;
    int state;
};

class PlayerHangMoveState : public PlayerDockAlignState
{
    URHO3D_OBJECT(PlayerHangMoveState, PlayerDockAlignState);

public:
    PlayerHangMoveState(Context* context, Character* c);
    ~PlayerHangMoveState();

    void Enter(State* lastState);
    void Exit(State* nextState);
    void CrossMove(Line* line, bool left, int convexIndex);
    void ParalleJumpMove(Line* line, bool left, bool bigJump);
    void MoveToLinePoint(bool left);
    void NormalMove(bool left);
    void OnMotionFinished();
    Vector3 PickDockInTarget();
    Vector3 PickDockOutTarget();

    Line* oldLine;
    int numOfAnimations;
    int type;
};

class PlayerHangOverState : public PlayerDockAlignState
{
    URHO3D_OBJECT(PlayerHangOverState, PlayerDockAlignState);

public:
    PlayerHangOverState(Context* context, Character* c);
    ~PlayerHangOverState();

    Vector3 PickDockOutTarget();
    void Enter(State* lastState);
    void OnMotionFinished();

    Vector3 groundPos;
    int type;
};

class PlayerDangleIdleState : public PlayerHangIdleState
{
    URHO3D_OBJECT(PlayerDangleIdleState, PlayerHangIdleState);

public:
    PlayerDangleIdleState(Context* context, Character* c);
    ~PlayerDangleIdleState();

    bool CheckFootBlocking();
};

class PlayerDangleOverState : public PlayerHangOverState
{
    URHO3D_OBJECT(PlayerDangleOverState, PlayerHangOverState);

public:
    PlayerDangleOverState(Context* context, Character* c);
    ~PlayerDangleOverState();
};

class PlayerDangleMoveState : public PlayerHangMoveState
{
    URHO3D_OBJECT(PlayerDangleMoveState, PlayerHangMoveState);

public:
    PlayerDangleMoveState(Context* context, Character* c);
    ~PlayerDangleMoveState();

    void OnMotionFinished();
};

class PlayerClimbDownState : public PlayerDockAlignState
{
    URHO3D_OBJECT(PlayerClimbDownState, PlayerDockAlignState);

public:
    PlayerClimbDownState(Context* context, Character* c);
    ~PlayerClimbDownState();

    void DebugDraw(DebugRenderer* debug);
    float PickDockInRotation();
    void Enter(State* lastState);
    void OnMotionFinished();
    
    Vector3 groundPos;
};








