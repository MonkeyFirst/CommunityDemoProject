#include <Urho3D/Urho3DAll.h>

#include "Character/Thug.h"
#include "Character/EnemyManager.h"

#include "States/GameManager.h"
#include "Motions/MotionManager.h"

#include "Objects/HeadIndicator.h"

#include "radio.h"
/*
// -- NON CONST
float PUNCH_DIST = 0.0f;
float KICK_DIST = 0.0f;
float STEP_MAX_DIST = 0.0f;
float STEP_MIN_DIST = 0.0f;
//float KEEP_DIST_WITH_PLAYER = -0.25f;  // declared in Enemy.h
float KEEP_DIST_WITH_PLR = -0.25f;
*/
// ThugStandState  =============================================

ThugStandState::ThugStandState(Context* context, Character* c) :
    MultiAnimationState(context, c)
{
    SetName("StandState");
    firstEnter = true;
    AddMotion(MOVEMENT_GROUP_THUG + "Stand_Idle_Additive_01");
    AddMotion(MOVEMENT_GROUP_THUG + "Stand_Idle_Additive_02");
    AddMotion(MOVEMENT_GROUP_THUG + "Stand_Idle_Additive_03");
    AddMotion(MOVEMENT_GROUP_THUG + "Stand_Idle_Additive_04");
    flags = FLAGS_REDIRECTED | FLAGS_ATTACK;
    looped = true;
}

ThugStandState::~ThugStandState()
{}

void ThugStandState::Enter(State* lastState)
{
    ownner->SetVelocity(Vector3(0,0,0));

    float min_think_time = MIN_THINK_TIME;
    float max_think_time = MAX_THINK_TIME;
    if (firstEnter)
    {
        min_think_time = 2.0f;
        max_think_time = 3.0f;
        firstEnter = false;
    }
    thinkTime = Random(min_think_time, max_think_time);
    if (RADIO::d_log)
        Log::Write(-1," " + ownner->GetName() + " thinkTime = " + String(thinkTime) + "\n");
    ownner->ClearAvoidance();
    attackRange = Random(0.0f, MAX_ATTACK_RANGE);
    MultiAnimationState::Enter(lastState);
}

void ThugStandState::Update(float dt)
{
    if (RADIO::freeze_ai != 0)
       return;

    float diff = Abs(ownner->ComputeAngleDiff());
    if (diff > MIN_TURN_ANGLE)
    {
        // I should always turn to look at player.
        ownner->ChangeState("TurnState");
        return;
    }

    if (timeInState > thinkTime)
    {
        OnThinkTimeOut();
        timeInState = 0.0f;
        thinkTime = Random(MIN_THINK_TIME, MAX_THINK_TIME);
    }

    MultiAnimationState::Update(dt);
}

void ThugStandState::OnThinkTimeOut()
{       
    if (ownner->target->HasFlag(FLAGS_INVINCIBLE))
        return;

    Node* _node = ownner->GetNode();
    EnemyManager* em = RADIO::g_em;
    float dist = ownner->GetTargetDistance()  - COLLISION_SAFE_DIST;
    if (ownner->CanAttack() && dist <= attackRange)
    {
        if (RADIO::d_log)
            Log::Write(-1," do attack because dist <= " + String(attackRange) + "\n");
        if (ownner->Attack())
            return;
    }

    int rand_i = 0;
    int num_of_moving_thugs = em->GetNumOfEnemyHasFlag(FLAGS_MOVING);
    //bool can_i_see_player = !ownner->IsTargetSightBlocked();
    int num_of_with_player = em->GetNumOfEnemyWithinDistance(PLAYER_NEAR_DIST);
    if (num_of_moving_thugs < MAX_NUM_OF_MOVING && num_of_with_player <  MAX_NUM_OF_NEAR && !ownner->HasFlag(FLAGS_NO_MOVE))
    {
        // try to move to player
        rand_i = Random(6);
        String nextState = "StepMoveState";
        float run_dist = RADIO::STEP_MAX_DIST + 1.0f;
        if (dist >= run_dist || rand_i == 1)
        {
            nextState = "RunState";
        }
        else
        {
            ThugStepMoveState* state = static_cast<ThugStepMoveState*>(ownner->FindState("StepMoveState"));
            int index = state->GetStepMoveIndex();
            //URHO3D_LOGINFO(ownner->GetName() + " apply animation index for step move in thug stand state: " + index);
            _node->SetVar(ANIMATION_INDEX, index);
        }
        ownner->ChangeState(nextState); // ("RunState")
    }
    else
    {
        if (dist >= KEEP_DIST_WITHIN_PLAYER)
        {
            ThugStepMoveState* state = static_cast<ThugStepMoveState*>(ownner->FindState("StepMoveState"));
            int index = state->GetStepMoveIndex();
            //URHO3D_LOGINFO(ownner->GetName() + " apply animation index for keep with with player in stand state: " + index);
            _node->SetVar(ANIMATION_INDEX, index);
            ownner->ChangeState("StepMoveState");
            return;
        }

        rand_i = Random(10);
        if (rand_i > 8)
        {
            int index = Random(4);
            _node->SetVar(ANIMATION_INDEX, index);
            //URHO3D_LOGINFO(ownner->GetName() + " apply animation index for random move in stand state: " + index);
            ownner->ChangeState("StepMoveState");
        }
    }

    attackRange = Random(0.0f, MAX_ATTACK_RANGE);
}

void ThugStandState::FixedUpdate(float dt)
{
    ownner->CheckAvoidance(dt);
    MultiAnimationState::FixedUpdate(dt);
}

int ThugStandState::PickIndex()
{
    int i = animations.Size();
    return Random(i);
}

// ThugStepMoveState  =============================================

ThugStepMoveState::ThugStepMoveState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    SetName("StepMoveState");
    // short step
    AddMotion(MOVEMENT_GROUP_THUG + "Step_Forward");
    AddMotion(MOVEMENT_GROUP_THUG + "Step_Right");
    AddMotion(MOVEMENT_GROUP_THUG + "Step_Back");
    AddMotion(MOVEMENT_GROUP_THUG + "Step_Left");
    // long step
    AddMotion(MOVEMENT_GROUP_THUG + "Step_Forward_Long");
    AddMotion(MOVEMENT_GROUP_THUG + "Step_Right_Long");
    AddMotion(MOVEMENT_GROUP_THUG + "Step_Back_Long");
    AddMotion(MOVEMENT_GROUP_THUG + "Step_Left_Long");
    flags = FLAGS_REDIRECTED | FLAGS_ATTACK | FLAGS_MOVING;

    if (RADIO::STEP_MAX_DIST != 0.0f)
    {
        RADIO::STEP_MIN_DIST = motions[0]->endDistance;
        RADIO::STEP_MAX_DIST = motions[4]->endDistance;
        
        if(RADIO::d_log)
            Log::Write(-1," " + this->name + "Thug min-step-dist=" + String(RADIO::STEP_MIN_DIST) + " max-step-dist=" + String(RADIO::STEP_MAX_DIST)+"\n");
    }
}

ThugStepMoveState::~ThugStepMoveState()
{}

void ThugStepMoveState::Update(float dt)
{
    if (motions[selectIndex]->Move(ownner, dt) == 1)
    {
        float dist = ownner->GetTargetDistance() - COLLISION_SAFE_DIST;
        bool attack = false;

        if (dist <= attackRange && dist >= -0.5f)
        {
            if (Abs(ownner->ComputeAngleDiff()) < MIN_TURN_ANGLE && RADIO::freeze_ai == 0)
            {
                if (ownner->Attack())
                    return;
            }
            else
            {
                ownner->ChangeState("TurnState");
                return;
            }
        }

        ownner->CommonStateFinishedOnGroud();
        return;
    }

    CharacterState::Update(dt);
}

int ThugStepMoveState::GetStepMoveIndex()
{
    int index = 0;
    float dist = ownner->GetTargetDistance() - COLLISION_SAFE_DIST;
    if (dist < RADIO::KEEP_DIST_WITH_PLR)
    {
        index = ownner->RadialSelectAnimation(4);
        index = (index + 2) % 4;
        // URHO3D_LOGINFO("ThugStepMoveState->GetStepMoveIndex() Keep Dist Within Player =->" + index);
    }
    else
    {
        if (dist > motions[4]->endDistance + 2.0f)
            index += 4;
        // URHO3D_LOGINFO("ThugStepMoveState->GetStepMoveIndex()=->" + index + " dist=" + dist);
    }
    return index;
}

void ThugStepMoveState::Enter(State* lastState)
{
    attackRange = Random(0.0, MAX_ATTACK_RANGE);
    MultiMotionState::Enter(lastState);
}

float ThugStepMoveState::GetThreatScore()
{
    return 0.333f;
}

// ThugRunState  =============================================

ThugRunState::ThugRunState(Context* context, Character* c) :
    SingleMotionState(context, c)
{
    SetName("RunState");
    turnSpeed = 5.0f;
    SetMotion(MOVEMENT_GROUP_THUG + "Run_Forward_Combat");
    flags = FLAGS_REDIRECTED | FLAGS_ATTACK | FLAGS_MOVING;
}

ThugRunState::~ThugRunState()
{}

void ThugRunState::Update(float dt)
{
    float characterDifference = ownner->ComputeAngleDiff();
    ownner->GetNode()->Yaw(characterDifference * turnSpeed * dt);

    // if the difference is large, then turn 180 degrees
    if (Abs(characterDifference) > FULLTURN_THRESHOLD)
    {
        ownner->ChangeState("TurnState");
        return;
    }

    float dist = ownner->GetTargetDistance() - COLLISION_SAFE_DIST;
    if (dist <= attackRange)
    {
        if (ownner->Attack() && RADIO::freeze_ai == 0)
            return;
        ownner->CommonStateFinishedOnGroud();
        return;
    }

    SingleMotionState::Update(dt);
}

void ThugRunState::Enter(State* lastState)
{
    SingleMotionState::Enter(lastState);
    attackRange = Random(0.0, MAX_ATTACK_RANGE);
    ownner->ClearAvoidance();
}

void ThugRunState::FixedUpdate(float dt)
{
    ownner->CheckAvoidance(dt);
    CharacterState::FixedUpdate(dt);
}

float ThugRunState::GetThreatScore()
{
    return 0.333f;
}

// ThugTurnState  =============================================

ThugTurnState::ThugTurnState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    SetName("TurnState");
    AddMotion(MOVEMENT_GROUP_THUG + "135_Turn_Right");
    AddMotion(MOVEMENT_GROUP_THUG + "135_Turn_Left");
    flags = FLAGS_REDIRECTED | FLAGS_ATTACK;
}

ThugTurnState::~ThugTurnState()
{}

void ThugTurnState::Update(float dt)
{
    Motion* motion = motions[selectIndex];
    float t = ownner->animCtrl->GetTime(motion->animationName);
    float characterDifference = Abs(ownner->ComputeAngleDiff());
    if (t >= endTime || characterDifference < 5)
    {
        ownner->CommonStateFinishedOnGroud();
        return;
    }
    ownner->GetNode()->Yaw(turnSpeed * dt);
    CharacterState::Update(dt);
}

void ThugTurnState::Enter(State* lastState)
{
    float diff = ownner->ComputeAngleDiff();
    int index = 0;
    if (diff < 0)
        index = 1;
    ownner->GetNode()->SetVar(ANIMATION_INDEX, index);
    endTime = motions[index]->endTime;
    turnSpeed = diff / endTime;
    ownner->ClearAvoidance();
    MultiMotionState::Enter(lastState);
}

void ThugTurnState::FixedUpdate(float dt)
{
    ownner->CheckAvoidance(dt);
    MultiMotionState::FixedUpdate(dt);
}

// ThugCounterState  =============================================

ThugCounterState::ThugCounterState(Context* context, Character* c) : 
    CharacterCounterState(context, c)
{
    Add_Counter_Animations("TG_BM_Counter/", false); // false - no player character
/*
    if (playerType == 0)
        AddBW_Counter_Animations("TG_BW_Counter/", "TG_BM_Counter/",false);
    else if (playerType == 1)
        AddCW_Counter_Animations("TG_CW_Counter/", "TG_CW_Counter/",false);
*/
}

ThugCounterState::~ThugCounterState()
{}

void ThugCounterState::OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData)
{
    StringHash name = eventData[NAME]->GetStringHash();
    if (name == READY_TO_FIGHT)
    {
        if(RADIO::d_log)
            Log::Write(-1," " + ownner->GetName() + " ThugCounterState: OnAnimationTrigger READY_TO_FIGHT\n");
        ownner->AddFlag(FLAGS_ATTACK | FLAGS_REDIRECTED);
        return;
    }
    //CharacterCounterState::OnAnimationTrigger(animState, eventData); // empty function
    CharacterState::OnAnimationTrigger(animState, eventData);
}

void ThugCounterState::Exit(State* nextState)
{
    ownner->RemoveFlag(FLAGS_ATTACK | FLAGS_REDIRECTED);
    CharacterCounterState::Exit(nextState);
}


// ThugAttackState  =============================================

ThugAttackState::ThugAttackState(Context* context, Character* c) :
    CharacterState(context, c)
{
    SetName("AttackState");
    turnSpeed = 1.25f;
    doAttackCheck = false;
    AddAttackMotion("Attack_Punch", 23, ATTACK_PUNCH, "Bip01_R_Hand");
    AddAttackMotion("Attack_Punch_01", 23, ATTACK_PUNCH, "Bip01_R_Hand");
    AddAttackMotion("Attack_Punch_02", 23, ATTACK_PUNCH, "Bip01_R_Hand");
    AddAttackMotion("Attack_Kick", 24, ATTACK_KICK, "Bip01_L_Foot");
    AddAttackMotion("Attack_Kick_01", 24, ATTACK_KICK, "Bip01_L_Foot");
    AddAttackMotion("Attack_Kick_02", 24, ATTACK_KICK, "Bip01_L_Foot");

    if (RADIO::PUNCH_DIST != 0.0f)
    {
        RADIO::PUNCH_DIST = attacks[0]->motion->endDistance;
        RADIO::KICK_DIST = attacks[3]->motion->endDistance;
        
        if(RADIO::d_log)
            Log::Write(-1," " + this->name + " Thug kick-dist=" + String(RADIO::KICK_DIST) + " punch-dist=" + String(RADIO::PUNCH_DIST)+"\n");
    }
}

ThugAttackState::~ThugAttackState()
{

}

void ThugAttackState::AddAttackMotion(const String& name, int impactFrame, int type, const String& bName)
{
    attacks.Push(new AttackMotion(context_, MOVEMENT_GROUP_THUG + name, impactFrame, type, bName));
}

void ThugAttackState::Update(float dt)
{
    if (firstUpdate)
    {
        Character* chr = ownner;
        Thug* tg = static_cast<Thug*>(chr);
        //if (static_cast<Thug>(ownner).KeepDistanceWithEnemy())
        if (tg->KeepDistanceWithEnemy())
            return;
    }

    Motion* motion = currentAttack->motion;
    ownner->CheckTargetDistance(ownner->target, COLLISION_SAFE_DIST);

    float characterDifference = ownner->ComputeAngleDiff();
    ownner->motion_deltaRotation += characterDifference * turnSpeed * dt;

    if (doAttackCheck)
        AttackCollisionCheck();

    if (motion->Move(ownner, dt) == 1)
    {
        ownner->CommonStateFinishedOnGroud();
        return;
    }

    if (!ownner->target->HasFlag(FLAGS_ATTACK))
    {
        ownner->CommonStateFinishedOnGroud();
        return;
    }

    CharacterState::Update(dt);
}

void ThugAttackState::Enter(State* lastState)
{
    float targetDistance = ownner->GetTargetDistance() - COLLISION_SAFE_DIST;
    float punchDist = attacks[0]->motion->endDistance;

    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " targetDistance=" + String(targetDistance) + " punchDist=" + String(punchDist) +"\n");

    int index = Random(3);
    if (targetDistance > punchDist + 1.0f)
        index += 3; // a kick attack
    currentAttack = attacks[index];
    ownner->GetNode()->SetVar(ATTACK_TYPE, currentAttack->type);
    Motion* motion = currentAttack->motion;
    motion->Start(ownner);
    ownner->AddFlag(FLAGS_REDIRECTED | FLAGS_ATTACK);
    doAttackCheck = false;
    CharacterState::Enter(lastState);
    
    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " Thug Pick attack motion = " + motion->animationName +"\n");
}

void ThugAttackState::Exit(State* nextState)
{
    if(RADIO::d_log)
        Log::Write(-1," " + ownner->GetName()+ " ThugAttackState: Exit to " + nextState->name + " and remove FLAGS_COUNTER\n");

    currentAttack = NULL;
    ownner->RemoveFlag(FLAGS_REDIRECTED | FLAGS_ATTACK | FLAGS_COUNTER);
    ownner->SetTimeScale(1.0f);
    attackCheckNode = NULL;
    ShowAttackIndicator(false);
    CharacterState::Exit(nextState);
}

void ThugAttackState::ShowAttackIndicator(bool bshow)
{
    //static_cast<HeadIndicator*>(ownner->GetNode().GetScriptObject("HeadIndicator"));
    HeadIndicator* indicator = ownner->GetNode()->GetComponent<HeadIndicator>();
    if (indicator)
        indicator->ChangeState(bshow ? STATE_INDICATOR_ATTACK : STATE_INDICATOR_HIDE);
}

void ThugAttackState::OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData)
{
    // Добавлено для получения VariantMap из входящего VariantMap
    // Переменную DATA типа VariantMap это единственное что получаем на входе. 
//    VariantMap xData = eventData[DATA]->GetVariantMap(); // add

    // Получает уже изъятые из P_DATA данные.

//    Log::Write(-1," ThugAttackState: OnAnimationTrigger\n");

    String logstring = String::EMPTY;

    StringHash name = eventData[NAME]->GetStringHash();

    if (name == TIME_SCALE)
    {
        if(RADIO::d_log)
            Log::Write(-1," " + ownner->GetName() + " ThugAttackState: OnAnimationTrigger TIME_SCALE\n");
    
        float scale = eventData[VALUE]->GetFloat();
        ownner->SetTimeScale(scale);
        return;
    }
    else if (name == COUNTER_CHECK)
    {
//        Log::Write(-1," " + ownner->GetName() + " ThugAttackState: OnAnimationTrigger COUNTER_CHECK\n");
        int value = eventData[VALUE]->GetInt();
        ShowAttackIndicator((value == 1));
        if (value == 1)
        {
            //ShowAttackIndicator(true);
            ownner->AddFlag(FLAGS_COUNTER);
            
            if(RADIO::d_log)
                Log::Write(-1," " + ownner->GetName()+ " ThugAttackState: OnAnimationTrigger COUNTER_CHECK add FLAGS_COUNTER\n");
        }
        else
        {
            //ShowAttackIndicator(false);
            ownner->RemoveFlag(FLAGS_COUNTER);
            
            if(RADIO::d_log)
                Log::Write(-1," " + ownner->GetName()+ " ThugAttackState: OnAnimationTrigger COUNTER_CHECK remove FLAGS_COUNTER\n");
        }
        return;
    }
    else if (name == ATTACK_CHECK)
    {
        if(RADIO::d_log)
            Log::Write(-1," " + ownner->GetName() + " ThugAttackState: OnAnimationTrigger ATTACK_CHECK\n");
    
        int value = eventData[VALUE]->GetInt();
        bool bCheck = value == 1;
        if (doAttackCheck == bCheck)
            return;

        doAttackCheck = bCheck;
        if (value == 1)
        {
            attackCheckNode = ownner->GetNode()->GetChild(eventData[BONE]->GetString(), true);
            
            if(RADIO::d_log)
                Log::Write(-1," " + ownner->GetName() + " Thug AttackCheck bone = " + attackCheckNode->GetName());
            AttackCollisionCheck();
        }

        return;
    }

    CharacterState::OnAnimationTrigger(animState, eventData);
    return;
}

void ThugAttackState::AttackCollisionCheck()
{
    if (!attackCheckNode) 
    {
        doAttackCheck = false;
        return;
    }

    Character* target = ownner->target;
    Vector3 position = attackCheckNode->GetWorldPosition(); 
    Vector3 targetPosition = target->sceneNode->GetWorldPosition();
    Vector3 diff = targetPosition - position;
    diff.y_ = 0;
    float distance = diff.Length();
    if (distance < ownner->attackRadius + COLLISION_RADIUS * 0.8f)
    {
        Vector3 dir = position - targetPosition;
        dir.y_ = 0;
        dir.Normalize();
        bool b = target->OnDamage(ownner, position, dir, ownner->attackDamage);
        if (!b)
            return;

        if (currentAttack->type == ATTACK_PUNCH)
        {
            ownner->PlaySound("Sfx/impact_10.ogg");
        }
        else
        {
            ownner->PlaySound("Sfx/impact_13.ogg");
        }
        ownner->OnAttackSuccess(target);
    }
}

float ThugAttackState::GetThreatScore()
{
    return 0.75f;
}


// ThugHitState  =============================================

ThugHitState::ThugHitState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    SetName("HitState");
    recoverTimer = 3 * SEC_PER_FRAME;

    String preFix = "TG_HitReaction/";
    AddMotion(preFix + "HitReaction_Right");
    AddMotion(preFix + "HitReaction_Left");
    AddMotion(preFix + "HitReaction_Back_NoTurn");
    AddMotion(preFix + "HitReaction_Back");
}

ThugHitState::~ThugHitState()
{}

void ThugHitState::Update(float dt)
{
    if (timeInState >= recoverTimer)
        ownner->AddFlag(FLAGS_ATTACK | FLAGS_REDIRECTED);
    MultiMotionState::Update(dt);
}

void ThugHitState::Exit(State* nextState)
{
    ownner->RemoveFlag(FLAGS_ATTACK | FLAGS_REDIRECTED);
    MultiMotionState::Exit(nextState);
}

bool ThugHitState::CanReEntered()
{
    return timeInState >= recoverTimer;
}

void ThugHitState::FixedUpdate(float dt)
{
    Node* _node = ownner->GetNode()->GetChild("Collision");
    if (!_node)
        return;

    RigidBody* body = _node->GetComponent<RigidBody>();
    if (!body)
        return;

// Return colliding rigid bodies from the last simulation step. Only returns collisions that were sent as events (depends on collision event mode) and excludes e.g. static-static collisions.
//    void GetCollidingBodies(PODVector<RigidBody*>& result) const;
    PODVector<RigidBody*> neighbors;
    body->GetCollidingBodies(neighbors);

    for (unsigned int i = 0; i < neighbors.Size(); ++i)
    {
        Node* n_node = neighbors[i]->GetNode()->GetParent();
        if (!n_node)
            continue;
        //Character* object = static_cast<Character*>(n_node.scriptObject); // TODO: xxx
        //Character* object = static_cast<Character*>(n_node->GetComponent<Character>()); // TODO: xxx
        Character* object = static_cast<Character*>(n_node->GetComponent<Character>());
        if (!object)
            continue;
        if (object->HasFlag(FLAGS_MOVING))
            continue;

        float dist = ownner->GetTargetDistance(n_node);
        if (dist < 1.0f)
        {
            State* state = ownner->GetState();
            if (state->nameHash == RUN_STATE || state->nameHash == STAND_STATE)
            {
                object->ChangeState("PushBack");
            }
        }
    }
}

// ThugGetUpState  =============================================

ThugGetUpState::ThugGetUpState(Context* context, Character* c) :
    CharacterGetUpState(context, c)
{
    String prefix = "TG_Getup/";
    AddMotion(prefix + "GetUp_Back");
    AddMotion(prefix + "GetUp_Front");
}

ThugGetUpState::~ThugGetUpState()
{}

void ThugGetUpState::OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData)
{
    // Добавлено для получения VariantMap из входящего VariantMap
    // Переменную DATA типа VariantMap это единственное что получаем на входе. 
//    VariantMap xData = eventData[DATA]->GetVariantMap(); // add

    // Получает уже изъятые из P_DATA данные.

    StringHash name = eventData[NAME]->GetStringHash();
    if (name == READY_TO_FIGHT)
    {
        if(RADIO::d_log)
            Log::Write(-1," " + ownner->GetName() + " ThugGetUpState: OnAnimationTrigger READY_TO_FIGHT\n");
        ownner->AddFlag(FLAGS_ATTACK | FLAGS_REDIRECTED);
        return;
    }
    CharacterGetUpState::OnAnimationTrigger(animState, eventData);
}

void ThugGetUpState::Enter(State* lastState)
{
    if(RADIO::d_log)
        Log::Write(-1," " + ownner->GetName() + " ThugGetUpState: Enter\n");

    CharacterGetUpState::Enter(lastState);
    ownner->SetPhysics(true);
}

void ThugGetUpState::Exit(State* nextState)
{
    ownner->RemoveFlag(FLAGS_ATTACK | FLAGS_REDIRECTED);
    CharacterGetUpState::Exit(nextState);
}

// ThugDeadState  =============================================

ThugDeadState::ThugDeadState(Context* context, Character* c) :
    CharacterState(context, c)
{
    SetName("DeadState");
    duration = 5.0f;
}

ThugDeadState::~ThugDeadState()
{}

void ThugDeadState::Enter(State* lastState)
{
    if(RADIO::d_log)
        Log::Write(-1," " + ownner->GetName() + " ThugDeadState: Enter\n");

    ownner->SetNodeEnabled("Collision", false);
    CharacterState::Enter(lastState);
}

void ThugDeadState::Update(float dt)
{
    duration -= dt;
    if (duration <= 0)
    {
        if (!ownner->IsVisible())
            ownner->duration = 0;
        else
            duration = 0;
    }
}

// ThugBeatDownHitState  =============================================

ThugBeatDownHitState::ThugBeatDownHitState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    SetName("BeatDownHitState");
//    if (playerType == 0)
//    {
        String preFix = "TG_BM_Beatdown/";
        AddMotion(preFix + "Beatdown_HitReaction_01");
        AddMotion(preFix + "Beatdown_HitReaction_02");
        AddMotion(preFix + "Beatdown_HitReaction_03");
        AddMotion(preFix + "Beatdown_HitReaction_04");
        AddMotion(preFix + "Beatdown_HitReaction_05");
        AddMotion(preFix + "Beatdown_HitReaction_06");
/*    }
    else if (playerType == 1)
    {
        String preFix = "TG_CW_Beatdown/";
        AddMotion(preFix + "Beatdown_01");
        AddMotion(preFix + "Beatdown_02");
        AddMotion(preFix + "Beatdown_03");
        AddMotion(preFix + "Beatdown_04");
        AddMotion(preFix + "Beatdown_05");
        AddMotion(preFix + "Beatdown_06");
    }
*/
    flags = FLAGS_STUN | FLAGS_ATTACK;
}

ThugBeatDownHitState::~ThugBeatDownHitState()
{}

bool ThugBeatDownHitState::CanReEntered()
{
    return true;
}

float ThugBeatDownHitState::GetThreatScore()
{
    return 0.9f;
}

void ThugBeatDownHitState::OnMotionFinished()
{
    // Print(ownner.GetName() + " state:" + name + " finshed motion:" + motions[selectIndex].animationName);
    ownner->ChangeState("StunState");
}

// ThugBeatDownEndState  =============================================

ThugBeatDownEndState::ThugBeatDownEndState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    SetName("BeatDownEndState");
//    if (playerType == 0)
//    {
        String preFix = "TG_BM_Beatdown/";
        AddMotion(preFix + "Beatdown_Strike_End_01");
        AddMotion(preFix + "Beatdown_Strike_End_02");
        AddMotion(preFix + "Beatdown_Strike_End_03");
        AddMotion(preFix + "Beatdown_Strike_End_04");
/*    }
    else if (playerType == 1)
    {
        String preFix = "TG_CW_Beatdown/";
        AddMotion(preFix + "Beatdown_End_01");
        AddMotion(preFix + "Beatdown_End_02");
        AddMotion(preFix + "Beatdown_End_03");
    }
*/
    flags = FLAGS_ATTACK;
}

ThugBeatDownEndState::~ThugBeatDownEndState()
{}

void ThugBeatDownEndState::Enter(State* lastState)
{
    ownner->SetHealth(0);
    MultiMotionState::Enter(lastState);
}

// ThugStunState  =============================================

ThugStunState::ThugStunState(Context* context, Character* c) :
    SingleAnimationState(context, c)
{
    SetName("StunState");
    flags = FLAGS_STUN | FLAGS_ATTACK;
    SetMotion("TG_HitReaction/CapeHitReaction_Idle");
    looped = true;
    stateTime = 5.0f;
}

ThugStunState::~ThugStunState()
{}

// ThugPushBackState  =============================================

ThugPushBackState::ThugPushBackState(Context* context, Character* c) :
    SingleMotionState(context, c)
{
    SetName("PushBack");
    flags = FLAGS_ATTACK;
    SetMotion("TG_HitReaction/HitReaction_Left");
}

ThugPushBackState::~ThugPushBackState()
{}

// Thug  =============================================

Thug::Thug(Context* context) :
    Enemy(context)
{
    checkAvoidanceTimer = 0.0f;
    checkAvoidanceTime = 0.1f;
}

Thug::~Thug()
{}

void Thug::ObjectStart()
{
    if(RADIO::d_log)
        Log::Write(-1," Thug: ObjectStart\n");
    Enemy::ObjectStart();
    stateMachine->AddState(new ThugStandState(context_, this));
    stateMachine->AddState(new ThugStepMoveState(context_, this));
    stateMachine->AddState(new ThugTurnState(context_, this));
    stateMachine->AddState(new ThugRunState(context_, this));
    stateMachine->AddState(new CharacterRagdollState(context_, this));
    stateMachine->AddState(new ThugPushBackState(context_, this));
    stateMachine->AddState(new CharacterAlignState(context_, this));
    stateMachine->AddState(new AnimationTestState(context_, this));

    if (RADIO::game_type == 0)
    {
        stateMachine->AddState(new ThugCounterState(context_, this));
        stateMachine->AddState(new ThugHitState(context_, this));
        stateMachine->AddState(new ThugAttackState(context_, this));
        stateMachine->AddState(new ThugGetUpState(context_, this));
        stateMachine->AddState(new ThugDeadState(context_, this));
//        stateMachine->AddState(new ThugBeatDownHitState(context_, this));
//        stateMachine->AddState(new ThugBeatDownEndState(context_, this));
        stateMachine->AddState(new ThugStunState(context_, this));
    }

    ChangeState("StandState");
///*
    // Красная капсула для AI только.
    if(RADIO::d_log)
        Log::Write(-1," Create Thug AI-Trigger Physics Capsule\n");
    
    Node* collisionNode = sceneNode->CreateChild("Collision");
    CollisionShape* shape = collisionNode->CreateComponent<CollisionShape>();
    shape->SetCapsule(KEEP_DIST * 2, CHARACTER_HEIGHT, Vector3(0, CHARACTER_HEIGHT/2, 0));
    RigidBody* body = collisionNode->CreateComponent<RigidBody>();
    body->SetMass(10);
    body->SetCollisionLayer(COLLISION_LAYER_AI);
    body->SetCollisionMask(COLLISION_LAYER_AI);
    body->SetKinematic(true);
    body->SetTrigger(true);
    body->SetCollisionEventMode(COLLISION_ALWAYS);
//*/
    attackDamage = 20;

    walkAlignAnimation = RADIO::GetAnimationName(MOVEMENT_GROUP_THUG + "Step_Forward");
}

void Thug::DebugDraw(DebugRenderer* debug)
{
    Character::DebugDraw(debug);
}

bool Thug::CanAttack()
{
    EnemyManager* em = RADIO::g_em;
    if (!em)
        return false;
    int num = em->GetNumOfEnemyAttackValid();
    if (num >= MAX_NUM_OF_ATTACK)
        return false;
    if (!target->CanBeAttacked())
        return false;
    return true;
}

bool Thug::Attack()
{
    if (!CanAttack())
        return false;
    if (KeepDistanceWithEnemy())
        return false;
    ChangeState("AttackState");
    return true;
}

bool Thug::Redirect()
{
    ChangeState("RedirectState");
    return true;
}

void Thug::CommonStateFinishedOnGroud()
{
    ChangeState("StandState");
}

bool Thug::OnDamage(GameObject* attacker, const Vector3& position, const Vector3& direction, int damage, bool weak)
{
    if (!CanBeAttacked())
    {
        if(RADIO::d_log)
            Log::Write(-1," OnDamage failed because I can no be attacked (" + GetName() + ")\n");
        return false;
    }
    
    if(RADIO::d_log)
        Log::Write(-1," " + GetName() + " OnDamage: pos = " + position.ToString() + " dir = " + direction.ToString() + " damage = " + String(damage) + " weak = " + String(weak) + "\n");

    health -= damage;
    health = Max(0, health);
    SetHealth(health);

    int r = Random(4);
    // if (r > 1)
    {
        Node* floorNode = GetScene()->GetChild("floor", true);
        if (floorNode)
        {
            DecalSet* decal = floorNode->GetComponent<DecalSet>();
            if (!decal)
            {
                ResourceCache* cache = GetSubsystem<ResourceCache>();
                decal = floorNode->CreateComponent<DecalSet>();
                decal->SetMaterial(cache->GetResource<Material>("Materials/Blood_Splat.xml"));
                decal->SetCastShadows(false);
                decal->SetViewMask(0x7fffffff);
                //decal.material = cache.GetResource("Material", "Materials/UrhoDecalAlpha.xml");
            }
            if(RADIO::d_log)
                Log::Write(-1," " + GetName() + " Creating Blood_Splat decal!\n");
            float size = Random(1.5f, 3.5f);
            float timeToLive = 5.0f;
            Vector3 pos = sceneNode->GetWorldPosition() + Vector3(0, 0, 0);
            decal->AddDecal(floorNode->GetComponent<StaticModel>(), pos, Quaternion(90, Random(360), 0), size, 1.0f, 1.0f, Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), timeToLive);
        }
    }

    Node* attackNode = attacker->GetNode();

    Vector3 v = direction * -1;
    v.y_ = 1.0f;
    v.Normalize();
    v *= RADIO::GetRagdollForce();

    if (health <= 0)
    {
        if(RADIO::d_log)
            Log::Write(-1," " + GetName() + " OnDamage HEALTH <= 0\n");
    
        v *= 1.5f;
        MakeMeRagdoll(v, position);
        OnDead();
    }
    else
    {
        float diff = ComputeAngleDiff(attackNode);
        if (weak) 
        {
            int index = 0;
            if (diff < 0)
                index = 1;
            if (Abs(diff) > 135)
                index = 2 + Random(2);
            sceneNode->SetVar(ANIMATION_INDEX, index);
            ChangeState("HitState");
        }
        else 
        {
            MakeMeRagdoll(v, position);
        }
    }
    return true;
}

void Thug::RequestDoNotMove()
{
    Character::RequestDoNotMove();
    StringHash nameHash = stateMachine->currentState->nameHash;
    if (HasFlag(FLAGS_MOVING))
    {
        ChangeState("StandState");
        return;
    }

    if (nameHash == HIT_STATE)
    {
        // I dont know how to do ...
        // special case
        motion_translateEnabled = false;
    }
}

int Thug::GetSperateDirection(int& outDir)
{
    Node* _node = sceneNode->GetChild("Collision");
    if (!_node)
        return 0;

    RigidBody* body = _node->GetComponent<RigidBody>();
    if (!body)
        return 0;

    int len = 0;
    Vector3 myPos = sceneNode->GetWorldPosition();
    PODVector<RigidBody*> neighbors;
    body->GetCollidingBodies(neighbors);
    float totalAngle = 0;

    for (unsigned int i = 0; i < neighbors.Size(); ++i)
    {
        Node* n_node = neighbors[i]->GetNode()->GetParent();
        if (!n_node)
            continue;

        //URHO3D_LOGINFO("neighbors[" + i + "] = " + n_node.name);

        Character* object = static_cast<Character*>(n_node->GetComponent<Character>()); // TODO:
        if (!object)
            continue;

        if (object->HasFlag(FLAGS_MOVING))
            continue;

        ++len;

        float angle = ComputeAngleDiff(object->sceneNode);
        if (angle < 0)
            angle += 180;
        else
            angle = 180 - angle;

        //URHO3D_LOGINFO("neighbors angle=" + angle);
        totalAngle += angle;
    }

    if (len == 0)
        return 0;

    outDir = RADIO::DirectionMapToIndex(totalAngle / len, 4);

    if (RADIO::d_log)
        Log::Write(-1," " + GetName() + " GetSperateDirection() totalAngle=" + String(totalAngle) 
            + " outDir=" + String(outDir) + " len=" + String(len)+"\n");

    return len;
}

void Thug::CheckAvoidance(float dt)
{
    checkAvoidanceTimer += dt;
    if (checkAvoidanceTimer >= checkAvoidanceTime)
    {
        checkAvoidanceTimer -= checkAvoidanceTime;
        CheckCollision();
    }
}

void Thug::ClearAvoidance()
{
    checkAvoidanceTimer = 0.0;
    checkAvoidanceTime = Random(0.05f, 0.1f);
}

bool Thug::KeepDistanceWithEnemy()
{
    if (HasFlag(FLAGS_NO_MOVE))
        return false;
    int dir = -1;
    if (GetSperateDirection(dir) == 0)
        return false;
    // URHO3D_LOGINFO(GetName() + " CollisionAvoidance index=" + dir);
    MultiMotionState* state = static_cast<MultiMotionState*>(FindState("StepMoveState"));
    Motion* motion = state->motions[dir];
    Vector4 motionOut = motion->GetKey(motion->endTime);
    Vector3 endPos = sceneNode->GetWorldRotation() * Vector3(motionOut.x_, motionOut.y_, motionOut.z_) + sceneNode->GetWorldPosition();
    Vector3 diff = endPos - target->sceneNode->GetWorldPosition();
    diff.y_ = 0;
    if((diff.Length() - COLLISION_SAFE_DIST) < -0.25f)
    {
        if(RADIO::d_log)
            Log::Write(-1," " + GetName() + " can not avoid collision because player is in front of me.\n");
        return false;
    }
    sceneNode->SetVar(ANIMATION_INDEX, dir);
    ChangeState("StepMoveState");
    return true;
}

bool Thug::KeepDistanceWithPlayer(float max_dist)
{
    if (HasFlag(FLAGS_NO_MOVE))
        return false;
    float dist = GetTargetDistance() - COLLISION_SAFE_DIST;
    if (dist >= max_dist)
        return false;
    int index = RadialSelectAnimation(4);
    index = (index + 2) % 4;
    // URHO3D_LOGINFO(GetName() + " KeepDistanceWithPlayer index=" + index + " max_dist=" + max_dist);
    sceneNode->SetVar(ANIMATION_INDEX, index);
    ChangeState("StepMoveState");
    return true;
}

void Thug::CheckCollision()
{
    if (KeepDistanceWithPlayer())
        return;
    KeepDistanceWithEnemy();
}

bool Thug::Distract()
{
    ChangeState("DistractState");
    return true;
}



