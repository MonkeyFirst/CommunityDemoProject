#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Motions/MotionRig.h"
#include "Motions/Motions.h"

//unsigned int game_type = 1;// временное решение - нужно в начале запуска проверять.

const String TITLE = "AssetProcess";
const String TranslateBoneName = "BPos";//"Bip01_$AssimpFbx$_Translation";
const String RotateBoneName = "BRot";//"Bip01_$AssimpFbx$_Rotation";
//const String ScaleBoneName = "Bip01_$AssimpFbx$_Scaling";

const String HEAD = "Bip01_Head";
const String L_HAND = "Bip01_L_Hand";
const String R_HAND = "Bip01_R_Hand";
const String L_FOOT = "Bip01_L_Foot";
const String R_FOOT = "Bip01_R_Foot";
const String L_ARM = "Bip01_L_Forearm";
const String R_ARM = "Bip01_R_Forearm";
const String L_CALF = "Bip01_L_Calf";
const String R_CALF = "Bip01_R_Calf";

const float FRAME_PER_SEC = 25.0f;//30.0f;
const float SEC_PER_FRAME = 1.0f/FRAME_PER_SEC;
const int   PROCESS_TIME_PER_FRAME = 60; // ms
//const float BONE_SCALE = 2.0f; // 100.0f in script used in Ragdoll
const float BIG_HEAD_SCALE = 2.0f;

enum RootMotionFlag
{
    kMotion_None= 0,
    kMotion_X   = (1 << 0),
    kMotion_Y   = (1 << 1),
    kMotion_Z   = (1 << 2),
    kMotion_R   = (1 << 3),

    kMotion_Ext_Rotate_From_Start = (1 << 4),
    kMotion_Ext_Debug_Dump = (1 << 5),
    kMotion_Ext_Adjust_Y = (1 << 6),
    kMotion_Ext_Foot_Based_Height = (1 << 7),

    kMotion_XZR = kMotion_X | kMotion_Z | kMotion_R,
    kMotion_YZR = kMotion_Y | kMotion_Z | kMotion_R,
    kMotion_XYR = kMotion_X | kMotion_Y | kMotion_R,

    kMotion_XZ  = kMotion_X | kMotion_Z,
    kMotion_XR  = kMotion_X | kMotion_R,
    kMotion_ZR  = kMotion_Z | kMotion_R,
    kMotion_XY  = kMotion_X | kMotion_Y,
    kMotion_YZ  = kMotion_Y | kMotion_Z,
    kMotion_XYZ = kMotion_XZ | kMotion_Y,
    kMotion_ALL = kMotion_XZR | kMotion_Y,
};


enum MotionLoadingState
{
    MOTION_LOADING_START = 0,
    MOTION_LOADING_MOTIONS,
    MOTION_LOADING_ANIMATIONS,
    MOTION_LOADING_FINISHED
};

class MotionManager : public Object
{
    URHO3D_OBJECT(MotionManager, Object);

public:
    MotionManager(Context* context);
    ~MotionManager();


    void Run();
    void Finalize();
    void Initialize();
    
    bool Update(float dt);
    void Finish();
    void PostProcess();
    void AssetPostProcess();

    void AddTriggers();
    void AddThugAnimationTriggers();
    void AddThugCombatAnimationTriggers();
    void Add_BM_AnimationTriggers();

    void AddMotions();
    void Create_BM_Motions();
    void Create_BM_ClimbAnimations();
    void CreateThugMotions();
    void CreateThugCombatMotions();
    
    void Create_BM_Attack_Motions();

    // ============= TRIGGERS =================================================================
    void Animation_AddTrigger(const String& name, int frame, const VariantMap& data);
    void AddAnimationTrigger(const String& name, int frame, const String& tag);
    void AddAnimationTrigger_Hash(const String& name, int frame, const StringHash& tag);
    void AddFloatAnimationTrigger(const String& name, int frame, const StringHash& tag, float value);
    void AddIntAnimationTrigger(const String& name, int frame, const StringHash& tag, int value);
    void AddStringAnimationTrigger(const String& name, int frame, const StringHash& tag, const String& value);
    void AddStringHashAnimationTrigger(const String& name, int frame, const StringHash& tag, const StringHash& value);
    void AddAttackTrigger(const String& name, int startFrame, int endFrame, const String& boneName);
    void AddRagdollTrigger(const String& name, int prepareFrame, int startFrame);
    void AddParticleTrigger(const String& name, int startFrame, const String& boneName, const String& effectName, float duration);
    void AddComplexAttackTrigger(const String& name, int counterStart, int counterEnd, int attackStart, int attackEnd, const String& attackBone);
    // ============= TRIGGERS =================================================================
    
    Scene* GetProcessScene(){return processScene_;}

    void AssignMotionRig(const String& rigName);
    void AddAnimation(const String& animation);

    // Used only in MotionManager::CreateMotion_InFolder(const String& folder)
    String FileNameToMotionName(const String& name);

    // Used only in Motion::Process()
// Array<Vector4>&out outKeys
    float ProcessAnimation(const String& animationFile, int motionFlag, int allowMotion, float rotateAngle, Vector<Vector4>& outKeys, Vector4& startFromOrigin);
    // Used in Motion::Process()
    Vector3 GetBoneWorldPosition(MotionRig* rig, const String& animationFile, const String& boneName, float t);
    // Used only in AssetProcess - only in next function GetRotationInXZPlane
    Vector3 GetProjectedAxis(MotionRig* rig, Node* node, const Vector3& axis);
    // Used only in AssetProcess
    Quaternion GetRotationInXZPlane(MotionRig* rig, const Quaternion& startLocalRot, const Quaternion& curLocalRot);
    // Used only in AssetProcess 
    void RotateAnimation(const String& animationFile, float rotateAngle);
    // Used only in AssetProcess
    void FixAnimationOrigin(MotionRig* rig, const String& animationFile, int motionFlag);
    // Used only in AssetProcess
    // Array<Vector3>* outPositions
    void CollectBoneWorldPositions(MotionRig* rig, const String& animationFile, const String& boneName, Vector<Vector3>& outPositions);
    // Used only in AssetProcess - in Batman this function commented
    void TranslateAnimation(const String& animationFile, const Vector3& diff);
    // int FindMotionIndex(const Array<Motion*>& motions, const String& name)
    int FindMotionIndex(const Vector<Motion*>& motions, const String& name);
    Motion* FindMotion(StringHash nameHash);
    Motion* FindMotion(const String& name);

    // Вроде как нигде не используется
    void ProcessAll();

    unsigned int assetProcessTime;
    int processedMotions;
    int processedAnimations;
    unsigned int assetProcessTime_;
    SharedPtr<Scene> processScene_;
//    unsigned int game_type_;
    unsigned int sub_state;

    MotionRig* curRig_;
    //SharedPtr<MotionRig> curRig_;
    //WeakPtr<MotionRig> curRig_;
    int memoryUse;
//    bool d_log;

private:

    void AssetPreProcess();
    //Motion* CreateMotion(const String& name, int motionFlag = kMotion_XZR, int allowMotion = kMotion_XZR,  int endFrame = -1, bool loop = false, float rotateAngle = 361);
    Motion* CreateMotion(const String& name, int motionFlag = kMotion_XZR, int allowMotion = kMotion_XZR,  int endFrame = -1, bool loop = false, float rotateAngle = 361);
    void CreateMotion_InFolder(const String& folder);

    Vector<Motion*> motions; // Array<Motion*> motions;
    Vector<String> animations; // Array<String> animations;

};











