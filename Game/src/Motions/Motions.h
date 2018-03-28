#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Character/Character.h"
/*
const unsigned int LAYER_MOVE = 0;
const unsigned int LAYER_ATTACK = 1;

enum AttackType
{
    ATTACK_PUNCH,
    ATTACK_KICK,
};
*/
class Character;

// MotionManager хранит вектор с motions
// Похоже что каждый Motion сопоставляется только с одной анимацией.
class Motion : public Object
{
    URHO3D_OBJECT(Motion, Object);
public:

    Motion(Context* context);
    Motion(Context* context, const Motion& other);
    ~Motion();

    void SetName(const String& _name);
    void Process(); // Вызывает MotionManager в своей функции Update на этапе MOTION_LOADING_MOTIONS

    void SetDockAlign(const String& boneName, float alignTime, const Vector3& offset);
    void SetEndFrame(int frame);

    void GetMotion(float t, float dt, bool loop, Vector4& out_motion);
    Vector4 GetKey(float t);
    Vector3 GetFuturePosition(Character* object, float t);
    float GetFutureRotation(Character* object, float t);

    void Start(Character* object, float localTime = 0.0f, float blendTime = 0.1, float speed = 1.0f);

    float GetDockAlignTime();
    Vector3 GetDockAlignPositionAtTime(Character* object, float targetRotation, float t);

    void InnerStart(Character* object);

    int Move(Character* object, float dt); // Отвечает за перемещение персонажа в реализации 2 варианта перемещения
    void DebugDraw(DebugRenderer* debug, Character* object);

    Vector3 GetStartPos();
    float GetStartRot();

    // Заполняет MotionManager в функции CreateMotion и запоминает motion в вектор
    String name;
    int motionFlag;
    int allowMotion;
    bool looped;
    int endFrame;
    float rotateAngle;
            
    String animationName;
    StringHash nameHash;
    Animation* animation; // Всего одна анимация.
    
    Vector<Vector4> motionKeys; // Array<Vector4> motionKeys;
    
    float endTime;
    Vector4 startFromOrigin;
    float endDistance;
    float maxHeight;
    bool processed = false;
    float dockAlignTime;
    Vector3 dockAlignOffset;
    String dockAlignBoneName;
};

// ==================================
// AttackMotion
// ==================================

class AttackMotion : public Object
{
    URHO3D_OBJECT(AttackMotion, Object);

public:
    AttackMotion(Context* context, const String& name, int impactFrame, int _type, const String& bName);
    ~AttackMotion();

    int opCmp(const AttackMotion& obj);

    Motion* motion;

    // ==============================================
    //   ATTACK VALUES
    // ==============================================

    float impactTime;
    float impactDist;;
    Vector3 impactPosition;
    int type;
    String boneName;

};






