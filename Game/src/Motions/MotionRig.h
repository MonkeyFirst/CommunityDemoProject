#pragma once

#include <Urho3D/Urho3DAll.h>

class MotionRig : public Object
{
    URHO3D_OBJECT(MotionRig, Object);

public:
    MotionRig(Context* context, const String& rigName);
    ~MotionRig();

    Node* processNode;
    Node* translateNode;
    Node* rotateNode;
    Node* alignNode;
    
    Skeleton skeleton;
    Vector3 pelvisRightAxis;
    Quaternion rotateBoneInitQ;
    Vector3 pelvisOrign;
    String rig;
    float left_foot_to_ground_height;
    float right_foot_to_ground_height;
};



