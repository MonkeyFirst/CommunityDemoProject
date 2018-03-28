
#include <Urho3D/Urho3DAll.h>

#include "Character/Player.h"
#include "Character/PlayerCombat.h"

#include "States/GameManager.h"
#include "Camera/CameraManager.h"
#include "Motions/MotionManager.h"

#include "radio.h"

enum AttackStateType
{
    ATTACK_STATE_ALIGN,
    ATTACK_STATE_BEFORE_IMPACT,
    ATTACK_STATE_AFTER_IMPACT,
};

// PlayerAttackState ==================================

PlayerAttackState::PlayerAttackState(Context* context, Character* c) :
    CharacterState(context, c)
{
    alignTime = 0.2f;

    forwadCloseNum = 0;
    leftCloseNum = 0;
    rightCloseNum = 0;
    backCloseNum = 0;

    slowMotionFrames = 2;

    lastAttackDirection = -1;
    lastAttackIndex = -1;

    weakAttack = true;
    slowMotion = false;
    lastKill = false;

    SetName("AttackState");
    flags = FLAGS_ATTACK;
}

PlayerAttackState::~PlayerAttackState()
{
    currentAttack = NULL;
}

void PlayerAttackState::DumpAttacks(Vector<AttackMotion*>& attacks)
{
    for (unsigned int i = 0; i < attacks.Size(); ++i)
    {
        Motion* m = attacks[i]->motion;
        if (m)
            Log::Write(-1," " + this->name + " " + m->animationName + " impactDist=" + String(attacks[i]->impactDist)+"\n");
    }
}

float PlayerAttackState::UpdateMaxDist(Vector<AttackMotion*>& attacks, float dist)
{
    if (attacks.Empty())
        return dist;

    float maxDist = attacks[attacks.Size()-1]->motion->endDistance;
    return (maxDist > dist) ? maxDist : dist;
}

void PlayerAttackState::Dump()
{
    Log::Write(-1,"\n forward attacks(closeNum=" + String(forwadCloseNum) + "): \n");
    DumpAttacks(forwardAttacks);
    Log::Write(-1,"\n right attacks(closeNum=" + String(rightCloseNum) + "): \n");
    DumpAttacks(rightAttacks);
    Log::Write(-1,"\n back attacks(closeNum=" + String(backCloseNum) + "): \n");
    DumpAttacks(backAttacks);
    Log::Write(-1,"\n left attacks(closeNum=" + String(leftCloseNum) + "): \n");
    DumpAttacks(leftAttacks);
}

void PlayerAttackState::ChangeSubState(int newState)
{
    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " changeSubState from " + String(state) + " to " + String(newState)+"\n");
    state = newState;
}

void PlayerAttackState::Update(float dt)
{
    Motion* motion = currentAttack->motion;

    Node* _node = ownner->GetNode();
    Node* tailNode = _node->GetChild("TailNode", true);
    Node* attackNode = _node->GetChild(currentAttack->boneName, true);

    if (tailNode && attackNode) 
    {
        tailNode->SetWorldPosition(attackNode->GetWorldPosition());
    }

    ownner->motion_velocity = (state == ATTACK_STATE_ALIGN) ? movePerSec : Vector3(0, 0, 0);

    float t = ownner->animCtrl->GetTime(motion->animationName);
    if (state == ATTACK_STATE_ALIGN)
    {
        if (t >= alignTime)
        {
            ChangeSubState(ATTACK_STATE_BEFORE_IMPACT);
            ownner->target->RemoveFlag(FLAGS_NO_MOVE);
        }
    }
    else if (state == ATTACK_STATE_BEFORE_IMPACT)
    {
        if (t > currentAttack->impactTime)
        {
            ChangeSubState(ATTACK_STATE_AFTER_IMPACT);
            AttackImpact();
        }
    }

    if (slowMotion)
    {
        float t_diff = currentAttack->impactTime - t;
        if (t_diff > 0 && t_diff < SEC_PER_FRAME * slowMotionFrames)
            ownner->SetSceneTimeScale(0.1f);
        else
            ownner->SetSceneTimeScale(1.0f);
    }

    ownner->CheckTargetDistance(ownner->target, PLAYER_COLLISION_DIST);

    bool finished = motion->Move(ownner, dt) == 1;
    if (finished) 
    {
        if(RADIO::d_log)
            Log::Write(-1," " + this->name + " finish attack movemont in sub state = " + String(state)+"\n");
        ownner->CommonStateFinishedOnGroud();
        return;
    }

    CheckInput(t);
    CharacterState::Update(dt);
}

void PlayerAttackState::CheckInput(float t)
{
    if (ownner->IsInAir())
        return;

    int addition_frames = slowMotion ? slowMotionFrames : 0;
    bool check_attack = t > currentAttack->impactTime + SEC_PER_FRAME * ( HIT_WAIT_FRAMES + 1 + addition_frames);
    bool check_others = t > currentAttack->impactTime + SEC_PER_FRAME * addition_frames;
    ownner->ActionCheck(check_attack, check_others, check_others, check_others); // TODO: 2 is  bool bDistract
}

void PlayerAttackState::PickBestMotion(Vector<AttackMotion*>& attacks, int dir)
{
    Vector3 myPos = ownner->GetNode()->GetWorldPosition();
    Vector3 enemyPos = ownner->target->GetNode()->GetWorldPosition();
    Vector3 diff = enemyPos - myPos;
    diff.y_ = 0;
    float toEnenmyDistance = diff.Length() - PLAYER_COLLISION_DIST;
    if (toEnenmyDistance < 0.0f)
        toEnenmyDistance = 0.0f;
    int bestIndex = 0;
    diff.Normalize();

    int index_start = -1;
    int index_num = 0;

    float min_dist = Max(0.0f, toEnenmyDistance - ATTACK_DIST_PICK_RANGE/2.0f);
    float max_dist = toEnenmyDistance + ATTACK_DIST_PICK_RANGE;
    
    if(RADIO::d_log)
        Log::Write(-1," " + this->name + "Player attack toEnenmyDistance = " + String(toEnenmyDistance) 
               + "(" + String(min_dist) + "," + String(max_dist) + ")\n");

    for (unsigned int i = 0; i < attacks.Size(); ++i)
    {
        AttackMotion* am = attacks[i];
        // URHO3D_LOGINFO("am.impactDist=" + am.impactDist);
        if (am->impactDist > max_dist)
            break;

        if (am->impactDist > min_dist)
        {
            if (index_start == -1)
                index_start = i;
            index_num ++;
        }
    }

    if (index_num == 0)
    {
        if (toEnenmyDistance > attacks[attacks.Size() - 1]->impactDist)
            bestIndex = attacks.Size() - 1;
        else
            bestIndex = 0;
    }
    else
    {
        int r_n = Random(index_num);
        bestIndex = index_start + r_n % index_num;
        if (lastAttackDirection == dir && bestIndex == lastAttackIndex)
        {
            if(RADIO::d_log)
                Log::Write(-1," " + this->name + " Repeat Attack index index_num=" + String(index_num)+"\n");
            bestIndex = index_start + (r_n + 1) % index_num;
        }
        lastAttackDirection = dir;
        lastAttackIndex = bestIndex;
    }

    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " Attack bestIndex=" + String(bestIndex) 
            + " index_start=" + String(index_start) + " index_num=" + String(index_num)+"\n");

    currentAttack = attacks[bestIndex];
    alignTime = currentAttack->impactTime;

    predictPosition = myPos + diff * toEnenmyDistance;
    
    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " PlayerAttack dir=" + String(lastAttackDirection) 
            + " index=" + String(lastAttackIndex) + " Pick attack motion = " + currentAttack->motion->animationName+"\n");
}

void PlayerAttackState::StartAttack()
{
    Character* chr = ownner; 
    Player* p = static_cast<Player*>(chr);
    if (ownner->target)
    {
        state = ATTACK_STATE_ALIGN;
        float diff = ownner->ComputeAngleDiff(ownner->target->GetNode());
        int r = RADIO::DirectionMapToIndex(diff, 4);

        if (RADIO::d_log)
            Log::Write(-1," " + this->name + " Attack-align r-index = " + String(r) + " diff = " + String(diff)+"\n");

        if (r == 0)
            PickBestMotion(forwardAttacks, r);
        else if (r == 1)
            PickBestMotion(rightAttacks, r);
        else if (r == 2)
            PickBestMotion(backAttacks, r);
        else if (r == 3)
            PickBestMotion(leftAttacks, r);

        ownner->target->RequestDoNotMove();
        p->lastAttackId = ownner->target->GetNode()->GetID();
    }
    else
    {
        int index = ownner->RadialSelectAnimation(4);
        if (index == 0)
            currentAttack = forwardAttacks[Random(forwadCloseNum)];
        else if (index == 1)
            currentAttack = rightAttacks[Random(rightCloseNum)];
        else if (index == 2)
            currentAttack = backAttacks[Random(backCloseNum)];
        else if (index == 3)
            currentAttack = leftAttacks[Random(leftCloseNum)];
        state = ATTACK_STATE_BEFORE_IMPACT;
        p->lastAttackId = M_MAX_UNSIGNED;

        // lost combo
        p->combo = 0;
        p->StatusChanged();
    }

    Motion* motion = currentAttack->motion;
    motion->Start(ownner);
    weakAttack = static_cast<Player*>(chr)->combo < MAX_WEAK_ATTACK_COMBO;
    slowMotion = (p->combo >= 3) ? (Random(10) == 1) : false;

    if (ownner->target)
    {
        motionPosition = motion->GetFuturePosition(ownner, currentAttack->impactTime);
        movePerSec = ( predictPosition - motionPosition ) / alignTime;
        movePerSec.y_ = 0;

        //if (attackEnemy.HasFlag(FLAGS_COUNTER))
        //    slowMotion = true;

        lastKill = p->CheckLastKill();
    }
    else
    {
        weakAttack = false;
        slowMotion = false;
    }

    if (lastKill)
    {
        ownner->SetSceneTimeScale(LAST_KILL_SPEED);
        weakAttack = false;
        slowMotion = false;
    }

    ownner->SetNodeEnabled("TailNode", true);
}

void PlayerAttackState::Enter(State* lastState)
{
//    Log::Write(-1," ### Player::AttackState Enter from " + lastState->name  + " \n");

    lastKill = false;
    slowMotion = false;
    currentAttack = NULL;
    state = ATTACK_STATE_ALIGN;
    movePerSec = Vector3(0, 0, 0);
    StartAttack();
    //ownner.SetSceneTimeScale(0.25f);
    //ownner.SetTimeScale(1.5f);
    CharacterState::Enter(lastState);
}

void PlayerAttackState::Exit(State* nextState)
{
    CharacterState::Exit(nextState);
    ownner->SetNodeEnabled("TailNode", false);
    //if (nextState !is this)
    //    cast<Player>(ownner).lastAttackId = M_MAX_UNSIGNED;
    if (ownner->target)
        ownner->target->RemoveFlag(FLAGS_NO_MOVE);
    currentAttack = NULL;
    ownner->SetSceneTimeScale(1.0f);
    
//    Log::Write(-1," ### Player::AttackState Exit to " + nextState->name  + " \n");
}

void PlayerAttackState::DebugDraw(DebugRenderer* debug)
{
    if (!currentAttack || !ownner->target)
        return;
    debug->AddLine(ownner->GetNode()->GetWorldPosition(), ownner->target->GetNode()->GetWorldPosition(), Color::RED, false);
    debug->AddCross(predictPosition, 0.5f, Color(0.25f, 0.28f, 0.7f), false);
    debug->AddCross(motionPosition, 0.5f, Color(0.75f, 0.28f, 0.27f), false);
}

String PlayerAttackState::GetDebugText()
{
    return " name=" + name + " timeInState=" + String(timeInState) + "\n" +
            "currentAttack=" + currentAttack->motion->animationName +
            " weakAttack=" + String(weakAttack) +
            " slowMotion=" + String(slowMotion) +
            "\n";
}

bool PlayerAttackState::CanReEntered()
{
    return true;
}

void PlayerAttackState::AttackImpact()
{
    Character* e = ownner->target;

    if (!e)
        return;

    Node* _node = ownner->GetNode();
    Vector3 dir = _node->GetWorldPosition() - e->GetNode()->GetWorldPosition();
    dir.y_ = 0;
    dir.Normalize();
    
    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " " +  e->GetName() + " OnDamage!!!\n");

    Node* n = _node->GetChild(currentAttack->boneName, true);
    Vector3 position = _node->GetWorldPosition();
    if (n)
        position = n->GetWorldPosition();

    int damage = ownner->attackDamage;
    if (lastKill)
        damage = 9999;
    else
        damage = Random(ownner->attackDamage, ownner->attackDamage + 20);
    bool b = e->OnDamage(ownner, position, dir, damage, weakAttack);
    if (!b)
        return;

    ownner->SpawnParticleEffect(position, "Particle/SnowExplosion.xml", 5.0f, 5.0f);
    ownner->SpawnParticleEffect(position, "Particle/HitSpark.xml", 1.0f, 0.6f);

    int sound_type = (e->health == 0) ? 1 : 0;
    ownner->PlayRandomSound(sound_type);
    ownner->OnAttackSuccess(e);
}

void PlayerAttackState::PostInit(float closeDist)
{
// Vector не имеет функции Sort()
/*
    forwardAttacks.Sort();
    leftAttacks.Sort();
    rightAttacks.Sort();
    backAttacks.Sort();
*/
    float dist = 0.0f;
    dist = UpdateMaxDist(forwardAttacks, dist);
    dist = UpdateMaxDist(leftAttacks, dist);
    dist = UpdateMaxDist(rightAttacks, dist);
    dist = UpdateMaxDist(backAttacks, dist);

    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " " + ownner->GetName() + " max attack dist = " + String(dist)+"\n");
    dist += 10.0f;

    RADIO::MAX_ATTACK_DIST = Min(RADIO::MAX_ATTACK_DIST, dist);
//    max_attack_dist = Min(MAX_ATTACK_DIST, dist);

    for (unsigned int i = 0; i < forwardAttacks.Size(); ++i)
    {
        if (forwardAttacks[i]->impactDist >= closeDist)
            break;
        forwadCloseNum++;
    }
    for (unsigned int i = 0; i < rightAttacks.Size(); ++i)
    {
        if (rightAttacks[i]->impactDist >= closeDist)
            break;
        rightCloseNum++;
    }
    for (unsigned int i = 0; i < backAttacks.Size(); ++i)
    {
        if (backAttacks[i]->impactDist >= closeDist)
            break;
        backCloseNum++;
    }
    for (unsigned int i = 0; i < leftAttacks.Size(); ++i)
    {
        if (leftAttacks[i]->impactDist >= closeDist)
            break;
        leftCloseNum++;
    }

    if (RADIO::d_log)
        Dump();
}


// PlayerCounterState ==================================


PlayerCounterState::PlayerCounterState(Context* context, Character* c) :
    CharacterCounterState(context, c)
{
    lastCounterIndex = -1;
    lastCounterDirection = -1;
    intCache.Reserve(50); // Vector<int> intCache
}

PlayerCounterState::~PlayerCounterState()
{}

void PlayerCounterState::Update(float dt)
{
    //if (!counterEnemy || !currentMotion) // SingleEnemy
    if (counterEnemies.Empty() || !currentMotion) // MultiCounter
    {
        ownner->CommonStateFinishedOnGroud(); // Something Error Happened
        return;
    }
    if (sub_state == COUNTER_WAITING)
    {
/*
        // SingleEnemy
        if (counterEnemy->GetState()->nameHash == this->nameHash)
            StartAnimating();
*/
        // MultiCounter
        unsigned int n = 0;
        for (unsigned int i = 0; i < counterEnemies.Size(); ++i)
        {
            if (counterEnemies[i]->GetState()->nameHash == this->nameHash)
                n ++;
        }

        if (n == counterEnemies.Size())
            StartAnimating();
    }
    CharacterCounterState::Update(dt);
}

void PlayerCounterState::Enter(State* lastState)
{
    if(RADIO::d_log)
        Log::Write(-1," PlayerCounterState: Enter (" + ownner->GetName() + ") lastState = " + lastState->name + "\n");

    unsigned int t = GetSubsystem<Time>()->GetSystemTime();

    if (lastState->nameHash == ALIGN_STATE)
    {
        StartAnimating();
    }
    else
    {

// MultiCounter
        if(RADIO::d_log)
            Log::Write(-1," PlayerCounter-> counterEnemies len = " + String(counterEnemies.Size()) +"\n");
        type = counterEnemies.Size();

        // POST_PROCESS
        for (int i = 0; i < type; ++i)
        {
            Enemy* e = counterEnemies[i];
            e->ChangeState("CounterState");

            CharacterCounterState* s = static_cast<CharacterCounterState*>(e->GetState());

            s->index = -1;
            s->type = type;
            s->ChangeSubState(COUNTER_NONE);
        }

        if (counterEnemies.Size() == 3)
        {
            for (unsigned int i = 0; i < tripleCounterMotions.Size(); ++i)
            {
                if (TestTrippleCounterMotions(i) == 3)
                    break;
            }

            for (unsigned int i = 0; i < counterEnemies.Size(); ++i)
            {
                Enemy* e = counterEnemies[i];
                CharacterCounterState* s = static_cast<CharacterCounterState*>(e->GetState());
                if (s->index < 0)
                {
                    e->CommonStateFinishedOnGroud();
                    counterEnemies.Erase(i);
                }
            }

            ChangeSubState(COUNTER_WAITING);
        }

        if (counterEnemies.Size() == 2)
        {
            for (unsigned int i = 0; i < doubleCounterMotions.Size(); ++i)
            {
                if (TestDoubleCounterMotions(i) == 2)
                    break;
            }

            for (unsigned int i = 0; i < counterEnemies.Size(); ++i)
            {
                Enemy* e = counterEnemies[i];
                CharacterCounterState* s = static_cast<CharacterCounterState*>(e->GetState());
                if (s->index < 0)
                {
                    e->CommonStateFinishedOnGroud();
                    counterEnemies.Erase(i);
                }
            }

            ChangeSubState(COUNTER_WAITING);
        }

        if (counterEnemies.Size() == 1)
        {

// SingleEnemy

            Node* myNode = ownner->GetNode();
            Vector3 myPos = myNode->GetWorldPosition();
    
            //Enemy* e = counterEnemy;
            Enemy* e = counterEnemies[0];

// Player::PickCounterEnemy() проходит по всему списку врагов и вызывает e->CanBeCountered() функцию персонажа
// которая просто проверяет наличие флага HasFlag(FLAGS_COUNTER) у него.
// Добавил удаление флага в состояние "RagdollState" в  CharacterRagdollState::Enter функцию.

// Flag удаляется также (должен):
// ThugAttackState::Exit
// ThugAttackState::OnAnimationTrigger (добавляется и удаляется)
// Тоесть, прежде чем войти в рэгдолл состояние бандит должен выйти из состояния атаки.

/*            
            if(e->IsInState(StringHash("RagdollState")))
            {
                Log::Write(-1," " + ownner->GetName() + " PlayerCounterState: ERROR CALL " + e->GetName() + " in RagdollState\n");
                ownner->ChangeState_Hash(lastState->nameHash);//("StandState");
                //e->ChangeState("StandState");
                counterEnemy = NULL;
                return;            
            }
*/            
            Node* eNode = e->GetNode();
            float dAngle = ownner->ComputeAngleDiff(eNode);
            bool isBack = false;
            if (Abs(dAngle) > 90)
                isBack = true;
    
            e->ChangeState("CounterState");
    
            int attackType = eNode->GetVars()[ATTACK_TYPE]->GetInt();
            CharacterCounterState* s = static_cast<CharacterCounterState*>(e->GetState());
    
            Vector<Motion*> counterMotions = GetCounterMotions(attackType, isBack);
            Vector<Motion*> eCounterMotions = s->GetCounterMotions(attackType, isBack);
            
            // Твик дабы избежать ситуации когда нужные анимации отсутствуют у игрока или врага.
            // Просто прервем выполнение.
            if(counterMotions.Empty() || eCounterMotions.Empty())
            {
                Log::Write(-1," PlayerCounterState: Enter CounterMotions NOT FOUND attackType = " + String(attackType) + " isBack = " + String(isBack) +"\n");
                ownner->ChangeState("StandState");
                e->ChangeState("StandState");
                return;
            }
            
            intCache.Clear(); // Vector<int> intCache
            float maxDistSQR = COUNTER_ALIGN_MAX_DIST * COUNTER_ALIGN_MAX_DIST;
            float bestDistSQR = 999999;
            int bestIndex = -1;
    
            for (unsigned int i = 0; i < counterMotions.Size(); ++i)
            {
                Motion* alignMotion = counterMotions[i];
                Motion* baseMotion = eCounterMotions[i];
                Vector4 v4 = RADIO::GetTargetTransform(eNode, alignMotion, baseMotion);
                Vector3 v3 = Vector3(v4.x_, myPos.y_, v4.z_);
                float distSQR = (v3 - myPos).LengthSquared();
                if (distSQR < bestDistSQR)
                {
                    bestDistSQR = distSQR;
                    bestIndex = int(i);
                }
                if (distSQR > maxDistSQR)
                    continue;
                intCache.Push(i); // Vector<int> intCache
            }
    
            if(RADIO::d_log)
                Log::Write(-1," PlayerCounterState: intCache.Size = " + String(intCache.Size()) + "\n");
            int cur_direction = GetCounterDirection(attackType, isBack);
            int idx;
            if (intCache.Empty())
            {
                idx = bestIndex;
            }
            else
            {
                int k = intCache.Size();
                int z = Random(k);
                //int k = Random(intCache.Size());
                idx = intCache[z];
                if (cur_direction == lastCounterDirection && idx == lastCounterIndex)
                {
                    z = (z + 1) % intCache.Size();
                    idx = intCache[z];
                }
            }
    
            lastCounterDirection = cur_direction;
            lastCounterIndex = idx;
    
            currentMotion = counterMotions[idx];
            s->currentMotion = eCounterMotions[idx];
    
            if(RADIO::d_log)
                Log::Write(-1," Counter-align angle-diff = " + String(dAngle) + " isBack = " + String(isBack) + " name: " + currentMotion->animationName + "\n");
    
            s->ChangeSubState(COUNTER_WAITING);
    
            Vector4 vt = RADIO::GetTargetTransform(eNode, currentMotion, s->currentMotion);
            SetTargetTransform(Vector3(vt.x_, myPos.y_, vt.z_), vt.w_);
            StartAligning();
        }
        else
        {
            for (unsigned int i = 0; i < counterEnemies.Size(); ++i)
            {
                CharacterCounterState* s = static_cast<CharacterCounterState*>(counterEnemies[i]->GetState());
                s->StartAligning();
            }
        }
    }

    if(RADIO::d_log)
        Log::Write(-1," PlayerCounterState: Enter time-cost = " + String(GetSubsystem<Time>()->GetSystemTime() - t) +"\n");
    CharacterState::Enter(lastState);

}

void PlayerCounterState::Exit(State* nextState)
{
    if(RADIO::d_log)
        Log::Write(-1," PlayerCounterState: Exit (" + ownner->GetName() + ") nextState = " + nextState->name + "\n");
    
    CharacterCounterState::Exit(nextState);
    
    if (nextState != this && nextState->nameHash != ALIGN_STATE)
        //counterEnemy = NULL; // SingleEnemy
        counterEnemies.Clear(); // MultiCounter

}

void PlayerCounterState::StartAnimating()
{
    StartCounterMotion();

// MultiCounter
    for (unsigned int i = 0; i < counterEnemies.Size(); ++i)
    {
        CharacterCounterState* s = static_cast<CharacterCounterState*>(counterEnemies[i]->GetState());
        s->StartCounterMotion();
    }
/*
// SingleEnemy
    if (counterEnemy)
    {
        CharacterCounterState* s = static_cast<CharacterCounterState*>(counterEnemy->GetState());
        s->StartCounterMotion();
    }
*/
}

// SingleEnemy and MultiCounter
void PlayerCounterState::StartCounterMotion()
{
    CharacterCounterState::StartCounterMotion();
    RADIO::g_cm->CheckCameraAnimation(currentMotion->name);
}

// SingleEnemy and MultiCounter
void PlayerCounterState::OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData)
{
    // Добавлено для получения VariantMap из входящего VariantMap
    // Переменную DATA типа VariantMap это единственное что получаем на входе. 
//    VariantMap xdata = eventData[DATA]->GetVariantMap(); // add

    // Получает уже изъятые из P_DATA данные.

    StringHash name = eventData[NAME]->GetStringHash();

    if (name == READY_TO_FIGHT)
        ownner->OnCounterSuccess();

    CharacterState::OnAnimationTrigger(animState, eventData);
}

// SingleEnemy and MultiCounter
bool PlayerCounterState::CanReEntered()
{
    return true;
}

// ONLY IN MULTICOUNTER FUNCTIONS
void PlayerCounterState::ChooseBestIndices(Motion* alignMotion, int index)
{
    Vector4 v4 = RADIO::GetTargetTransform(ownner->GetNode(), alignMotion, currentMotion);
    Vector3 v3 = Vector3(v4.x_, 0.0f, v4.z_);

    float minDistSQR = 999999;
    int possed = -1;

    for (unsigned int i = 0; i < counterEnemies.Size(); ++i)
    {
        Enemy* e = counterEnemies[i];
        CharacterCounterState* s = static_cast<CharacterCounterState*>(e->GetState());

        if (s->index >= 0)
            continue;

        Vector3 ePos = e->GetNode()->GetWorldPosition();
        Vector3 diff = v3 - ePos;
        diff.y_ = 0;

        float disSQR = diff.LengthSquared();
        if (disSQR < minDistSQR)
        {
            minDistSQR = disSQR;
            possed = i;
        }
    }

    Enemy* e = counterEnemies[possed];
    if (minDistSQR > GOOD_COUNTER_DIST * GOOD_COUNTER_DIST)
    {
        if(RADIO::d_log)
            Log::Write(-1," " + alignMotion->name + " too far\n");
        return;
    }

    CharacterCounterState* s = static_cast<CharacterCounterState*>(e->GetState());
    s->currentMotion = alignMotion;
    s->index = possed;
    s->SetTargetTransform(Vector3(v4.x_, e->GetNode()->GetWorldPosition().y_, v4.z_), v4.w_);
}

int PlayerCounterState::GetValidNumOfCounterEnemy()
{
    int num = 0;
    for (unsigned int i = 0; i < counterEnemies.Size(); ++i)
    {
        Enemy* e = counterEnemies[i];
        CharacterCounterState* s = static_cast<CharacterCounterState*>(e->GetState());
        if (s->index >= 0)
            num ++;
    }
    return num;
}

int PlayerCounterState::TestTrippleCounterMotions(int i)
{
    for (unsigned int k = 0; k < counterEnemies.Size(); ++k)
        static_cast<CharacterCounterState*>(counterEnemies[k]->GetState())->index = -1;

    CharacterCounterState* s = static_cast<CharacterCounterState*>(counterEnemies[0]->GetState());
    currentMotion = tripleCounterMotions[i];
    Motion* m1 = s->tripleCounterMotions[i * 3 + 0];
    ChooseBestIndices(m1, 0);
    Motion* m2 = s->tripleCounterMotions[i * 3 + 1];
    ChooseBestIndices(m2, 1);
    Motion* m3 = s->tripleCounterMotions[i * 3 + 2];
    ChooseBestIndices(m3, 2);
    return GetValidNumOfCounterEnemy();
}

int PlayerCounterState::TestDoubleCounterMotions(int i)
{
    for (unsigned int k = 0; k < counterEnemies.Size(); ++k)
        static_cast<CharacterCounterState*>(counterEnemies[k]->GetState())->index = -1;

    CharacterCounterState* s = static_cast<CharacterCounterState*>(counterEnemies[0]->GetState());
    currentMotion = doubleCounterMotions[i];
    Motion* m1 = s->doubleCounterMotions[i * 2 + 0];
    ChooseBestIndices(m1, 0);
    Motion* m2 = s->doubleCounterMotions[i * 2 + 1];
    ChooseBestIndices(m2, 1);
    return GetValidNumOfCounterEnemy();
}


// PlayerHitState ==================================

PlayerHitState::PlayerHitState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    SetName("HitState");
}

PlayerHitState::~PlayerHitState()
{}

// PlayerDeadState ==================================

PlayerDeadState::PlayerDeadState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    SetName("DeadState");
    state = 0;
}

PlayerDeadState::~PlayerDeadState()
{}

void PlayerDeadState::Enter(State* lastState)
{
    state = 0;
    MultiMotionState::Enter(lastState);
}

void PlayerDeadState::Update(float dt)
{
    if (state == 0)
    {
        if (motions[selectIndex]->Move(ownner, dt) == 1)
        {
            state = 1;
            RADIO::g_gm->OnCharacterKilled(NULL, ownner);
        }
    }
    CharacterState::Update(dt);
}

// PlayerBeatDownEndState ==================================

PlayerBeatDownEndState::PlayerBeatDownEndState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    SetName("BeatDownEndState");
}

PlayerBeatDownEndState::~PlayerBeatDownEndState()
{}

void PlayerBeatDownEndState::Enter(State* lastState)
{
    selectIndex = PickIndex();
    if (selectIndex >= int(motions.Size()))
    {
        if(RADIO::d_log)
            Log::Write(-1," ERROR: a large animation index=" + String(selectIndex) + " name:" + ownner->GetName() + "\n");
        selectIndex = 0;
    }

    Character* chr = ownner; 
    Player* p = static_cast<Player*>(chr);
    if (p->CheckLastKill())
        ownner->SetSceneTimeScale(LAST_KILL_SPEED);

    Character* target = ownner->target;
    if (target)
    {
        Motion* m1 = motions[selectIndex];
        ThugBeatDownEndState* state = static_cast<ThugBeatDownEndState*>(target->FindState("BeatDownEndState"));
        Motion* m2 = state->motions[selectIndex];
        Vector4 t = RADIO::GetTargetTransform(target->GetNode(), m1, m2);
        ownner->Transform(Vector3(t.x_, ownner->GetNode()->GetWorldPosition().y_, t.z_), Quaternion(0, t.w_, 0));
        ownner->GetNode()->SetVar(ANIMATION_INDEX, selectIndex);
        target->ChangeState("BeatDownEndState");
    }

    if(RADIO::d_log)
        Log::Write(-1, ownner->GetName() + " state=" + name + " pick " + motions[selectIndex]->animationName);
    motions[selectIndex]->Start(ownner);

    CharacterState::Enter(lastState);
}

void PlayerBeatDownEndState::Exit(State* nextState)
{
    if(RADIO::d_log)
        Log::Write(-1," BeatDownEndState Exit!!\n");
    ownner->SetSceneTimeScale(1.0f);
    MultiMotionState::Exit(nextState);
}

int PlayerBeatDownEndState::PickIndex()
{
    int tmp = motions.Size();
    return Random(tmp);
}

void PlayerBeatDownEndState::OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData)
{
    StringHash name = eventData[NAME]->GetStringHash();
    if (name == IMPACT)
    {
        Node* boneNode = ownner->GetNode()->GetChild(eventData[VALUE]->GetString(), true);
        Vector3 position = ownner->GetNode()->GetWorldPosition();
        if (boneNode)
            position = boneNode->GetWorldPosition();
        ownner->SpawnParticleEffect(position, "Particle/SnowExplosionFade.xml", 5, 10.0f);
        ownner->SpawnParticleEffect(position, "Particle/HitSpark.xml", 0.5f, 0.5f);
        ownner->PlayRandomSound(1);
        combatReady = true;
        Character* target = ownner->target;
        if (target)
        {
            Vector3 dir = ownner->motion_startPosition - target->GetNode()->GetWorldPosition();
            dir.y_ = 0;
            target->OnDamage(ownner, position, dir, 9999, false);
            ownner->OnAttackSuccess(target);
        }
        return;
    }
    CharacterState::OnAnimationTrigger(animState, eventData);
}

// PlayerBeatDownHitState ==================================

PlayerBeatDownHitState::PlayerBeatDownHitState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    SetName("BeatDownHitState");
    beatIndex = 0;
    beatNum = 0;
    maxBeatNum = 15;
    minBeatNum = 7;
    beatTotal = 0;
    attackPressed = false;
    flags = FLAGS_ATTACK;
}

PlayerBeatDownHitState::~PlayerBeatDownHitState()
{}

bool PlayerBeatDownHitState::CanReEntered()
{
    return true;
}

bool PlayerBeatDownHitState::IsTransitionNeeded(float curDist)
{
    return false;
}

void PlayerBeatDownHitState::Update(float dt)
{
    InputManager* gInput = RADIO::g_im;
    // Print("PlayerBeatDownHitState::Update() " + dt);
    Character* target = ownner->target;
    if (!target)
    {
        ownner->CommonStateFinishedOnGroud();
        return;
    }

    if (gInput->IsAttackPressed())
        attackPressed = true;

    if (combatReady && attackPressed)
    {
        ++ beatIndex;
        ++ beatNum;
        beatIndex = beatIndex % motions.Size();
        ownner->ChangeState("BeatDownHitState");
        return;
    }

    if (gInput->IsCounterPressed())
    {
        ownner->Counter();
        return;
    }

    MultiMotionState::Update(dt);
}

void PlayerBeatDownHitState::Enter(State* lastState)
{
    float curDist = ownner->GetTargetDistance();
    if (IsTransitionNeeded(curDist - PLAYER_COLLISION_DIST))
    {
        ownner->ChangeStateQueue(StringHash("TransitionState"));
        PlayerTransitionState* s = static_cast<PlayerTransitionState*>(ownner->FindState_Hash(StringHash("TransitionState")));
        s->nextStateName = this->name;
        return;
    }

    attackPressed = false;
    if (lastState != this)
    {
        beatNum = 0;
        beatTotal = Random(minBeatNum, maxBeatNum);
    }
    int index = beatIndex;

    Character* target = ownner->target;
    MultiMotionState* s = static_cast<MultiMotionState*>(ownner->target->FindState("BeatDownHitState"));
    Motion* m1 = motions[index];
    Motion* m2 = s->motions[index];

    Vector3 myPos = ownner->GetNode()->GetWorldPosition();
    if (lastState != this && lastState->nameHash != ALIGN_STATE)
    {
        Vector3 dir = myPos - target->GetNode()->GetWorldPosition();
        float e_targetRotation = Atan2(dir.x_, dir.z_);
        target->GetNode()->SetWorldRotation(Quaternion(0, e_targetRotation, 0));
    }

    Vector4 t = RADIO::GetTargetTransform(target->GetNode(), m1, m2);
    targetRotation = t.w_;
    targetPosition = Vector3(t.x_, myPos.y_, t.z_);
    if (lastState != this && lastState->nameHash != ALIGN_STATE)
    {
        CharacterAlignState* state = static_cast<CharacterAlignState*>(ownner->FindState_Hash(ALIGN_STATE));
        state->Start(this->nameHash, targetPosition, targetRotation, 0.1f);
        ownner->ChangeStateQueue(ALIGN_STATE);
    }
    else
    {
        ownner->GetNode()->SetWorldRotation(Quaternion(0, targetRotation, 0));
        ownner->GetNode()->SetWorldPosition(targetPosition);
        target->GetNode()->SetVar(ANIMATION_INDEX, index);
        motions[index]->Start(ownner);
        selectIndex = index;
        target->ChangeState("BeatDownHitState");
    }

    CharacterState::Enter(lastState);
}

void PlayerBeatDownHitState::OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData)
{
    StringHash name = eventData[NAME]->GetStringHash();
    if (name == IMPACT)
    {
        // Print("BeatDownHitState On Impact");
        combatReady = true;
        Node* boneNode = ownner->GetNode()->GetChild(eventData[VALUE]->GetString(), true);
        Vector3 position = ownner->GetNode()->GetWorldPosition();
        if (boneNode)
            position = boneNode->GetWorldPosition();
        ownner->SpawnParticleEffect(position, "Particle/SnowExplosionFade.xml", 5, 10.0f);
        ownner->SpawnParticleEffect(position, "Particle/HitSpark.xml", 0.5f, 0.4f);
        ownner->PlayRandomSound(0);

        ownner->OnAttackSuccess(ownner->target);

        if (beatNum >= beatTotal)
            ownner->ChangeState("BeatDownEndState");
        return;
    }
    CharacterState::OnAnimationTrigger(animState, eventData);
}

int PlayerBeatDownHitState::PickIndex()
{
    return beatIndex;
}

void PlayerBeatDownHitState::DebugDraw(DebugRenderer* debug)
{
    debug->AddCross(targetPosition, 1.0f, Color::RED, false);
    RADIO::DebugDrawDirection(debug, ownner->GetNode()->GetWorldPosition(), targetRotation, Color::YELLOW);
}

String PlayerBeatDownHitState::GetDebugText()
{
    return " name=" + name + " timeInState=" + String(timeInState) + " current motion=" + motions[selectIndex]->animationName + "\n" +
    " combatReady=" + String(combatReady) + " attackPressed=" + String(attackPressed) + "\n";
}

// PlayerTransitionState ==================================

PlayerTransitionState::PlayerTransitionState(Context* context, Character* c) :
    SingleMotionState(context, c)
{
    SetName("TransitionState");
}

PlayerTransitionState::~PlayerTransitionState()
{}

void PlayerTransitionState::OnMotionFinished()
{
    // Print(ownner.GetName() + " state:" + name + " finshed motion:" + motion.animationName);
    if (nextStateName != String::EMPTY)
        ownner->ChangeState(nextStateName);
    else
        ownner->CommonStateFinishedOnGroud();
}

void PlayerTransitionState::Enter(State* lastState)
{
    Character* target = ownner->target;
    if (target)
    {
        target->RequestDoNotMove();
        Vector3 dir = target->GetNode()->GetWorldPosition() - ownner->GetNode()->GetWorldPosition();
        float angle = Atan2(dir.x_, dir.z_);
        ownner->GetNode()->SetWorldRotation(Quaternion(0, angle, 0));
        target->GetNode()->SetWorldRotation(Quaternion(0, angle + 180, 0));
    }
    SingleMotionState::Enter(lastState);
}

void PlayerTransitionState::Exit(State* nextState)
{
    SingleMotionState::Exit(nextState);
    if (ownner->target)
        ownner->target->RemoveFlag(FLAGS_NO_MOVE);
    if(RADIO::d_log)
        Log::Write(-1," After Player Transition Target dist = " + String(ownner->GetTargetDistance()));
}

String PlayerTransitionState::GetDebugText()
{
    return " name=" + name + " timeInState=" + String(timeInState) + " nextState=" + nextStateName + "\n";
}










