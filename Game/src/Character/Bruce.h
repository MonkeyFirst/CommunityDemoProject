#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Character/Character.h"
#include "Character/Player.h"
#include "Character/PlayerMovement.h"
#include "Character/PlayerCombat.h"

// STATES ===================================================

class BruceStandState : public PlayerStandState
{
    URHO3D_OBJECT(BruceStandState, PlayerStandState);

public:
    BruceStandState(Context* context, Character* c);
    ~BruceStandState();

};

class BruceTurnState : public PlayerTurnState
{
    URHO3D_OBJECT(BruceTurnState, PlayerTurnState);

public:
    BruceTurnState(Context* context, Character* c);
    ~BruceTurnState();

};

class BruceStandToWalkState : public PlayerStandToWalkState
{
    URHO3D_OBJECT(BruceStandToWalkState, PlayerStandToWalkState);

public:
    BruceStandToWalkState(Context* context, Character* c);
    ~BruceStandToWalkState();

};

class BruceWalkState : public PlayerWalkState
{
    URHO3D_OBJECT(BruceWalkState, PlayerWalkState);

public:
    BruceWalkState(Context* context, Character* c);
    ~BruceWalkState();
};

class BruceStandToRunState : public PlayerStandToRunState
{
    URHO3D_OBJECT(BruceStandToRunState, PlayerStandToRunState);

public:
    BruceStandToRunState(Context* context, Character* c);
    ~BruceStandToRunState();

};

class BruceRunState : public PlayerRunState
{
    URHO3D_OBJECT(BruceRunState, PlayerRunState);

public:
    BruceRunState(Context* context, Character* c);
    ~BruceRunState();
};

class BruceRunToStandState : public PlayerRunToStandState
{
    URHO3D_OBJECT(BruceRunToStandState, PlayerRunToStandState);

public:
    BruceRunToStandState(Context* context, Character* c);
    ~BruceRunToStandState();

};

class BruceRunTurn180State : public PlayerRunTurn180State
{
    URHO3D_OBJECT(BruceRunTurn180State, PlayerRunTurn180State);

public:
    BruceRunTurn180State(Context* context, Character* c);
    ~BruceRunTurn180State();

};

class BruceEvadeState : public PlayerEvadeState
{
    URHO3D_OBJECT(BruceEvadeState, PlayerEvadeState);

public:
    BruceEvadeState(Context* context, Character* c);
    ~BruceEvadeState();

};

class BruceAttackState : public PlayerAttackState
{
    URHO3D_OBJECT(BruceAttackState, PlayerAttackState);

public:
    BruceAttackState(Context* context, Character* c);
    ~BruceAttackState();

    //void AddAttackMotion(Array<AttackMotion*>* attacks, const String& name, int frame, int type, const String& bName);
    void AddAttackMotion(Vector<AttackMotion*>& attacks, const String& name, int frame, int type, const String& bName);

//    Character* character_;
};

class BruceCounterState : public PlayerCounterState
{
    URHO3D_OBJECT(BruceCounterState, PlayerCounterState);

public:
    BruceCounterState(Context* context, Character* c);
    ~BruceCounterState();
};

class BruceHitState : public PlayerHitState
{
    URHO3D_OBJECT(BruceHitState, PlayerHitState);

public:
    BruceHitState(Context* context, Character* c);
    ~BruceHitState();
};

class BruceDeadState : public PlayerDeadState
{
    URHO3D_OBJECT(BruceDeadState, PlayerDeadState);

public:
    BruceDeadState(Context* context, Character* c);
    ~BruceDeadState();

};

class BruceBeatDownEndState : public PlayerBeatDownEndState
{
    URHO3D_OBJECT(BruceBeatDownEndState, PlayerBeatDownEndState);

public:
    BruceBeatDownEndState(Context* context, Character* c);
    ~BruceBeatDownEndState();

};

class BruceBeatDownHitState : public PlayerBeatDownHitState
{
    URHO3D_OBJECT(BruceBeatDownHitState, PlayerBeatDownHitState);

public:
    BruceBeatDownHitState(Context* context, Character* c);
    ~BruceBeatDownHitState();
    
    bool IsTransitionNeeded(float curDist);

};

class BruceTransitionState : public PlayerTransitionState
{
    URHO3D_OBJECT(BruceTransitionState, PlayerTransitionState);

public:
    BruceTransitionState(Context* context, Character* c);
    ~BruceTransitionState();

};

class BruceSlideInState : public PlayerSlideInState
{
    URHO3D_OBJECT(BruceSlideInState, PlayerSlideInState);

public:
    BruceSlideInState(Context* context, Character* c);
    ~BruceSlideInState();

};

class BruceSlideOutState : public PlayerSlideOutState
{
    URHO3D_OBJECT(BruceSlideOutState, PlayerSlideOutState);

public:
    BruceSlideOutState(Context* context, Character* c);
    ~BruceSlideOutState();

};

class BruceCrouchState : public PlayerCrouchState
{
    URHO3D_OBJECT(BruceCrouchState, PlayerCrouchState);

public:
    BruceCrouchState(Context* context, Character* c);
    ~BruceCrouchState();

};

class BruceCrouchTurnState : public PlayerCrouchTurnState
{
    URHO3D_OBJECT(BruceCrouchTurnState, PlayerCrouchTurnState);

public:
    BruceCrouchTurnState(Context* context, Character* c);
    ~BruceCrouchTurnState();

};

class BruceCrouchMoveState : public PlayerCrouchMoveState
{
    URHO3D_OBJECT(BruceCrouchMoveState, PlayerCrouchMoveState);

public:
    BruceCrouchMoveState(Context* context, Character* c);
    ~BruceCrouchMoveState();

};

class BruceFallState : public PlayerFallState
{
    URHO3D_OBJECT(BruceFallState, PlayerFallState);

public:
    BruceFallState(Context* context, Character* c);
    ~BruceFallState();

};

class BruceLandState : public PlayerLandState
{
    URHO3D_OBJECT(BruceLandState, PlayerLandState);

public:
    BruceLandState(Context* context, Character* c);
    ~BruceLandState();

};

class BruceCoverState : public PlayerCoverState
{
    URHO3D_OBJECT(BruceCoverState, PlayerCoverState);

public:
    BruceCoverState(Context* context, Character* c);
    ~BruceCoverState();

};

class BruceCoverRunState : public PlayerCoverRunState
{
    URHO3D_OBJECT(BruceCoverRunState, PlayerCoverRunState);

public:
    BruceCoverRunState(Context* context, Character* c);
    ~BruceCoverRunState();

};

class BruceCoverTransitionState : public PlayerCoverTransitionState
{
    URHO3D_OBJECT(BruceCoverTransitionState, PlayerCoverTransitionState);

public:
    BruceCoverTransitionState(Context* context, Character* c);
    ~BruceCoverTransitionState();

};

class BruceClimbOverState : public PlayerClimbOverState
{
    URHO3D_OBJECT(BruceClimbOverState, PlayerClimbOverState);

public:
    BruceClimbOverState(Context* context, Character* c);
    ~BruceClimbOverState();

};

class BruceClimbUpState : public PlayerClimbUpState
{
    URHO3D_OBJECT(BruceClimbUpState, PlayerClimbUpState);

public:
    BruceClimbUpState(Context* context, Character* c);
    ~BruceClimbUpState();

};

//
class BruceRailUpState : public PlayerRailUpState
{
    URHO3D_OBJECT(BruceRailUpState, PlayerRailUpState);

public:
    BruceRailUpState(Context* context, Character* c);
    ~BruceRailUpState();

};

class BruceRailIdleState : public PlayerRailIdleState
{
    URHO3D_OBJECT(BruceRailIdleState, PlayerRailIdleState);

public:
    BruceRailIdleState(Context* context, Character* c);
    ~BruceRailIdleState();

};

class BruceRailTurnState : public PlayerRailTurnState
{
    URHO3D_OBJECT(BruceRailTurnState, PlayerRailTurnState);

public:
    BruceRailTurnState(Context* context, Character* c);
    ~BruceRailTurnState();

};

class BruceRailDownState : public PlayerRailDownState
{
    URHO3D_OBJECT(BruceRailDownState, PlayerRailDownState);

public:
    BruceRailDownState(Context* context, Character* c);
    ~BruceRailDownState();

};
/*
class BruceRailFwdIdleState : public PlayerRailFwdIdleState
{
    URHO3D_OBJECT(BruceRailFwdIdleState, PlayerRailFwdIdleState);

public:
    BruceRailFwdIdleState(Context* context, Character* c);
    ~BruceRailFwdIdleState();

};

class BruceRailRunForwardState : public PlayerRailRunForwardState
{
    URHO3D_OBJECT(BruceRailRunForwardState, PlayerRailRunForwardState);

public:
    BruceRailRunForwardState(Context* context, Character* c);
    ~BruceRailRunForwardState();

};
*/
class BruceRailRunTurn180State : public PlayerRailTurn180State
{
    URHO3D_OBJECT(BruceRailRunTurn180State, PlayerRailTurn180State);

public:
    BruceRailRunTurn180State(Context* context, Character* c);
    ~BruceRailRunTurn180State();

};

class BruceHangUpState : public PlayerHangUpState
{
    URHO3D_OBJECT(BruceHangUpState, PlayerHangUpState);

public:
    BruceHangUpState(Context* context, Character* c);
    ~BruceHangUpState();

};

class BruceHangIdleState : public PlayerHangIdleState
{
    URHO3D_OBJECT(BruceHangIdleState, PlayerHangIdleState);

public:
    BruceHangIdleState(Context* context, Character* c);
    ~BruceHangIdleState();

};

class BruceHangOverState : public PlayerHangOverState
{
    URHO3D_OBJECT(BruceHangOverState, PlayerHangOverState);

public:
    BruceHangOverState(Context* context, Character* c);
    ~BruceHangOverState();

};

class BruceHangMoveState : public PlayerHangMoveState
{
    URHO3D_OBJECT(BruceHangMoveState, PlayerHangMoveState);

public:
    BruceHangMoveState(Context* context, Character* c);
    ~BruceHangMoveState();

};

class BruceDangleIdleState : public PlayerDangleIdleState
{
    URHO3D_OBJECT(BruceDangleIdleState, PlayerDangleIdleState);

public:
    BruceDangleIdleState(Context* context, Character* c);
    ~BruceDangleIdleState();

};

class BruceDangleOverState : public PlayerDangleOverState
{
    URHO3D_OBJECT(BruceDangleOverState, PlayerDangleOverState);

public:
    BruceDangleOverState(Context* context, Character* c);
    ~BruceDangleOverState();

};


class BruceDangleMoveState : public PlayerDangleMoveState
{
    URHO3D_OBJECT(BruceDangleMoveState, PlayerDangleMoveState);

public:
    BruceDangleMoveState(Context* context, Character* c);
    ~BruceDangleMoveState();

};

class BruceClimbDownState : public PlayerClimbDownState
{
    URHO3D_OBJECT(BruceClimbDownState, PlayerClimbDownState);

public:
    BruceClimbDownState(Context* context, Character* c);
    ~BruceClimbDownState();

};

// BATMAN ===================================================

class Bruce : public Player
{
	URHO3D_OBJECT(Bruce, Player);

public:
    Bruce(Context* context);
    ~Bruce();

    void AddStates();
};

























