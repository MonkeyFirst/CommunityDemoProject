#include <Urho3D/Urho3DAll.h>

#include "Character/Bruce.h"

#include "Motions/MotionManager.h"
//#include "Levels/GameManager.h"

#include "radio.h"

// -- non cost
//float BRUCE_TRANSITION_DIST = 0.0f;

// BruceStandState ==============================================

BruceStandState::BruceStandState(Context* context, Character* c) : 
    PlayerStandState(context, c)
{
//    Log::Write(-1," BruceStandState: Constructor\n");
    AddMotion("BM_Movement/Stand_Idle");
}

BruceStandState::~BruceStandState()
{}

// BruceTurnState ==============================================

BruceTurnState::BruceTurnState(Context* context, Character* c) : 
    PlayerTurnState(context, c)
{
//    Log::Write(-1," BruceTurnState: Constructor\n");
    AddMotion("BM_Movement/Turn_Right_90");
    AddMotion("BM_Movement/Turn_Right_180");
    AddMotion("BM_Movement/Turn_Left_90");
}

BruceTurnState::~BruceTurnState()
{}

// BruceStandToWalkState ==============================================

BruceStandToWalkState::BruceStandToWalkState(Context* context, Character* c) : 
    PlayerStandToWalkState(context, c)
{
//    Log::Write(-1," BruceStandToWalkState: Constructor\n");
    AddMotion("BM_Movement/Stand_To_Walk_Right_90");
    AddMotion("BM_Movement/Stand_To_Walk_Right_180");
    AddMotion("BM_Movement/Stand_To_Walk_Right_180");
}

BruceStandToWalkState::~BruceStandToWalkState()
{}

// BruceWalkState ==============================================

BruceWalkState::BruceWalkState(Context* context, Character* c) : 
    PlayerWalkState(context, c)
{
//    Log::Write(-1," BruceWalkState: Constructor\n");
    SetMotion("BM_Movement/Walk_Forward");
}

BruceWalkState::~BruceWalkState()
{}

// BruceStandToRunState ==============================================

BruceStandToRunState::BruceStandToRunState(Context* context, Character* c) : 
    PlayerStandToRunState(context, c)
{
    AddMotion("BM_Movement/Stand_To_Run_Right_90");
    AddMotion("BM_Movement/Stand_To_Run_Right_180");
    AddMotion("BM_Movement/Stand_To_Run_Right_180");
}

BruceStandToRunState::~BruceStandToRunState()
{}

// BruceRunState ==============================================

BruceRunState::BruceRunState(Context* context, Character* c) : 
    PlayerRunState(context, c)
{
    SetMotion("BM_Movement/Run_Forward");
}

BruceRunState::~BruceRunState()
{}

// BruceRunToStandState ==============================================

BruceRunToStandState::BruceRunToStandState(Context* context, Character* c) : 
    PlayerRunToStandState(context, c)
{
    SetMotion("BM_Movement/Run_Right_Passing_To_Stand");
}

BruceRunToStandState::~BruceRunToStandState()
{}

// BruceRunTurn180State ==============================================

BruceRunTurn180State::BruceRunTurn180State(Context* context, Character* c) : 
    PlayerRunTurn180State(context, c)
{
    SetMotion("BM_Movement/Run_Right_Passing_To_Run_Right_180");
}

BruceRunTurn180State::~BruceRunTurn180State()
{}

// BruceEvadeState ==============================================

BruceEvadeState::BruceEvadeState(Context* context, Character* c) : 
    PlayerEvadeState(context, c)
{
//    SetMotion("BM_Combat/Evade_Forward_03");
    String prefix = "BM_Combat/";
    AddMotion(prefix + "Evade_Forward_01");
    AddMotion(prefix + "Evade_Right_01");
    AddMotion(prefix + "Evade_Back_01");
    AddMotion(prefix + "Evade_Left_01");

}

BruceEvadeState::~BruceEvadeState()
{}

// BruceAttackState ==============================================

BruceAttackState::BruceAttackState(Context* context, Character* c) :
    PlayerAttackState(context, c)
{
//    character_ = c;

     //========================================================================
    // FORWARD
    //========================================================================
    AddAttackMotion(forwardAttacks, "Attack_Close_Weak_Forward", 11, ATTACK_PUNCH, R_HAND);
    AddAttackMotion(forwardAttacks, "Attack_Close_Weak_Forward_01", 12, ATTACK_PUNCH, L_HAND);
    AddAttackMotion(forwardAttacks, "Attack_Close_Weak_Forward_02", 12, ATTACK_PUNCH, L_HAND);
    AddAttackMotion(forwardAttacks, "Attack_Close_Weak_Forward_03", 11, ATTACK_PUNCH, L_HAND);
    AddAttackMotion(forwardAttacks, "Attack_Close_Weak_Forward_04", 16, ATTACK_PUNCH, R_HAND);
    AddAttackMotion(forwardAttacks, "Attack_Close_Forward_02", 14, ATTACK_PUNCH, L_HAND);
    AddAttackMotion(forwardAttacks, "Attack_Close_Forward_03", 11, ATTACK_KICK, L_FOOT);
    AddAttackMotion(forwardAttacks, "Attack_Close_Forward_06", 20, ATTACK_PUNCH, R_HAND);
    AddAttackMotion(forwardAttacks, "Attack_Close_Forward_08", 18, ATTACK_PUNCH, R_ARM);
    AddAttackMotion(forwardAttacks, "Attack_Close_Run_Forward", 12, ATTACK_PUNCH, R_HAND);
    AddAttackMotion(forwardAttacks, "Attack_Far_Forward", 25, ATTACK_PUNCH, R_HAND);
    AddAttackMotion(forwardAttacks, "Attack_Far_Forward_03", 22, ATTACK_PUNCH, R_HAND);
    AddAttackMotion(forwardAttacks, "Attack_Run_Far_Forward", 18, ATTACK_KICK, R_FOOT);

    //========================================================================
    // RIGHT
    //========================================================================
    AddAttackMotion(rightAttacks, "Attack_Close_Weak_Right", 12, ATTACK_PUNCH, L_HAND);
    AddAttackMotion(rightAttacks, "Attack_Close_Weak_Right_01", 10, ATTACK_PUNCH, R_ARM);
    AddAttackMotion(rightAttacks, "Attack_Close_Weak_Right_02", 15, ATTACK_PUNCH, R_HAND);
    AddAttackMotion(rightAttacks, "Attack_Close_Right", 16, ATTACK_PUNCH, L_HAND);
    AddAttackMotion(rightAttacks, "Attack_Close_Right_01", 18, ATTACK_PUNCH, R_ARM);
    AddAttackMotion(rightAttacks, "Attack_Close_Right_05", 15, ATTACK_KICK, L_CALF);
    AddAttackMotion(rightAttacks, "Attack_Close_Right_07", 18, ATTACK_PUNCH, R_ARM);
    //AddAttackMotion(rightAttacks, "Attack_Far_Right_02", 21, ATTACK_PUNCH, R_HAND);

    //========================================================================
    // BACK
    //========================================================================
    // back weak
    AddAttackMotion(backAttacks, "Attack_Close_Weak_Back", 12, ATTACK_PUNCH, L_ARM);
    AddAttackMotion(backAttacks, "Attack_Close_Weak_Back_01", 12, ATTACK_PUNCH, R_HAND);
    AddAttackMotion(backAttacks, "Attack_Close_Back", 11, ATTACK_PUNCH, L_ARM);
    AddAttackMotion(backAttacks, "Attack_Close_Back_01", 16, ATTACK_PUNCH, L_HAND);
    AddAttackMotion(backAttacks, "Attack_Close_Back_03", 21, ATTACK_KICK, R_FOOT);
    AddAttackMotion(backAttacks, "Attack_Close_Back_04", 18, ATTACK_KICK, R_FOOT);
    AddAttackMotion(backAttacks, "Attack_Close_Back_05", 14, ATTACK_PUNCH, L_HAND);
    AddAttackMotion(backAttacks, "Attack_Close_Back_06", 15, ATTACK_PUNCH, R_HAND);
    AddAttackMotion(backAttacks, "Attack_Close_Back_08", 17, ATTACK_KICK, L_FOOT);
    AddAttackMotion(backAttacks, "Attack_Far_Back", 14, ATTACK_KICK, L_FOOT);
    AddAttackMotion(backAttacks, "Attack_Far_Back_01", 15, ATTACK_KICK, L_FOOT);

    //========================================================================
    // LEFT
    //========================================================================
    // left weak
    AddAttackMotion(leftAttacks, "Attack_Close_Weak_Left", 13, ATTACK_PUNCH, R_HAND);
    AddAttackMotion(leftAttacks, "Attack_Close_Weak_Left_01", 12, ATTACK_PUNCH, R_HAND);
    AddAttackMotion(leftAttacks, "Attack_Close_Weak_Left_02", 13, ATTACK_PUNCH, L_HAND);
    AddAttackMotion(leftAttacks, "Attack_Close_Left", 7, ATTACK_PUNCH, R_HAND);
    AddAttackMotion(leftAttacks, "Attack_Close_Left_02", 13, ATTACK_KICK, R_FOOT);
    AddAttackMotion(leftAttacks, "Attack_Close_Left_05", 15, ATTACK_KICK, L_FOOT);
    AddAttackMotion(leftAttacks, "Attack_Close_Left_06", 12, ATTACK_KICK, R_FOOT);
    AddAttackMotion(leftAttacks, "Attack_Close_Left_07", 15, ATTACK_PUNCH, L_HAND);
    AddAttackMotion(leftAttacks, "Attack_Far_Left_02", 22, ATTACK_PUNCH, R_ARM);
    AddAttackMotion(leftAttacks, "Attack_Far_Left_03", 21, ATTACK_KICK, L_FOOT);

    PostInit();
}

BruceAttackState::~BruceAttackState()
{}

// AddAttackMotion(Array<AttackMotion*>* attacks, const String& name, int frame, int type, const String& bName)
void BruceAttackState::AddAttackMotion(Vector<AttackMotion*>& attacks, const String& name, int frame, int type, const String& bName)
{
    // character_->->GetContext()
    attacks.Push(new AttackMotion(context_, "BW_Attack/" + name, frame, type, bName));
}

// BruceCounterState ==============================================

BruceCounterState::BruceCounterState(Context* context, Character* c) : 
    PlayerCounterState(context, c)
{
    // true - player. See CharacterCounterState (CharacterStates .h .cpp)
    Add_Counter_Animations("BM_TG_Counter/", true); 
}

BruceCounterState::~BruceCounterState()
{}

// BruceHitState ==============================================

BruceHitState::BruceHitState(Context* context, Character* c) : 
    PlayerHitState(context, c)
{
    String hitPrefix = "BM_HitReaction/";
    AddMotion(hitPrefix + "HitReaction_Face_Right");
    AddMotion(hitPrefix + "Hit_Reaction_SideLeft");
    AddMotion(hitPrefix + "HitReaction_Back");
    AddMotion(hitPrefix + "Hit_Reaction_SideRight");
}

BruceHitState::~BruceHitState()
{}

// BruceDeadState ==============================================

BruceDeadState::BruceDeadState(Context* context, Character* c) : 
    PlayerDeadState(context, c)
{
    String prefix = "BM_Death_Primers/";
    AddMotion(prefix + "Death_Front");
    AddMotion(prefix + "Death_Side_Left");
    AddMotion(prefix + "Death_Back");
    AddMotion(prefix + "Death_Side_Right");
}

BruceDeadState::~BruceDeadState()
{}

// BruceBeatDownEndState ==============================================

BruceBeatDownEndState::BruceBeatDownEndState(Context* context, Character* c) : 
    PlayerBeatDownEndState(context, c)
{
    String preFix = "BM_TG_Beatdown/";
    AddMotion(preFix + "Beatdown_Strike_End_01");
    AddMotion(preFix + "Beatdown_Strike_End_02");
    AddMotion(preFix + "Beatdown_Strike_End_03");
    AddMotion(preFix + "Beatdown_Strike_End_04");
}

BruceBeatDownEndState::~BruceBeatDownEndState()
{}

// BruceBeatDownHitState ==============================================

BruceBeatDownHitState::BruceBeatDownHitState(Context* context, Character* c) : 
    PlayerBeatDownHitState(context, c)
{
    String preFix = "BM_Attack/";
    AddMotion(preFix + "Beatdown_Test_01");
    AddMotion(preFix + "Beatdown_Test_02");
    AddMotion(preFix + "Beatdown_Test_03");
    AddMotion(preFix + "Beatdown_Test_04");
    AddMotion(preFix + "Beatdown_Test_05");
    AddMotion(preFix + "Beatdown_Test_06");
}

BruceBeatDownHitState::~BruceBeatDownHitState()
{}

bool BruceBeatDownHitState::IsTransitionNeeded(float curDist)
{
    return curDist > RADIO::BRUCE_TRANSITION_DIST + 0.5f;
}

// BruceTransitionState ==============================================

BruceTransitionState::BruceTransitionState(Context* context, Character* c) : 
    PlayerTransitionState(context, c)
{
    SetMotion("BM_Combat/Into_Takedown");
    RADIO::BRUCE_TRANSITION_DIST = motion->endDistance;

    if(RADIO::d_log)
        Log::Write(-1," Bruce-Transition Dist=" + String(RADIO::BRUCE_TRANSITION_DIST) +"\n");
}

BruceTransitionState::~BruceTransitionState()
{}

// BruceSlideInState ==============================================

BruceSlideInState::BruceSlideInState(Context* context, Character* c) : 
    PlayerSlideInState(context, c)
{
    SetMotion("BM_Climb/Slide_Floor_In");
}

BruceSlideInState::~BruceSlideInState()
{}

// BruceSlideOutState ==============================================

BruceSlideOutState::BruceSlideOutState(Context* context, Character* c) : 
    PlayerSlideOutState(context, c)
{
    String preFix = "BM_Climb/";
    AddMotion(preFix + "Slide_Floor_Stop");
    AddMotion(preFix + "Slide_Floor_Out");
}

BruceSlideOutState::~BruceSlideOutState()
{}

// BruceCrouchState ==============================================

BruceCrouchState::BruceCrouchState(Context* context, Character* c) : 
    PlayerCrouchState(context, c)
{
//    Log::Write(-1," BruceCrouchState: Constructor\n");
    SetMotion("BM_Movement/Crouch_Idle");
}

BruceCrouchState::~BruceCrouchState()
{}

// BruceCrouchTurnState ==============================================

BruceCrouchTurnState::BruceCrouchTurnState(Context* context, Character* c) : 
    PlayerCrouchTurnState(context, c)
{
    AddMotion("BM_Crouch_Turns/Turn_Right_90");
    AddMotion("BM_Crouch_Turns/Turn_Right_180");
    AddMotion("BM_Crouch_Turns/Turn_Left_90");
}

BruceCrouchTurnState::~BruceCrouchTurnState()
{}

// BruceCrouchMoveState ==============================================

BruceCrouchMoveState::BruceCrouchMoveState(Context* context, Character* c) : 
    PlayerCrouchMoveState(context, c)
{
    SetMotion("BM_Movement/Cover_Run");
    animSpeed = 0.5f;
}

BruceCrouchMoveState::~BruceCrouchMoveState()
{}

// BruceFallState ==============================================

BruceFallState::BruceFallState(Context* context, Character* c) : 
    PlayerFallState(context, c)
{
    SetMotion("BM_Movement/Fall");
}

BruceFallState::~BruceFallState()
{}

// BruceLandState ==============================================

BruceLandState::BruceLandState(Context* context, Character* c) : 
    PlayerLandState(context, c)
{
    SetMotion("BM_Movement/Land");
}

BruceLandState::~BruceLandState()
{}

// BruceCoverState ==============================================

BruceCoverState::BruceCoverState(Context* context, Character* c) : 
    PlayerCoverState(context, c)
{
    SetMotion("BM_Movement/Cover_Idle");
}

BruceCoverState::~BruceCoverState()
{}

// BruceCoverRunState ==============================================

BruceCoverRunState::BruceCoverRunState(Context* context, Character* c) : 
    PlayerCoverRunState(context, c)
{
    SetMotion("BM_Movement/Cover_Run");
    animSpeed = 0.5f;
}

BruceCoverRunState::~BruceCoverRunState()
{}

// BruceCoverTransitionState ==============================================

BruceCoverTransitionState::BruceCoverTransitionState(Context* context, Character* c) : 
    PlayerCoverTransitionState(context, c)
{
    SetMotion("BM_Movement/Cover_Transition");
}

BruceCoverTransitionState::~BruceCoverTransitionState()
{}

// BruceClimbOverState ==============================================

BruceClimbOverState::BruceClimbOverState(Context* context, Character* c) : 
    PlayerClimbOverState(context, c)
{
    AddMotion("BM_Climb/Stand_Climb_Over_128_Fall");
    AddMotion("BM_Climb/Stand_Climb_Over_256_Fall");
    //AddMotion("BM_Climb/Stand_Climb_Over_384_Fall");

    AddMotion("BM_Climb/Run_Climb_Over_128_Fall");
    AddMotion("BM_Climb/Run_Climb_Over_256_Fall");
    //AddMotion("BM_Climb/Run_Climb_Over_384_Fall");

    AddMotion("BM_Climb/Stand_Climb_Over_128");
    AddMotion("BM_Climb/Run_Climb_Over_128");

    AddMotion("BM_Climb/Crouch_Jump_128_To_Hang");
    AddMotion("BM_Climb/Crouch_Jump_128_To_Dangle");
    //AddMotion("BM_Climb/Crouch_Jump_128_To_Dangle_Wall");
}

BruceClimbOverState::~BruceClimbOverState()
{}

// BruceClimbUpState ==============================================

BruceClimbUpState::BruceClimbUpState(Context* context, Character* c) : 
    PlayerClimbUpState(context, c)
{
    AddMotion("BM_Climb/Stand_Climb_Up_128");
    AddMotion("BM_Climb/Stand_Climb_Up_256");
    AddMotion("BM_Climb/Stand_Climb_Up_384");
    AddMotion("BM_Climb/Run_Climb_Up_128");
    AddMotion("BM_Climb/Run_Climb_Up_256");
    AddMotion("BM_Climb/Run_Climb_Up_384");
}

BruceClimbUpState::~BruceClimbUpState()
{}

// BruceRailUpState ==============================================

BruceRailUpState::BruceRailUpState(Context* context, Character* c) : 
    PlayerRailUpState(context, c)
{
    AddMotion("BM_Railing/Railing_Climb_Up");
    AddMotion("BM_Railing/Stand_Climb_Onto_256_Railing");
    AddMotion("BM_Railing/Stand_Climb_Onto_384_Railing");
    AddMotion("BM_Railing/Railing_Climb_Up");
    AddMotion("BM_Railing/Run_Climb_Onto_256_Railing");
    AddMotion("BM_Railing/Run_Climb_Onto_384_Railing");
}

BruceRailUpState::~BruceRailUpState()
{}

// BruceRailIdleState ==============================================

BruceRailIdleState::BruceRailIdleState(Context* context, Character* c) : 
    PlayerRailIdleState(context, c)
{
    SetMotion("BM_Railing/Railing_Idle");
}

BruceRailIdleState::~BruceRailIdleState()
{}

// BruceRailTurnState ==============================================

BruceRailTurnState::BruceRailTurnState(Context* context, Character* c) : 
    PlayerRailTurnState(context, c)
{
    AddMotion("BM_Railing/Railing_Idle_Turn_180_Right");
    AddMotion("BM_Railing/Railing_Idle_Turn_180_Left");
}

BruceRailTurnState::~BruceRailTurnState()
{}

// BruceRailDownState ==============================================

BruceRailDownState::BruceRailDownState(Context* context, Character* c) : 
    PlayerRailDownState(context, c)
{
    AddMotion("BM_Railing/Railing_Climb_Down_Forward");
    AddMotion("BM_Railing/Railing_Jump_To_Fall");

    AddMotion("BM_Railing/Railing_To_Hang");
    AddMotion("BM_Railing/Railing_To_Dangle");
    AddMotion("BM_Railing/Railing_To_Dangle_Wall");

    AddMotion("BM_Railing/Railing_To_Hang_128");
    AddMotion("BM_Railing/Railing_To_Dangle_128");
    AddMotion("BM_Railing/Railing_To_Dangle_128_Wall");
}

BruceRailDownState::~BruceRailDownState()
{}
/*
// BruceRailFwdIdleState ==============================================

BruceRailFwdIdleState::BruceRailFwdIdleState(Context* context, Character* c) : 
    PlayerRailFwdIdleState(context, c)
{
    SetMotion("BM_Railing/Railing_Run_Forward_Idle");
}

BruceRailFwdIdleState::~BruceRailFwdIdleState()
{}

// BruceRailRunForwardState ==============================================

BruceRailRunForwardState::BruceRailRunForwardState(Context* context, Character* c) : 
    PlayerRailRunForwardState(context, c)
{
    SetMotion("BM_Railing/Railing_Run_Forward");
}

BruceRailRunForwardState::~BruceRailRunForwardState()
{}
*/
// BruceRailRunTurn180State ==============================================

BruceRailRunTurn180State::BruceRailRunTurn180State(Context* context, Character* c) : 
    PlayerRailTurn180State(context, c)
{
    SetMotion("BM_Railing/Stand_To_Walk_Right_180");
}

BruceRailRunTurn180State::~BruceRailRunTurn180State()
{}

// BruceHangUpState ==============================================

BruceHangUpState::BruceHangUpState(Context* context, Character* c) : 
    PlayerHangUpState(context, c)
{
    AddMotion("BM_Climb/Stand_Climb_Up_256_Hang");
    AddMotion("BM_Climb/Stand_Climb_Up_384_Hang");
    AddMotion("BM_Climb/Run_Climb_Up_256_Hang");
    AddMotion("BM_Climb/Run_Climb_Up_384_Hang");
}

BruceHangUpState::~BruceHangUpState()
{}

// BruceHangIdleState ==============================================

BruceHangIdleState::BruceHangIdleState(Context* context, Character* c) : 
    PlayerHangIdleState(context, c)
{
    AddMotion("BM_Climb/Hang_Left_End");
    //AddMotion("BM_Climb/Hang_Left_End_1");
    AddMotion("BM_Climb/Hang_Right_End");
    //AddMotion("BM_Climb/Hang_Right_End_1");
}

BruceHangIdleState::~BruceHangIdleState()
{}

// BruceHangOverState ==============================================

BruceHangOverState::BruceHangOverState(Context* context, Character* c) : 
    PlayerHangOverState(context, c)
{   
    AddMotion("BM_Climb/Hang_Climb_Up_Run");
    //AddMotion("BM_Climb/Hang_Climb_Up_Rail");
    AddMotion("BM_Climb/Hang_Climb_Up_Over_128");
    //AddMotion("BM_Climb/Hang_Climb_Up_Rail_128");
    AddMotion("BM_Climb/Hang_Jump_Over_128");
    AddMotion("BM_Climb/Hang_Jump_Over_Fall");
}

BruceHangOverState::~BruceHangOverState()
{}

// BruceHangMoveState ==============================================

BruceHangMoveState::BruceHangMoveState(Context* context, Character* c) : 
    PlayerHangMoveState(context, c)
{
    AddMotion("BM_Climb/Hang_Left");
    AddMotion("BM_Climb/Hang_Left_Convex");
    AddMotion("BM_Climb/Hang_Left_Concave");
    //AddMotion("BM_Climb/Hang_Left_1");
    AddMotion("BM_Climb/Hang_Right");
    AddMotion("BM_Climb/Hang_Right_Convex");
    AddMotion("BM_Climb/Hang_Right_Concave");
    //AddMotion("BM_Climb/Hang_Right_1");
}

BruceHangMoveState::~BruceHangMoveState()
{}

// BruceDangleIdleState ==============================================

BruceDangleIdleState::BruceDangleIdleState(Context* context, Character* c) : 
    PlayerDangleIdleState(context, c)
{
    AddMotion("BM_Climb/Dangle_Left_End");
    AddMotion("BM_Climb/Dangle_Right_End");

    //AddMotion("BM_Climb/Dangle_Idle");
    idleAnim = "Animations/BM_Climb/Dangle_Idle.ani";
}

BruceDangleIdleState::~BruceDangleIdleState()
{}

// BruceDangleOverState ==============================================

BruceDangleOverState::BruceDangleOverState(Context* context, Character* c) : 
    PlayerDangleOverState(context, c)
{
    AddMotion("BM_Climb/Dangle_Climb_Up_Run");
    //AddMotion("BM_Climb/Dangle_Climb_Up_Rail");
    AddMotion("BM_Climb/Dangle_Climb_Up_Over_128");
    //AddMotion("BM_Climb/Dangle_Climb_Up_Rail_128");
    AddMotion("BM_Climb/Hang_Jump_Over_128");
    AddMotion("BM_Climb/Hang_Jump_Over_Fall");
}

BruceDangleOverState::~BruceDangleOverState()
{}

// BruceDangleMoveState ==============================================

BruceDangleMoveState::BruceDangleMoveState(Context* context, Character* c) : 
    PlayerDangleMoveState(context, c)
{
    AddMotion("BM_Climb/Dangle_Left");
    AddMotion("BM_Climb/Dangle_Convex_90_L");
    AddMotion("BM_Climb/Dangle_Concave_90_L");
    AddMotion("BM_Climb/Dangle_Right");
    AddMotion("BM_Climb/Dangle_Convex_90_R");
    AddMotion("BM_Climb/Dangle_Concave_90_R");
}

BruceDangleMoveState::~BruceDangleMoveState()
{}

// BruceClimbDownState ==============================================

BruceClimbDownState::BruceClimbDownState(Context* context, Character* c) : 
    PlayerClimbDownState(context, c)
{
    //AddMotion("BM_Climb/Walk_Climb_Down_128");
    //AddMotion("BM_Climb/Run_Climb_Down_128");
    //AddMotion("BM_Climb/Crouch_Down_128");

    AddMotion("BM_Climb/Crouch_To_Hang");
    AddMotion("BM_Climb/Crouch_To_Dangle");
    //AddMotion("BM_Climb/Crouch_To_Dangle_Wall");
}

BruceClimbDownState::~BruceClimbDownState()
{}

// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// BRUCE
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

Bruce::Bruce(Context* context) :
    Player(context)
{
//    MotionManager* mm = RADIO::g_mm;
//    walkAlignAnimation = mm->GetAnimationName("BM_Movement/Walk");
    walkAlignAnimation = "Animations/BM_Movement/Walk_Forward.ani"; // Character member (String)
//    Log::Write(-1," Bruce: Constructor\n");
}

Bruce::~Bruce()
{}

void Bruce::AddStates()
{
//    Player::AddStates(); // empty

    if(RADIO::d_log)
        Log::Write(-1," Bruce: AddStates\n");

    stateMachine->AddState(new BruceStandState(context_, this));
    stateMachine->AddState(new BruceTurnState(context_, this));
    stateMachine->AddState(new BruceWalkState(context_, this));
    stateMachine->AddState(new BruceRunState(context_, this));
    stateMachine->AddState(new BruceRunToStandState(context_, this));
    stateMachine->AddState(new BruceRunTurn180State(context_, this));
    stateMachine->AddState(new BruceEvadeState(context_, this));
    stateMachine->AddState(new CharacterAlignState(context_, this));
    stateMachine->AddState(new AnimationTestState(context_, this));
    stateMachine->AddState(new BruceStandToWalkState(context_, this));
    stateMachine->AddState(new BruceStandToRunState(context_, this));

    stateMachine->AddState(new BruceFallState(context_, this));
    stateMachine->AddState(new BruceLandState(context_, this));

    if (RADIO::game_type == 0)
    {
        stateMachine->AddState(new BruceAttackState(context_, this));
        stateMachine->AddState(new BruceCounterState(context_, this));
        stateMachine->AddState(new BruceHitState(context_, this));
        stateMachine->AddState(new BruceDeadState(context_, this));
//        stateMachine->AddState(new BruceBeatDownHitState(context_, this));
//        stateMachine->AddState(new BruceBeatDownEndState(context_, this));
        stateMachine->AddState(new BruceTransitionState(context_, this));
    }
    else if (RADIO::game_type == 1)
    {
        stateMachine->AddState(new BruceSlideInState(context_, this));
        stateMachine->AddState(new BruceSlideOutState(context_, this));
        stateMachine->AddState(new BruceCrouchState(context_, this));
        stateMachine->AddState(new BruceCrouchTurnState(context_, this));
        stateMachine->AddState(new BruceCrouchMoveState(context_, this));
        stateMachine->AddState(new BruceCoverState(context_, this));
        stateMachine->AddState(new BruceCoverRunState(context_, this));
        stateMachine->AddState(new BruceCoverTransitionState(context_, this));
        // climb
        stateMachine->AddState(new BruceClimbOverState(context_, this));
        stateMachine->AddState(new BruceClimbUpState(context_, this));
        stateMachine->AddState(new BruceClimbDownState(context_, this));

        // rail states
        stateMachine->AddState(new BruceRailUpState(context_, this));
        stateMachine->AddState(new BruceRailIdleState(context_, this));
        stateMachine->AddState(new BruceRailTurnState(context_, this));
        stateMachine->AddState(new BruceRailDownState(context_, this));
////            stateMachine.AddState(BruceRailFwdIdleState(this));
////            stateMachine.AddState(BruceRailRunForwardState(this));
        stateMachine->AddState(new BruceRailRunTurn180State(context_, this));

        // hang
        stateMachine->AddState(new BruceHangUpState(context_, this));
        stateMachine->AddState(new BruceHangIdleState(context_, this));
        stateMachine->AddState(new BruceHangOverState(context_, this));
        stateMachine->AddState(new BruceHangMoveState(context_, this));
        // dangle
        stateMachine->AddState(new BruceDangleIdleState(context_, this));
        stateMachine->AddState(new BruceDangleOverState(context_, this));
        stateMachine->AddState(new BruceDangleMoveState(context_, this));
    }
    if(RADIO::d_log)
        Log::Write(-1," Bruce: AddStates End\n");
}


















