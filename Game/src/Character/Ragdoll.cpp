#include <Urho3D/Urho3DAll.h>

#include "Character/Ragdoll.h"
#include "Objects/GameObject.h" // for COLLISION_LAYER_RAGDOLL and other
#include "Character/CharacterStates.h" // for TIME_SCALE
#include "Motions/MotionManager.h"
#include "radio.h"

bool blend_to_anim = false;
// На данный момент известно, что: 
// 0 создает регдолл в узлах костей. (существует всегда)
// 1 создает красные риги (триггеры) всегда видимые в сцене.
// 2 видимых в сцене изменений не делает (предпологаю создение в ран-тайм)
int ragdoll_method = 1; // 2 in script


Ragdoll::Ragdoll(Context* context):
    LogicComponent(context)
{
    Log::Write(-1," Ragdoll: Constructor\n");

    state = RAGDOLL_NONE;
    stateRequest = -1;
    hasVelRequest = false;
    velocityRequest = Vector3(0, 0, 0);
    ragdollToAnimBlendTime = 1.0f;
    minRagdollStateTime = 5.0f;
    maxRagdollStateTime = 10.0f;
    getUpIndex = 0;
}

Ragdoll::~Ragdoll()
{
    Log::Write(-1," Ragdoll: Destructor\n");
//    DestroyRagdoll();
//    boneNodes.Clear();
    rootNode = NULL;
}


void Ragdoll::Start()
{
    Vector<String> boneNames =
    {
        "Bip01_Head",
        "BRot",// Bip01_Pelvis "Bip01_$AssimpFbx$_Translation",
        "Bip01_Spine1",
        "Bip01_L_Thigh",
        "Bip01_R_Thigh",
        "Bip01_L_Calf",
        "Bip01_R_Calf",
        "Bip01_L_UpperArm",
        "Bip01_R_UpperArm",
        "Bip01_L_Forearm",
        "Bip01_R_Forearm",
        "Bip01_L_Hand",
        "Bip01_R_Hand",
        "Bip01_L_Foot",
        "Bip01_R_Foot",
        // ----------------- end of ragdoll bone -------------------
        "BPos",//"Bip01_$AssimpFbx$_Translation",
        "BRot",//"Bip01_$AssimpFbx$_Rotation",
        //"BPos",//"Bip01_$AssimpFbx$_PreRotation",
        "Bip01_Pelvis",
        "Bip01_Spine",
        "Bip01_Spine2",
        "Bip01_Spine3",
        "Bip01_Neck",
        "Bip01_L_Clavicle",
        "Bip01_R_Clavicle"//,
        //"Bip01"
    };

    rootNode = GetNode(); //node;

    int maxLen = RAGDOLL_BONE_NUM;
    if (blend_to_anim)
        maxLen = boneNames.Size(); //length;

    boneNodes.Resize(maxLen);
    boneLastPositions.Resize(maxLen);

    if (blend_to_anim)
        boneLastRotations.Resize(maxLen);

    for (int i = 0; i < maxLen; ++i)
        {
            // получаем из скелета Узлы костей в соответствии со списком выше.
            boneNodes[i] = GetNode()->GetChild(boneNames[i], true);
            
        }

    Node* renderNode = GetNode(); // GetNode()->GetChild("RenderNode", false);
    
    AnimatedModel* model = GetNode()->GetComponent<AnimatedModel>(); //
    if (!model)
        renderNode = GetNode()->GetChild("RenderNode", false);

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    if (blend_to_anim)
    {
        String st1 = RADIO::GetAnimationName("TG_Getup/GetUp_Back");
        String st2 = RADIO::GetAnimationName("TG_Getup/GetUp_Front");    
        blendingAnim_1 = cache->GetResource<Animation>(st1);
        blendingAnim_2 = cache->GetResource<Animation>(st2);
    }
    // "AnimationTrigger"
    SubscribeToEvent(renderNode, E_ANIMATIONTRIGGER, URHO3D_HANDLER(Ragdoll, HandleAnimationTrigger));

    if (ragdoll_method != 2)
    {
        CreateRagdoll();
        SetPhysicsEnabled(false);
    }
}

void Ragdoll::Stop()
{
Log::Write(-1," Ragdoll: Stop\n");
    DestroyRagdoll();
    boneNodes.Clear();
    rootNode = NULL;
}

void Ragdoll::SetPhysicsEnabled(bool bEnable)
{
    Log::Write(-1," " + rootNode->GetName() + " Ragdoll: SetPhysicsEnabled("+ String(bEnable) + ")\n");

    if (ragdoll_method == 0)
    {
        EnableRagdoll(bEnable);
    }
    else if (ragdoll_method == 1)
    {
        SetRagdollDynamic(bEnable);
        unsigned int mask = COLLISION_LAYER_PROP | COLLISION_LAYER_LANDSCAPE;
        if (bEnable)
            mask |= COLLISION_LAYER_RAGDOLL;
        SetCollisionMask(mask);
    }
    else if (ragdoll_method == 2)
    {
        if (bEnable)
            CreateRagdoll();
        else
            DestroyRagdoll();
    }
}

void Ragdoll::ChangeState(int newState)
{
    if (state == newState)
        return;

    int old_state = state;
//    if (d_log)
        Log::Write(-1, " " + rootNode->GetName() + " Ragdoll ChangeState from " + String(old_state) + " to " + String(newState) + "\n");
    state = newState;

    if (newState == RAGDOLL_STATIC)
    {
        for (unsigned int i = 0; i < RAGDOLL_BONE_NUM; ++i)
        {
            boneLastPositions[i] = boneNodes[i]->GetWorldPosition(); // .worldPosition;
        }
    }
    else if (newState == RAGDOLL_DYNAMIC)
    {
        SetAnimationEnabled(false);
        SetPhysicsEnabled(true);

        if (timeInState > 0.033f)
        {
            for (unsigned int i = 0; i < RAGDOLL_BONE_NUM; ++i)
            {
                RigidBody* rb = boneNodes[i]->GetComponent<RigidBody>();

                if (rb)
                {
                    Vector3 velocity = boneNodes[i]->GetWorldPosition() - boneLastPositions[i];
                    float scale = rootNode->GetVars()[TIME_SCALE]->GetFloat(); // .vars[TIME_SCALE].GetFloat();
                    velocity /= timeInState;
                    velocity *= scale;
                    velocity *= 1.5f; // 1.5f in script
                    // URHO3D_LOGINFO(boneNodes[i].name + " velocity=" + velocity.ToString()); // comment from script
                    // if (i == BONE_PELVIS || i == BONE_SPINE) // comment from script
                    rb->SetLinearVelocity(velocity); // .linearVelocity = velocity;
                }
            }
        }

        if (hasVelRequest)
        {
            for (unsigned int i = 0; i < RAGDOLL_BONE_NUM; ++i)
            {
                RigidBody* rb = boneNodes[i]->GetComponent<RigidBody>(); // ("RigidBody");

                if (rb)
                {
                    Vector3 pos = boneNodes[i]->GetWorldPosition(); // .worldPosition;
                    float y_diff = Abs(pos.y_ - hitPosition.y_);
//                    if (d_log)
                        Log::Write(-1, " " + rootNode->GetName() + " Ragdoll -- " + boneNodes[i]->GetName() + " y_diff = " + String(y_diff)+ "\n");
                    if (y_diff < 1.0f)
                        rb->SetLinearVelocity(velocityRequest); // .linearVelocity = velocityRequest;
                }
            }
            velocityRequest = Vector3(0, 0, 0);
            hasVelRequest = false;

            // rootNode.scene.timeScale = 0.0f; // comment from script
        }
    }
    else if (newState == RAGDOLL_BLEND_TO_ANIMATION)
    {
        SetPhysicsEnabled(false);
        SetAnimationEnabled(true);
        ResetBonePositions(newState);

        for (unsigned int i = 0; i < boneNodes.Size(); ++i)
        {
            boneLastPositions[i] = boneNodes[i]->GetPosition(); // .position;
            boneLastRotations[i] = boneNodes[i]->GetRotation(); // .rotation;
            URHO3D_LOGINFO(boneNodes[i]->GetName() + " last-position=" + boneLastPositions[i].ToString() + " last-rotation=" + boneLastRotations[i].EulerAngles().ToString());
        }
    }
    else if (newState == RAGDOLL_NONE)
    {
        SetPhysicsEnabled(false); // do SetKinematic for all bones to enable (if ragdoll metod 1)
        SetAnimationEnabled(true);
        ResetBonePositions(newState);
    }
    // SetVar(StringHash key, const Variant& value);
    rootNode->SetVar(RAGDOLL_STATE, newState); //.vars[RAGDOLL_STATE] = newState;
    timeInState = 0.0f;
}

void Ragdoll::FixedUpdate(float dt)
{
    if (stateRequest >= 0) 
    {
        ChangeState(stateRequest);
        stateRequest = -1;
    }

    if (state == RAGDOLL_STATIC)
    {
        timeInState += dt;
    }
    else if (state == RAGDOLL_DYNAMIC)
    {
        // URHO3D_LOGINFO("Ragdoll Dynamic time " + timeInState);
        timeInState += dt;

        unsigned int num_of_freeze_objects = 0;
        for (unsigned int i = 0; i < RAGDOLL_BONE_NUM; ++i)
        {
            // Vector3 curPos = boneNodes[i].worldPosition;
            RigidBody* rb = boneNodes[i]->GetComponent<RigidBody>();
            if (!rb || !rb->IsActive()) // .active) 
            {
                num_of_freeze_objects ++;
                continue;
            }

            Vector3 vel = rb->GetLinearVelocity(); // .linearVelocity;
            if (vel.LengthSquared() < 0.1f)
                num_of_freeze_objects ++;
            //URHO3D_LOGINFO(boneNodes[i].name + " vel=" + vel.ToString());
        }

        // URHO3D_LOGINFO("num_of_freeze_objects=" + num_of_freeze_objects);
        if (num_of_freeze_objects == RAGDOLL_BONE_NUM && timeInState >= minRagdollStateTime)
            ChangeState(blend_to_anim ? RAGDOLL_BLEND_TO_ANIMATION : RAGDOLL_NONE);
        else if (timeInState > maxRagdollStateTime)
            ChangeState(blend_to_anim ? RAGDOLL_BLEND_TO_ANIMATION : RAGDOLL_NONE);
    }
    else if (state == RAGDOLL_BLEND_TO_ANIMATION)
    {
        //compute the ragdoll blend amount in the range 0...1
        float ragdollBlendAmount = timeInState / ragdollToAnimBlendTime;
        ragdollBlendAmount = Clamp(ragdollBlendAmount, 0.0f, 1.0f);

        timeInState += dt;

        Animation* anim = blendingAnim_1;
        if (getUpIndex == 1)
            anim = blendingAnim_2;

        for (unsigned int i = 0; i < boneNodes.Size(); ++i)
        {
            AnimationTrack* track = anim->GetTracks()[boneNodes[i]->GetName()];
            if (!track)
                continue;

            Node* n = boneNodes[i];
            Vector3 src_position = boneLastPositions[i];
            Vector3 dst_position = track->keyFrames_[0].position_;

            Quaternion src_rotation = boneLastRotations[i];
            Quaternion dst_rotation = track->keyFrames_[0].rotation_;

            //n.position = src_position.Lerp(dst_position, ragdollBlendAmount);
            n->SetPosition(src_position.Lerp(dst_position, ragdollBlendAmount));
            //n.rotation = src_rotation.Slerp(dst_rotation, ragdollBlendAmount);
            n->SetRotation(src_rotation.Slerp(dst_rotation, ragdollBlendAmount));
        }

        //if the ragdoll blend amount has decreased to zero, move to animated state
        if (ragdollBlendAmount >= 0.9999999f)
            ChangeState(RAGDOLL_NONE);
    }
}

void Ragdoll::CreateRagdoll()
{
    // unsigned int t = time.systemTime;

    Log::Write(-1," Ragdoll: CreateRagdoll\n");

    // Create RigidBody & CollisionShape components to bones
    Quaternion identityQ(0, 0, 0);
    // (0, 0, 90) in script
    Quaternion common_rotation(0, 0, 0); // model exported from 3DS MAX need to roll 90 

    // Create RigidBody & CollisionShape components to bones

    Vector3 upper_leg_size(0.22f, 0.56f, 0.22f);
    Vector3 uppper_leg_offset(0.0f, 0.32f, 0.0f);

    Vector3 lower_leg_size(0.18f, 0.68f, 0.18f);
    Vector3 lower_leg_offset(0.0f, 0.35f, 0.0f);
// size position Vector3(0.15f, 0.35f, 0.15f), Vector3(0.1f, 0.0f, 0.0f)        
    Vector3 upper_arm_size(0.15f, 0.5f, 0.15f);
    Vector3 upper_arm_offset_left(0.04f, 0.18f, 0.05f);
    Vector3 upper_arm_offset_right(-0.04f, 0.18f, 0.05f);

    Vector3 lower_arm_size(0.15f, 0.5f, 0.15f);
    Vector3 lower_arm_offset_left(0.0f, 0.3f, 0.04f);
    Vector3 lower_arm_offset_right(0.0f, 0.3f, 0.04f);

    // Params: RagdollBoneType, ShapeType, size, position, rotation
    CreateRagdollBone(BONE_PELVIS, SHAPE_BOX, Vector3(0.2f, 0.32f, 0.46f), Vector3(0.1f, 0.0f, 0.0f), identityQ);
    CreateRagdollBone(BONE_SPINE, SHAPE_BOX, Vector3(0.28f, 0.5f, 0.3f), Vector3(0.0f, 0.26f, 0.0f), identityQ);
    CreateRagdollBone(BONE_HEAD, SHAPE_BOX, Vector3(0.275f, 0.3f, 0.25f), Vector3(0.0f, 0.12f, -0.05f), identityQ);

    CreateRagdollBone(BONE_L_THIGH, SHAPE_CAPSULE, upper_leg_size, uppper_leg_offset, common_rotation);
    CreateRagdollBone(BONE_R_THIGH, SHAPE_CAPSULE, upper_leg_size, uppper_leg_offset, common_rotation);

    CreateRagdollBone(BONE_L_CALF, SHAPE_CAPSULE, lower_leg_size, lower_leg_offset, common_rotation);
    CreateRagdollBone(BONE_R_CALF, SHAPE_CAPSULE, lower_leg_size, lower_leg_offset, common_rotation);

    CreateRagdollBone(BONE_L_UPPERARM, SHAPE_CAPSULE, upper_arm_size, upper_arm_offset_left, common_rotation);
    CreateRagdollBone(BONE_R_UPPERARM, SHAPE_CAPSULE, upper_arm_size, upper_arm_offset_right, common_rotation);

    CreateRagdollBone(BONE_L_FOREARM, SHAPE_CAPSULE, lower_arm_size, lower_arm_offset_left, common_rotation);
    CreateRagdollBone(BONE_R_FOREARM, SHAPE_CAPSULE, lower_arm_size, lower_arm_offset_right, common_rotation);

    // Create Constraints between bones

    CreateRagdollConstraint(boneNodes[BONE_HEAD], boneNodes[BONE_SPINE], CONSTRAINT_CONETWIST, // boneNode, parentNode, ConstraintType
        Vector3(-1.0f, 0.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f), // axis, parentAxis
        Vector2(45.0f, 30.0f), Vector2(0.0f, 0.0f)); // Vector2 highLimit, Vector2 lowLimit, disableCollision
    //
    CreateRagdollConstraint(boneNodes[BONE_SPINE], boneNodes[BONE_PELVIS], CONSTRAINT_HINGE, // boneNode, parentNode, ConstraintType
        Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), // axis, parentAxis
        Vector2(0.0f, 0.0f), Vector2(0.0f, 45.0f)); // Vector2 highLimit, Vector2 lowLimit, disableCollision
    //
    CreateRagdollConstraint(boneNodes[BONE_L_THIGH], boneNodes[BONE_PELVIS], CONSTRAINT_CONETWIST, // boneNode, parentNode, ConstraintType
        Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f), // axis, parentAxis
        Vector2(35.0f, 35.0f), Vector2(0.0f, 0.0f)); // Vector2 highLimit, Vector2 lowLimit, disableCollision
    //    
    CreateRagdollConstraint(boneNodes[BONE_R_THIGH], boneNodes[BONE_PELVIS], CONSTRAINT_CONETWIST, // boneNode, parentNode, ConstraintType
        Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f), // axis, parentAxis
        Vector2(35.0f, 35.0f), Vector2(0.0f, 0.0f)); // Vector2 highLimit, Vector2 lowLimit, disableCollision
    //
    CreateRagdollConstraint(boneNodes[BONE_L_CALF], boneNodes[BONE_L_THIGH], CONSTRAINT_HINGE, // boneNode, parentNode, ConstraintType
        Vector3(1.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), // axis, parentAxis
        Vector2(100.0f, 0.0f), Vector2(0.0f, 0.0f)); // Vector2 highLimit, Vector2 lowLimit, disableCollision
    //    
    CreateRagdollConstraint(boneNodes[BONE_R_CALF], boneNodes[BONE_R_THIGH], CONSTRAINT_HINGE, // boneNode, parentNode, ConstraintType
        Vector3(1.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), // axis, parentAxis
        Vector2(100.0f, 0.0f), Vector2(0.0f, 0.0f)); // Vector2 highLimit, Vector2 lowLimit, disableCollision

    CreateRagdollConstraint(boneNodes[BONE_L_UPPERARM], boneNodes[BONE_SPINE], // boneNode, parentNode
        CONSTRAINT_CONETWIST, // ConstraintType
        Vector3(0.0f, 0.0f, 1.0f), // axis
        Vector3(0.0f, 0.0f, -1.0f), // parentAxis
        // Первое - руки машут вверх-вниз (лимит вверх слишком большой)
        // Второе - влияет на угол движения рук вперед-назад
        Vector2(160.0f, 50.0f), // highLimit
        Vector2(100.0f, 0.0f), // lowLimit
        false); // disableCollision

    CreateRagdollConstraint(boneNodes[BONE_R_UPPERARM], boneNodes[BONE_SPINE], // boneNode, parentNode
        CONSTRAINT_CONETWIST, // ConstraintType
        Vector3(0.0f, 0.0f, 1.0f), // axis
        Vector3(0.0f, 0.0f, -1.0f), // parentAxis
        Vector2(160.0f, 50.0f), // highLimit
        Vector2(0.0f, 0.0f), // lowLimit
        false); // disableCollision


    CreateRagdollConstraint(boneNodes[BONE_L_FOREARM], boneNodes[BONE_L_UPPERARM], 
        CONSTRAINT_HINGE, // boneNode, parentNode, ConstraintType
        Vector3(0.0f, 0.0f, 1.0f), 
        Vector3(0.0f, 0.0f, 1.0f), // axis, parentAxis
        Vector2(80.0f, 0.0f), 
        Vector2(0.0f, 0.0f)); // Vector2 highLimit, Vector2 lowLimit, disableCollision
        
    CreateRagdollConstraint(boneNodes[BONE_R_FOREARM], boneNodes[BONE_R_UPPERARM], 
        CONSTRAINT_HINGE, // boneNode, parentNode, ConstraintType
        Vector3(0.0f, 0.0f, -1.0f), 
        Vector3(0.0f, 0.0f, -1.0f), // axis, parentAxis
        Vector2(80.0f, 0.0f), 
        Vector2(0.0f, 0.0f)); // Vector2 highLimit, Vector2 lowLimit, disableCollision

    // URHO3D_LOGINFO("CreateRagdoll time-cost=" + (time.systemTime - t) + " ms");
}

void Ragdoll::CreateRagdollBone(RagdollBoneType boneType, ShapeType type, const Vector3& size, const Vector3& position, const Quaternion& rotation)
{
    Node* boneNode = boneNodes[boneType];
    RigidBody* body = boneNode->CreateComponent<RigidBody>();
    // Set mass to make movable
    body->SetMass(1.0f); // .mass = 1.0f;
    // Set damping parameters to smooth out the motion
    body->SetLinearDamping(0.05f); // .linearDamping = 0.1f;
    body->SetAngularDamping(0.85f); // .angularDamping = 0.85f;
    // Set rest thresholds to ensure the ragdoll rigid bodies come to rest to not consume CPU endlessly
    body->SetLinearRestThreshold(1.5f); // linearRestThreshold = 2.5f;
    body->SetAngularRestThreshold(2.5f); // angularRestThreshold = 1.5f;

    body->SetCollisionLayer(COLLISION_LAYER_RAGDOLL); // collisionLayer = COLLISION_LAYER_RAGDOLL;
    body->SetCollisionMask(COLLISION_LAYER_RAGDOLL | COLLISION_LAYER_PROP | COLLISION_LAYER_LANDSCAPE | COLLISION_LAYER_CHARACTER); // collisionMask = ;
    body->SetFriction(1.0f); // friction = 1.0f;
    body->SetGravityOverride(Vector3(0, -32, 0)); // gravityOverride = Vector3(0, -32, 0);
    //body->node.vars[RAGDOLL_ROOT] = rootNode.id;
    body->GetNode()->SetVar(RAGDOLL_ROOT, rootNode->GetID()); //vars[RAGDOLL_ROOT] = rootNode.id;
    // body.kinematic = true;

    //if (boneType == BONE_PELVIS)
    //    body.angularFactor = Vector3(0, 0, 0);

    float BS = 2.1f;

    CollisionShape* shape = boneNode->CreateComponent<CollisionShape>();

    // We use either a box or a capsule shape for all of the bones
    if (type == SHAPE_BOX)
        shape->SetBox(size * BS, position * BS, rotation); // BONE_SCALE in MotionManager.h 100.0f (in script)
    else if (type == SHAPE_SPHERE)
        shape->SetSphere(size.x_ * BS, position * BS, rotation);
    else
        shape->SetCapsule(size.x_ * BS, size.y_ * BS, position * BS, rotation); 
}

void Ragdoll::CreateRagdollConstraint(Node* boneNode, Node* parentNode, ConstraintType type,
    const Vector3& axis, const Vector3& parentAxis, const Vector2& highLimit, const Vector2& lowLimit,
    bool disableCollision)
{
    Constraint* constraint = boneNode->CreateComponent<Constraint>(); // ("Constraint");
    constraint->SetConstraintType(type); // constraintType = type;
    // Most of the constraints in the ragdoll will work better when the connected bodies don't collide against each other
    constraint->SetDisableCollision(disableCollision); // disableCollision = disableCollision;
    // The connected body must be specified before setting the world position
    constraint->SetOtherBody(parentNode->GetComponent<RigidBody>()); // otherBody = parentNode.GetComponent("RigidBody");
    // Position the constraint at the child bone we are connecting
    constraint->SetWorldPosition(boneNode->GetWorldPosition()); // worldPosition = boneNode.worldPosition;
    // Configure axes and limits
    constraint->SetAxis(axis); // axis = axis;
    constraint->SetOtherAxis(parentAxis); // otherAxis = parentAxis;
    constraint->SetHighLimit(highLimit); // highLimit = highLimit;
    constraint->SetLowLimit(lowLimit); // lowLimit = lowLimit;
}

void Ragdoll::DestroyRagdoll()
{
Log::Write(-1," Ragdoll: DestroyRagdoll 1\n");
    for (unsigned int i = 0; i < RAGDOLL_BONE_NUM; ++i)
    {
        boneNodes[i]->RemoveComponent<RigidBody>(); // ("RigidBody");
        boneNodes[i]->RemoveComponent<Constraint>(); // ("Constraint");
    }
Log::Write(-1," Ragdoll: DestroyRagdoll 2\n");
}

void Ragdoll::EnableRagdoll(bool bEnable)
{
    for (unsigned int i = 0; i < RAGDOLL_BONE_NUM; ++i)
    {
        RigidBody* rb = boneNodes[i]->GetComponent<RigidBody>();
        Constraint* cs = boneNodes[i]->GetComponent<Constraint>(); 
        if (rb)
            rb->SetEnabled(bEnable); // enabled_ = bEnable; // enabled_ in Component protected:
        if (cs) {
            cs->SetEnabled(bEnable); // enabled_ = bEnable;
        }
    }
}

void Ragdoll::SetAnimationEnabled(bool bEnable)
{
    // Disable keyframe animation from all bones so that they will not interfere with the ragdoll
    AnimatedModel* model = GetNode()->GetComponent<AnimatedModel>();

    if (!model)
        model = GetNode()->GetChildren()[0]->GetComponent<AnimatedModel>(); // .children[0].GetComponent("AnimatedModel");
    if (!model)
        return;

    Skeleton skeleton = model->GetSkeleton(); //.skeleton;
    for (unsigned int i = 0; i < skeleton.GetNumBones(); ++i)
        // skeleton.GetBones()[i].animated_ = bEnable;
        skeleton.GetBone(i)->animated_ = bEnable;

    if (!bEnable)
        model->RemoveAllAnimationStates();
}

void Ragdoll::SetRagdollDynamic(bool dynamic)
{
    for (unsigned int i = 0; i < RAGDOLL_BONE_NUM; ++i)
    {
        RigidBody* rb = boneNodes[i]->GetComponent<RigidBody>();
        if (rb)
            rb->SetKinematic(!dynamic); // .kinematic = !dynamic;
    }
}

void Ragdoll::SetCollisionMask(unsigned int mask)
{
    for (unsigned int i = 0; i < RAGDOLL_BONE_NUM; ++i)
    {
        RigidBody* rb = boneNodes[i]->GetComponent<RigidBody>();
        if (rb)
            rb->SetCollisionMask(mask); // .collisionMask = mask;
    }
}

void Ragdoll::HandleAnimationTrigger(StringHash eventType, VariantMap& eventData)
{
//    Log::Write(-1," Ragdoll: HandleAnimationTrigger\n");
    using namespace AnimationTrigger;

    VariantMap xdata = eventData[P_DATA].GetVariantMap();

    //OnAnimationTrigger(eventData[P_DATA].GetVariantMap()); // script version
    OnAnimationTrigger(xdata);

    // Отправит уже изъятые из P_DATA данные.
}

void Ragdoll::OnAnimationTrigger(VariantMap& data)
{
    // Добавлено для получения VariantMap из входящего VariantMap
    // Переменную DATA типа VariantMap это единственное что получаем на входе. 
//    VariantMap xdata = data[DATA].GetVariantMap(); // add

    // Получает уже изъятые из P_DATA данные.

    StringHash name = data[NAME].GetStringHash();

    int new_state = RAGDOLL_NONE;

    if (name == RAGDOLL_PERPARE)
    {
        Log::Write(-1, " " + rootNode->GetName() + " Ragdoll: OnAnimationTrigger RAGDOLL_PERPARE\n");
        new_state = RAGDOLL_STATIC;
    }

    else if (name == RAGDOLL_START) // Character: MakeMyRagdoll and Animation Triggers send this event.
    {
        if(RADIO::d_log)
            Log::Write(-1, " " + rootNode->GetName() + " Ragdoll: OnAnimationTrigger RAGDOLL_START\n");
        new_state = RAGDOLL_DYNAMIC;
    }

    else if (name == RAGDOLL_STOP)
        new_state = RAGDOLL_NONE;
//    else
//        Log::Write(-1," Ragdoll: OnAnimationTrigger NO RAGDOLL DATA\n");

    stateRequest = new_state;

    if (data.Contains(VELOCITY))
    {
        hasVelRequest = true;
        velocityRequest = data[VELOCITY].GetVector3();
        hitPosition = data[POSITION].GetVector3();

        Log::Write(-1," " + rootNode->GetName() +" Ragdoll: velocityRequest = "
        + velocityRequest.ToString() + " hitPosition = " + hitPosition.ToString() + "\n");
    }

}

// See RAGDOLL_BLEND_TO_ANIMATION and RAGDOLL_NONE in ChangeState function  
void Ragdoll::ResetBonePositions(int ragdoll_state)
{
/*
    Node* nd = rootNode->GetChild("RenderNode", true);
//    Skeleton sk = nd->GetComponent<AnimatedModel>()->GetSkeleton();
//    sk.Reset();

    Vector<SharedPtr<AnimationState> > sts = nd->GetComponent<AnimatedModel>()->GetAnimationStates();
    for (Vector<SharedPtr<AnimationState> >::Iterator i = sts.Begin(); i != sts.End(); ++i)
        (*i)->Apply();
*/
///*
    Node* pelvis_bone = boneNodes[BONE_PELVIS];
    Quaternion oldRot = pelvis_bone->GetWorldRotation();

    // Определяем верх-низ для анимации состояния GetUp и запомним в переменную узла.
    Vector3 pelvis_up = oldRot * Vector3(0, 1, 0);
    Log::Write(-1, " " + rootNode->GetName() + " Ragdoll: pelvis_up = " + pelvis_up.ToString() + "\n");

    getUpIndex = 0;
    if (pelvis_up.y_ < 0) // вычисляется нормально. Если Y меньше нуля, то 1 ( 0 если наоборот)
        getUpIndex = 1;

    Log::Write(-1, " " + rootNode->GetName() + " Ragdoll: getUpIndex = " + String(getUpIndex) + "\n");
    // Узел в котором компонент (топ узел персонажа)
    rootNode->SetVar(ANIMATION_INDEX, getUpIndex); // .vars[ANIMATION_INDEX] = getUpIndex;

    Vector3 head_pos = boneNodes[BONE_HEAD]->GetWorldPosition(); // .worldPosition;
    Vector3 pelvis_pos = pelvis_bone->GetWorldPosition(); // .worldPosition;

    Node* bpos_node = rootNode->GetChild("BPos", true);//("Bip01_$AssimpFbx$_Translation", true);
    Node* brot_node = rootNode->GetChild("BRot", true);//("Bip01_$AssimpFbx$_Rotation", true);
    Vector3 cur_root_pos = rootNode->GetWorldPosition(); // .worldPosition;
    Vector3 dest_root_pos = cur_root_pos;
    dest_root_pos.x_ = pelvis_pos.x_;
    dest_root_pos.z_ = pelvis_pos.z_;

/*
    // Hack!!!
    if (getUpIndex == 0)
    {
        boneNodes[BONE_SPINE]->SetPosition(Vector3(8.78568, -0.00968838, 0)); 
        bpos_node->SetPosition(Vector3(-0.0264441, 0.282345, 0.461603));
    }
    else
    {
        boneNodes[BONE_SPINE]->SetPosition(Vector3(8.78568, -0.00968742, 0));
        bpos_node->SetPosition(Vector3(-0.0246718, 0.465134, -0.135913)); 
    }
*/
///*
//boneNodes[BONE_SPINE]->SetPosition(Vector3(0.0, 0.0, 0.0));
//bpos_node->SetPosition(Vector3(0.0, 0.28, 0.0));
//bpos_node->SetPosition(Vector3(0.0, 0.28, 0.0));

/*
    boneNodes[BONE_HEAD]->SetPosition(Vector3(0, 0, 0));
    boneNodes[BONE_PELVIS]->SetPosition(Vector3(0, 0, 0)); // BRot
    boneNodes[BONE_SPINE]->SetPosition(Vector3(0, 0, 0));
    boneNodes[BONE_L_THIGH]->SetPosition(Vector3(0, 0, 0));
    boneNodes[BONE_R_THIGH]->SetPosition(Vector3(0, 0, 0));
    boneNodes[BONE_L_CALF]->SetPosition(Vector3(0, 0, 0));
    boneNodes[BONE_R_CALF]->SetPosition(Vector3(0, 0, 0));
    boneNodes[BONE_L_UPPERARM]->SetPosition(Vector3(0, 0, 0));
    boneNodes[BONE_R_UPPERARM]->SetPosition(Vector3(0, 0, 0));
    boneNodes[BONE_L_FOREARM]->SetPosition(Vector3(0, 0, 0));
    boneNodes[BONE_R_FOREARM]->SetPosition(Vector3(0, 0, 0));
    boneNodes[BONE_L_HAND]->SetPosition(Vector3(0, 0, 0));
    boneNodes[BONE_L_FOOT]->SetPosition(Vector3(0, 0, 0));
    boneNodes[BONE_R_FOOT]->SetPosition(Vector3(0, 0, 0));
*/
    pelvis_bone->SetPosition(Vector3(0, 0, 0));
    rootNode->SetWorldPosition(dest_root_pos);
    bpos_node->SetWorldPosition(pelvis_pos);

//    if(ragdoll_state == RAGDOLL_BLEND_TO_ANIMATION)
//    {
        Vector3 ragdolledDirection = head_pos - pelvis_pos;
        ragdolledDirection.y_ = 0.0f;
        Vector3 currentDirection = rootNode->GetWorldRotation() * Vector3(0, 0, 1);
        currentDirection.y_ = 0.0f;

        Quaternion dRot;
        dRot.FromRotationTo(currentDirection, ragdolledDirection);
        Quaternion oldRootRot = rootNode->GetWorldRotation();
        Quaternion targetRootRot = oldRootRot * dRot;

        if (getUpIndex == 0)
            targetRootRot = targetRootRot * Quaternion(0, 180, 0); // (0, 180, 0) in script

        rootNode->SetWorldRotation(targetRootRot);
//    }

/*
    Quaternion q(90, 0, -90); // (90, 0, -90) in script
    // 0.5 0.5 0.5 -0.5 (90 0 -90)
//    Log::Write(-1, " Ragdoll: Quaternion q = " + String(q) + " Euler = " + String(q.EulerAngles()) + "\n"); 

//    pelvis_bone->SetRotation(q);

    q = oldRot * q.Inverse();
//    brot_node->SetWorldRotation(q); // вращает в лежачее положение. Иначе, будет развернут в вертикальное.

    // URHO3D_LOGINFO("targetRootRot=" + targetRootRot.eulerAngles.ToString()); // original comment
    // q = brot_node.worldRotation; // original comment
    rootNode->SetWorldRotation(targetRootRot);

    // повторный вызов (лишний)
    pelvis_bone->SetWorldRotation(q); // .worldRotation = q; 
*/

//rootNode->SetWorldRotation(targetRootRot);
}









