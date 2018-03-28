#include "radio.h"

namespace RADIO
{
    String CFG_ROOT = String::EMPTY;
    
    float MAX_ATTACK_DIST = 25.0f;
    float BRUCE_TRANSITION_DIST = 0.0f;

    float PUNCH_DIST = 0.0f;
    float KICK_DIST = 0.0f;
    float STEP_MAX_DIST = 0.0f;
    float STEP_MIN_DIST = 0.0f;
    //float KEEP_DIST_WITH_PLAYER = -0.25f;  // declared in Enemy.h
    float KEEP_DIST_WITH_PLR = -0.25f;
    
    Vector3 WORLD_HALF_SIZE(1000, 0, 1000);
    int num_of_sounds = 36; // 37 (in Character Class Random(num_of_sounds) + 1)
    int num_of_big_sounds = 5; // 6

    SharedPtr<CfgManager> g_cfg = NULL;
    SharedPtr<InputManager> g_im = NULL;
    SharedPtr<GameManager> g_gm = NULL;
    SharedPtr<MotionManager> g_mm = NULL;
    SharedPtr<CameraManager> g_cm = NULL;
    SharedPtr<EnemyManager> g_em = NULL;
    SharedPtr<LineWorld> g_lw = NULL;
    
    bool bHdr = true;
    int colorGradingIndex = 0; // shader
    int render_features = RF_FULL;
    bool reflection = false;
    
    int freeze_ai = 0;
    int game_type = 0;
    int collision_type = 1;
    int tmpdebugvalue = 1; // initialize

    unsigned int cameraId = M_MAX_UNSIGNED;
    unsigned int playerId = M_MAX_UNSIGNED;
    
    int drawDebug = 1; 
    bool d_log = true;
    bool mm_d_log = false;
    bool updateEnable = false;

    float BGM_BASE_FREQ = 44100;
    bool nobgm = false;
    
    String CAMERA_NAME = "Camera";
    String LUT = "";
    String UI_FONT = "Fonts/GAEN.ttf";
    int UI_FONT_SIZE = 40;
    String LOAD_FONT = "Fonts/Anonymous Pro.ttf";
    int LOAD_FONT_SIZE = 16;

    int test_enemy_num_override = -1;
    int test_beat_index = 1;
    bool base_on_player = false;
    int test_counter_index = 0;
    int test_double_counter_index = 0;
    int test_triple_counter_index = 0;
    int maxKilled = 5;

    int playerType = 0;

    // clamps an angle to the rangle of [-2PI, 2PI]
    float AngleDiff( float diff )
    {
        if (diff > 180)
            diff -= 360;
        if (diff < -180)
            diff += 360;
        return diff;
    }

    float FaceAngleDiff(Node* thisNode, Node* targetNode)
    {
        Vector3 posDiff = targetNode->GetWorldPosition() - thisNode->GetWorldPosition();
        Vector3 thisDir = thisNode->GetWorldRotation() * Vector3(0, 0, 1);
        float thisAngle = Atan2(thisDir.x_, thisDir.z_);
        float targetAngle = Atan2(posDiff.x_, posDiff.y_);
        return AngleDiff(targetAngle - thisAngle);
    }

    int DirectionMapToIndex(float directionDifference, int numDirections)
    {
        float directionVariable = Floor(directionDifference / (180 / (numDirections / 2)) + 0.5f);
        // since the range of the direction variable is [-3, 3] we need to map negative
        // values to the animation index range in our selector which is [0,7]
        if( directionVariable < 0 )
            directionVariable += numDirections;
        return int(directionVariable);
    }

    Vector3 FilterPosition(const Vector3& position)
    {
        float x = position.x_;
        float z = position.z_;
        float radius = COLLISION_RADIUS + 1.0f;
        x = Clamp(x, radius - WORLD_HALF_SIZE.x_, WORLD_HALF_SIZE.x_ - radius);
        z = Clamp(z, radius - WORLD_HALF_SIZE.z_, WORLD_HALF_SIZE.z_ - radius);
        return Vector3(x, position.y_, z);
    }

    void FillAnimationWithCurrentPose(Animation* anim, Node* _node)
    {
        Vector<String> boneNames =
        {
            "BPos",//"Bip01_$AssimpFbx$_Translation",
            //"Bip01_$AssimpFbx$_PreRotation",
            "BRot",//"Bip01_$AssimpFbx$_Rotation",
            "Bip01_Pelvis",
            "Bip01_Spine",
            "Bip01_Spine1",
            "Bip01_Spine2",
            "Bip01_Spine3",
            "Bip01_Neck",
            "Bip01_Head",
            "Bip01_L_Thigh",
            "Bip01_L_Calf",
            "Bip01_L_Foot",
            "Bip01_R_Thigh",
            "Bip01_R_Calf",
            "Bip01_R_Foot",
            "Bip01_L_Clavicle",
            "Bip01_L_UpperArm",
            "Bip01_L_Forearm",
            "Bip01_L_Hand",
            "Bip01_R_Clavicle",
            "Bip01_R_UpperArm",
            "Bip01_R_Forearm",
            "Bip01_R_Hand"
        };

        anim->RemoveAllTracks();
        for (unsigned int i = 0; i < boneNames.Size(); ++i)
        {
            Node* n = _node->GetChild(boneNames[i], true);
            if (!n)
            {
                Log::Write(-1," FillAnimationWithCurrentPose can not find bone " + boneNames[i] +"\n");
                continue;
            }
            AnimationTrack* track = anim->CreateTrack(boneNames[i]);
            track->channelMask_ = CHANNEL_POSITION | CHANNEL_ROTATION;
            AnimationKeyFrame kf;
            kf.time_ = 0.0f;
            kf.position_ = n->GetPosition(); // .position;
            kf.rotation_ = n->GetRotation(); // .rotation;
            track->AddKeyFrame(kf);
        }
    }
    
    String GetAnimationName(const String& name)
    {
        return "Animations/" + name + ".ani";
    }

    void SendAnimationTriger(Node* _node, const StringHash& nameHash, int value)
    {
        using namespace AnimationTrigger;

        VariantMap anim_data;
        anim_data[NAME] = nameHash;
        anim_data[VALUE] = value;

        VariantMap data;
        data[P_DATA] = anim_data; // User-defined data type

        _node->SendEvent(E_ANIMATIONTRIGGER, data); // "AnimationTrigger"
    }

    void DebugDrawDirection(DebugRenderer* debug, const Vector3& start, float angle, const Color& color, float radius)
    {
        Vector3 end = start + Vector3(Sin(angle) * radius, 0, Cos(angle) * radius);
        debug->AddLine(start, end, color, false);
    }

    Vector4 GetTargetTransform(Node* baseNode, Motion* alignMotion, Motion* baseMotion)
    {
        float r1 = alignMotion->GetStartRot();
        float r2 = baseMotion->GetStartRot();
        Vector3 s1 = alignMotion->GetStartPos();
        Vector3 s2 = baseMotion->GetStartPos();

        float baseYaw = baseNode->GetWorldRotation().EulerAngles().y_;
        float targetRotation = baseYaw + (r1 - r2);
        Vector3 diff_ws = Quaternion(0, baseYaw - r2, 0) * (s1 - s2);
        Vector3 targetPosition = baseNode->GetWorldPosition() + diff_ws;
/*
        if (RADIO::mm_d_log)
        {
            URHO3D_LOGINFO("-----------------------------------------------------------------------------------");
            URHO3D_LOGINFO("GetTargetTransform align-motion=" + alignMotion->name + " base-motion=" + baseMotion->name);
            URHO3D_LOGINFO("GetTargetTransform base=" + baseNode->GetName() + " align-start-pos=" + s1.ToString() + " base-start-pos=" + s2.ToString() + " p-diff=" + (s1 - s2).ToString());
            URHO3D_LOGINFO("baseYaw=" + String(baseYaw) + " targetRotation=" + String(targetRotation) + " align-start-rot=" + String(r1) + " base-start-rot=" + String(r2) + " r-diff=" + String(r1 - r2));
            URHO3D_LOGINFO("basePosition=" + baseNode->GetWorldPosition().ToString() + " diff_ws=" + diff_ws.ToString() + " targetPosition=" + targetPosition.ToString());
            URHO3D_LOGINFO("-----------------------------------------------------------------------------------");
        }
*/
        return Vector4(targetPosition.x_,  targetPosition.y_, targetPosition.z_, targetRotation);
    }

    Vector3 GetRagdollForce()
    {
        float x = Random(HIT_RAGDOLL_FORCE.x_ * 0.75f, HIT_RAGDOLL_FORCE.x_ * 1.25f);
        float y = Random(HIT_RAGDOLL_FORCE.y_ * 0.75f, HIT_RAGDOLL_FORCE.y_ * 1.25f);
        return Vector3(x, y, x);
    }
}

