#pragma once

#include <Urho3D/Urho3DAll.h>
//#include "Motions/Motions.h"

enum RagdollBoneType
{
    BONE_HEAD,
    BONE_PELVIS,
    BONE_SPINE,
    BONE_L_THIGH,
    BONE_R_THIGH,
    BONE_L_CALF,
    BONE_R_CALF,
    BONE_L_UPPERARM,
    BONE_R_UPPERARM,
    BONE_L_FOREARM,
    BONE_R_FOREARM,
    BONE_L_HAND,
    BONE_R_HAND,
    BONE_L_FOOT,
    BONE_R_FOOT,
    RAGDOLL_BONE_NUM
};

enum RagdollState
{
    RAGDOLL_NONE,
    RAGDOLL_STATIC,
    RAGDOLL_DYNAMIC,
    RAGDOLL_BLEND_TO_ANIMATION,
};

const StringHash RAGDOLL_STATE("Ragdoll_State");
const StringHash RAGDOLL_PERPARE("Ragdoll_Prepare");
const StringHash RAGDOLL_START("Ragdoll_Start");
const StringHash RAGDOLL_STOP("Ragdoll_Stop");
const StringHash RAGDOLL_ROOT("Ragdoll_Root");
const StringHash VELOCITY("Velocity");
const StringHash POSITION("Position");

const float BONE_SCALE = 2.1f; // 100.0f in script used in Ragdoll

class Ragdoll : public LogicComponent
{
    URHO3D_OBJECT(Ragdoll, LogicComponent);

public:
    Ragdoll(Context* context);
    ~Ragdoll();

    // Called when the component is added to a scene node. Other components may not yet exist.
    virtual void Start();
    // Called when the component is detached from a scene node, usually on destruction. 
    // Note that you will no longer have access to the node and scene at that point.
    virtual void Stop();
    // Called on physics update, fixed timestep.
    virtual void FixedUpdate(float dt);
    
    void SetPhysicsEnabled(bool bEnable);
    void ChangeState(int newState);
    void CreateRagdoll();
    void CreateRagdollBone(RagdollBoneType boneType, ShapeType type, const Vector3& size, const Vector3& position, const Quaternion& rotation);
    void CreateRagdollConstraint(Node* boneNode, Node* parentNode, ConstraintType type, const Vector3& axis, const Vector3& parentAxis, const Vector2& highLimit, const Vector2& lowLimit, bool disableCollision = true);
    void DestroyRagdoll();
    void EnableRagdoll(bool bEnable);
    void SetAnimationEnabled(bool bEnable);
    void SetRagdollDynamic(bool dynamic);
    void SetCollisionMask(unsigned int mask);
    void HandleAnimationTrigger(StringHash eventType, VariantMap& eventData);
    void OnAnimationTrigger(VariantMap& data);
    void ResetBonePositions(int ragdoll_state);

    Vector<Node*> boneNodes;
    Vector<Vector3> boneLastPositions;
    Vector<Quaternion> boneLastRotations;
    Node* rootNode;

    int state;
    int stateRequest;
    bool hasVelRequest;
    Vector3 velocityRequest;
    Vector3 hitPosition;

    float timeInState;

    Animation* blendingAnim_1;
    Animation* blendingAnim_2;

    float ragdollToAnimBlendTime;
    float minRagdollStateTime;
    float maxRagdollStateTime;

    int getUpIndex;
};