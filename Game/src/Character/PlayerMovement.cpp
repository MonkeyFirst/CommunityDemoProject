#include <Urho3D/Urho3DAll.h>

#include "Character/Player.h"
#include "Character/PlayerMovement.h"

#include "Objects/InputManager.h"
#include "Motions/MotionManager.h"

#include "radio.h"

// PlayerStandState  =============================================

PlayerStandState::PlayerStandState(Context* context, Character* c) :
    MultiAnimationState(context, c)
{
    SetName("StandState");
    flags = FLAGS_ATTACK;
    looped = true;
}

PlayerStandState::~PlayerStandState()
{

}

void PlayerStandState::Enter(State* lastState)
{
    ownner->SetTarget(NULL);
    ownner->SetVelocity(Vector3(0,0,0));
    MultiAnimationState::Enter(lastState);
}

void PlayerStandState::Update(float dt)
{
    InputManager* gInput = RADIO::g_im;
    if (!gInput->IsLeftStickInDeadZone() && gInput->IsLeftStickStationary())
    {
        int index = ownner->RadialSelectAnimation(4);
        ownner->GetNode()->SetVar(ANIMATION_INDEX, index -1);

        if(RADIO::d_log)
            Log::Write(-1," " + this->name +" Stand->Move|Turn index = " + String(index) + " hold-frames = " 
            + String(gInput->GetLeftAxisHoldingFrames()) + " hold-time = " + String(gInput->GetLeftAxisHoldingTime())+"\n");

        if (index == 0)
            ownner->ChangeState(gInput->IsRunHolding() ? "RunState" : "WalkState");
        else
            ownner->ChangeState(gInput->IsRunHolding() ? "StandToRunState" : "StandToWalkState");
        //ownner.ChangeState("TurnState");

        return;
    }

    if (ownner->CheckFalling())
        return;

    if (ownner->CheckDocking())
        return;

    if (ownner->ActionCheck(true, true, true, true))
        return;
    
    if (timeInState > 0.25f && gInput->IsCrouchDown())
        ownner->ChangeState("CrouchState");

    MultiAnimationState::Update(dt);
}

int PlayerStandState::PickIndex()
{
    int i = animations.Size();
    return Random(i);
}

// PlayerEvadeState  =============================================

PlayerEvadeState::PlayerEvadeState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    SetName("EvadeState");
}

PlayerEvadeState::~PlayerEvadeState()
{}

void PlayerEvadeState::Enter(State* lastState)
{
    ownner->GetNode()->SetVar(ANIMATION_INDEX, ownner->RadialSelectAnimation(4));
    MultiMotionState::Enter(lastState);
}

// PlayerTurnState  =============================================

PlayerTurnState::PlayerTurnState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    dockDist = -1;
    SetName("TurnState");
    flags = FLAGS_ATTACK;
}

PlayerTurnState::~PlayerTurnState()
{}

void PlayerTurnState::Update(float dt)
{
    if (dockDist > 0 && timeInState > 0.2f)
    {
        if (ownner->CheckDocking(dockDist))
            return;
    }
    ownner->motion_deltaRotation += turnSpeed * dt;
    MultiMotionState::Update(dt);
}

void PlayerTurnState::Enter(State* lastState)
{
    // Player класс отработает и если NULL, то не будет вызвана аналогичная функция родителя (класса Character)
    ownner->SetTarget(NULL); 
    CaculateTargetRotation(); // 0.0f если нет цели.
    MultiMotionState::Enter(lastState);
    Motion* motion = motions[selectIndex];

    float alignTime = motion->endTime;
    float motionTargetAngle = motion->GetFutureRotation(ownner, alignTime);
    float diff = RADIO::AngleDiff(targetRotation - motionTargetAngle);
    turnSpeed = diff / alignTime;
    combatReady = true;

    if(RADIO::d_log)
        Log::Write(-1," "+ this->name + " motionTargetAngle=" + String(motionTargetAngle) + " targetRotation=" + String(targetRotation) + " diff=" + String(diff) + " turnSpeed=" + String(turnSpeed)+"\n");
}

void PlayerTurnState::OnMotionFinished()
{
    ownner->GetNode()->SetWorldRotation(Quaternion(0, targetRotation, 0));
    MultiMotionState::OnMotionFinished();
}

void PlayerTurnState::CaculateTargetRotation()
{
    targetRotation = ownner->GetTargetAngle();
}

void PlayerTurnState::DebugDraw(DebugRenderer* debug)
{
    RADIO::DebugDrawDirection(debug, ownner->GetNode()->GetWorldPosition(), targetRotation, Color::YELLOW, 2.0f);
    MultiMotionState::DebugDraw(debug);
}

// PlayerStandToWalkState  =============================================

PlayerStandToWalkState::PlayerStandToWalkState(Context* context, Character* c) :
    PlayerTurnState(context, c)
{
    SetName("StandToWalkState");
    flags = FLAGS_ATTACK | FLAGS_MOVING;
    dockDist = 3.0f;
}

PlayerStandToWalkState::~PlayerStandToWalkState()
{}

// PlayerStandToRunState  =============================================

PlayerStandToRunState::PlayerStandToRunState(Context* context, Character* c) :
    PlayerTurnState(context, c)
{
    SetName("StandToRunState");
    flags = FLAGS_ATTACK | FLAGS_MOVING;// INFO: + FLAGS_RUN in new script version.
    dockDist = 6.0f;
}

PlayerStandToRunState::~PlayerStandToRunState()
{}

// PlayerMoveForwardState  =============================================

PlayerMoveForwardState::PlayerMoveForwardState(Context* context, Character* c) :
    SingleMotionState(context, c)
{
    flags = FLAGS_ATTACK | FLAGS_MOVING;
    turnSpeed = 5.0f;
    dockDist = 3.0f;
}

PlayerMoveForwardState::~PlayerMoveForwardState()
{}

void PlayerMoveForwardState::Enter(State* lastState)
{
    SingleMotionState::Enter(lastState);
    ownner->SetTarget(NULL);
    combatReady = true;
}

void PlayerMoveForwardState::OnStop()
{

}

void PlayerMoveForwardState::OnTurn180()
{

}

void PlayerMoveForwardState::Update(float dt)
{
    InputManager* gInput = RADIO::g_im;
    float characterDifference = ownner->ComputeAngleDiff();
    Node* _node = ownner->GetNode();
    _node->Yaw(characterDifference * turnSpeed * dt);
    // if the difference is large, then turn 180 degrees
    if ( (Abs(characterDifference) > FULLTURN_THRESHOLD) && gInput->IsLeftStickStationary() )
    {
        if(RADIO::d_log)
            Log::Write(-1," "+ this->name + " turn 180!!!\n");
        OnTurn180();
        return;
    }

    if (gInput->IsLeftStickInDeadZone() && gInput->HasLeftStickBeenStationary(0.1f))
    {
        OnStop();
        return;
    }

    if (ownner->CheckFalling())
        return;
    if (ownner->CheckDocking(dockDist))
        return;
    if (ownner->ActionCheck(true, true, true, true))
        return;

    SingleMotionState::Update(dt);
}

// PlayerWalkState  =============================================

PlayerWalkState::PlayerWalkState(Context* context, Character* c) :
    PlayerMoveForwardState(context, c)
{
    SetName("WalkState");
    turnSpeed = 5.0f;
    runHoldingFrames = 0;
}

PlayerWalkState::~PlayerWalkState()
{}

void PlayerWalkState::OnStop()
{
    ownner->ChangeState("StandState");
}

void PlayerWalkState::OnTurn180()
{
    ownner->GetNode()->SetVar(ANIMATION_INDEX, 1);
    ownner->ChangeState("StandToWalkState");
}

void PlayerWalkState::Update(float dt)
{
    InputManager* gInput = RADIO::g_im;
    if (gInput->IsCrouchDown())
    {
        ownner->ChangeState("CrouchMoveState");
        return;
    }

    if (gInput->IsRunHolding())
        runHoldingFrames ++;
    else
        runHoldingFrames = 0;

    if (runHoldingFrames > 4)
    {
        ownner->ChangeState("RunState");
        return;
    }

    PlayerMoveForwardState::Update(dt);
}

// PlayerRunState  =============================================

PlayerRunState::PlayerRunState(Context* context, Character* c) :
    PlayerMoveForwardState(context, c)
{
    SetName("RunState");
    turnSpeed = 7.5f;
    dockDist = 6.0f;
    flags |= FLAGS_RUN;
    walkHoldingFrames = 0;
    maxWalkHoldFrames = 4;
}

PlayerRunState::~PlayerRunState()
{}

void PlayerRunState::OnStop()
{
    ownner->ChangeState("RunToStandState");
}

void PlayerRunState::OnTurn180()
{
    ownner->ChangeState("RunTurn180State");
}

void PlayerRunState::Update(float dt)
{
    if (!RADIO::g_im->IsRunHolding())
        walkHoldingFrames ++;
    else
        walkHoldingFrames = 0;

    if (walkHoldingFrames > maxWalkHoldFrames)
    {
        ownner->ChangeState("WalkState");
        return;
    }

    PlayerMoveForwardState::Update(dt);
}

// PlayerRunToStandState  =============================================

PlayerRunToStandState::PlayerRunToStandState(Context* context, Character* c) :
    SingleMotionState(context, c)
{
    SetName("RunToStandState");
    flags = FLAGS_ATTACK | FLAGS_MOVING; // INFO: + FLAGS_RUN in new script version.
}

PlayerRunToStandState::~PlayerRunToStandState()
{}

// PlayerRunTurn180State  =============================================

PlayerRunTurn180State::PlayerRunTurn180State(Context* context, Character* c) :
    SingleMotionState(context, c)
{
    SetName("RunTurn180State");
    flags = FLAGS_ATTACK | FLAGS_MOVING; // INFO: + FLAGS_RUN in new script version.
}

PlayerRunTurn180State::~PlayerRunTurn180State()
{}

void PlayerRunTurn180State::Enter(State* lastState)
{
    SingleMotionState::Enter(lastState);
    targetAngle = RADIO::AngleDiff(ownner->GetTargetAngle());
    float alignTime = motion->endTime;
    // INFO: далее большое отличие от новых скриптов.
    Vector4 tFinnal = motion->GetKey(alignTime);
    Vector4 t1 = motion->GetKey(0.78f);
    float dist = Abs(t1.z_ - tFinnal.z_);
    targetPos = ownner->GetNode()->GetWorldPosition() + Quaternion(0, targetAngle, 0) * Vector3(0, 0, dist);
    float rotation = motion->GetFutureRotation(ownner, alignTime);
    yawPerSec = RADIO::AngleDiff(targetAngle - rotation) / alignTime;
    Vector3 v = motion->GetFuturePosition(ownner, alignTime);
    ownner->motion_velocity = (targetPos - v) / alignTime;
}

void PlayerRunTurn180State::Update(float dt)
{
    ownner->motion_deltaRotation += yawPerSec * dt;
    SingleMotionState::Update(dt);
}

void PlayerRunTurn180State::DebugDraw(DebugRenderer* debug)
{
    SingleMotionState::DebugDraw(debug);
    RADIO::DebugDrawDirection(debug, ownner->GetNode()->GetWorldPosition(), targetAngle, Color(0.75f, 0.5f, 0.45f), 4.0f);
    debug->AddCross(targetPos, 0.5f, Color::YELLOW, false);
}

// PlayerSlideInState  =============================================

PlayerSlideInState::PlayerSlideInState(Context* context, Character* c) :
    SingleMotionState(context, c)
{
    state = 0;
    slideTimer = 1.0f;
    idleVelocity = Vector3(0, 0, 10);
    SetName("SlideInState");
    flags = FLAGS_ATTACK | FLAGS_MOVING;
}

PlayerSlideInState::~PlayerSlideInState()
{}

void PlayerSlideInState::OnMotionFinished()
{
    state = 1;
    timeInState = 0.0f;
}

void PlayerSlideInState::Update(float dt)
{
    if (state == 0)
    {
        SingleMotionState::Update(dt);
    }
    else if (state == 1)
    {
        if (timeInState >= slideTimer)
        {
            int index = 0;
            InputManager* gInput = RADIO::g_im;
            if (!gInput->IsLeftStickInDeadZone() && gInput->IsLeftStickStationary() && gInput->IsRunHolding())
            {
                index = 1;
            }
            ownner->GetNode()->SetVar(ANIMATION_INDEX, index);
            ownner->ChangeState("SlideOutState");
            return;
        }
        if (ownner->physicsType == 0)
        {
            Vector3 oldPos = ownner->GetNode()->GetWorldPosition();
            oldPos += (ownner->GetNode()->GetWorldRotation() * idleVelocity * dt);
            ownner->MoveTo(oldPos, dt);
        }
        else
            ownner->SetVelocity(ownner->GetNode()->GetWorldRotation() * idleVelocity);

        CharacterState::Update(dt);
    }
}

void PlayerSlideInState::Enter(State* lastState)
{
    SingleMotionState::Enter(lastState);
    ownner->SetHeight(CHARACTER_CROUCH_HEIGHT);
}

void PlayerSlideInState::Exit(State* nextState)
{
    SingleMotionState::Exit(nextState);
    ownner->SetHeight(CHARACTER_HEIGHT);
}

// PlayerSlideOutState  =============================================

PlayerSlideOutState::PlayerSlideOutState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    SetName("SlideOutState");
    flags = FLAGS_ATTACK | FLAGS_MOVING;
}

PlayerSlideOutState::~PlayerSlideOutState()
{}

void PlayerSlideOutState::OnMotionFinished()
{
    if (selectIndex == 1)
    {
        ownner->ChangeState("RunState");
        return;
    }
    MultiMotionState::OnMotionFinished();
}

// PlayerCrouchState  =============================================

PlayerCrouchState::PlayerCrouchState(Context* context, Character* c) :
    SingleAnimationState(context, c)
{
    SetName("CrouchState");
    flags = FLAGS_ATTACK;
    looped = true;
}

PlayerCrouchState::~PlayerCrouchState()
{}

void PlayerCrouchState::Enter(State* lastState)
{
    ownner->SetTarget(NULL);
    ownner->SetVelocity(Vector3(0,0,0));
    ownner->SetHeight(CHARACTER_CROUCH_HEIGHT);
    SingleAnimationState::Enter(lastState);
}

void PlayerCrouchState::Exit(State* nextState)
{
    SingleAnimationState::Exit(nextState);
    ownner->SetHeight(CHARACTER_HEIGHT);
}

void PlayerCrouchState::Update(float dt)
{
    InputManager* gInput = RADIO::g_im;
    if (timeInState > 0.25f && !gInput->IsCrouchDown())
    {
        ownner->ChangeState("StandState");
        return;
    }
    if (ownner->CheckFalling())
        return;
    if (ownner->CheckDocking())
        return;

    if (!gInput->IsLeftStickInDeadZone() && gInput->IsLeftStickStationary())
    {
        int index = ownner->RadialSelectAnimation(4);
        ownner->GetNode()->SetVar(ANIMATION_INDEX, index -1);

        if(RADIO::d_log)
            Log::Write(-1," "+ this->name + " Crouch->Move|Turn hold-frames=" + String(gInput->GetLeftAxisHoldingFrames()) + " hold-time=" + String(gInput->GetLeftAxisHoldingTime())+"\n");

        if (index == 0)
            ownner->ChangeState("CrouchMoveState");
        else
            ownner->ChangeState("CrouchTurnState");
    }

    SingleAnimationState::Update(dt);
}

// PlayerCrouchTurnState  =============================================

PlayerCrouchTurnState::PlayerCrouchTurnState(Context* context, Character* c) :
    PlayerTurnState(context, c)
{
    SetName("CrouchTurnState");
    dockDist = 3;
}

PlayerCrouchTurnState::~PlayerCrouchTurnState()
{}

void PlayerCrouchTurnState::Enter(State* lastState)
{
    PlayerTurnState::Enter(lastState);
    ownner->SetHeight(CHARACTER_CROUCH_HEIGHT);
}

void PlayerCrouchTurnState::Exit(State* nextState)
{
    PlayerTurnState::Exit(nextState);
    ownner->SetHeight(CHARACTER_HEIGHT);
}

// PlayerCrouchMoveState  =============================================

PlayerCrouchMoveState::PlayerCrouchMoveState(Context* context, Character* c) :
    PlayerMoveForwardState(context, c)
{
    SetName("CrouchMoveState");
    turnSpeed = 2.0f;
}

PlayerCrouchMoveState::~PlayerCrouchMoveState()
{}

void PlayerCrouchMoveState::OnTurn180()
{
    ownner->GetNode()->SetVar(ANIMATION_INDEX, 1);
    ownner->ChangeState("CrouchTurnState");
}

void PlayerCrouchMoveState::OnStop()
{
    ownner->ChangeState("CrouchState");
}

void PlayerCrouchMoveState::Enter(State* lastState)
{
    PlayerMoveForwardState::Enter(lastState);
    ownner->SetHeight(CHARACTER_CROUCH_HEIGHT);
}

void PlayerCrouchMoveState::Exit(State* nextState)
{
    PlayerMoveForwardState::Exit(nextState);
    ownner->SetHeight(CHARACTER_HEIGHT);
}

// PlayerFallState  =============================================

PlayerFallState::PlayerFallState(Context* context, Character* c) :
    SingleAnimationState(context, c)
{
    SetName("FallState");
    flags = FLAGS_ATTACK;
}

PlayerFallState::~PlayerFallState()
{}

void PlayerFallState::Update(float dt)
{
    // используется здесь и ниже в этой функции нужно для адекватной работы static_cast из-за SharedPtr<Character> ownner; в CharacterStates.h
    Character* chr = ownner;
    Player* p = static_cast<Player*>(chr);
    if (p->sensor->grounded)
    {
        ownner->ChangeState("LandState");
        return;
    }
    SingleAnimationState::Update(dt);
}

void PlayerFallState::OnMotionFinished()
{}

// PlayerLandState  =============================================

PlayerLandState::PlayerLandState(Context* context, Character* c) :
    SingleAnimationState(context, c)
{
    SetName("LandState");
    flags = FLAGS_ATTACK;
}

PlayerLandState::~PlayerLandState()
{}

void PlayerLandState::Enter(State* lastState)
{
    ownner->SetVelocity(Vector3(0, 0, 0));
    SingleAnimationState::Enter(lastState);
}

// PlayerCoverState  =============================================

PlayerCoverState::PlayerCoverState(Context* context, Character* c) :
    SingleAnimationState(context, c)
{
    SetName("CoverState");
    alignTime = 0.25f;
    yawAdjustSpeed = 15.0f;
    looped = true;
    blendTime = alignTime;
}

PlayerCoverState::~PlayerCoverState()
{}

void PlayerCoverState::Enter(State* lastState)
{
    ownner->SetVelocity(Vector3(0, 0, 0));

    Line* l = ownner->dockLine;
    Vector3 proj = l->Project(ownner->GetNode()->GetWorldPosition());
    dockPosition = proj;

    if (!lastState->name.StartsWith("Cover"))
    {
        alignTime = 0.4f;

        float to_start = (proj - l->ray.origin_).LengthSquared();
        float to_end = (proj - l->end).LengthSquared();
        float min_dist_sqr = COLLISION_RADIUS * COLLISION_RADIUS;
        int head = l->GetHead(ownner->GetNode()->GetWorldRotation());

        Vector3 dir;
        if (head == 1)
        {
            dir = l->ray.origin_ - l->end;
            if (to_start < min_dist_sqr)
                dockPosition = l->ray.origin_ - dir.Normalized() * COLLISION_RADIUS;
            else if (to_end < min_dist_sqr)
                dockPosition = l->end + dir.Normalized() * COLLISION_RADIUS;
        }
        else
        {
            dir = l->end - l->ray.origin_;
            if (to_start < min_dist_sqr)
                dockPosition = l->ray.origin_ + dir.Normalized() * COLLISION_RADIUS;
            else if (to_end < min_dist_sqr)
               dockPosition = l->end - dir.Normalized() * COLLISION_RADIUS;
        }
        dockDirection = Atan2(dir.x_, dir.z_);
    }
    else
    {
        alignTime = 0.1f;
        dockDirection = l->GetHeadDirection(ownner->GetNode()->GetWorldRotation());
    }

    dockPosition.y_ = ownner->GetNode()->GetWorldPosition().y_;
    Vector3 diff = dockPosition - ownner->GetNode()->GetWorldPosition();
    ownner->SetVelocity(diff / alignTime);
    startYaw = RADIO::AngleDiff(ownner->GetNode()->GetWorldRotation().EulerAngles().y_);
    yawPerSec = RADIO::AngleDiff(dockDirection - startYaw) / alignTime;
    state = 0;

    SingleAnimationState::Enter(lastState);
}

void PlayerCoverState::Exit(State* nextState)
{
    SingleAnimationState::Exit(nextState);
}

void PlayerCoverState::DebugDraw(DebugRenderer* debug)
{
    debug->AddCross(dockPosition, 0.5f, Color::RED, false);
    RADIO::DebugDrawDirection(debug, dockPosition, dockDirection, Color::YELLOW, 2.0f);
}

void PlayerCoverState::Update(float dt)
{
    if (state == 0)
    {
        if (timeInState >= alignTime)
        {
            state = 1;
            ownner->SetVelocity(Vector3(0, 0, 0));
        }

        startYaw += yawPerSec * dt;
        ownner->GetNode()->SetWorldRotation(Quaternion(0, startYaw, 0));
    }
    else if (state == 1)
    {
        InputManager* gInput = RADIO::g_im;
        if (!gInput->IsLeftStickInDeadZone() && gInput->IsLeftStickStationary())
        {
            float characterDifference = ownner->ComputeAngleDiff();
            if ( (Abs(characterDifference) > 135) && gInput->IsLeftStickStationary() )
            {
                ownner->ChangeState("CoverTransitionState");
                return;
            }
            else
            {
                float faceDiff = ownner->dockLine->GetProjectFacingDir(ownner->GetNode()->GetWorldPosition(), ownner->GetTargetAngle());
                
                if(RADIO::d_log)
                    Log::Write(-1," "+ this->name + "CoverState faceDiff=" + String(faceDiff)+"\n");
                if (faceDiff > 145)
                    ownner->ChangeState("WalkState");
                else if (faceDiff > 45)
                    ownner->ChangeState("CoverRunState");
            }
        }

        float curAngle = ownner->GetCharacterAngle();
        float diff = RADIO::AngleDiff(dockDirection - curAngle);
        ownner->GetNode()->Yaw(diff * yawAdjustSpeed * dt);
    }
    SingleAnimationState::Update(dt);
}

// PlayerCoverRunState  =============================================

PlayerCoverRunState::PlayerCoverRunState(Context* context, Character* c) :
    SingleMotionState(context, c)
{
    SetName("CoverRunState");
    flags = FLAGS_MOVING;
}

PlayerCoverRunState::~PlayerCoverRunState()
{}

void PlayerCoverRunState::Update(float dt)
{
    Vector3 proj = ownner->dockLine->Project(ownner->GetNode()->GetWorldPosition());
    dockPosition = proj;
    dockPosition.y_ = ownner->GetNode()->GetWorldPosition().y_;

    if (!ownner->dockLine->IsProjectPositionInLine(proj))
    {
        ownner->CommonStateFinishedOnGroud();
        return;
    }
    InputManager* gInput = RADIO::g_im;
    if (gInput->IsLeftStickInDeadZone() && gInput->HasLeftStickBeenStationary(0.1f))
    {
        ownner->ChangeState("CoverState");
        return;
    }

    float characterDifference = ownner->ComputeAngleDiff();
    if ( (Abs(characterDifference) > 135) && gInput->IsLeftStickStationary() )
    {
        ownner->ChangeState("CoverTransitionState");
        return;
    }
    else
    {
        float faceDiff = ownner->dockLine->GetProjectFacingDir(ownner->GetNode()->GetWorldPosition(), ownner->GetTargetAngle());
        if (faceDiff > 145)
            ownner->ChangeState("WalkState");
    }

    SingleMotionState::Update(dt);
}

void PlayerCoverRunState::DebugDraw(DebugRenderer* debug)
{
    debug->AddCross(dockPosition, 0.5f, Color::RED, false);
}

// PlayerCoverTransitionState  =============================================

PlayerCoverTransitionState::PlayerCoverTransitionState(Context* context, Character* c) :
    SingleMotionState(context, c)
{
    SetName("CoverTransitionState");
}

PlayerCoverTransitionState::~PlayerCoverTransitionState()
{}

void PlayerCoverTransitionState::Update(float dt)
{
    ownner->motion_deltaRotation += yawPerSec * dt;
    SingleMotionState::Update(dt);
}

void PlayerCoverTransitionState::Enter(State* lastState)
{
    SingleMotionState::Enter(lastState);

    float curAngle = ownner->GetNode()->GetWorldRotation().EulerAngles().y_;
    float targetAngle = RADIO::AngleDiff(curAngle + 180);
    float alignTime = motion->endTime;
    float motionAngle = motion->GetFutureRotation(ownner, alignTime);
    yawPerSec = RADIO::AngleDiff(targetAngle - motionAngle) / alignTime;
}

void PlayerCoverTransitionState::OnMotionFinished()
{
    ownner->ChangeState("CoverState");
}

// PlayerDockAlignState  =============================================

PlayerDockAlignState::PlayerDockAlignState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    motionFlagAfterAlign = 0;
    motionFlagBeforeAlign = kMotion_ALL;
    alignTime = 0.1f;
    dockInTargetBound = 0.25f;
    dockInCheckThinWall = true;
    debug = false;
    physicsType = 0;
}

PlayerDockAlignState::~PlayerDockAlignState()
{}

void PlayerDockAlignState::Update(float dt)
{
    ownner->motion_deltaRotation += turnSpeed * dt;
    MultiMotionState::Update(dt);
}

void PlayerDockAlignState::OnMotionAlignTimeOut()
{
    if (dockBlendingMethod > 0)
    {
        turnSpeed = 0;
        ownner->motion_velocity = Vector3(0, 0, 0);

        Motion* m = motions[selectIndex];
        // TODO: motionFlagAfterAlign != 0
        if (motionFlagAfterAlign && !(m->dockAlignBoneName == String::EMPTY))
        {
            motionRotation = m->GetFutureRotation(ownner, m->endTime);
            motionPositon = m->GetFuturePosition(ownner, m->endTime);

            targetRotation = PickDockOutRotation();
            targetPosition = PickDockOutTarget();

            float t = m->endTime - m->dockAlignTime;
            Vector3 diff = (targetPosition - motionPositon) / t;
            Vector3 v(0, 0, 0);
            if (motionFlagAfterAlign & kMotion_X)// TODO: != 0
                v.x_ = diff.x_;
            if (motionFlagAfterAlign & kMotion_Y)// TODO: != 0
                v.y_ = diff.y_;
            if (motionFlagAfterAlign & kMotion_Z)// TODO: != 0
                v.z_ = diff.z_;
            ownner->motion_velocity = v;

            if (motionFlagAfterAlign & kMotion_R)// TODO: != 0
                turnSpeed = RADIO::AngleDiff(targetRotation - motionRotation) / t;

            if(RADIO::d_log)
                Log::Write(-1," "+ this->name + " animation:" + m->name + " OnMotionAlignTimeOut vel=" + v.ToString() + " turnSpeed=" + String(turnSpeed)+"\n");
        }

        if (debug)
            ownner->SetSceneTimeScale(0.0);
    }
}

int PlayerDockAlignState::PickTargetMotionByHeight(State* lastState, int numOfStandAnimations)
{
    if (lastState->nameHash == ALIGN_STATE)
        return 0;

    int index = 0, startIndex = 0;
    if (lastState->nameHash == RUN_STATE)
        startIndex = numOfStandAnimations;

    alignTime = (lastState->nameHash != RUN_STATE) ? 0.2f : 0.1f;

    float curHeight = ownner->GetNode()->GetWorldPosition().y_;
    float lineHeight = ownner->dockLine->end.y_;
    float minHeightDiff = 9999;

    for (int i = 0; i < numOfStandAnimations; ++i)
    {
        Motion* m = motions[startIndex + i];
        float motionHeight = curHeight + m->maxHeight + climbBaseHeight;
        float curHeightDiff = Abs(lineHeight - motionHeight);
        
        if(RADIO::d_log)
            Log::Write(-1," "+ this->name + " "  + m->name + " maxHeight=" + String(m->maxHeight) + " heightDiff=" + String(curHeightDiff)+ "\n");

        if (curHeightDiff < minHeightDiff)
        {
            minHeightDiff = curHeightDiff;
            index = startIndex + i;
        }
    }

    ownner->GetNode()->SetVar(ANIMATION_INDEX, index);
    return index;
}

Vector3 PlayerDockAlignState::PickDockOutTarget()
{
    return ownner->dockLine->Project(ownner->GetNode()->GetWorldPosition());
}

Vector3 PlayerDockAlignState::PickDockInTarget()
{
    Line* l = ownner->dockLine;
    Motion* m = motions[selectIndex];
    float t = m->GetDockAlignTime();
    Vector3 v = m->GetDockAlignPositionAtTime(ownner, ownner->GetCharacterAngle(), t);
    v = l->Project(v);
    v = l->FixProjectPosition(v, dockInTargetBound);
    if (dockInCheckThinWall && l->HasFlag(LINE_THIN_WALL))
    {
        Vector3 dir = Quaternion(0, targetRotation, 0) * Vector3(0, 0, -1);
        float dist = Min(l->size.x_, l->size.z_) / 2;
        v += dir.Normalized() * dist;
    }
    return v;
}

float PlayerDockAlignState::PickDockInRotation()
{
    Vector3 v = ownner->GetNode()->GetWorldPosition();
    Vector3 proj = ownner->dockLine->Project(v);
    Vector3 dir = proj - v;
    float r = Atan2(dir.x_, dir.z_);
    if (!ownner->dockLine->IsAngleValid(r))
        r = RADIO::AngleDiff(r + 180);
    return r;
}

float PlayerDockAlignState::PickDockOutRotation()
{
    return PickDockInRotation();
}

void PlayerDockAlignState::DebugDraw(DebugRenderer* debug)
{
    debug->AddCross(targetPosition, 0.5f, Color::BLUE, false);
    debug->AddCross(motionPositon, 0.5f, Color::RED, false);
    Motion* m = motions[selectIndex];
    if (m)
    {
        Vector3 v = (m->dockAlignBoneName == String::EMPTY) ? ownner->GetNode()->GetWorldPosition() : ownner->GetNode()->GetChild(m->dockAlignBoneName, true)->GetWorldPosition();
        debug->AddLine(v, targetPosition, Color(0.25, 0.75, 0.75), false);
    }

    RADIO::DebugDrawDirection(debug, targetPosition, targetRotation, Color::BLUE, 2.0f);
    RADIO::DebugDrawDirection(debug, targetPosition, motionRotation, Color::RED, 2.0f);

    MultiMotionState::DebugDraw(debug);
}

void PlayerDockAlignState::Enter(State* lastState)
{
    if (debug)
        ownner->SetSceneTimeScale(0);

    turnSpeed = 0;

    if (dockBlendingMethod == 1)
    {
        MultiMotionState::Enter(lastState);
        Motion* m = motions[selectIndex];
        targetPosition = ownner->dockLine->Project(ownner->GetNode()->GetWorldPosition());

        float t = m->GetDockAlignTime();
        motionRotation = ownner->GetCharacterAngle(); //m.GetFutureRotation(ownner, t);
        // TODO: kMotion_R != 0
        targetRotation = (motionFlagBeforeAlign & kMotion_R) ? PickDockInRotation() : motionRotation;

        turnSpeed = RADIO::AngleDiff(targetRotation - motionRotation) / t;
        motionPositon = m->GetDockAlignPositionAtTime(ownner, targetRotation, t);
        targetPosition = PickDockInTarget();

        Vector3 vel = (targetPosition - motionPositon) / t;
        Vector3 filterV = Vector3(0, 0, 0);

        if (motionFlagBeforeAlign & kMotion_X)// TODO: != 0
            filterV.x_ = vel.x_;
        if (motionFlagBeforeAlign & kMotion_Y)// TODO: != 0
            filterV.y_ = vel.y_;
        if (motionFlagBeforeAlign & kMotion_Z)// TODO: != 0
            filterV.z_ = vel.z_;

        ownner->motion_velocity = filterV;
        
        if(RADIO::d_log)
            Log::Write(-1," "+ this->name + " animation:" + m->name + " vel=" + ownner->motion_velocity.ToString() + " turnSpeed=" + String(turnSpeed)+"\n");
    }
    else
    {
        if (lastState->nameHash == ALIGN_STATE)
        {
            MultiMotionState::Enter(lastState);

            if (motionFlagAfterAlign)// TODO: != 0
            {
                Motion* motion = motions[selectIndex];
                Vector3 targetPos = ownner->dockLine->Project(ownner->GetNode()->GetWorldPosition());
                float t = motion->endTime;
                Vector3 motionPos = motion->GetFuturePosition(ownner, t);
                Vector3 diff = targetPos - motionPos;
                diff /= t;
                Vector3 v(0, 0, 0);
                if (motionFlagAfterAlign & kMotion_X)// TODO: != 0
                    v.x_ = diff.x_;
                if (motionFlagAfterAlign & kMotion_Y)// TODO: != 0
                    v.y_ = diff.y_;
                if (motionFlagAfterAlign & kMotion_Z)// TODO: != 0
                    v.z_ = diff.z_;
                ownner->motion_velocity = v;
                
                if(RADIO::d_log)
                    Log::Write(-1," " + this->name + " animation:" + motion->name + " after align vel=" + v.ToString()+"\n");
            }
        }
        else
        {
            selectIndex = PickIndex();
            Vector3 myPos = ownner->GetNode()->GetWorldPosition();
            Vector3 proj = ownner->dockLine->Project(myPos);
            proj.y_ = myPos.y_;
            Vector3 dir = proj - myPos;
            float targetAngle = Atan2(dir.x_, dir.z_);
            Vector3 targetPos = proj + Quaternion(0, targetAngle, 0) * targetOffsets[selectIndex];

            // Run-Case
            if (selectIndex >= 3)
            {
                float len_diff = (targetPos - myPos).Length();
                
                if(RADIO::d_log)
                    Log::Write(-1," " + this->name + " selectIndex=" + String(selectIndex) + " len_diff=" + String(len_diff)+"\n");
                
                if (len_diff > 3)
                    selectIndex -= 3;
                targetPos = proj + Quaternion(0, targetAngle, 0) * targetOffsets[selectIndex];
                ownner->GetNode()->SetVar(ANIMATION_INDEX, selectIndex);
            }

            CharacterAlignState* s = static_cast<CharacterAlignState*>(ownner->FindState_Hash(ALIGN_STATE));
            String alignAnim = "";
            // walk
            if (selectIndex == 0)
                alignAnim = ownner->walkAlignAnimation;
            s->Start(this->nameHash, targetPos,  targetAngle, alignTime, 0, alignAnim);

            ownner->ChangeStateQueue(ALIGN_STATE);
        }
    }
}

void PlayerDockAlignState::Exit(State* nextState)
{
    if (debug)
        ownner->SetSceneTimeScale(0);
    MultiMotionState::Exit(nextState);
}

// PlayerClimbOverState  =============================================

PlayerClimbOverState::PlayerClimbOverState(Context* context, Character* c) :
    PlayerDockAlignState(context, c)
{
    SetName("ClimbOverState");
    dockBlendingMethod = 1;
    dockInCheckThinWall = false;
}

PlayerClimbOverState::~PlayerClimbOverState()
{}

void PlayerClimbOverState::Enter(State* lastState)
{
    int index = 0;
    if (!downLine)
    {
        motionFlagAfterAlign = 0;
        index = PickTargetMotionByHeight(lastState);
        if (index == 0 || index == 3)
        {
            Vector3 myPos = ownner->GetNode()->GetWorldPosition();
            Motion* m = motions[index];
            Vector4 motionOut = m->GetKey(m->endTime);
            Vector3 proj = ownner->dockLine->Project(myPos);
            Vector3 dir = proj - myPos;
            float targetRotation = Atan2(dir.x_, dir.z_);
            Vector3 futurePos = Quaternion(0, targetRotation, 0) * Vector3(0, 0, 2.0f) + proj;
            futurePos.y_ = myPos.y_;
            
            // используется здесь и ниже в этой функции нужно для адекватной работы static_cast из-за SharedPtr<Character> ownner; в CharacterStates.h
            Character* chr = ownner;            
            Player* p = static_cast<Player*>(chr);
            groundPos = p->sensor->GetGround(futurePos);
            float height = Abs(groundPos.y_ - myPos.y_);
            if (height < 1.5f)
            {
                if (index == 0)
                    index = 6;
                else
                    index = 7;
                motionFlagAfterAlign = kMotion_Y;
            }
        }
    }
    else
    {
        index = 8;
        if (downLine->HasFlag(LINE_SHORT_WALL))
            index += (Random(2) + 1);
        motionFlagAfterAlign = kMotion_XYZ;
        startPos = downLine->Project(ownner->GetNode()->GetWorldPosition());
    }

    if(RADIO::d_log)
        Log::Write(-1," "+ this->name + " animation index=" + String(index) + "\n");
    ownner->GetNode()->SetVar(ANIMATION_INDEX, index);
    PlayerDockAlignState::Enter(lastState);
}

Vector3 PlayerClimbOverState::PickDockOutTarget()
{
    return (selectIndex >= 8) ? down128Pos : groundPos;
}

float PlayerClimbOverState::PickDockOutRotation()
{
    if (!downLine)
        return PlayerDockAlignState::PickDockOutRotation();
    Vector3 v = ownner->GetNode()->GetWorldPosition();
    Vector3 proj = downLine->Project(v);
    Vector3 dir = v - proj;
    dir.y_ = 0;
    return Atan2(dir.x_, dir.z_);
}

void PlayerClimbOverState::DebugDraw(DebugRenderer* debug)
{
    PlayerDockAlignState::DebugDraw(debug);
    debug->AddCross(groundPos, 0.5f, Color::BLUE, false);
    //debug->AddCross(down128Pos, 0.5f, Color(1, 0, 1), false);
}

void PlayerClimbOverState::OnMotionFinished()
{
    if (selectIndex == 6 || selectIndex == 7)
        PlayerDockAlignState::OnMotionFinished();
    else if (selectIndex == 8)
        ownner->ChangeState("HangIdleState");
    else if (selectIndex == 9 || selectIndex == 10)
        ownner->ChangeState("DangleIdleState");
    else
        ownner->ChangeState("FallState");
}

void PlayerClimbOverState::OnMotionAlignTimeOut()
{
    if (downLine && selectIndex >= 8)
    {
        Vector3 offset;
        if (selectIndex == 8)
            offset = Vector3(0, -3.7, 1.45);
        else
            offset = Vector3(0, -3.7, 1.8);
        down128Pos = startPos + Quaternion(0, targetRotation, 0) * offset;
        ownner->AssignDockLine(downLine);
    }
    PlayerDockAlignState::OnMotionAlignTimeOut();
}

// PlayerClimbUpState  =============================================

PlayerClimbUpState::PlayerClimbUpState(Context* context, Character* c) :
    PlayerDockAlignState(context, c)
{
    SetName("ClimbUpState");
    motionFlagAfterAlign = kMotion_Y;
    dockBlendingMethod = 1;
}

PlayerClimbUpState::~PlayerClimbUpState()
{}

void PlayerClimbUpState::Enter(State* lastState)
{
    PickTargetMotionByHeight(lastState);
    PlayerDockAlignState::Enter(lastState);
}

// PlayerRailUpState  =============================================

PlayerRailUpState::PlayerRailUpState(Context* context, Character* c) : 
    PlayerDockAlignState(context, c)
{
    SetName("RailUpState");
    motionFlagAfterAlign = kMotion_XYZ;
    dockBlendingMethod = 1;
}

PlayerRailUpState::~PlayerRailUpState()
{}

void PlayerRailUpState::Enter(State* lastState)
{
    PickTargetMotionByHeight(lastState);
    PlayerDockAlignState::Enter(lastState);
}

void PlayerRailUpState::OnMotionFinished()
{
    ownner->ChangeState("RailIdleState");
}

// PlayerRailIdleState  =============================================

PlayerRailIdleState::PlayerRailIdleState(Context* context, Character* c) : 
    SingleAnimationState(context, c)
{
    SetName("RailIdleState");
    looped = true;
    physicsType = 0;
}

PlayerRailIdleState::~PlayerRailIdleState()
{}

void PlayerRailIdleState::Enter(State* lastState)
{
    ownner->SetVelocity(Vector3(0,0,0));
    SingleAnimationState::Enter(lastState);
}

void PlayerRailIdleState::Update(float dt)
{
    InputManager* gInput = RADIO::g_im;
    if (!gInput->IsLeftStickInDeadZone() && gInput->IsLeftStickStationary())
    {
        int index = ownner->RadialSelectAnimation(4);
        
        if(RADIO::d_log)
            Log::Write(-1," "+ this->name + " Idle->Turn hold-frames=" + String(gInput->GetLeftAxisHoldingFrames()) + " hold-time=" + String(gInput->GetLeftAxisHoldingTime()) +"\n");

        // TODO: FindState_Hash need string hash constant in CharacterStates.h
        PlayerRailTurnState* s = static_cast<PlayerRailTurnState*>(ownner->FindState("RailTurnState"));
        float turnAngle = 0;
        int animIndex = 0;
        StringHash nextState;

        if (index == 0)
        {
            ownner->ChangeState("RailDownState");
            return;
        }
        else if (index == 1)
        {
            turnAngle = 90;
            animIndex = 0;
            nextState = StringHash("RailFwdIdleState");
        }
        else if (index == 2)
        {
            turnAngle = 180;
            animIndex = 0;
            nextState = StringHash("RailIdleState");
        }
        else if (index == 3)
        {
            turnAngle = -90;
            animIndex = 1;
            nextState = StringHash("RailFwdIdleState");
        }

        s->turnAngle = turnAngle;
        s->nextStateName = nextState;
        ownner->GetNode()->SetVar(ANIMATION_INDEX, animIndex);
        ownner->ChangeState("RailTurnState");

        return;
    }

    SingleAnimationState::Update(dt);
}

// PlayerRailTurnState  =============================================

PlayerRailTurnState::PlayerRailTurnState(Context* context, Character* c) : 
    PlayerTurnState(context, c)
{
    SetName("RailTurnState");
    physicsType = 0;
}

PlayerRailTurnState::~PlayerRailTurnState()
{}

void PlayerRailTurnState::CaculateTargetRotation()
{
    targetRotation = RADIO::AngleDiff(ownner->GetCharacterAngle() + turnAngle);
}

void PlayerRailTurnState::OnMotionFinished()
{
    ownner->GetNode()->SetWorldRotation(Quaternion(0, targetRotation, 0));
    ownner->ChangeState_Hash(nextStateName);
}

// PlayerRailFwdIdleState  =============================================

PlayerRailFwdIdleState::PlayerRailFwdIdleState(Context* context, Character* c) : 
    SingleAnimationState(context, c)
{
    SetName("RailFwdIdleState");
    looped = true;
    physicsType = 0;
}

PlayerRailFwdIdleState::~PlayerRailFwdIdleState()
{}

void PlayerRailFwdIdleState::Update(float dt)
{
    InputManager* gInput = RADIO::g_im;
    if (!gInput->IsLeftStickInDeadZone() && gInput->IsLeftStickStationary())
    {
        int index = ownner->RadialSelectAnimation(4);
        
        if(RADIO::d_log)
            Log::Write(-1," "+ this->name + " Idle->Turn hold-frames=" + String(gInput->GetLeftAxisHoldingFrames()) + " hold-time=" + String(gInput->GetLeftAxisHoldingTime()));

        // TODO: FindState_Hash need string hash constant in CharacterStates.h
        PlayerRailTurnState* s = static_cast<PlayerRailTurnState*>(ownner->FindState("RailTurnState"));
        float turnAngle = 0;
        int animIndex = 0;
        StringHash nextState;

        if (index == 0)
        {
            ownner->ChangeState("RailRunForwardState");
            return;
        }
        else if (index == 2)
        {
            ownner->ChangeState("RailRunTurn180State");
            return;
        }
        else if (index == 1)
        {
            turnAngle = 90;
            animIndex = 0;
            nextState = StringHash("RailIdleState");
        }
        else if (index == 3)
        {
            turnAngle = -90;
            animIndex = 1;
            nextState = StringHash("RailIdleState");
        }

        s->turnAngle = turnAngle;
        s->nextStateName = nextState;
        ownner->GetNode()->SetVar(ANIMATION_INDEX, animIndex);
        ownner->ChangeState("RailTurnState");

        return;
    }

    SingleAnimationState::Update(dt);
}

// PlayerRailDownState  =============================================

PlayerRailDownState::PlayerRailDownState(Context* context, Character* c) : 
    PlayerDockAlignState(context, c)
{
    SetName("RailDownState");
    physicsType = 0;
    dockBlendingMethod = 1;
}

PlayerRailDownState::~PlayerRailDownState()
{}

Vector3 PlayerRailDownState::PickDockInTarget()
{
    if (selectIndex == 0)
        return groundPos;
    else if (selectIndex >= 5)
        return PlayerDockAlignState::PickDockInTarget();
    else
    {
        Line* l = ownner->dockLine;
        Vector3 v = ownner->GetNode()->GetWorldPosition();
        v = l->Project(v);

        Vector3 dir = ownner->GetNode()->GetWorldRotation() * Vector3(0, 0, 1);
        float dist = Min(l->size.x_, l->size.z_) / 2;
        v += dir.Normalized() * dist;

        return v;
    }
}

void PlayerRailDownState::Enter(State* lastState)
{
    int animIndex = 0;
    if (lastState->name == "RailRunForwardState")
    {
        animIndex = 1;
        motionFlagBeforeAlign = 0;
    }
    else
    {
        Character* chr = ownner;
        Player* p = static_cast<Player*>(chr);
        Line* l = p->FindDownLine(ownner->dockLine);

        bool hitForward = p->results[0].body_;
        bool hitDown = p->results[1].body_;
        bool hitBack = p->results[2].body_;
        groundPos = p->results[1].position_;
        float lineToGround = ownner->dockLine->end.y_ - groundPos.y_;

        if(RADIO::d_log)
            Log::Write(-1," "+this->name + " lineToGround=" + String(lineToGround) + " hitForward=" + String(hitForward) + " hitDown=" + String(hitDown) + " hitBack=" + String(hitBack)+"\n");

        if (lineToGround < (HEIGHT_128 + HEIGHT_256) / 2)
        {
            animIndex = 0;
            motionFlagBeforeAlign = kMotion_Y;
        }
        else
        {
            motionFlagBeforeAlign = kMotion_XYZ;
            if (l)
            {
                animIndex = l->HasFlag(LINE_SHORT_WALL) ? (6 + Random(2)) : 5;
                ownner->AssignDockLine(l);
            }
            else
                animIndex = ownner->dockLine->HasFlag(LINE_SHORT_WALL) ? (3 + Random(2)) : 2;
        }
    }

    ownner->GetNode()->SetVar(ANIMATION_INDEX, animIndex);
    PlayerDockAlignState::Enter(lastState);
}

void PlayerRailDownState::OnMotionFinished()
{
    if (selectIndex == 0)
    {
        PlayerDockAlignState::OnMotionFinished();
    }
    else
    {
        if (selectIndex == 1)
            ownner->ChangeState("FallState");
        else if (selectIndex == 2 || selectIndex == 5)
            ownner->ChangeState("HangIdleState");
        else
            ownner->ChangeState("DangleIdleState");
    }
}

void PlayerRailDownState::DebugDraw(DebugRenderer* debug)
{
    debug->AddCross(groundPos, 0.5f, Color::BLACK, false);
    PlayerDockAlignState::DebugDraw(debug);
}

// PlayerRailRunForwardState  =============================================

PlayerRailRunForwardState::PlayerRailRunForwardState(Context* context, Character* c) : 
    SingleMotionState(context, c)
{
    SetName("RailRunForwardState");
    physicsType = 0;
}

PlayerRailRunForwardState::~PlayerRailRunForwardState()
{}

void PlayerRailRunForwardState::Update(float dt)
{
    InputManager* gInput = RADIO::g_im;
    if (gInput->IsLeftStickInDeadZone() && gInput->HasLeftStickBeenStationary(0.1f))
    {
        ownner->ChangeState("RailFwdIdleState");
        return;
    }
    float characterDifference = ownner->ComputeAngleDiff();
    // if the difference is large, then turn 180 degrees
    if ( (Abs(characterDifference) > FULLTURN_THRESHOLD) && gInput->IsLeftStickStationary() )
    {
        ownner->ChangeState("RailRunTurn180State");
        return;
    }

    Vector3 facePoint;
    if (ownner->dockLine->GetHead(ownner->GetNode()->GetWorldRotation()) == 0)
        facePoint = ownner->dockLine->end;
    else
        facePoint = ownner->dockLine->ray.origin_;

    float dist_sqr = (ownner->GetNode()->GetWorldPosition() - facePoint).LengthSquared();
    float max_dist = 1.0f;

    if (dist_sqr < max_dist * max_dist)
    {
        ownner->ChangeState("RailDownState");
        return;
    }

    SingleMotionState::Update(dt);
}

// PlayerRailTurn180State  =============================================

PlayerRailTurn180State::PlayerRailTurn180State(Context* context, Character* c) : 
    SingleMotionState(context, c)
{
    SetName("RailRunTurn180State");
}

PlayerRailTurn180State::~PlayerRailTurn180State()
{}

void PlayerRailTurn180State::Enter(State* lastState)
{
    SingleMotionState::Enter(lastState);
    targetRotation = RADIO::AngleDiff(ownner->GetCharacterAngle() + 180);
    float alignTime = motion->endTime;
    float motionTargetAngle = motion->GetFutureRotation(ownner, alignTime);
    Vector3 motionPos = motion->GetFuturePosition(ownner, alignTime);
    targetPosition = ownner->dockLine->Project(motionPos);
    motionPos.y_ = targetPosition.y_;
    ownner->motion_velocity = (targetPosition - motionPos) / alignTime;

    float diff = RADIO::AngleDiff(targetRotation - motionTargetAngle);
    turnSpeed = diff / alignTime;
    
    if(RADIO::d_log)
        Log::Write(-1," "+ this->name + " motionTargetAngle=" + String(motionTargetAngle) + " targetRotation=" + String(targetRotation) + " diff=" + String(diff) + " turnSpeed=" + String(turnSpeed)+"\n");

    SingleMotionState::Enter(lastState);
}

void PlayerRailTurn180State::Update(float dt)
{
    ownner->motion_deltaRotation += turnSpeed * dt;
    SingleMotionState::Update(dt);
}

void PlayerRailTurn180State::OnMotionFinished()
{
    ownner->GetNode()->SetWorldPosition(targetPosition);
    ownner->GetNode()->SetWorldRotation(Quaternion(0, targetRotation, 0));
    ownner->ChangeState("RailRunForwardState");
}

void PlayerRailTurn180State::DebugDraw(DebugRenderer* debug)
{
    RADIO::DebugDrawDirection(debug, ownner->GetNode()->GetWorldPosition(), targetRotation, Color::YELLOW, 2.0f);
    debug->AddCross(targetPosition, 0.5f, Color::RED, false);
}

// PlayerHangUpState  =============================================

PlayerHangUpState::PlayerHangUpState(Context* context, Character* c) :
    PlayerDockAlignState(context, c)
{
    SetName("HangUpState");
    climbBaseHeight = 3.0f;
    dockBlendingMethod = 1;
}

PlayerHangUpState::~PlayerHangUpState()
{}

void PlayerHangUpState::Enter(State* lastState)
{
    PickTargetMotionByHeight(lastState, 2);
    PlayerDockAlignState::Enter(lastState);
}

void PlayerHangUpState::OnMotionFinished()
{
    ownner->ChangeState("HangIdleState");
}

// PlayerHangIdleState  =============================================

PlayerHangIdleState::PlayerHangIdleState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    SetName("HangIdleState");
    overStateName = StringHash("HangOverState");
    moveStateName = StringHash("HangMoveState");
    moveToLinePtDist = 1.5f;
    turnSpeed = 0.0f;
    alignTime = 0.2f;
    inputCheckTime = 0.2f;
    physicsType = 0;
    blendTime = 0.0f;
}

PlayerHangIdleState::~PlayerHangIdleState()
{}

void PlayerHangIdleState::OnMotionFinished()
{
    if (idleAnim != String::EMPTY)
        ownner->PlayAnimation(idleAnim, LAYER_MOVE, true, 0.0f, startTime, animSpeed);

    ownner->motion_velocity = Vector3(0, 0, 0);
    turnSpeed = 0.0f;
}

void PlayerHangIdleState::DebugDraw(DebugRenderer* debug)
{
    debug->AddCross(targetPosition, 0.5f, Color::BLUE, false);
    debug->AddCross(motionPositon, 0.5f, Color::RED, false);
    Motion* m = motions[selectIndex];
    if (m)
    {
        Vector3 v = (m->dockAlignBoneName == String::EMPTY) ? ownner->GetNode()->GetWorldPosition() : ownner->GetNode()->GetChild(m->dockAlignBoneName, true)->GetWorldPosition();
        debug->AddLine(v, targetPosition, Color(0.25, 0.75, 0.75), false);
    }

    RADIO::DebugDrawDirection(debug, targetPosition, targetRotation, Color::BLUE, 2.0f);
    RADIO::DebugDrawDirection(debug, targetPosition, motionRotation, Color::RED, 2.0f);

    MultiMotionState::DebugDraw(debug);
}

void PlayerHangIdleState::Enter(State* lastState)
{
    ownner->SetVelocity(Vector3(0,0,0));
    state = 0;

    int curAnimationIndex = ownner->GetNode()->GetVars()[ANIMATION_INDEX]->GetInt();
    int index = 0;

    bool fromMove = (lastState->name == "HangMoveState" || lastState->name == "DangleMoveState");
    blendTime = fromMove ? 0.0f : 0.3f;
    if (fromMove)

        // OLD SCRIPT
        // hack (original comment)
        if (curAnimationIndex == 0 || curAnimationIndex == 1 || curAnimationIndex == 2)
            index = 0;
        else if (curAnimationIndex == 3)
            index = 1;
        else if (curAnimationIndex == 4 || curAnimationIndex == 5 || curAnimationIndex == 6)
            index = 2;
        else if (curAnimationIndex == 7)
            index = 3;

        // NEW SCRIPT
        //index = (curAnimationIndex <= 2) ? 0 : 1;

    else
        index = (Random(2) == 0) ? 0 : 1;

    ownner->GetNode()->SetVar(ANIMATION_INDEX, index);
    MultiMotionState::Enter(lastState);

    Motion* m = motions[index];
    alignTime = m->dockAlignTime;
    inputCheckTime = fromMove ? 0.0f : 0.2f;

    motionRotation = ownner->GetCharacterAngle();
    targetRotation = ownner->dockLine->GetTargetRotation(ownner->GetNode()->GetWorldPosition());
    turnSpeed = RADIO::AngleDiff(targetRotation - motionRotation) / alignTime;

    motionPositon = m->GetDockAlignPositionAtTime(ownner, targetRotation, alignTime);
    targetPosition = ownner->dockLine->Project(motionPositon);

    ownner->motion_velocity = fromMove ? Vector3(0, 0, 0) : (targetPosition - motionPositon) / alignTime;
}

bool PlayerHangIdleState::CheckFootBlocking()
{
    if (ownner->dockLine->HasFlag(LINE_SHORT_WALL))
    {
        ownner->ChangeState("DangleIdleState");
        return true;
    }
    return false;
}

void PlayerHangIdleState::Update(float dt)
{
    CheckFootBlocking();

    if (state == 0)
    {
        ownner->motion_deltaRotation += turnSpeed * dt;

        if (timeInState >= alignTime)
        {
            ownner->GetNode()->SetWorldRotation(Quaternion(0, targetRotation, 0)); 
            state = 1;
        }
    }

    if (timeInState > inputCheckTime)
    {
        InputManager* gInput = RADIO::g_im;
        if (!gInput->IsLeftStickInDeadZone() && gInput->IsLeftStickStationary())
        {
            int index = ownner->RadialSelectAnimation(4); 
//DirectionMapToIndex(gInput.GetLeftAxisAngle(), 4);
            if (index == 0)
                VerticalMove();
            else if (index == 2)
                ownner->ChangeState("FallState");
            else
                HorizontalMove(index == 3);
        }
    }

    MultiMotionState::Update(dt);
}

bool PlayerHangIdleState::VerticalMove()
{
    // используется здесь и ниже в этой функции нужно для адекватной работы static_cast из-за SharedPtr<Character> ownner; в CharacterStates.h
    Character* chr = ownner;

    Player* p = static_cast<Player*>(chr);
    Line* oldLine = ownner->dockLine;
    p->ClimbUpRaycasts(oldLine);

    bool hitUp = p->results[0].body_;
    bool hitForward = p->results[1].body_;
    bool hitDown = p->results[2].body_;

    int animIndex = 0;
    bool changeToOverState = false;

    if(RADIO::d_log)
        Log::Write(-1," "+ this->name + " VerticalMove hit1=" + String(hitUp) + " hitForward=" + String(hitForward) + " hitDown=" + String(hitDown)+"\n");

    if (hitUp)
        return false;

    if (oldLine->type == LINE_RAILING)
    {
        changeToOverState = true;
        animIndex = 1;
    }
    else
    {
        if (hitForward)
        {
            LineWorld* gLineWorld = RADIO::g_lw;
            // hit a front wall
            Vector<Line*> lines = gLineWorld->cacheLines; // Array<Line*>* lines = gLineWorld.cacheLines;
            lines.Clear();

            gLineWorld->CollectLinesByNode(p->results[1].body_->GetNode(), lines);
            if (lines.Empty())
                return false;
            
            if(RADIO::d_log)
                Log::Write(-1," "+ this->name + " hit front wall lines.num=" + String(lines.Size())+"\n");
            Line* bestLine = NULL;
            float maxDistSQR = 4.0f * 4.0f;
            float minHeightDiff = 1.0f;
            float maxHeightDiff = 4.5f;
            Vector3 comparePot = oldLine->Project(ownner->GetNode()->GetWorldPosition());

            for (unsigned int i = 0; i < lines.Size(); ++i)
            {
                Line* l = lines[i];

                if (l == oldLine)
                    continue;

                if (!l->TestAngleDiff(oldLine, 0) && !l->TestAngleDiff(oldLine, 180))
                    continue;

                float dh = l->end.y_ - oldLine->end.y_;
                if (dh < minHeightDiff || dh > maxHeightDiff)
                    continue;

                Vector3 tmpV = l->Project(comparePot);
                tmpV.y_ = comparePot.y_;
                float distSQR = (tmpV - comparePot).LengthSquared();
                
                if(RADIO::d_log)
                    Log::Write(-1," "+ this->name + " hit front wall distSQR=" + String(distSQR)+"\n");

                if (distSQR < maxDistSQR)
                {
                    bestLine = l;
                    maxDistSQR = distSQR;
                }
            }

            if (!bestLine)
            {
                if(RADIO::d_log)
                    Log::Write(-1," "+ this->name + " hit front wall no best line!!!\n");
                oldLine = NULL;
                return false;
            }

            animIndex = (bestLine->type == LINE_RAILING) ? 3 : 2;
            changeToOverState = true;
            ownner->AssignDockLine(bestLine);
        }
        else
        {
            // no front wall
            if (hitDown)
            {
                // hit gournd
                float hitGroundH = p->results[2].position_.y_;
                float lineToGround = oldLine->end.y_ - hitGroundH;

                if (lineToGround < (0 + HEIGHT_128) / 2)
                {
                    // if gound is not low just stand and run
                    animIndex = 0;
                }
                else if (lineToGround < (HEIGHT_128 + HEIGHT_256) / 2)
                {
                    // if gound is lower not than 4.5 we can perform a over jump
                    animIndex = 4;
                }
                else
                {
                    // if gound is lower enough just jump and fall
                    animIndex = 5;
                }

                changeToOverState = true;
            }
            else
            {
                // dont hit gournd
                animIndex = 5;
                changeToOverState = true;
            }
        }
    }

    if (changeToOverState)
    {
        if(RADIO::d_log)
            Log::Write(-1," "+ this->name + " Climb over animIndex=" + String(animIndex)+"\n");
        ownner->GetNode()->SetVar(ANIMATION_INDEX, animIndex);

        PlayerHangOverState* s = static_cast<PlayerHangOverState*>(ownner->FindState_Hash(overStateName));
        s->groundPos = p->results[2].position_;
        ownner->ChangeState_Hash(overStateName);
        return true;
    }
    return false;
}

PlayerHangMoveState* PlayerHangIdleState::GetMoveState(Line* l)
{
    if (l->HasFlag(LINE_SHORT_WALL))
        return static_cast<PlayerHangMoveState*>(ownner->FindState("DangleMoveState"));
    else
        return static_cast<PlayerHangMoveState*>(ownner->FindState("HangMoveState"));
    // return cast<PlayerHangMoveState>(ownner.FindState(moveStateName));
}

bool PlayerHangIdleState::TryToMoveToLinePoint(bool left)
{
    Node* n = ownner->GetNode();
    Vector3 myPos = n->GetWorldPosition(); 
    Vector3 towardDir = left ? Vector3(-1, 0, 0) : Vector3(1, 0, 0);
    towardDir = n->GetWorldRotation() * towardDir;
    Vector3 linePt = ownner->dockLine->GetLinePoint(towardDir);
    String handName = left ? L_HAND : R_HAND;
    Vector3 handPos = n->GetChild(handName, true)->GetWorldPosition();
    handPos = ownner->dockLine->Project(handPos);

    float distSQR = (handPos - linePt).LengthSquared();
    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " TryToMoveToLinePoint distSQR=" + String(distSQR)+"\n");

    if (distSQR < moveToLinePtDist * moveToLinePtDist)
        return false;

    GetMoveState(ownner->dockLine)->MoveToLinePoint(left);
    return true;
}

bool PlayerHangIdleState::HorizontalMove(bool left)
{
    PlayerHangMoveState* s = GetMoveState(ownner->dockLine);

    // используется здесь и ниже в этой функции нужно для адекватной работы static_cast из-за SharedPtr<Character> ownner; в CharacterStates.h
    Character* chr = ownner;
    Player* p = static_cast<Player*>(chr);

    Line* oldLine = ownner->dockLine;

    int index = left ? 0 : s->numOfAnimations;
    Motion* m = s->motions[index];
    Vector4 motionOut = m->GetKey(m->endTime);
    Node* n = ownner->GetNode();
    Vector3 myPos = n->GetWorldPosition(); 
    Vector3 proj = oldLine->Project(myPos);
    Vector3 dir = proj - myPos;
    Quaternion q(0, Atan2(dir.x_, dir.z_), 0);

    Vector3 towardDir = left ? Vector3(-1, 0, 0) : Vector3(1, 0, 0);
    towardDir = q * towardDir;
    towardDir.y_ = 0;
    Vector3 linePt = oldLine->GetLinePoint(towardDir);

    Vector3 futurePos = q * Vector3(motionOut.x_, motionOut.y_, motionOut.z_) + myPos;
    Vector3 futureProj = oldLine->Project(futurePos);
    bool outOfLine = !oldLine->IsProjectPositionInLine(futureProj, 0.5f);
    Ray ray;
    ray.Define(myPos, towardDir);
    float dist = (futureProj - proj).Length() + 1.0f;

    PhysicsRaycastResult result;
    bool blocked = false;
    ownner->GetScene()->GetComponent<PhysicsWorld>()->RaycastSingle(result, ray, dist, COLLISION_LAYER_LANDSCAPE);
    if (result.body_)
        blocked = true;

    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " Move left=" + String(left) + " outOfLine=" + String(outOfLine) + " blocked=" + String(blocked)+"\n");

    if (outOfLine || blocked)
    {
        int convexIndex = 1;
        Line* l = p->FindCrossLine(left, convexIndex);
        if (l)
        {
            GetMoveState(l)->CrossMove(l, left, convexIndex);
            return true;
        }
    }

    if (blocked)
        return false;

    if (outOfLine)
    {
        // test if we are a little bit futher to the linePt
        if (TryToMoveToLinePoint(left))
            return true;

        float distErrSQR = 0;
        Line* l = p->FindParalleLine(left, distErrSQR);
        if (l)
        {
            const float bigJumpErrSQR = 6.0f * 6.0f;
            GetMoveState(l)->ParalleJumpMove(l, left, distErrSQR > bigJumpErrSQR);
            return true;
        }
        return false;
    }

    GetMoveState(ownner->dockLine)->NormalMove(left);
    return true;
}


enum HangMoveType
{
    HANG_NORMAL,
    HANG_CONVEX,
    HANG_CONCAVE,
    HANG_JUMP,
};

// PlayerHangMoveState  =============================================

PlayerHangMoveState::PlayerHangMoveState(Context* context, Character* ownner) :
    PlayerDockAlignState(context, ownner)
{
    SetName("HangMoveState");
    numOfAnimations = 3;
    type = 0;
    dockBlendingMethod = 1;
}

PlayerHangMoveState::~PlayerHangMoveState()
{}

void PlayerHangMoveState::Enter(State* lastState)
{
    motionFlagBeforeAlign = (type == 1) ? kMotion_XYZ : kMotion_ALL;
    motionFlagAfterAlign = (type == 1) ? kMotion_XZR : kMotion_None;

    PlayerDockAlignState::Enter(lastState);
    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " enter type = " + String(type)+"\n");
}

void PlayerHangMoveState::Exit(State* nextState)
{
    oldLine = NULL;
    PlayerDockAlignState::Exit(nextState);
}

void PlayerHangMoveState::CrossMove(Line* line, bool left, int convexIndex)
{
    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " CrossMove\n");

    oldLine = ownner->dockLine;
    ownner->AssignDockLine(line);

    int index = left ? 0 : numOfAnimations;
    index += convexIndex;
    dockBlendingMethod = 1;
    type = 1;
    ownner->GetNode()->SetVar(ANIMATION_INDEX, index);
    dockInTargetBound = 1.5f;
    ownner->ChangeState_Hash(this->nameHash);
}

void PlayerHangMoveState::ParalleJumpMove(Line* line, bool left, bool bigJump)
{
    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " ParalleJumpMove\n");

    oldLine = ownner->dockLine;
    ownner->AssignDockLine(line);

    int index = left ? 0 : numOfAnimations;
    dockBlendingMethod = 1;
    dockInTargetBound = 1.5f;

    type = bigJump ? 3 : 2;

    if (type == 3)
        index += (numOfAnimations - 1);

    ownner->GetNode()->SetVar(ANIMATION_INDEX, index);
    ownner->ChangeState_Hash(this->nameHash);
}

void PlayerHangMoveState::MoveToLinePoint(bool left)
{
    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " MoveToLinePoint\n");

    oldLine = NULL;
    type = 4;
    dockBlendingMethod = 1;
    dockInTargetBound = 0.1f;
    ownner->GetNode()->SetVar(ANIMATION_INDEX, left ? 0 : numOfAnimations);
    ownner->ChangeState_Hash(this->nameHash);
}

void PlayerHangMoveState::NormalMove(bool left)
{
    dockBlendingMethod = 1;
    oldLine = NULL;
    type = 0;
    dockInTargetBound = 0.1;
    ownner->GetNode()->SetVar(ANIMATION_INDEX, left ? 0 : numOfAnimations);
    ownner->ChangeState_Hash(this->nameHash);
}

void PlayerHangMoveState::OnMotionFinished()
{
    ownner->ChangeState("HangIdleState");
}

Vector3 PlayerHangMoveState::PickDockInTarget()
{
    Line* l = ownner->dockLine;
    Vector3 v = l->Project(motionPositon);
    v = l->FixProjectPosition(v, dockInTargetBound);
    if (l->HasFlag(LINE_THIN_WALL))
    {
        Character* chr = ownner;
        Player* p = static_cast<Player*>(chr);
        Vector3 dir;
        bool convex = selectIndex % 2 != 0;
        if (type == 1)
            dir = convex ? (p->points[2] - p->points[3]) : (p->points[0] - p->points[1]);
        else
            dir = Quaternion(0, targetRotation, 0) * Vector3(0, 0, -1);
        dir.y_ = 0;
        float dist = Min(l->size.x_, l->size.z_) / 2;
        v += dir.Normalized() * dist;
    }
    return v;
}

Vector3 PlayerHangMoveState::PickDockOutTarget()
{
    Vector3 offset = Vector3(0, 0, -1.15);
    return ownner->dockLine->Project(motionPositon) + Quaternion(0, targetRotation, 0) * offset;
}

// PlayerHangOverState  =============================================

PlayerHangOverState::PlayerHangOverState(Context* context, Character* ownner) :
    PlayerDockAlignState(context, ownner)
{
    SetName("HangOverState");
    type = 0;
    physicsType = 0;
    dockBlendingMethod = 1;
}

PlayerHangOverState::~PlayerHangOverState()
{}

Vector3 PlayerHangOverState::PickDockOutTarget()
{
    if (type == 0)
        return groundPos;
    else
        return PlayerDockAlignState::PickDockOutTarget();
}

void PlayerHangOverState::Enter(State* lastState)
{
    PlayerDockAlignState::Enter(lastState);
    if (selectIndex == 0 || selectIndex == 2 || selectIndex == 4)
        type = 0;
    else if (selectIndex == 1 || selectIndex == 3)
        type = 1;
    else
        type = 2;

    if (type == 0)
        motionFlagAfterAlign = kMotion_Y;
    else if (type == 1)
        motionFlagAfterAlign = kMotion_XYZ;
    else
        motionFlagAfterAlign = 0;
}

void PlayerHangOverState::OnMotionFinished()
{
    if (type == 0)
        PlayerDockAlignState::OnMotionFinished();
    else if (type == 1)
        ownner->ChangeState("RailIdleState");
    else
        ownner->ChangeState("FallState");
}

// PlayerDangleIdleState  =============================================

PlayerDangleIdleState::PlayerDangleIdleState(Context* context, Character* c) :
    PlayerHangIdleState(context, c)
{
    SetName("DangleIdleState");
    overStateName = StringHash("DangleOverState");
    moveStateName = StringHash("DangleMoveState");
}

PlayerDangleIdleState::~PlayerDangleIdleState()
{}

bool PlayerDangleIdleState::CheckFootBlocking()
{
    if (!ownner->dockLine->HasFlag(LINE_SHORT_WALL))
    {
        ownner->ChangeState("HangIdleState");
        return true;
    }
    return false;
}

// PlayerDangleOverState  =============================================

PlayerDangleOverState::PlayerDangleOverState(Context* context, Character* ownner) :
    PlayerHangOverState(context, ownner)
{
    SetName("DangleOverState");
}

PlayerDangleOverState::~PlayerDangleOverState()
{}

// PlayerDangleMoveState  =============================================

PlayerDangleMoveState::PlayerDangleMoveState(Context* context, Character* ownner) :
    PlayerHangMoveState(context, ownner)
{
    SetName("DangleMoveState");
}

PlayerDangleMoveState::~PlayerDangleMoveState()
{}

void PlayerDangleMoveState::OnMotionFinished()
{
    ownner->ChangeState("DangleIdleState");
}

// PlayerClimbDownState =============================================

PlayerClimbDownState::PlayerClimbDownState(Context* context, Character* c) :
    PlayerDockAlignState(context, c)
{
    SetName("ClimbDownState");
    dockBlendingMethod = 1;
}

PlayerClimbDownState::~PlayerClimbDownState()
{}

void PlayerClimbDownState::DebugDraw(DebugRenderer* debug)
{
    PlayerDockAlignState::DebugDraw(debug);
    debug->AddCross(groundPos, 0.5f, Color::BLACK, false);
}

float PlayerClimbDownState::PickDockInRotation()
{
    return ownner->dockLine->GetTargetRotation(ownner->GetNode()->GetWorldPosition());
}

void PlayerClimbDownState::Enter(State* lastState)
{
    int animIndex = 0;
    Character* chr = ownner; 
    Player* p = static_cast<Player*>(chr);
    p->ClimbDownRaycasts(ownner->dockLine);

    bool hitForward = p->results[0].body_;
    bool hitDown = p->results[1].body_;
    bool hitBack = p->results[2].body_;
    groundPos = p->results[1].position_;
    float lineToGround = ownner->dockLine->end.y_ - groundPos.y_;

    if(RADIO::d_log)
        Log::Write(-1," " +this->name + " lineToGround=" + String(lineToGround) + " hitForward=" + String(hitForward) + " hitDown=" + String(hitDown) + " hitBack=" + String(hitBack)+"\n");

    if (lineToGround < (HEIGHT_128 + HEIGHT_256) / 2)
    {
        animIndex = 0;
        if (lastState->name == "RunState")
            animIndex = 1;
        else if (lastState->name == "CrouchMoveState")
            animIndex = 2;

        motionFlagBeforeAlign = kMotion_Y;
        animSpeed = 1.5f;
    }
    else
    {
        animSpeed = 1.0f;
        motionFlagBeforeAlign = kMotion_ALL;
        animIndex = ownner->dockLine->HasFlag(LINE_SHORT_WALL) ? (4 + Random(2)) : 3;
    }

    ownner->GetNode()->SetVar(ANIMATION_INDEX, animIndex);
    PlayerDockAlignState::Enter(lastState);
}

void PlayerClimbDownState::OnMotionFinished()
{
    if (selectIndex < 3)
        PlayerDockAlignState::OnMotionFinished();
    else
    {
        if (selectIndex == 3)
            ownner->ChangeState("HangIdleState");
        else
            ownner->ChangeState("DangleIdleState");
    }
}






















