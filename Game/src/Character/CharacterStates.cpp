#include <Urho3D/Urho3DAll.h>

#include "Character/CharacterStates.h"
#include "Character/Ragdoll.h"

#include "Motions/MotionManager.h"
//#include "Levels/GameManager.h"
#include "Camera/CameraManager.h"

#include "radio.h"

CharacterState::CharacterState(Context* context, Character* c):
    State(context)
{
//    Log::Write(-1," CharacterState: Constructor\n");
    ownner = c;

    animSpeed = 1.0f;
    blendTime = 0.2f;
    startTime = 0.0f;

    combatReady = false;
    firstUpdate = true;

    // HACK: Get physics from Character is not original metod.
    lastPhysicsType = c->physicsType; //0;
    physicsType = -1;
}

CharacterState::~CharacterState()
{
//    Log::Write(-1," CharacterState: Destructor\n");
    ownner.Reset();
    ownner = NULL;
}

void CharacterState::OnAnimationTrigger(AnimationState* animState, const VariantMap& eventData)
{
//    Log::Write(-1," " + ownner->GetName() + " CharacterState ("+name+"): OnAnimationTrigger\n");

    // Изменение позволило добится адекватного исчезновения или перемещения AI-капсулы персонажей 
    // (переход в Регдолл состояние) и иногда адекватное восстановление GetUp из Регдолл

    // Добавлено для получения VariantMap из входящего VariantMap
    // Переменную DATA типа VariantMap это единственное что получаем на входе. 
//    VariantMap xdata = eventData[DATA]->GetVariantMap(); // add

    // Получает уже изъятые из P_DATA данные от Character::HandleAnimationTrigger

//Log::Write(-1, " CharacterState::OnAnimationTrigger\n");
    String logstring = String::EMPTY;//"";

    StringHash name = eventData[NAME]->GetStringHash();
    if (name == RAGDOLL_START)// || name == RAGDOLL_PERPARE)
    {
        logstring = "RAGDOLL_START";
        ownner->ChangeState("RagdollState");
    }
    else if (name == COMBAT_SOUND)
    {
        logstring = "COMBAT_SOUND";
        OnCombatSound(eventData[VALUE]->GetString(), false);
    }
    else if (name == COMBAT_SOUND_LARGE)
    {
        logstring = "COMBAT_SOUND_LARGE";
        OnCombatSound(eventData[VALUE]->GetString(), true);
    }
    else if (name == PARTICLE)
    {
        logstring = "PARTICLE";
        OnCombatParticle(eventData[VALUE]->GetString(), eventData[PARTICLE]->GetString());
    }
    else if (name == FOOT_STEP)
    {
        logstring = "FOOT_STEP";
        if (animState && animState->GetWeight() > 0.5f)
            OnFootStep(eventData[VALUE]->GetString());
    }
    else if (name == SOUND)
    {
        logstring = "SOUND";
        ownner->PlaySound(eventData[VALUE]->GetString());
    }
    else if (name == CHANGE_STATE)
    {
        logstring = "CHANGE_STATE";
        ownner->ChangeState_Hash(eventData[VALUE]->GetStringHash());
    }
    else if (name == HEALTH)
    {
        logstring = "HEALTH";
        ownner->SetHealth(eventData[VALUE]->GetInt());
    }
    else if (name == IMPACT)
    {
        logstring = "IMPACT";
        combatReady = true;
    }
    else if (name == READY_TO_FIGHT)
    {
        logstring = "READY_TO_FIGHT";
        combatReady = true;
    }
    if(RADIO::d_log && logstring != String::EMPTY)
        Log::Write(-1," " + ownner->GetName() +" CharacterState (" + this->name + "): OnAnimationTrigger " + logstring +"\n");
}

void CharacterState::OnFootStep(const String& boneName)
{
    if(RADIO::d_log)
        Log::Write(-1, " " + ownner->GetName() + " CharacterState: OnFootStep\n");

    Node* boneNode = ownner->GetNode()->GetChild(boneName, true);
    if (!boneNode)
        return;
    Vector3 pos = boneNode->GetWorldPosition();
    pos.y_ = 0.1f;
    ownner->SpawnParticleEffect(pos, "Particle/SnowExplosionFade.xml", 2, 2.5f);
}

void CharacterState::OnCombatSound(const String& boneName, bool large)
{
    if(RADIO::d_log)
        Log::Write(-1, " " + ownner->GetName() + " CharacterState: OnCombatSound\n");

    ownner->PlayRandomSound(large ? 1 : 0);

    Node* boneNode = ownner->renderNode->GetChild(boneName, true);
    if (boneNode)
        ownner->SpawnParticleEffect(boneNode->GetWorldPosition(), "Particle/SnowExplosionFade.xml", 5, 5.0f);
}

void CharacterState::OnCombatParticle(const String& boneName, const String& particleName)
{
    if(RADIO::d_log)
        Log::Write(-1, " " + ownner->GetName() + " CharacterState: OnCombatParticle\n");

    Node* boneNode = ownner->renderNode->GetChild(boneName, true);
    if (boneNode)
        ownner->SpawnParticleEffect(boneNode->GetWorldPosition(), (particleName == String::EMPTY) ? "Particle/SnowExplosionFade.xml" : particleName, 5, 5.0f);
}

float CharacterState::GetThreatScore()
{
    return 0.0f;
}

void CharacterState::Enter(State* lastState)
{
//    Log::Write(-1," CharacterState: Enter\n");
    if (flags >= 0)
        ownner->AddFlag(flags); // добавляем флаги этого состояния объекту Character (GameObject)
    State::Enter(lastState);
    combatReady = false;
    firstUpdate = true;

    // Это срабатывает если State назначает physicsType своему состоянию. Состояние в котором физика другая.
    if (physicsType >= 0)
    {
        lastPhysicsType = ownner->physicsType; // Запомним физику которая была у персонажа.
        ownner->SetPhysicsType(physicsType); // Назначим ему физику которая требуется в текущем состоянии.
    }
//    Log::Write(-1," CharacterState: Enter 2\n");
}

void CharacterState::Exit(State* nextState)
{
//    Log::Write(-1," CharacterState: Exit\n");
    if (flags >= 0)
        ownner->RemoveFlag(flags); // удаляем флаги этого состояния из объекта Character (GameObject)
    State::Exit(nextState);
    
    // Это срабатывает если State назначает physicsType своему состоянию. Состояние в котором физика другая.
    if (physicsType >= 0)
    {
        ownner->SetPhysicsType(lastPhysicsType); // Возвращаем персонажу его физику при выходе из текущего состояния.
    }
}

void CharacterState::Update(float dt)
{
    if (combatReady)
    {
        if (!ownner->IsInAir())
        {
            if (ownner->ActionCheck(true, true, true, true))
                return;
        }
    }
    State::Update(dt);
    firstUpdate = false;
}

// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


SingleAnimationState::SingleAnimationState(Context* context, Character* c):
    CharacterState(context, c)
{
    looped = false;
    stateTime = -1;
}

SingleAnimationState::~SingleAnimationState()
{
}

void SingleAnimationState::Update(float dt)
{
    bool finished = false;
    if (looped)
    {
        if (stateTime > 0 && timeInState > stateTime)
            finished = true;
    }
    else
    {
        if (animSpeed < 0)
        {
            finished = ownner->animCtrl->GetTime(animation) < 0.0001f;
        }
        else
            finished = ownner->animCtrl->IsAtEnd(animation);
    }

    if (finished)
        OnMotionFinished();

    CharacterState::Update(dt);
}

void SingleAnimationState::Enter(State* lastState)
{
    ownner->PlayAnimation(animation, LAYER_MOVE, looped, blendTime, startTime, animSpeed);
    CharacterState::Enter(lastState);
}

void SingleAnimationState::OnMotionFinished()
{
    ownner->CommonStateFinishedOnGroud();
}

void SingleAnimationState::SetMotion(const String& name)
{
    animation = RADIO::GetAnimationName(name);
}

// ===============================


SingleMotionState::SingleMotionState(Context* context, Character* c):
    CharacterState(context, c)
{

}

SingleMotionState::~SingleMotionState()
{
    if(motion)
        motion.Reset();
}

void SingleMotionState::Update(float dt)
{
    if (motion->Move(ownner, dt) == 1)
        OnMotionFinished();
    CharacterState::Update(dt);
}

void SingleMotionState::Enter(State* lastState)
{
    motion->Start(ownner, startTime, blendTime, animSpeed);
    CharacterState::Enter(lastState);
}

void SingleMotionState::DebugDraw(DebugRenderer* debug)
{
    motion->DebugDraw(debug, ownner);
}

void SingleMotionState::SetMotion(const String& name)
{
    Motion* m = RADIO::g_mm->FindMotion(name);
    if (!m)
        return;
    if(motion)
        motion.Reset();
    motion = m;
}

void SingleMotionState::OnMotionFinished()
{
    // URHO3D_LOGINFO(ownner.GetName() + " state:" + name + " finshed motion:" + motion.animationName);
    ownner->CommonStateFinishedOnGroud();
}

// ===============================


MultiAnimationState::MultiAnimationState(Context* context, Character* c):
    CharacterState(context, c)
{
    looped = false;
    stateTime = -1;
}

MultiAnimationState::~MultiAnimationState()
{
    animations.Clear();
}


void MultiAnimationState::Update(float dt)
{
    bool finished = false;
    if (looped)
    {
        if (stateTime > 0 && timeInState > stateTime)
            finished = true;
    }
    else
    {
        if (animSpeed < 0)
        {
            finished = ownner->animCtrl->GetTime(animations[selectIndex]) < 0.0001f;
        }
        else
            finished = ownner->animCtrl->IsAtEnd(animations[selectIndex]);
    }

    if (finished)
        OnMotionFinished();

    CharacterState::Update(dt);
}

void MultiAnimationState::Enter(State* lastState)
{
    selectIndex = PickIndex();
    ownner->PlayAnimation(animations[selectIndex], LAYER_MOVE, looped, blendTime, startTime, animSpeed);
    CharacterState::Enter(lastState);
}

void MultiAnimationState::OnMotionFinished()
{
    ownner->CommonStateFinishedOnGroud();
}

void MultiAnimationState::AddMotion(const String& name)
{
    animations.Push(RADIO::GetAnimationName(name));
}

int MultiAnimationState::PickIndex()
{
    int index = ownner->GetNode()->GetVars()[ANIMATION_INDEX]->GetInt();
    if(RADIO::d_log)
        Log::Write(-1," MultiAnimationState: PickIndex = " + String(index) + "\n");
    return index;
}

// ===============================

MultiMotionState::MultiMotionState(Context* context, Character* c):
    CharacterState(context, c)
{
}

MultiMotionState::~MultiMotionState()
{
//    for (unsigned int i = 0; i < motions.Size(); ++i)
//    {
//        if(motions[i])
//            motions[i].Reset();
//    }
    motions.Clear();
}

void MultiMotionState::Update(float dt)
{
    int ret = motions[selectIndex]->Move(ownner, dt);
    if (ret == 1)
        OnMotionFinished();
    else if (ret == 2)
        OnMotionAlignTimeOut();
    CharacterState::Update(dt);
}

void MultiMotionState::Enter(State* lastState)
{
    Start();
    CharacterState::Enter(lastState);
}

void MultiMotionState::Start()
{
    selectIndex = PickIndex();
    if (selectIndex >= motions.Size())
    {
        Log::Write(-1," ERROR: a large animation index=" + String(selectIndex) + " name:" + ownner->GetName()+"\n");
        selectIndex = 0;
    }

    if (RADIO::d_log)
        Log::Write(-1," "+ownner->GetName() + " state=" + name + " pick " + motions[selectIndex]->animationName+"\n");
    motions[selectIndex]->Start(ownner, startTime, blendTime, animSpeed);
}

void MultiMotionState::DebugDraw(DebugRenderer* debug)
{
    motions[selectIndex]->DebugDraw(debug, ownner);
}

int MultiMotionState::PickIndex()
{
    int index = ownner->GetNode()->GetVars()[ANIMATION_INDEX]->GetInt();
    if(RADIO::d_log)
        Log::Write(-1," MultiMotionState: PickIndex = " + String(index) + "\n");
    return index;
}

String MultiMotionState::GetDebugText()
{
    return " name=" + name + " timeInState=" + String(timeInState) + " current motion=" + motions[selectIndex]->animationName + "\n";
}

void MultiMotionState::AddMotion(const String& name)
{
    Motion* motion = RADIO::g_mm->FindMotion(name);
    if (!motion)
        return;
//    SharedPtr<Motion> motionShared(motion);
//    motions.Push(motionShared);
    motions.Push(motion);
}

void MultiMotionState::OnMotionFinished()
{
    // URHO3D_LOGINFO(ownner.GetName() + " state:" + name + " finshed motion:" + motions[selectIndex].animationName);
    ownner->CommonStateFinishedOnGroud();
}

void MultiMotionState::OnMotionAlignTimeOut()
{
}

// ===============================

AnimationTestState::AnimationTestState(Context* context, Character* c):
    CharacterState(context, c)
{
    SetName("AnimationTestState");
    physicsType = 0;
}

AnimationTestState::~AnimationTestState()
{
    testMotions.Clear();
    testAnimations.Clear();
}

void AnimationTestState::Enter(State* lastState)
{
    RADIO::SendAnimationTriger(ownner->renderNode, RAGDOLL_STOP);
    currentIndex = 0;
    allFinished = false;
    Start();
    CharacterState::Enter(lastState);
}

void AnimationTestState::Exit(State* nextState)
{
    if (nextState != this)
        testMotions.Clear();
    CharacterState::Exit(nextState);
}

void AnimationTestState::Process(Vector<String> animations)
{
    testAnimations.Clear();
    testMotions.Clear();
    testAnimations.Resize(animations.Size());
    testMotions.Resize(animations.Size());
    for (unsigned int i = 0; i < animations.Size(); ++i)
    {
        testAnimations[i] = animations[i];
        testMotions[i] = RADIO::g_mm->FindMotion(animations[i]);
    }
}

void AnimationTestState::Start()
{
    Motion* motion = testMotions[currentIndex];
    blendTime = (currentIndex == 0) ? 0.2f : 0.0f;
    if (motion)
    {
        motion->Start(ownner, startTime, blendTime, animSpeed);
        if (ownner->side == 1)
            RADIO::g_cm->CheckCameraAnimation(motion->name);
    }
    else
    {
        ownner->PlayAnimation(testAnimations[currentIndex], LAYER_MOVE, false, blendTime, startTime, animSpeed);
        if (ownner->side == 1)
            RADIO::g_cm->CheckCameraAnimation(testAnimations[currentIndex]);
    }
}

void AnimationTestState::Update(float dt)
{
    if (allFinished)
    {
        Input* input = GetSubsystem<Input>();
        if (input->GetKeyDown(KEY_RETURN))
            ownner->CommonStateFinishedOnGroud();
        return;
    }

    bool finished = false;
    Motion* motion = testMotions[currentIndex];
    if (motion)
    {
        if (motion->dockAlignBoneName != String::EMPTY)
        {
            float t = ownner->animCtrl->GetTime(motion->animationName);
            if (t < motion->dockAlignTime && (t + dt) > motion->dockAlignTime)
            {
                // ownner.SetSceneTimeScale(0.0f);
            }
        }

        finished = motion->Move(ownner, dt) == 1;
        if (motion->looped && timeInState > 2.0f)
            finished = true;
    }
    else
    {
        if (animSpeed < 0)
        {
            finished = ownner->animCtrl->GetTime(testAnimations[currentIndex]) < 0.0001f;
        }
        else
            finished = ownner->animCtrl->IsAtEnd(testAnimations[currentIndex]);
    }

    if (finished) 
    {
        if(RADIO::d_log)
            Log::Write(-1, "AnimationTestState finished, currentIndex=" + String(currentIndex)+"\n");
        currentIndex ++;
        if (currentIndex >= testAnimations.Size()) // int(testAnimations.length))
        {
            //ownner.CommonStateFinishedOnGroud();
            allFinished = true;
        }
        else
            Start();
    }

    CharacterState::Update(dt);
}

void AnimationTestState::DebugDraw(DebugRenderer* debug)
{
    if (currentIndex >= testMotions.Size()) // int(testMotions.length))
        return;
    Motion* motion = testMotions[currentIndex];
    if (motion)
        motion->DebugDraw(debug, ownner);
}

String AnimationTestState::GetDebugText()
{
    if (currentIndex >= testAnimations.Size()) // int(testAnimations.length))
        return CharacterState::GetDebugText();
    return " name=" + this->name + " timeInState=" + String(timeInState) + " animation=" + testAnimations[currentIndex] + "\n";
}

bool AnimationTestState::CanReEntered()
{
    return true;
}

// ===============================

CharacterCounterState::CharacterCounterState(Context* context, Character* c):
    CharacterState(context, c)
{
    alignTime = 0.2f;
    SetName("CounterState");
}

CharacterCounterState::~CharacterCounterState()
{
}

void CharacterCounterState::Enter(State* lastState)
{
    if(RADIO::d_log)
        Log::Write(-1, " CharacterCounterState: Enter (" + ownner->GetName() + ") lastState = " + lastState->name + "\n");

    if (lastState->nameHash != ALIGN_STATE)
        sub_state = COUNTER_NONE;
    CharacterState::Enter(lastState);
}

void CharacterCounterState::Exit(State* nextState)
{
    if(RADIO::d_log)
        Log::Write(-1, " CharacterCounterState: Exit (" + ownner->GetName() + ") nextState = " + nextState->name + "\n");

    CharacterState::Exit(nextState);
    if (nextState->nameHash != ALIGN_STATE)
    {
        currentMotion = NULL;
        sub_state = COUNTER_NONE;
    }
}

void CharacterCounterState::StartCounterMotion()
{
    if (!currentMotion)
        return;
    if(RADIO::d_log)
        Log::Write(-1, " " + ownner->GetName() + " start counter motion " + currentMotion->animationName + "\n");
    ChangeSubState(COUNTER_ANIMATING);
    currentMotion->Start(ownner);
}

int CharacterCounterState::GetCounterDirection(int attackType, bool isBack)
{
    if (attackType == ATTACK_PUNCH)
        return isBack ? 1 : 0;
    else
        return isBack ? 3 : 2;
}

Vector<Motion*> CharacterCounterState::GetCounterMotions(int attackType, bool isBack)
{
    if(RADIO::d_log)
        Log::Write(-1, " CharacterCounterState: GetCounterMotions (" + ownner->GetName() + ")\n");

    if (isBack)
        return attackType == ATTACK_PUNCH ? backArmMotions : backLegMotions;
    else
        return attackType == ATTACK_PUNCH ? frontArmMotions : frontLegMotions;
}

void CharacterCounterState::DumpCounterMotions(Vector<Motion*> motions)
{
    if(RADIO::d_log)
        Log::Write(-1, " CharacterCounterState: DumpCounterMotions (" + ownner->GetName() + ")\n");

    for (unsigned int i = 0; i < motions.Size(); ++i)
    {
        Motion* motion = motions[i];
        String other_name = motion->name.Replaced("BM_TG_Counter", "TG_BM_Counter");
        Motion* other_motion = RADIO::g_mm->FindMotion(other_name);
        Vector3 startDiff = other_motion->GetStartPos() - motion->GetStartPos();
        if(RADIO::d_log)
            Log::Write(-1," couter-motion " + motion->name + " diff-len = " + String(startDiff.Length()) + "\n");
    }
}

void CharacterCounterState::Update(float dt)
{
    if (sub_state == COUNTER_ANIMATING)
    {
         if (currentMotion->Move(ownner, dt) == 1)
         {
            ownner->CommonStateFinishedOnGroud();
            return;
         }
    }
    CharacterState::Update(dt);
}

void CharacterCounterState::ChangeSubState(int newState)
{
    if (sub_state == newState)
        return;

    if(RADIO::d_log)
        Log::Write(-1, " " + ownner->GetName() + " CharacterCounterState: ChangeSubState from " + String(sub_state) + " to " + String(newState) + "\n");
    sub_state = newState;
}

void CharacterCounterState::Dump()
{
    DumpCounterMotions(frontArmMotions);
    DumpCounterMotions(backArmMotions);
    DumpCounterMotions(frontLegMotions);
    DumpCounterMotions(backLegMotions);
}

void CharacterCounterState::AddMultiCounterMotions(const String& preFix, bool isPlayer)
{
    MotionManager* mm = RADIO::g_mm;

    if (isPlayer)
    {
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsA"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsB"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsD"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsE"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsF"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsG"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsH"));

        tripleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_3ThugsA"));
        tripleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_3ThugsB"));
        tripleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_3ThugsC"));
    }
    else
    {
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsA_01"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsA_02"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsB_01"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsB_02"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsD_01"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsD_02"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsE_01"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsE_02"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsF_01"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsF_02"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsG_01"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsG_02"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsH_01"));
        doubleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_2ThugsH_02"));

        tripleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_3ThugsA_01"));
        tripleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_3ThugsA_02"));
        tripleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_3ThugsA_03"));
        tripleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_3ThugsB_01"));
        tripleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_3ThugsB_02"));
        tripleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_3ThugsB_03"));
        tripleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_3ThugsC_01"));
        tripleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_3ThugsC_02"));
        tripleCounterMotions.Push(mm->FindMotion(preFix + "Double_Counter_3ThugsC_03"));
    }
}

void CharacterCounterState::Add_Counter_Animations(const String& preFix, bool isPlayer)
{
    if(RADIO::d_log)
        Log::Write(-1," CharacterCounterState: Add_Counter_Animations from " + preFix + "\n");
/*
    if(isPlayer) //(preFix == "BM_TG_Counter/")
    {
        // Front Arm
        frontArmMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Arm_Front_Weak_02"));
        for(int i = 1; i <= 9; ++i)
            frontArmMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Arm_Front_0" + String(i)));
        frontArmMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Arm_Front_10"));

        // Front Leg
        frontLegMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Leg_Front_Weak"));
    
        for(int i = 1; i <= 6; ++i)
            frontLegMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Leg_Front_0" + String(i)));
        frontLegMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Leg_Front_01"));
    
        // Back Arm
        backArmMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Arm_Back_Weak_01"));
    
        for(int i = 1; i <= 4; ++i)
            frontArmMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Arm_Back_0" + String(i)));
        frontArmMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Arm_Back_04"));
    
        // Back Leg
        backLegMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Leg_Back_Weak_01"));
        backLegMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Leg_Back_01"));
        backLegMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Leg_Back_02"));
    }
    else
    {
*/
        // Front Arm
        frontArmMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Arm_Front_Weak_02"));

        // Front Leg
        frontLegMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Leg_Front_01"));
    
        // Back Arm
        //frontArmMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Arm_Back_04")); 
        backArmMotions.Push(RADIO::g_mm->FindMotion(preFix + "Counter_Arm_Back_04"));
//    }

    // ЗДЕСЬ МОЖНО ВЫЗВАТЬ ФУНКЦИЮ AddMultiCounterMotions С ПЕРЕДАЧЕЙ ЕЙ isPlayer И preFix
    // AddMultiCounterMotions(preFix, isPlayer);

}

void CharacterCounterState::SetTargetTransform(const Vector3& pos, float rot)
{
    Vector3 pos1 = ownner->GetNode()->GetWorldPosition();
    targetPosition = pos;
    targetPosition.y_ = pos1.y_;
    targetRotation = rot;
}

void CharacterCounterState::StartAligning()
{
    if(RADIO::d_log)
        Log::Write(-1," " + ownner->GetName() + " CharacterCounterState: StartAligning\n");

    CharacterAlignState* state = static_cast<CharacterAlignState*>(ownner->FindState_Hash(ALIGN_STATE));
    state->Start(this->nameHash, targetPosition, targetRotation, alignTime, 0, ownner->walkAlignAnimation);
    ownner->ChangeStateQueue(ALIGN_STATE);
}

String CharacterCounterState::GetDebugText()
{
    return " current motion = " + currentMotion->animationName;
}

void CharacterCounterState::DebugDraw(DebugRenderer* debug)
{
    debug->AddCross(targetPosition, 1.0f, Color::RED, false);
    RADIO::DebugDrawDirection(debug, ownner->GetNode()->GetWorldPosition(), targetRotation, Color::YELLOW);
}
// ===============================

CharacterRagdollState::CharacterRagdollState(Context* context, Character* c):
    CharacterState(context, c)
{
    SetName("RagdollState");
}

CharacterRagdollState::~CharacterRagdollState()
{
}

void CharacterRagdollState::Update(float dt)
{
    if (timeInState > 0.1f)
    {
        int ragdoll_state = ownner->GetNode()->GetVars()[RAGDOLL_STATE]->GetInt();
        if (ragdoll_state == RAGDOLL_NONE)
        {
            if (ownner->health > 0)
            {
                if(RADIO::d_log)
                    Log::Write(-1," " + ownner->GetName() + " CharacterRagdollState: Go to GetUpState\n");
                ownner->PlayCurrentPose();
                ownner->ChangeState("GetUpState");
            }
            else
            {
                if(RADIO::d_log)
                    Log::Write(-1," " + ownner->GetName() + " CharacterRagdollState: Go to DeadState\n");
                ownner->ChangeState("DeadState");
            }
        }
    }
    CharacterState::Update(dt);
}

void CharacterRagdollState::Enter(State* lastState)
{
    if(RADIO::d_log)
        Log::Write(-1," " + ownner->GetName() + " CharacterRagdollState: Enter\n");

    CharacterState::Enter(lastState);
    ownner->SetPhysics(false);

    // HACK: FLAGS_COUNTER 
    if(ownner->HasFlag(FLAGS_COUNTER)) // Добавка
        ownner->RemoveFlag(FLAGS_COUNTER);//(FLAGS_REDIRECTED | FLAGS_ATTACK | FLAGS_COUNTER); // Добавка
}
// Player::PickCounterEnemy() проходит по всему списку врагов и вызывает e->CanBeCountered() функцию персонажа
// которая просто проверяет наличие флага HasFlag(FLAGS_COUNTER) у него.
// Добавил удаление флага в состояние "RagdollState" в  CharacterRagdollState::Enter функцию.

// Flag удаляется также (должен):
// ThugAttackState::Exit
// ThugAttackState::OnAnimationTrigger (добавляется и удаляется)
// Тоесть, прежде чем войти в рэгдолл состояние бандит должен выйти из состояния атаки.

// Добавлено так как появлялась ситуация когда из этого состояния персонаж пытался войти в Counter состояние.

// ===============================

CharacterGetUpState::CharacterGetUpState(Context* context, Character* c) :
    MultiMotionState(context, c)
{
    SetName("GetUpState");
    state = 0;
    ragdollToAnimTime = 0.0f;
}

CharacterGetUpState::~CharacterGetUpState()
{
}

void CharacterGetUpState::Enter(State* lastState)
{
    if(RADIO::d_log)
        Log::Write(-1," " + ownner->GetName() + " CharacterGetUpState: Enter\n");

    state = 0;
    selectIndex = PickIndex();
    if (selectIndex >= motions.Size())//int(motions.Size())
    {
        Log::Write(-1," ERROR: CharacterGetUpState: a large animation index=" + String(selectIndex) + " name:" + ownner->GetName()+"\n");
        selectIndex = 0;
    }

    Motion* motion = motions[selectIndex];
    //if (blend_to_anim)
    //    ragdollToAnimTime = 0.2f;
    if(RADIO::d_log)
        Log::Write(-1, " CharacterGetUpState: Enter animationName = " + motion->animationName + "\n");
    
    ownner->PlayAnimation(motion->animationName, LAYER_MOVE, false, ragdollToAnimTime, 0.0f, 0.0f);
    CharacterState::Enter(lastState);
}

void CharacterGetUpState::Update(float dt)
{
    Motion* motion = motions[selectIndex];
    if (state == 0)
    {
        if (timeInState >= ragdollToAnimTime)
        {
            ownner->animCtrl->SetSpeed(motion->animationName, 1.0f);
            motion->InnerStart(ownner);
            state = 1;
        }
    }
    else
    {
        if (motion->Move(ownner, dt) == 1)
        {
            ownner->CommonStateFinishedOnGroud();
            return;
        }
    }

    CharacterState::Update(dt);
}
// ===============================

CharacterAlignState::CharacterAlignState(Context* context, Character* c):
    CharacterState(context, c)
{
    SetName("AlignState");
    alignTime = 0.2f;
}

CharacterAlignState::~CharacterAlignState()
{
}

void CharacterAlignState::Start(StringHash nextState, const Vector3& tPos, float tRot, float duration, int physicsType, const String& anim)
{
    if(RADIO::d_log)
        Log::Write(-1," " + this->name +" CharacterAlign--start duration=" + String(duration)+"\n");
    nextStateName = nextState;
    targetPosition = tPos;
    targetRotation = tRot;
    alignTime = duration;
    alignAnimation = anim;
    
    memOwnnerPhysics = ownner->physicsType;
    ownner->SetPhysicsType(physicsType);

    Vector3 curPos = ownner->GetNode()->GetWorldPosition();
    float curAngle = ownner->GetCharacterAngle();
    movePerSec = (tPos - curPos) / duration;
    rotatePerSec = RADIO::AngleDiff(tRot - curAngle) / duration;

    if (anim != "")
    {
        if(RADIO::d_log)
            Log::Write(-1," " + this->name +" align-animation : " + anim +"\n");
        ownner->PlayAnimation(anim, LAYER_MOVE, true);
    }
}

void CharacterAlignState::Update(float dt)
{
    if (ownner->physicsType == 0)
        ownner->MoveTo(ownner->GetNode()->GetWorldPosition() + movePerSec * dt, dt);
    else
        ownner->SetVelocity(movePerSec);
    ownner->GetNode()->Yaw(rotatePerSec * dt);
    CharacterState::Update(dt);
    if (timeInState >= alignTime)
        OnAlignTimeOut();
}

void CharacterAlignState::DebugDraw(DebugRenderer* debug)
{
    RADIO::DebugDrawDirection(debug, ownner->GetNode()->GetWorldPosition(), targetRotation, Color::RED, 2.0f);
    debug->AddCross(targetPosition, 0.5f, Color::YELLOW, false);
}

void CharacterAlignState::OnAlignTimeOut()
{
    if(RADIO::d_log)
        Log::Write(-1," " + this->name + " " + ownner->GetName() + " On_Align_Finished-- at: " + String(GetSubsystem<Time>()->GetSystemTime())+"\n");
    ownner->Transform(targetPosition, Quaternion(0, targetRotation, 0));

//    memOwnnerPhysics = ownner->physicsType;
    ownner->SetPhysicsType(memOwnnerPhysics);

    ownner->ChangeState_Hash(nextStateName);
}
// ===============================





















