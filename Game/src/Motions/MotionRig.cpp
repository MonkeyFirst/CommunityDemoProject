
#include <Urho3D/Urho3DAll.h>

#include "Motions/MotionRig.h"
#include "Motions/MotionManager.h"

#include "radio.h"

// MotionRig ==============================================

MotionRig::MotionRig(Context* context, const String& rigName):
    Object(context)
{
    Log::Write(-1,"\n MotionRig Constructor " + rigName + "\n");

    pelvisRightAxis = Vector3(0, 0, 1); // 1, 0, 0 - приводит к анимации бега в обратном направлении, на стене задом-наперед.
    left_foot_to_ground_height = 0.0f;
    right_foot_to_ground_height = 0.0f;

    rig = rigName;
//    if (bigHeadMode)
//    {
//        Vector3 v(BIG_HEAD_SCALE, BIG_HEAD_SCALE, BIG_HEAD_SCALE);
//        Model* m = cache.GetResource("Model",  rigName);
//        Skeleton* s = m.skeleton;
//        s.GetBone(HEAD).initialScale = v;
//        s.GetBone(L_HAND).initialScale = v;
//        s.GetBone(R_HAND).initialScale = v;
//        s.GetBone(L_FOOT).initialScale = v;
//        s.GetBone(R_FOOT).initialScale = v;
//    }
    Scene* processScene = RADIO::g_mm->GetProcessScene();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    processNode = processScene->CreateChild(rig + "_Character");
    processNode->SetWorldRotation(Quaternion(0, 0, 0));//.worldRotation = Quaternion(0, 0, 0);//(0, 180, 0)

    AnimatedModel* am = processNode->CreateComponent<AnimatedModel>();//("AnimatedModel");
    //am.model = cache.GetResource("Model", rigName);
    am->SetModel(cache->GetResource<Model>(rigName));
//Log::Write(-1,"\n AAAAAAAAAA\n");
    skeleton = am->GetSkeleton();//am.skeleton;
//Log::Write(-1,"\n AAAAAAAAAA 1\n");
    Bone* bone = skeleton.GetBone(RotateBoneName);
//Log::Write(-1,"\n AAAAAAAAAA 2\n");
    rotateBoneInitQ = bone->initialRotation_;//initialRotation;
//Log::Write(-1,"\n AAAAAAAAAA 3\n");
    // * -1 добавлено для соответствия значения pelvisRightAxis как в скрипте (самого умножения в скрипте нет)
    //pelvisRightAxis = (rotateBoneInitQ * Vector3(0, 0, 1)) * -1; 
    pelvisRightAxis = rotateBoneInitQ * Vector3(0, 0, 1);
    pelvisRightAxis.Normalize();

    translateNode = processNode->GetChild(TranslateBoneName, true);
    rotateNode = processNode->GetChild(RotateBoneName, true);
    pelvisOrign = skeleton.GetBone(TranslateBoneName)->initialPosition_;

    left_foot_to_ground_height = processNode->GetChild(L_FOOT, true)->GetWorldPosition().y_;
    right_foot_to_ground_height = processNode->GetChild(R_FOOT, true)->GetWorldPosition().y_;

    alignNode = processScene->CreateChild(rig + "_Align");
    alignNode->SetWorldRotation(Quaternion(0, 0, 0));//worldRotation = Quaternion(0, 0, 0);//(0, 180, 0)
    AnimatedModel* am2 = alignNode->CreateComponent<AnimatedModel>();//CreateComponent("AnimatedModel");
    //am2.model = am.model;
    am2->SetModel(am->GetModel());

    if (RADIO::mm_d_log)
    {
        Log::Write(-1,"\n xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
        Log::Write(-1, " MotionRig = " + rigName + "\n");
        Log::Write(-1, " rotateBoneInitQ (BRot->initialRotation_ euler Y) = " + String(bone->initialRotation_.EulerAngles().y_) + "\n");
        Log::Write(-1, " pelvisOrign (BPos->initialPosition_) = " + pelvisOrign.ToString() + "\n");
        Log::Write(-1, " pelvisRightAxis = " + pelvisRightAxis.ToString() + "\n");
        Log::Write(-1, " left_foot_to_ground_height = " + String(left_foot_to_ground_height) + "\n");
        Log::Write(-1, " right_foot_to_ground_height = " + String(right_foot_to_ground_height) + "\n");
        Log::Write(-1," xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n\n");   
    }

//    URHO3D_LOGINFO(rigName + " pelvisRightAxis=" + pelvisRightAxis.ToString() + " pelvisOrign=" + pelvisOrign.ToString());
}

MotionRig::~MotionRig()
{
    processNode->Remove();
    alignNode->Remove();
}