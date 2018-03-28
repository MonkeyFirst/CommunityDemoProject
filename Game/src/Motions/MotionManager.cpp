
#include <Urho3D/Urho3DAll.h>

#include "Motions/MotionManager.h"
#include "States/GameManager.h"

#include "Objects/HeadIndicator.h"

#include "Character/Ragdoll.h"
#include "Character/CharacterStates.h"
#include "Character/Bruce.h"
#include "Character/Thug.h"

#include "IMUI/IMUI.h"

#include "radio.h"

MotionManager::MotionManager(Context* context):
    Object(context)
{
    Log::Write(-1," MotionManager: Constructor\n");
//    d_log = false;
}

MotionManager::~MotionManager()
{
    Log::Write(-1," MotionManager: Destructor\n");
}

void MotionManager::Finalize()//(unsigned int gameType)
{   
    Log::Write(-1," MotionManager: Finalize\n");
}

void MotionManager::Initialize()
{   
    Log::Write(-1," MotionManager: Initialize\n");

    sub_state  = MOTION_LOADING_START;
    processedMotions = 0;
    processedAnimations = 0;
    assetProcessTime = 0;
}

void MotionManager::Run()
{   
    Log::Write(-1," MotionManager: Run\n");
//    game_type_ = RADIO::game_type;
    assetProcessTime_ = GetSubsystem<Time>()->GetSystemTime(); // time.systemTime;
    AssetPreProcess();
    AddMotions();
    sub_state  = MOTION_LOADING_MOTIONS;
    Log::Write(-1,"\n MotionManager: LOADING_MOTIONS:\n");
}

void MotionManager::Finish()
{
    PostProcess();
    AssetPostProcess();
    Log::Write(-1,"*************************************************************************************\n");
    Log::Write(-1,"Motion Process time-cost=" + String(GetSubsystem<Time>()->GetSystemTime() - assetProcessTime) + " ms num-of-motions=" + String(motions.Size()) + " memory-use=" + String(memoryUse/1024) + " KB\n");
    Log::Write(-1,"*************************************************************************************\n");
    return;
}

void MotionManager::PostProcess()
{
    unsigned int t = GetSubsystem<Time>()->GetSystemTime();
    AddTriggers();
    Log::Write(-1," MotionManager::PostProcess time-cost=" + String(GetSubsystem<Time>()->GetSystemTime() - t) + " ms\n");
    return;
}

void MotionManager::AssetPostProcess()
{
    curRig_ = NULL;
    if (processScene_)
        processScene_->Remove();
    processScene_ = NULL;
    return;
}


bool MotionManager::Update(float dt)
{
    if (sub_state  == MOTION_LOADING_FINISHED)
        return true;

    unsigned int t = GetSubsystem<Time>()->GetSystemTime();

    if (sub_state  == MOTION_LOADING_MOTIONS)
    {
        //unsigned int t = GetSubsystem<Time>()->GetSystemTime(); // time.systemTime;
        int len = int(motions.Size());
        for (int i= processedMotions; i < len; ++i)
        {
            motions[i]->Process();
            ++processedMotions;
            int time_diff = int(GetSubsystem<Time>()->GetSystemTime() - t);
            if (time_diff >= PROCESS_TIME_PER_FRAME)
                break;
        }

//        Log::Write(-1,"MotionManager Process this frame time=" + String(GetSubsystem<Time>()->GetSystemTime() - t) + " ms " + " processedMotions=" + String(processedMotions) +"\n");
        if (processedMotions >= len)
        {
            sub_state  = MOTION_LOADING_ANIMATIONS;
            Log::Write(-1," MotionManager: LOADING_ANIMATIONS\n");
        }
    }
    else if (sub_state  == MOTION_LOADING_ANIMATIONS)
    {
        int len = int(animations.Size());
        //unsigned int t = GetSubsystem<Time>()->GetSystemTime();
        for (int i = processedAnimations; i < len; ++i)
        {
            GetSubsystem<ResourceCache>()->GetResource<Animation>(RADIO::GetAnimationName(animations[i]));
            ++processedAnimations;
            int time_diff = int(GetSubsystem<Time>()->GetSystemTime() - t);
            if (time_diff >= PROCESS_TIME_PER_FRAME)
                break;
        }

//        Log::Write(-1," MotionManager Process this frame time=" + String(GetSubsystem<Time>()->GetSystemTime() - t) + " ms " + " processedAnimations=" + String(processedAnimations) + "\n");

        if (processedAnimations >= len)
        {
            sub_state  = MOTION_LOADING_FINISHED;
            Log::Write(-1," Animations: " + String(processedAnimations) +" \n");
            Log::Write(-1," MotionManager: LOADING_FINISHED\n");
            return true;
        }
    }
    return false;
}

// Вроде как нигде не используется
void MotionManager::ProcessAll()
{
    for (unsigned int i = 0; i < motions.Size(); ++i)
        motions[i]->Process();
}

void MotionManager::AssetPreProcess()
{
    processScene_ = new Scene(context_);
}

void MotionManager::AddMotions()
{
    Log::Write(-1," MotionManager: AddMotions START\n");

    Create_BM_Motions();
    if (RADIO::game_type == 0)
    {
        Create_BM_Attack_Motions();
        
        CreateThugMotions();
    }
        
    Log::Write(-1," MotionManager: AddMotions END\n");
}

// добавлена функция так как Батман создавая состояние Аттака будет искать эти моушены.
void MotionManager::Create_BM_Attack_Motions()
{
    //CreateMotion(preFix + "Walk", kMotion_Z, kMotion_Z, -1, true);
    CreateMotion_InFolder("BW_Attack/"); 
}

//(const String& name, int motionFlag = kMotion_XZR, int allowMotion = kMotion_XZR,  int endFrame = -1, bool loop = false, float rotateAngle = 361)


void MotionManager::AddTriggers()
{
    Log::Write(-1," MotionManager: AddTriggers START\n");
    Add_BM_AnimationTriggers();

    if (RADIO::game_type == 0)
    {
        AddThugAnimationTriggers();
    }
    
    Log::Write(-1," MotionManager: AddTriggers END\n");
}

//Motion* CreateMotion(const String& name, int motionFlag = kMotion_XZR, int allowMotion = kMotion_XZR,  int endFrame = -1, bool loop = false, float rotateAngle = 361)
Motion* MotionManager::CreateMotion(const String& name, int motionFlag, int allowMotion,  int endFrame, bool loop, float rotateAngle)
{
    if (RADIO::mm_d_log)
    {
        Log::Write(-1, " MotionManager: CreateMotion " + name + "\n");
    //    Log::Write(-1, " motionFlag " + String(motionFlag) + "\n");
    //    Log::Write(-1, " allowMotion " + String(allowMotion) + "\n");
        Log::Write(-1, " ----------------------------------\n");
    }


    Motion* motion = new Motion(context_);
    motion->SetName(name);
    motion->motionFlag = motionFlag;
    motion->allowMotion = allowMotion;
    motion->looped = loop;
    motion->endFrame = endFrame;
    motion->rotateAngle = rotateAngle;
    motions.Push(motion);
    return motion;

}

void MotionManager::CreateMotion_InFolder(const String& folder)
{
    FileSystem* fs = GetSubsystem<FileSystem>();
    // void ScanDir(Vector<String>& result, const String& pathName, const String& filter, unsigned flags, bool recursive)
    // Array<String> attack_animations = fileSystem.ScanDir("MyData/Animations/" + folder, "*.ani", SCAN_FILES, false);
    Vector<String> attack_animations;
    fs->ScanDir(attack_animations, "MyData/Animations/" + folder, "*.ani", SCAN_FILES, false);
    for (unsigned int i = 0; i < attack_animations.Size(); ++i)//length; ++i)
        CreateMotion(folder + FileNameToMotionName(attack_animations[i]));
}

void MotionManager::AddAnimation(const String& animation)
{
    animations.Push(animation);
}

// Used in Batman and Thug
void MotionManager::AssignMotionRig(const String& rigName)
{
    curRig_ = new MotionRig(context_, rigName);
}

// ============= TRIGGERS =================================================================

void MotionManager::Animation_AddTrigger(const String& name, int frame, const VariantMap& data)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Animation* anim = cache->GetResource<Animation>(RADIO::GetAnimationName(name));
    if (anim)
        anim->AddTrigger(float(frame) * SEC_PER_FRAME, false, Variant(data));
}

void MotionManager::AddAnimationTrigger(const String& name, int frame, const String& tag)
{
    AddAnimationTrigger_Hash(name, frame, StringHash(tag));
}

void MotionManager::AddAnimationTrigger_Hash(const String& name, int frame, const StringHash& tag)
{
    VariantMap eventData;
    eventData[NAME] = tag;
    Animation_AddTrigger(name, frame, eventData);
}

void MotionManager::AddFloatAnimationTrigger(const String& name, int frame, const StringHash& tag, float value)
{
    VariantMap eventData;
    eventData[NAME] = tag;
    eventData[VALUE] = value;
    Animation_AddTrigger(name, frame, eventData);
}

void MotionManager::AddIntAnimationTrigger(const String& name, int frame, const StringHash& tag, int value)
{
    VariantMap eventData;
    eventData[NAME] = tag;
    eventData[VALUE] = value;
    Animation_AddTrigger(name, frame, eventData);
}

void MotionManager::AddStringAnimationTrigger(const String& name, int frame, const StringHash& tag, const String& value)
{
    VariantMap eventData;
    eventData[NAME] = tag;
    eventData[VALUE] = value;
    Animation_AddTrigger(name, frame, eventData);
}

void MotionManager::AddStringHashAnimationTrigger(const String& name, int frame, const StringHash& tag, const StringHash& value)
{
    VariantMap eventData;
    eventData[NAME] = tag;
    eventData[VALUE] = value;
    Animation_AddTrigger(name, frame, eventData);
}

void MotionManager::AddAttackTrigger(const String& name, int startFrame, int endFrame, const String& boneName)
{
    VariantMap eventData;
    eventData[NAME] = ATTACK_CHECK;
    eventData[VALUE] = 1;
    eventData[BONE] = boneName;
    Animation_AddTrigger(name, startFrame, eventData);
    eventData[VALUE] = 0;
    Animation_AddTrigger(name, endFrame + 1, eventData);
}

void MotionManager::AddRagdollTrigger(const String& name, int prepareFrame, int startFrame)
{
    if (prepareFrame >= 0)
        AddAnimationTrigger_Hash(name, prepareFrame, RAGDOLL_PERPARE);
    AddAnimationTrigger_Hash(name, startFrame, RAGDOLL_START);
}

void MotionManager::AddParticleTrigger(const String& name, int startFrame, const String& boneName, const String& effectName, float duration)
{
    VariantMap eventData;
    eventData[NAME] = ATTACK_CHECK;
    eventData[VALUE] = effectName;
    eventData[BONE] = boneName;
    eventData[DURATION] = duration;
    Animation_AddTrigger(name, startFrame, eventData);
}

void MotionManager::AddComplexAttackTrigger(const String& name, int counterStart, int counterEnd, int attackStart, int attackEnd, const String& attackBone)
{
    AddFloatAnimationTrigger(name, counterStart, TIME_SCALE, 0.3f);
    AddFloatAnimationTrigger(name, counterEnd, TIME_SCALE, 1.0f);
    AddIntAnimationTrigger(name, counterStart, COUNTER_CHECK, 1);
    AddIntAnimationTrigger(name, counterEnd, COUNTER_CHECK, 0);
    AddAttackTrigger(name, attackStart, attackEnd, attackBone);
}

// BATMAN =================================================================================
void MotionManager::Create_BM_Motions()
{
    Log::Write(-1," MotionManager: Create_BM_Motions\n");

    AssignMotionRig("Objects/BATMAN/bruce/my_bruce.mdl");

    String preFix = "BM_Movement/";
    CreateMotion(preFix + "Turn_Right_90", kMotion_R, kMotion_R, 30);
    CreateMotion(preFix + "Turn_Right_180", kMotion_R, kMotion_R, 25);
    CreateMotion(preFix + "Turn_Left_90", kMotion_R, kMotion_R, 14);
    CreateMotion(preFix + "Walk_Forward", kMotion_Z, kMotion_Z, -1, true);
//(const String& name, int motionFlag = kMotion_XZR, int allowMotion = kMotion_XZR,  int endFrame = -1, bool loop = false, float rotateAngle = 361)
    CreateMotion(preFix + "Stand_To_Walk_Right_90", kMotion_XZR, kMotion_ALL, 17);
    CreateMotion(preFix + "Stand_To_Walk_Right_180", kMotion_XZR, kMotion_ALL, 17);
    CreateMotion(preFix + "Stand_To_Run", kMotion_Z, kMotion_ALL, 15);
    CreateMotion(preFix + "Stand_To_Run_Right_90", kMotion_XZR, kMotion_ALL, 17);
    CreateMotion(preFix + "Stand_To_Run_Right_180", kMotion_XZR, kMotion_ALL, 25);

    preFix = "BM_Movement/";
    CreateMotion(preFix + "Run_Forward", kMotion_Z, kMotion_Z, -1, true);
    CreateMotion(preFix + "Run_Right_Passing_To_Stand");
    CreateMotion(preFix + "Run_Right_Passing_To_Run_Right_180", kMotion_XZR, kMotion_ZR, 20);
    AddAnimation(preFix + "Stand_Idle");
    AddAnimation(preFix + "Fall");
    AddAnimation(preFix + "Land");
    CreateMotion(preFix + "Cover_Transition", kMotion_R, kMotion_R);

    // Добавлено. Нет уверенности что правильное использование CreateMotion функции
    preFix = "BM_Combat/";
    CreateMotion(preFix + "Evade_Forward_01");
    CreateMotion(preFix + "Evade_Forward_03");
    CreateMotion(preFix + "Evade_Left_01");
    CreateMotion(preFix + "Evade_Right_01");
    CreateMotion(preFix + "Evade_Back_01");
    CreateMotion(preFix + "Into_Takedown");

    if (RADIO::game_type == 1)
        Create_BM_ClimbAnimations();
    
    // Добавлено. Нет уверенности что правильное использование CreateMotion функции
    if (RADIO::game_type == 0)
    {
        preFix = "BM_HitReaction/";
        CreateMotion(preFix + "HitReaction_Face_Right");
        CreateMotion(preFix + "Hit_Reaction_SideLeft");
        CreateMotion(preFix + "HitReaction_Back");
        CreateMotion(preFix + "Hit_Reaction_SideRight");
        
        preFix = "BM_Death_Primers/";
        CreateMotion(preFix + "Death_Front");
        CreateMotion(preFix + "Death_Back");
        CreateMotion(preFix + "Death_Side_Left");
        CreateMotion(preFix + "Death_Side_Right");
        
        CreateMotion_InFolder("BM_TG_Counter/"); // add
    }
}

void MotionManager::Create_BM_ClimbAnimations()
{
    Log::Write(-1," Create_BM_ClimbAnimations\n");

    Vector3 offset;
    String preFix = "BM_Climb/";
    int flags = kMotion_YZ | kMotion_Ext_Adjust_Y;

    // Climb Over
    CreateMotion(preFix + "Run_Climb_Over_128", kMotion_Z, kMotion_ALL, 37)->SetDockAlign(L_HAND, 0.5f, Vector3(0, -0.1, 0));
    CreateMotion(preFix + "Run_Climb_Over_128_Fall", kMotion_YZ)->SetDockAlign(L_HAND, 0.5f, Vector3(0, -0.1, 0));
    CreateMotion(preFix + "Run_Climb_Over_256_Fall", kMotion_YZ)->SetDockAlign(L_HAND, 0.5f, Vector3(0, -0.1, 0));

    CreateMotion(preFix + "Stand_Climb_Over_128", kMotion_Z)->SetDockAlign(L_HAND, 0.5f, Vector3(0, -0.1, 0));
    CreateMotion(preFix + "Stand_Climb_Over_128_Fall", kMotion_YZ)->SetDockAlign(L_HAND, 0.5f, Vector3(0, -0.1, 0));
    CreateMotion(preFix + "Stand_Climb_Over_256_Fall", kMotion_YZ)->SetDockAlign(L_HAND, 0.5f, Vector3(0, -0.1, 0));

    // Climb Up
    CreateMotion(preFix + "Stand_Climb_Up_128", kMotion_YZ)->SetDockAlign(L_FOOT, 0.27f, Vector3(0, -0.3, 0));
    CreateMotion(preFix + "Run_Climb_Up_128", kMotion_YZ, kMotion_ALL, 20)->SetDockAlign(L_FOOT, 0.35f, Vector3(0, -0.1, 0));

    CreateMotion(preFix + "Stand_Climb_Up_256", kMotion_YZ)->SetDockAlign(L_HAND, 0.8f, Vector3(0, -0.1, 0.3));
    CreateMotion(preFix + "Run_Climb_Up_256", kMotion_YZ)->SetDockAlign(L_HAND, 0.8f, Vector3(0, -0.1, 0.2));
    CreateMotion(preFix + "Stand_Climb_Up_384", kMotion_YZ)->SetDockAlign(L_HAND, 0.8f, Vector3(0, -0.1, 0.1));
    CreateMotion(preFix + "Run_Climb_Up_384", kMotion_YZ)->SetDockAlign(L_HAND, 0.8f, Vector3(0, -0.1, 0.1));

    // ==================================================
    // DANGLE
    // ==================================================
    int dangle_add_flags = kMotion_Ext_Foot_Based_Height;
    offset = Vector3(0, 0.25, 0.75);
    flags = kMotion_XYZ | dangle_add_flags;

    CreateMotion(preFix + "Dangle_Left",  flags)->SetDockAlign(L_HAND, 0.5f, offset);
    CreateMotion(preFix + "Dangle_Right",  flags)->SetDockAlign(R_HAND, 0.5f, offset);

    //offset = Vector3(0, 0.20, 0.25);
    flags = kMotion_ALL | dangle_add_flags;
    CreateMotion(preFix + "Dangle_Convex_90_L",  flags)->SetDockAlign(L_HAND, 0.6f, offset);
    CreateMotion(preFix + "Dangle_Concave_90_L",  flags)->SetDockAlign(L_HAND, 0.5f, offset);
    CreateMotion(preFix + "Dangle_Convex_90_R",  flags)->SetDockAlign(R_HAND, 0.6f, offset);
    CreateMotion(preFix + "Dangle_Concave_90_R",  flags)->SetDockAlign(R_HAND, 0.3f, offset);

    flags = kMotion_Y | dangle_add_flags;
    CreateMotion(preFix + "Dangle_Left_End", flags)->SetDockAlign(L_HAND, 0.6f, offset);
    CreateMotion(preFix + "Dangle_Right_End", flags)->SetDockAlign(R_HAND, 0.6f, offset);

    CreateMotion(preFix + "Dangle_Idle", dangle_add_flags);

    // offset = Vector3(0, 0.25, 0.75);
    flags = kMotion_YZR | dangle_add_flags;
    CreateMotion(preFix + "Crouch_To_Dangle", flags)->SetDockAlign(R_HAND, 0.7f, offset);
    CreateMotion(preFix + "Crouch_Jump_128_To_Dangle", flags)->SetDockAlign(L_HAND, 0.6, offset);

    flags = kMotion_YZ | dangle_add_flags;
    CreateMotion(preFix + "Dangle_Climb_Up_Run", flags)->SetDockAlign(L_HAND, 0.4f, Vector3(0, -0.1, 0.1));
    CreateMotion(preFix + "Dangle_Climb_Up_Over_128", flags)->SetDockAlign(L_HAND, 1.2f, Vector3(0, -0.1, 0.05));

    // ==================================================
    // HANG
    // ==================================================

    flags = kMotion_YZ | kMotion_Ext_Foot_Based_Height;
    // Climb Hang
    CreateMotion(preFix + "Stand_Climb_Up_256_Hang", flags, kMotion_ALL, 30)->SetDockAlign(L_HAND, 0.8f, Vector3(0, 0.16, 0.3));
    CreateMotion(preFix + "Stand_Climb_Up_384_Hang", flags, kMotion_ALL, 30)->SetDockAlign(L_HAND, 0.8f, Vector3(0, 0.18, 0.27));
    CreateMotion(preFix + "Run_Climb_Up_256_Hang", flags, kMotion_ALL, 30)->SetDockAlign(L_HAND, 0.8f, Vector3(0, 0.16, 0.3));
    CreateMotion(preFix + "Run_Climb_Up_384_Hang", flags, kMotion_ALL, 30)->SetDockAlign(L_HAND, 0.8f, Vector3(0, 0.1, 0.24));

    offset = Vector3(0, 0.15, 0.25);
    flags = kMotion_X | kMotion_Ext_Foot_Based_Height;

    CreateMotion(preFix + "Hang_Left", flags)->SetDockAlign(L_HAND, 0.5f, offset);
    CreateMotion(preFix + "Hang_Right", flags)->SetDockAlign(R_HAND, 0.5f, offset);

    flags = kMotion_XZR | kMotion_Ext_Foot_Based_Height;
    offset = Vector3(0, 0.15, 0.75);
    CreateMotion(preFix + "Hang_Left_Convex", flags)->SetDockAlign(L_HAND, 0.8f, offset);
    CreateMotion(preFix + "Hang_Right_Convex", flags)->SetDockAlign(R_HAND, 0.8f, offset);

    offset = Vector3(0, 0.15, 0.5);
    CreateMotion(preFix + "Hang_Left_Concave", flags)->SetDockAlign(L_HAND, 0.4f, offset);
    CreateMotion(preFix + "Hang_Right_Concave", flags)->SetDockAlign(R_HAND, 0.4f, offset);

    flags = kMotion_Y | kMotion_Ext_Foot_Based_Height;
    offset = Vector3(0, 0.15, 0.4);
    CreateMotion(preFix + "Hang_Left_End", flags)->SetDockAlign(L_HAND, 0.6f, offset);
    CreateMotion(preFix + "Hang_Right_End", flags)->SetDockAlign(R_HAND, 0.6f, offset);

    CreateMotion(preFix + "Dangle_To_Hang", flags);

    flags = kMotion_YZ | kMotion_Ext_Foot_Based_Height;
    CreateMotion(preFix + "Hang_Climb_Up_Run", flags)->SetDockAlign(L_HAND, 0.4f, Vector3(0, -0.1, 0.1));
    CreateMotion(preFix + "Hang_Climb_Up_Over_128", flags)->SetDockAlign(L_HAND, 1.2f, Vector3(0, -0.1, 0.05));
    CreateMotion(preFix + "Hang_Jump_Over_Fall", flags)->SetDockAlign(L_HAND, 0.4f, Vector3(0, -0.1, 0.1));
    CreateMotion(preFix + "Hang_Jump_Over_128", flags)->SetDockAlign(L_HAND, 0.4f, Vector3(0, -0.1, 0.1));


    flags = kMotion_YZR | kMotion_Ext_Foot_Based_Height;
    offset = Vector3(0, 0.1, 0.2);
    CreateMotion(preFix + "Crouch_To_Hang", flags)->SetDockAlign(R_HAND, 0.7f, offset);

    offset = Vector3(0, 0.0, 0.0);
    CreateMotion(preFix + "Crouch_Jump_128_To_Hang", flags)->SetDockAlign(L_HAND, 0.6, offset);


    preFix = "BM_Movement/";
    CreateMotion(preFix + "Cover_Run", kMotion_Z, kMotion_Z, -1, true);
    AddAnimation(preFix + "Crouch_Idle");
    AddAnimation(preFix + "Cover_Idle");

    preFix = "BM_Crouch_Turns/";
    CreateMotion(preFix + "Turn_Right_90", kMotion_XZR, kMotion_XZR, 12);
    CreateMotion(preFix + "Turn_Right_180", kMotion_XZR, kMotion_XZR, 20);
    CreateMotion(preFix + "Turn_Left_90", kMotion_XZR, kMotion_XZR, 12);
}

void MotionManager::Add_BM_AnimationTriggers()
{
    Log::Write(-1," Add_BM_AnimationTriggers\n");

    String preFix = "BM_Combat/";
    AddAnimationTrigger_Hash(preFix + "Evade_Forward_03", 48, READY_TO_FIGHT);
    AddAnimationTrigger_Hash(preFix + "Evade_Back_01", 48, READY_TO_FIGHT);
    AddAnimationTrigger_Hash(preFix + "Evade_Left_01", 48, READY_TO_FIGHT);
    AddAnimationTrigger_Hash(preFix + "Evade_Right_01", 48, READY_TO_FIGHT);

    preFix = "BM_Movement/";
    AddStringAnimationTrigger(preFix + "Walk", 11, FOOT_STEP, R_FOOT);
    AddStringAnimationTrigger(preFix + "Walk", 24, FOOT_STEP, L_FOOT);
    AddStringAnimationTrigger(preFix + "Turn_Right_90", 11, FOOT_STEP, R_FOOT);
    AddStringAnimationTrigger(preFix + "Turn_Right_90", 15, FOOT_STEP, L_FOOT);
    AddStringAnimationTrigger(preFix + "Turn_Right_180", 13, FOOT_STEP, R_FOOT);
    AddStringAnimationTrigger(preFix + "Turn_Right_180", 20, FOOT_STEP, L_FOOT);
    AddStringAnimationTrigger(preFix + "Turn_Left_90", 13, FOOT_STEP, L_FOOT);
    AddStringAnimationTrigger(preFix + "Turn_Left_90", 20, FOOT_STEP, R_FOOT);

    if (RADIO::game_type == 1)
    {
        preFix = "BM_Climb/";
        Vector3 offset = Vector3(0, -2.5f, 0);
        //TranslateAnimation(GetAnimationName(preFix + "Dangle_Idle"), offset);
        //TranslateAnimation(GetAnimationName(preFix + "Dangle_Left"), offset);
        //TranslateAnimation(GetAnimationName(preFix + "Dangle_Left_1"), offset);
        //TranslateAnimation(GetAnimationName(preFix + "Dangle_Left_End"), offset);
        //TranslateAnimation(GetAnimationName(preFix + "Dangle_Left_End_1"), offset);

        //TranslateAnimation(GetAnimationName(preFix + "Dangle_Right"), offset);
        //TranslateAnimation(GetAnimationName(preFix + "Dangle_Right_1"), offset);
        //TranslateAnimation(GetAnimationName(preFix + "Dangle_Right_End"), offset);
        //TranslateAnimation(GetAnimationName(preFix + "Dangle_Right_End_1"), offset);
    }
}

// THUG =================================================================================
void MotionManager::CreateThugMotions()
{
    Log::Write(-1," MotionManager: CreateThugMotions\n");

    AssignMotionRig("Objects/BATMAN/thug/thug.mdl");

    String preFix = "TG_Combat/";
    CreateMotion(preFix + "Step_Forward");
    CreateMotion(preFix + "Step_Right");
    CreateMotion(preFix + "Step_Back");
    CreateMotion(preFix + "Step_Left");
    CreateMotion(preFix + "Step_Forward_Long");
    CreateMotion(preFix + "Step_Right_Long");
    CreateMotion(preFix + "Step_Back_Long");
    CreateMotion(preFix + "Step_Left_Long");

    CreateMotion(preFix + "135_Turn_Left", kMotion_R, kMotion_R, 32);
    CreateMotion(preFix + "135_Turn_Right", kMotion_R, kMotion_R, 32);

    CreateMotion(preFix + "Run_Forward_Combat", kMotion_Z, kMotion_Z, -1, true);
    CreateMotion(preFix + "Walk_Forward_Combat", kMotion_Z, kMotion_Z, -1, true);

    if (RADIO::game_type == 0)
        CreateThugCombatMotions();

}

void MotionManager::CreateThugCombatMotions()
{
    Log::Write(-1," MotionManager: CreateThugCombatMotions\n");

    String preFix = "TG_Combat/";

    CreateMotion(preFix + "Attack_Kick");
    CreateMotion(preFix + "Attack_Kick_01");
    CreateMotion(preFix + "Attack_Kick_02");
    CreateMotion(preFix + "Attack_Punch");
    CreateMotion(preFix + "Attack_Punch_01");
    CreateMotion(preFix + "Attack_Punch_02");

    preFix = "TG_HitReaction/";
    CreateMotion(preFix + "HitReaction_Left");
    CreateMotion(preFix + "HitReaction_Right");
    CreateMotion(preFix + "HitReaction_Back_NoTurn");
    CreateMotion(preFix + "HitReaction_Back");
    AddAnimation(preFix + "CapeHitReaction_Idle");

    preFix = "TG_Getup/";
    CreateMotion(preFix + "GetUp_Front", kMotion_XZ);
    CreateMotion(preFix + "GetUp_Back", kMotion_XZ);
///*
    CreateMotion_InFolder("TG_BM_Counter/");
//*/
    preFix = "TG_Combat/";
    AddAnimation(preFix + "Stand_Idle_Additive_01");
    AddAnimation(preFix + "Stand_Idle_Additive_02");
    AddAnimation(preFix + "Stand_Idle_Additive_03");
    AddAnimation(preFix + "Stand_Idle_Additive_04");
}

void MotionManager::AddThugCombatAnimationTriggers()
{
    Log::Write(-1," AddThugCombatAnimationTriggers\n");

    String preFix = "TG_BM_Counter/";
///*
    AddRagdollTrigger(preFix + "Counter_Leg_Front_01", 25, 40);
/*
    AddRagdollTrigger(preFix + "Counter_Leg_Front_02", 25, 54);
    AddRagdollTrigger(preFix + "Counter_Leg_Front_03", 15, 23);
    AddRagdollTrigger(preFix + "Counter_Leg_Front_04", 32, 35);
    AddRagdollTrigger(preFix + "Counter_Leg_Front_05", 55, 65);
    AddRagdollTrigger(preFix + "Counter_Leg_Front_06", -1, 32);
    AddAnimationTrigger_Hash(preFix + "Counter_Leg_Front_Weak", 52, READY_TO_FIGHT);

    AddRagdollTrigger(preFix + "Counter_Arm_Front_01", -1, 34);
    AddRagdollTrigger(preFix + "Counter_Arm_Front_02", 38, 45);
    AddRagdollTrigger(preFix + "Counter_Arm_Front_03", 34, 40);
    AddRagdollTrigger(preFix + "Counter_Arm_Front_04", 33, 39);
    AddRagdollTrigger(preFix + "Counter_Arm_Front_05", -1, 43);
    AddRagdollTrigger(preFix + "Counter_Arm_Front_06", 40, 42);
    AddRagdollTrigger(preFix + "Counter_Arm_Front_07", 20, 25);
    AddRagdollTrigger(preFix + "Counter_Arm_Front_08", 29, 32);
    AddRagdollTrigger(preFix + "Counter_Arm_Front_09", 35, 40);
    AddRagdollTrigger(preFix + "Counter_Arm_Front_10", 20, 32);
*/

    AddAnimationTrigger_Hash(preFix + "Counter_Arm_Front_Weak_02", 45, READY_TO_FIGHT);
/*
    AddRagdollTrigger(preFix + "Counter_Arm_Back_01", 35, 40);
    AddRagdollTrigger(preFix + "Counter_Arm_Back_02", -1, 46);
    AddRagdollTrigger(preFix + "Counter_Arm_Back_03", 32, 34);
*/
    AddRagdollTrigger(preFix + "Counter_Arm_Back_04", 30, 40);
/*
    AddAnimationTrigger_Hash(preFix + "Counter_Arm_Back_Weak_01", 54, READY_TO_FIGHT);

    AddRagdollTrigger(preFix + "Counter_Leg_Back_01", 45, 54);
    AddRagdollTrigger(preFix + "Counter_Leg_Back_02", 50, 60);
    AddAnimationTrigger_Hash(preFix + "Counter_Leg_Back_Weak_01", 52, READY_TO_FIGHT);
*/

//*/
    preFix = "TG_Combat/";
    int frame_fixup = 6;
    // name counter-start counter-end attack-start attack-end attack-bone
    AddComplexAttackTrigger(preFix + "Attack_Kick", 15 - frame_fixup, 24, 24, 27, "Bip01_L_Foot");
    AddComplexAttackTrigger(preFix + "Attack_Kick_01", 12 - frame_fixup, 24, 24, 27, "Bip01_L_Foot");
    AddComplexAttackTrigger(preFix + "Attack_Kick_02", 19 - frame_fixup, 24, 24, 27, "Bip01_L_Foot");
    AddComplexAttackTrigger(preFix + "Attack_Punch", 15 - frame_fixup, 22, 22, 24, "Bip01_R_Hand");
    AddComplexAttackTrigger(preFix + "Attack_Punch_01", 15 - frame_fixup, 23, 23, 24, "Bip01_R_Hand");
    AddComplexAttackTrigger(preFix + "Attack_Punch_02", 15 - frame_fixup, 23, 23, 24, "Bip01_R_Hand");

    preFix = "TG_Getup/";
    AddAnimationTrigger_Hash(preFix + "GetUp_Front", 44, READY_TO_FIGHT);
    AddAnimationTrigger_Hash(preFix + "GetUp_Back", 68, READY_TO_FIGHT);
}

void MotionManager::AddThugAnimationTriggers()
{
    Log::Write(-1," AddThugAnimationTriggers\n");

    String preFix = "TG_Combat/";

    AddStringAnimationTrigger(preFix + "Run_Forward_Combat", 2, FOOT_STEP, L_FOOT);
    AddStringAnimationTrigger(preFix + "Run_Forward_Combat", 13, FOOT_STEP, R_FOOT);

    AddStringAnimationTrigger(preFix + "Step_Back", 15, FOOT_STEP, R_FOOT);
    AddStringAnimationTrigger(preFix + "Step_Back_Long", 9, FOOT_STEP, R_FOOT);
    AddStringAnimationTrigger(preFix + "Step_Back_Long", 19, FOOT_STEP, L_FOOT);

    AddStringAnimationTrigger(preFix + "Step_Forward", 12, FOOT_STEP, L_FOOT);
    AddStringAnimationTrigger(preFix + "Step_Forward_Long", 10, FOOT_STEP, L_FOOT);
    AddStringAnimationTrigger(preFix + "Step_Forward_Long", 22, FOOT_STEP, R_FOOT);

    AddStringAnimationTrigger(preFix + "Step_Left", 11, FOOT_STEP, L_FOOT);
    AddStringAnimationTrigger(preFix + "Step_Left_Long", 8, FOOT_STEP, L_FOOT);
    AddStringAnimationTrigger(preFix + "Step_Left_Long", 22, FOOT_STEP, R_FOOT);

    AddStringAnimationTrigger(preFix + "Step_Right", 11, FOOT_STEP, R_FOOT);
    AddStringAnimationTrigger(preFix + "Step_Right_Long", 15, FOOT_STEP, R_FOOT);
    AddStringAnimationTrigger(preFix + "Step_Right_Long", 28, FOOT_STEP, L_FOOT);

    AddStringAnimationTrigger(preFix + "135_Turn_Left", 8, FOOT_STEP, R_FOOT);
    AddStringAnimationTrigger(preFix + "135_Turn_Left", 20, FOOT_STEP, L_FOOT);
    AddStringAnimationTrigger(preFix + "135_Turn_Left", 31, FOOT_STEP, R_FOOT);

    AddStringAnimationTrigger(preFix + "135_Turn_Right", 11, FOOT_STEP, R_FOOT);
    AddStringAnimationTrigger(preFix + "135_Turn_Right", 24, FOOT_STEP, L_FOOT);
    AddStringAnimationTrigger(preFix + "135_Turn_Right", 39, FOOT_STEP, R_FOOT);

    if (RADIO::game_type == 0)
        AddThugCombatAnimationTriggers();

}

// ===== BATMAN and THUG END ===========================================================================

// Used only in MotionManager::CreateMotion_InFolder(const String& folder)
String MotionManager::FileNameToMotionName(const String& name)
{
    return name.Substring(0, name.Length() - 4);
}

// Used only in Motion::Process()
// Array<Vector4>&out outKeys

// Вызывает только Motion класс передавая только animationFile и значение по умолчанию для rotateAngle
// Остальные параметры заполняются в Motion классе когда оно создается (см. CreateMotion функцию)
// Вероятно происходит только на этапе загрузки MOTION_LOADING_MOTIONS
// Вернет угол вращения rotateAngle (переопределит) и заполнит outKeys и startFromOrigin
float MotionManager::ProcessAnimation(const String& animationFile, int motionFlag, int allowMotion, float rotateAngle, Vector<Vector4>& outKeys, Vector4& startFromOrigin)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    if (RADIO::mm_d_log)
    {
//    {Log::Write(-1,"loading-scene XML --> time-cost " + String(time->GetSystemTime() - t) + " ms\n");
        Log::Write(-1," ----------------------------------------------------------------------\n");
        Log::Write(-1," ProcessAnimation: " + animationFile + "\n");
//        Log::Write(-1,"motionFlag = " + String(motionFlag) + " allowMotion = " + String(allowMotion) + "\n");
//    }
    }

    // Получаем файл анимации
    Animation* anim = cache->GetResource<Animation>(animationFile);
    if (!anim) 
    {
        ErrorDialog(TITLE, animationFile + " not found!");
        GetSubsystem<Engine>()->Exit(); // Сердито
        return 0; // rotateAngle
    }

    // Получаем трек RootBone ("BPos") косточки отвечающей только за перемещение всего скелета
    AnimationTrack* translateTrack = anim->GetTracks()[TranslateBoneName];
    if (!translateTrack)
    {
        Log::Write(-1, animationFile + " translation track not found!!!\n");
        return 0; // rotateAngle
    }

    // Получаем трек косточки отвечающей за вращение всего скелета ("BRot")
    AnimationTrack* rotateTrack = anim->GetTracks()[RotateBoneName];
    //Quaternion flipZ_Rot(0, 180, 0); закоментировано в оригинале

    // Batman и Thug классы вызывают функцию AssignMotionRig которая создает текущий риг
    // MotionRig по сути, processScene которая создана в MotionManager классе 
    // и Узел (processNode) с загруженным скелетоном (AnimatedModel) и его копию в другом Узле (alignNode)
    Node* rotateNode = curRig_->rotateNode; // Узел BRot косточки
    Node* translateNode = curRig_->translateNode; // Узел BPos косточки
    MotionRig* rig = curRig_;

    // какие-то первоначальные манипуляции с флагами (учить матчасть)
    bool cutRotation = motionFlag & kMotion_Ext_Rotate_From_Start; // != 0
    bool dump = motionFlag & kMotion_Ext_Debug_Dump; // != 0
    float firstRotateFromRoot = 0;
    bool flip = false;
    bool footBased = motionFlag & kMotion_Ext_Foot_Based_Height; // != 0
    int translateFlag = 0;

    if (RADIO::mm_d_log)
    {
        Log::Write(-1, " rotateTrack keyframes = " + String(rotateTrack->GetNumKeyFrames()) + "\n");
        Log::Write(-1, " translateTrack keyframes = " + String(translateTrack->GetNumKeyFrames()) + "\n");
    }

/*
Log::Write(-1,"\n (rotateTrack->keyFrames_[i].rotation_).EulerAngles().y_\n");
for (unsigned int i = 0; i < rotateTrack->GetNumKeyFrames(); ++i)
{
    float angle = (rotateTrack->keyFrames_[i].rotation_).EulerAngles().y_;
    Log::Write(-1," kf [" + String(i) + "] = " + String(angle) + "\n");
}
*/
/*
Log::Write(-1,"\n translateTrack->keyFrames_[i].position_\n");
for (unsigned int i = 0; i < translateTrack->GetNumKeyFrames(); ++i)
{
    Vector3 pos = translateTrack->keyFrames_[i].position_;
    Log::Write(-1," keyFrames_[" + String(i) + "] = " + String(pos) + "\n");
}
*/
    // ==============================================================
    // pre process key frames
    
    // rotateAngle передается в функцию со значением по умолчанию 361
    // может переопределятся при создании Motion, но по факту вроде это не делается. (где-то в AddMotion)
    if (rotateTrack && rotateAngle > 360) 
    {
/*
    float kf1 = (rotateTrack->keyFrames_[0].rotation_).EulerAngles().y_;
    float kf2 = (rotateTrack->keyFrames_[1].rotation_).EulerAngles().y_;
    float kf3 = (rotateTrack->keyFrames_[2].rotation_).EulerAngles().y_;
    
    Log::Write(-1," rotateTrack->keyFrames_[0] (euler Y) " + String(kf1) + "\n");
    Log::Write(-1," rotateTrack->keyFrames_[1] (euler Y) " + String(kf2) + "\n");
    Log::Write(-1," rotateTrack->keyFrames_[2] (euler Y) " + String(kf3) + "\n");
*/
        // Получаем вращение по оси Y вероятно кости в первом ключе трека относительно изначального вращения кости в Риге.
        // rotateBoneInitQ = bone->initialRotation_;
        // GetRotationInXZPlane(MotionRig* rig, const Quaternion& startLocalRot, const Quaternion& curLocalRot)
        // Короче, не очень понятная функция
        firstRotateFromRoot = GetRotationInXZPlane(rig, rig->rotateBoneInitQ, rotateTrack->keyFrames_[0].rotation_).EulerAngles().y_;

        if (RADIO::mm_d_log)
            Log::Write(-1, " firstRotateFromRoot = " + String(firstRotateFromRoot) + "\n");
/*

Функция abs рассчитывает абсолютное значение (модуль) целого числа 
(тип int с разрядностью 16 или 32 бита в зависимости от аппаратной платформы, на которой компилируется программа). 

Функция labs рассчитывает абсолютное значение (модуль) 32-х разрядного целого числа. 
Отличие от функции abs в типе аргумента и возвращаемого значения. В функции labs используется тип long, 
который всегда 32 разрядный, не зависимо от архитектуры процессора. 

Функция llabs рассчитывает абсолютное значение (модуль) 64-х разрядного целого числа.

*/

        // Abs - вернет число без знака. (math.h)
        if (Abs(firstRotateFromRoot) > 75) // BRot кость в скелете -> BRot кость в первом кадре (в градусах)
        {
            if (RADIO::mm_d_log)
                Log::Write(-1, animationFile + " abs > 75 Need to flip rotate track since object is start opposite, rotation = " + String(firstRotateFromRoot) + "\n");
            flip = true;
        }
        // startFromOrigin параметр переданный в функцию Первое заполнение.
        startFromOrigin.w_ = firstRotateFromRoot;
        
        if (RADIO::mm_d_log)
            Log::Write(-1, " Motion: startFromOrigin.w_ = " + String(startFromOrigin.w_) + "\n");
    }

    // Теперь тоже, но вычислим для кости перемещения (BPos) в скелете и начальном кадре.

    // Установили Узел перемещения Рига (BPos косточки) в позицию как в первом Кадре
    translateNode->SetPosition(translateTrack->keyFrames_[0].position_);
    Vector3 t_ws1 = translateNode->GetWorldPosition();

    // Установили Узел перемещения Рига (BPos косточки) в позицию pelvisOrign 
    // pelvisOrign = skeleton.GetBone(TranslateBoneName)->initialPosition_
    // Хм..., он у нас всегда там если судить по скелету. Может в скелете нужно переделать, но сомнительно.
    translateNode->SetPosition(rig->pelvisOrign); 
    Vector3 t_ws2 = translateNode->GetWorldPosition();

    Vector3 diff = t_ws1 - t_ws2;
    // разницу будем возвращать
    // Получается разница между позициями BPos кости в скелетоне и первом кадре анимации.
    startFromOrigin.x_ = diff.x_;
    startFromOrigin.y_ = diff.y_;
    startFromOrigin.z_ = diff.z_;
    // startFromOrigin заполнен w-вращение xyz-перемещение. (разница между скелетоном и первым кадром)
    if (RADIO::mm_d_log)
    {
        Log::Write(-1, " Motion: startFromOrigin.x_ = " + String(startFromOrigin.x_) + "\n");
        Log::Write(-1, " Motion: startFromOrigin.y_ = " + String(startFromOrigin.y_) + "\n");
        Log::Write(-1, " Motion: startFromOrigin.z_ = " + String(startFromOrigin.z_) + "\n");
    }

    if (rotateAngle < 360)
        RotateAnimation(animationFile, rotateAngle);
    // Если разница вращения для BRot кости в скелете и первом кадре более 75 градусов.
    else if (flip)
        // Короче: повернули Риг и треки для BRot и BPos костей.
        RotateAnimation(animationFile, 180); // Почему на 180?!

    // Вроде как позиция BPos косточки в каждом кадре ее анимации корректируется 
    // на разницу между ее изначальной позицией в Риге и позицией в первом кадре.
    FixAnimationOrigin(rig, animationFile, motionFlag);

    // Пустой цикл - только для лога и установки значения firstRotateFromRoot как разницу вращения в Риге и первом кадре.
    if (rotateTrack)
    {
        for (unsigned int i = 0; i < rotateTrack->GetNumKeyFrames(); ++i)
        {
            Quaternion q = GetRotationInXZPlane(rig, rig->rotateBoneInitQ, rotateTrack->keyFrames_[i].rotation_);
//            if (RADIO::mm_d_log)
//            {
//                if (i == 0 || i == rotateTrack->GetNumKeyFrames() - 1)
//                    URHO3D_LOGINFO("frame=" + String(i) + " rotation from identical in xz plane=" + q.EulerAngles().ToString());
//            }
            if (i == 0)
                firstRotateFromRoot = q.EulerAngles().y_;
        }
    }

    // Коррекция размера outKeys в соответствии с количеством кадров в треке анимации
    if(translateTrack->GetNumKeyFrames() < rotateTrack->GetNumKeyFrames())
    {
        outKeys.Resize(rotateTrack->GetNumKeyFrames());
    }
    else
    {
        outKeys.Resize(translateTrack->GetNumKeyFrames());
    }
    
    // process rotate key frames first
    // Здесь будем вращать трек BRot кости и запоминать вращение в outKeys (в Motion классе)
    if ((motionFlag & kMotion_R) && rotateTrack) // motionFlag & kMotion_R != 0
    {
        // lastRot Вращение в первом кадре
        Quaternion lastRot = rotateTrack->keyFrames_[0].rotation_;
        // cutRotation не менялось и установлено в самом начале процесса - bool cutRotation = motionFlag & kMotion_Ext_Rotate_From_Start != 0;
        // firstRotateFromRoot разница вращения в Риге и первом кадре (чуть выше вычисляли)
        float rotateFromStart = cutRotation ? firstRotateFromRoot : 0;

        // Прройдем по всем кадрам анимации для BRot кости
        for (unsigned int i = 0; i < rotateTrack->GetNumKeyFrames(); ++i)
        {
            AnimationKeyFrame kf(rotateTrack->keyFrames_[i]);
            // вычислили разницу вращения между Ригом и текущим кадром по Y
            Quaternion q = GetRotationInXZPlane(rig, lastRot, kf.rotation_);
            // Запомним вращение текущего кадра
            lastRot = kf.rotation_;

            outKeys[i].w_ = rotateFromStart;

            rotateFromStart += q.EulerAngles().y_;

//            if (dump)
//                URHO3D_LOGINFO("rotation from last frame = " + String(q.EulerAngles().y_) + " rotateFromStart=" + String(rotateFromStart));

            q = Quaternion(0, rotateFromStart, 0).Inverse();

            // Поворачиваем Риг и записываем его поворот в трек анимации
            Quaternion wq = rotateNode->GetWorldRotation();
            wq = q * wq;
            rotateNode->SetWorldRotation(wq);
            kf.rotation_ = rotateNode->GetRotation();

            rotateTrack->keyFrames_[i] = kf;
        }
    }

    bool rotateMotion = motionFlag & kMotion_R;// != 0
    motionFlag &= (~kMotion_R);

    if (motionFlag)// != 0
    {
        if (RADIO::mm_d_log)
            Log::Write(-1,"motionFlag Exist and value is " + String(motionFlag) + "\n");
        
        // firstKeyPos позиция в первом кадре
        Vector3 firstKeyPos = translateTrack->keyFrames_[0].position_;

        // Проходим по всем кадрам translateTrack
        for (unsigned int i = 0; i < translateTrack->GetNumKeyFrames(); ++i)
        {
            AnimationKeyFrame kf(translateTrack->keyFrames_[i]);
            // translateNode ("BPos") для каждого кадра ставим как в первом кадре
            translateNode->SetPosition(firstKeyPos);
            Vector3 t1_ws = translateNode->GetWorldPosition();
            // translateNode ("BPos") для каждого кадра ставим как в текущем кадре
            translateNode->SetPosition(kf.position_);
            Vector3 t2_ws = translateNode->GetWorldPosition();

            //rootKeys[i] = t2_ws; //need get world position

            // Находим разницу
            Vector3 translation = t2_ws - t1_ws;
            
            // Записываем разницу в возвращаемуый outKeys (Vector<Vector4> motionKeys в Motion классе)
            if (motionFlag & kMotion_X) // != 0
            {
                //Log::Write(-1," kMotion_X\n");
                
                outKeys[i].x_ = translation.x_;
                t2_ws.x_  = t1_ws.x_;
            }
            if (motionFlag & kMotion_Y && !footBased) // != 0
            {
                //Log::Write(-1," kMotion_Y\n");
                
                outKeys[i].y_ = translation.y_;
                t2_ws.y_ = t1_ws.y_;
            }
            if (motionFlag & kMotion_Z) // != 0
            {
                //Log::Write(-1," kMotion_Z\n");
                
                outKeys[i].z_ = translation.z_;
                t2_ws.z_ = t1_ws.z_;
            }

            // Узел кости BPos в Риге ставим как в первом кадре
            translateNode->SetWorldPosition(t2_ws);
            Vector3 local_pos = translateNode->GetPosition();

            //Log::Write(-1," kf.position from " + kf.position_.ToString() + " to " + local_pos.ToString() + "\n");
            //Log::Write(-1," outKeys " + outKeys[i].ToString() + "\n");
//            Log::Write(-1," rootKeys " + rootKeys[i].ToString() + "\n");

            // Получается в трек BPos кости все кадры обнуляются до первого кадра.
            // А в outKeys записана будет разница межде первым и текущим кадром (для всех кадров)
            kf.position_ = local_pos;
            translateTrack->keyFrames_[i] = kf;
        }
    }

    if (footBased)
    {
        Log::Write(-1," footBased Animation\n");
    
        Vector<Vector3> leftFootPositions; // Позиции левой ноги
        Vector<Vector3> rightFootPositions; // Позиции правой ноги
        
        CollectBoneWorldPositions(curRig_, animationFile, L_FOOT, leftFootPositions);
        CollectBoneWorldPositions(curRig_, animationFile, R_FOOT, rightFootPositions);

        Vector<float> ground_heights; // Array<float> ground_heights;
        ground_heights.Resize(leftFootPositions.Size()); // length);

        for (unsigned int i = 0; i < translateTrack->GetNumKeyFrames(); ++i)
        {
            AnimationKeyFrame kf(translateTrack->keyFrames_[i]);
            float ground_y = 0;
            if (rightFootPositions[i].y_ < leftFootPositions[i].y_)
                ground_y = rightFootPositions[i].y_ - curRig_->right_foot_to_ground_height;
            else
                ground_y = leftFootPositions[i].y_ - curRig_->left_foot_to_ground_height;
            kf.position_.y_ -= ground_y;
            translateTrack->keyFrames_[i] = kf;
            ground_heights[i] = ground_y;
        }

        if (motionFlag & kMotion_Y) // != 0
        {
            for (unsigned int i = 0; i < ground_heights.Size(); ++i) //length; ++i)
                outKeys[i].y_ = ground_heights[i] - ground_heights[0];
        }
    }

    for (unsigned int i = 0; i < outKeys.Size(); ++i)
    {
        Vector3 v_motion(outKeys[i].x_, outKeys[i].y_, outKeys[i].z_);
        outKeys[i].x_ = v_motion.x_;
        outKeys[i].y_ = v_motion.y_;
        outKeys[i].z_ = v_motion.z_;

        if (!allowMotion & kMotion_X) // allowMotion & kMotion_X == 0
            outKeys[i].x_ = 0;
        if (!allowMotion & kMotion_Y) // allowMotion & kMotion_Y == 0
            outKeys[i].y_ = 0;
        if (!allowMotion & kMotion_Z) // allowMotion & kMotion_Z == 0
            outKeys[i].z_ = 0;
        if (!allowMotion & kMotion_R) // allowMotion & kMotion_R == 0
            outKeys[i].w_ = 0;
    }

//    if (dump)
//    {
//        for (unsigned int i=0; i< outKeys.Size(); ++i) // outKeys.length; ++i)
//        {
//            URHO3D_LOGINFO("Frame " + String(i) + " motion-key=" + outKeys[i].ToString());
//        }
 //   }

//    if (RADIO::mm_d_log)
//        Log::Write(-1,"----------------------------------------------------------------------\n");

    if (rotateAngle < 360)
        return rotateAngle;
    else
        return flip ? 180 : 0;
}

// Used in Motion::Process()
Vector3 MotionManager::GetBoneWorldPosition(MotionRig* rig, const String& animationFile, const String& boneName, float t)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Animation* anim = cache->GetResource<Animation>(animationFile);
    if (!anim) {
        ErrorDialog(TITLE, animationFile + " not found!");
        GetSubsystem<Engine>()->Exit(); //engine.Exit();
        return Vector3();
    }

    AnimatedModel* am = rig->alignNode->GetComponent<AnimatedModel>(); //("AnimatedModel");
    am->RemoveAllAnimationStates();
    AnimationState* state = am->AddAnimationState(anim);
    state->SetWeight(1.0f); //weight = 1.0f;
    state->SetLooped(false); //looped = false;
    state->SetTime(t); //time = t;
    state->Apply();
    rig->alignNode->MarkDirty();
    return  rig->alignNode->GetChild(boneName, true)->GetWorldPosition(); //worldPosition;
}

// Used only in AssetProcess - only in next function ::GetRotationInXZPlane
Vector3 MotionManager::GetProjectedAxis(MotionRig* rig, Node* node, const Vector3& axis)
{
    Vector3 p = node->GetWorldRotation() * axis;
    p.Normalize();
    Vector3 ret = rig->processNode->GetWorldRotation().Inverse() * p;
    ret.Normalize();
    ret.y_ = 0;
    return ret;
}

// Used only in AssetProcess
Quaternion MotionManager::GetRotationInXZPlane(MotionRig* rig, const Quaternion& startLocalRot, const Quaternion& curLocalRot)
{
    Node* rotateNode = rig->rotateNode;
    rotateNode->SetRotation(startLocalRot); //rotateNode.rotation = startLocalRot;
    Vector3 startAxis = GetProjectedAxis(rig, rotateNode, rig->pelvisRightAxis);
    rotateNode->SetRotation(curLocalRot); //rotateNode.rotation = curLocalRot;
    Vector3 curAxis = GetProjectedAxis(rig, rotateNode, rig->pelvisRightAxis);
    return Quaternion(startAxis, curAxis);
}

// Used only in AssetProcess 
void MotionManager::RotateAnimation(const String& animationFile, float rotateAngle)
{
    if (RADIO::mm_d_log)
        URHO3D_LOGINFO("Rotating animation " + animationFile);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Animation* anim = cache->GetResource<Animation>(animationFile); //("Animation", animationFile);

    if (!anim) 
    {
        ErrorDialog(TITLE, animationFile + " not found!");
        GetSubsystem<Engine>()->Exit(); // Грубо!
        return;
    }

    // Получаем треки вращения и перемещения всего скелета (кости BPos и BRot)
    AnimationTrack* translateTrack = anim->GetTracks()[TranslateBoneName];
    AnimationTrack* rotateTrack = anim->GetTracks()[RotateBoneName];

    // На это значение нужно повернуть (Y ось)
    Quaternion q(0, rotateAngle, 0);

    Node* rotateNode = curRig_->rotateNode; // Узел BRot косточки

    if (rotateTrack)
    {
        // Проходим по всем кадрам трека BRot кости
        for (unsigned int i = 0; i < rotateTrack->GetNumKeyFrames(); ++i)
        {
            AnimationKeyFrame kf(rotateTrack->keyFrames_[i]);
            // Установим локальное вращение Рига как в кадре.
            rotateNode->SetRotation(kf.rotation_);
            // Получим вращение Рига в мировых координатах
            Quaternion wq = rotateNode->GetWorldRotation();
            // Поворачиваем Риг
            wq = q * wq;
            rotateNode->SetWorldRotation(wq);
            // Поворачиваем Кадр как у Рига и назначаем его треку.
            kf.rotation_ = rotateNode->GetRotation();
            rotateTrack->keyFrames_[i] = kf;
        }
    }
    if (translateTrack)
    {
        // Идем по всем кадрам трека BPos кости и корректируем их вращение.
        for (unsigned int i = 0; i < translateTrack->GetNumKeyFrames(); ++i)
        {
            AnimationKeyFrame kf(translateTrack->keyFrames_[i]);
            kf.position_ = q * kf.position_;
            translateTrack->keyFrames_[i] = kf;
        }
    }
    // Короче: повернули Риг и треки для BRot и BPos костей.
}

// Used only in AssetProcess
void MotionManager::FixAnimationOrigin(MotionRig* rig, const String& animationFile, int motionFlag)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Animation* anim = cache->GetResource<Animation>(animationFile);
    if (!anim) 
    {
        ErrorDialog(TITLE, animationFile + " not found!");
        GetSubsystem<Engine>()->Exit(); // Грубо!
        return;
    }

    // Получаем трек BPos кости из анимации.
    AnimationTrack* translateTrack = anim->GetTracks()[TranslateBoneName]; // tracks[TranslateBoneName];
    if (!translateTrack)
    {
        URHO3D_LOGINFO(animationFile + " translation track not found!!!");
        return;
    }

    // Узел BPos кости в Риге
    Node* translateNode = curRig_->translateNode;

    int translateFlag = 0; // ниже будем его заполнять.
    // Позиция (локальная) BPos кости в первом кадре минус initialPosition_ этой кости в Риге (если эта разница есть)
    Vector3 position = translateTrack->keyFrames_[0].position_ - rig->pelvisOrign;
    const float minDist = 0.5f;

    // если есть разница по оси X
    if (Abs(position.x_) > minDist) 
    {
        if (RADIO::mm_d_log)
            URHO3D_LOGINFO(animationFile + " Need reset x position");
        translateFlag |= kMotion_X; // нужна коррекция по Х
    }
    // если есть разница по оси Y больше 2 и что-то с флагами
    if (Abs(position.y_) > 2.0f && (motionFlag & kMotion_Ext_Adjust_Y)) // kMotion_Ext_Adjust_Y != 0
    {
        if (RADIO::mm_d_log)
            URHO3D_LOGINFO(animationFile + " Need reset y position");
        translateFlag |= kMotion_Y; // нужна коррекция по Y (добавили)
    }
    // если есть разница по оси Z
    if (Abs(position.z_) > minDist) 
    {
        if (RADIO::mm_d_log)
            URHO3D_LOGINFO(animationFile + " Need reset z position");
        translateFlag |= kMotion_Z; // нужна коррекция по Z (добавили)
    }
    if (RADIO::mm_d_log)
        URHO3D_LOGINFO("t-diff-position = " + position.ToString());

    if (translateFlag == 0) // если коррекция не нужна
        return;

    // Помещаем BPos Узел Рига в позицию как в первом кадре
    Vector3 firstKeyPos = translateTrack->keyFrames_[0].position_;
    translateNode->SetPosition(firstKeyPos);
    // Запомним WorldPosition этого Узла в две переменные.
    Vector3 currentWS = translateNode->GetWorldPosition();
    Vector3 oldWS = currentWS; // не используется нигде в этой функции

    // Позиция в первом кадре currentWS сбрасываем до изначальной позиции BPos в Риге (initialPosition_)
    if (translateFlag & kMotion_X) // != 0
        currentWS.x_ = rig->pelvisOrign.x_; // pelvisOrign это skeleton.GetBone(TranslateBoneName)->initialPosition_
    if (translateFlag & kMotion_Y) // != 0
        currentWS.y_ = rig->pelvisOrign.y_;
    if (translateFlag & kMotion_Z) // != 0
        currentWS.z_ = rig->pelvisOrign.z_;

    // Узел BPos косточки в Риге устанавливаем в эту "сброшенную" позицию. (вернули из позиции первого кадра)
    translateNode->SetWorldPosition(currentWS);
    // Берем локальную позицию этого Узла
    Vector3 currentLS = translateNode->GetPosition();
    // Находим разницу между локальными позициями "сброшенной" и в первом кадре.
    Vector3 originDiffLS = currentLS - firstKeyPos;
    // Смещаем на originDiffLS все кадры анимации для BPos косточки.
    TranslateAnimation(animationFile, originDiffLS);
}

// Used only in AssetProcess
// Array<Vector3>* outPositions
void MotionManager::CollectBoneWorldPositions(MotionRig* rig, const String& animationFile, const String& boneName, Vector<Vector3>& outPositions)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Animation* anim = cache->GetResource<Animation>(animationFile); //("Animation", animationFile);
    if (!anim) {
        ErrorDialog(TITLE, animationFile + " not found!");
        GetSubsystem<Engine>()->Exit(); //engine.Exit();
        return;
    }

    AnimationTrack* track = anim->GetTracks()[boneName]; //tracks[boneName];
    if (!track)
        return;

    AnimatedModel* am = rig->alignNode->GetComponent<AnimatedModel>(); //("AnimatedModel");
    am->RemoveAllAnimationStates();
    AnimationState* state = am->AddAnimationState(anim);
    state->SetWeight(1.0f); // weight = 1.0f;
    state->SetLooped(false); // looped = false;

    outPositions.Resize(track->GetNumKeyFrames());
    Node* boneNode = rig->alignNode->GetChild(boneName, true);

    for (unsigned int i = 0; i < track->GetNumKeyFrames(); ++i)
    {
        state->SetTime(track->keyFrames_[i].time_); //.time = track->keyFrames_[i].time;
        state->Apply();
        rig->alignNode->MarkDirty();
        outPositions[i] = boneNode->GetWorldPosition(); //.worldPosition;
        // URHO3D_LOGINFO("out-position=" + outPositions[i].ToString());
    }
}

// Used only in AssetProcess - in Batman this function commented
void MotionManager::TranslateAnimation(const String& animationFile, const Vector3& diff)
{
    if (RADIO::mm_d_log)
        URHO3D_LOGINFO("Translating animation " + animationFile);

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Animation* anim = cache->GetResource<Animation>(animationFile);
    if (!anim) {
        ErrorDialog(TITLE, animationFile + " not found!");
        GetSubsystem<Engine>()->Exit(); // Грубо!
        return;
    }

    AnimationTrack* translateTrack = anim->GetTracks()[TranslateBoneName];
    if (translateTrack)
    {
        // Смещаем на diff все кадры анимации BPos косточки.
        for (unsigned int i = 0; i < translateTrack->GetNumKeyFrames(); ++i)
        {
            AnimationKeyFrame kf(translateTrack->keyFrames_[i]);
            kf.position_ += diff;
            translateTrack->keyFrames_[i] = kf;
        }
    }
}

// int FindMotionIndex(const Array<Motion*>& motions, const String& name)
int MotionManager::FindMotionIndex(const Vector<Motion*>& motions, const String& name)
{
    for (unsigned int i = 0; i < motions.Size(); ++i) //length; ++i)
    {
        if (motions[i]->name == name)
            return i;
    }
    return -1;
}

Motion* MotionManager::FindMotion(StringHash nameHash)
{
    for (unsigned int i = 0; i < motions.Size(); ++i) //length; ++i)
    {
        if (motions[i]->nameHash == nameHash)
            return motions[i];
    }
    return NULL;
}

Motion* MotionManager::FindMotion(const String& name)
{
    Motion* m = FindMotion(StringHash(name));
    if (!m)
        Log::Write(-1," ERROR: MotionManager: Motion not exist " + name +"\n");
    return m;
}








