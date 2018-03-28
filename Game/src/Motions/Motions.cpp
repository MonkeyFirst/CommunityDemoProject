
#include <Urho3D/Urho3DAll.h>
#include "Motions/Motions.h"
#include "Motions/MotionManager.h"

#include "radio.h"

Motion::Motion(Context* context):
    Object(context)
{
    rotateAngle = 361; // 361
}

Motion::Motion(Context* context, const Motion& other):
    Object(context)
{
    animationName = other.animationName;
    animation = other.animation;
    motionKeys = other.motionKeys;
    endTime = other.endTime;
    looped = other.looped;
    startFromOrigin = other.startFromOrigin;
    endDistance = other.endDistance;
    endFrame = other.endFrame;
    motionFlag = other.motionFlag;
    allowMotion = other.allowMotion;
    
    rotateAngle = 361; // 361
}

void Motion::SetName(const String& _name)
{
    name = _name;
    nameHash = StringHash(name);
}

Motion::~Motion()
{
    animation = NULL;
    //cache.ReleaseResource("Animation", animationName);
    GetSubsystem<ResourceCache>()->ReleaseResource<Animation>(animationName);
}

// Вызывает MotionManager для каждого Motion в векторе в своей функции Update на этапе MOTION_LOADING_MOTIONS
void Motion::Process()
{
    if (processed)
        return;
    MotionManager* mm = RADIO::g_mm;
    unsigned int startTime = GetSubsystem<Time>()->GetSystemTime(); // time.systemTime;

    // Получаем имя анимации и указатель на саму анимацию
    this->animationName = RADIO::GetAnimationName(this->name);
    this->animation = GetSubsystem<ResourceCache>()->GetResource<Animation>(animationName); // ("Animation", animationName);

    if (!this->animation)
        return;

    //gMotionMgr.memoryUse += this.animation.memoryUse;
    RADIO::g_mm->memoryUse += this->animation->GetMemoryUse(); // .memoryUse;

    // Передаем имя анимации, а остальные параметры получат свои значения при создании Motion
    // Значение по умолчанию имеет только rotateAngle = 361
    // motionKeys и startFromOrigin будут заполнены в процессе выполнения функции
    rotateAngle = mm->ProcessAnimation(animationName, motionFlag, allowMotion, rotateAngle, motionKeys, startFromOrigin);
    SetEndFrame(endFrame); // endFrame значение получает при создании Motion

    //if (!dockAlignBoneName.empty)
    if (dockAlignBoneName != String::EMPTY)
    {
        Vector3 v = mm->GetBoneWorldPosition(mm->curRig_, animationName, dockAlignBoneName, dockAlignTime);
        if (RADIO::mm_d_log)
            URHO3D_LOGINFO(this->name + " bone " + dockAlignBoneName + " world-pos=" + v.ToString() + " at time:" + String(dockAlignTime));
        dockAlignOffset += v;
    }

    if (!motionKeys.Empty())
    {
        Vector4 v = motionKeys[0];
        Vector4 diff = motionKeys[endFrame - 1] - motionKeys[0];
        endDistance = Vector3(diff.x_, diff.y_, diff.z_).Length(); //.length;
    }

    maxHeight = -9999;
    for (unsigned int i = 0; i < motionKeys.Size(); ++i)
    {
        if (motionKeys[i].y_ > maxHeight)
            maxHeight = motionKeys[i].y_;
    }
    if (RADIO::mm_d_log)
    {
        Log::Write(-1,"\n Motion " + name +": motionKeys.Size = " + String(motionKeys.Size()) + "\n");

        for (unsigned int i = 0; i < motionKeys.Size(); ++i)
        {
            Log::Write(-1," motionKeys[" + String(i) + "] w = " + String(motionKeys[i].w_) + " xyz = " + String(motionKeys[i].x_) + "|" + String(motionKeys[i].y_) + "|" + String(motionKeys[i].z_) + "\n");
        }
    }
    
    processed = true;

    if (RADIO::mm_d_log)
        URHO3D_LOGINFO("Motion " + name + " endDistance="  + String(endDistance) + " startFromOrigin=" + startFromOrigin.ToString()  + " timeCost=" + String(GetSubsystem<Time>()->GetSystemTime() - startTime) + " ms");
}

void Motion::SetDockAlign(const String& boneName, float alignTime, const Vector3& offset)
{
    dockAlignBoneName = boneName;
    dockAlignOffset = offset;
    dockAlignTime = alignTime;
}

void Motion::SetEndFrame(int frame)
{
    endFrame = frame;
    if (endFrame < 0)
    {
        endFrame = motionKeys.Size() - 1; //.length - 1;
        endTime = this->animation->GetLength(); // length;
    }
    else
        endTime = float(endFrame) * SEC_PER_FRAME;
}

// t is Local Time of animation ctrl->GetTime(animationName)
void Motion::GetMotion(float t, float dt, bool loop, Vector4& out_motion)
{
    if (motionKeys.Empty()) // motionKeys.empty
    {
        Log::Write(-1," Motion: motionKeys EMPTY!!!\n");
        return;
    }
    // Шаг времени который будет сделан Локальное вермя анимации плюс текущее время кадра.
    float future_time = t + dt;

    // Для зацикленной анимации при условии что Шаг больше самой Анимации
    // Срабатывает в конце зацикленных анимаций (сбрасывая персонажа в начальную точку)
    if (future_time > animation->GetLength() && loop)
    {
//        Log::Write(-1," !!!!!!!!!!!!!!\n"); 
        Vector4 t1 = Vector4(0,0,0,0);
        Vector4 t2 = Vector4(0,0,0,0);

        // Эти 
        GetMotion(t, animation->GetLength() - t, false, t1); // отработает нижняя часть функции
//        Log::Write(-1," 1 END\n");
        GetMotion(0, t + dt - animation->GetLength(), false, t2); // отработает нижняя часть функции
//        Log::Write(-1," 2 END\n");

        out_motion = t1 + t2; // t1 + t2
//        Log::Write(-1," END out_motion = " + String(out_motion) + "\n");
    }
    else
    {
        // каждый кадр в нутри анимации.
        Vector4 k1 = GetKey(t);
        Vector4 k2 = GetKey(future_time);
        out_motion = k2 - k1;
//        Log::Write(-1," out_motion = " + String(out_motion) + "\n");
    }
}

Vector4 Motion::GetKey(float t)
{
    if (motionKeys.Empty())
        return Vector4(0, 0, 0, 0);

    unsigned int i = int(t * FRAME_PER_SEC); // FRAME_PER_SEC = 25.0f (MotionManager.h)
    if (i >= motionKeys.Size())
        i = motionKeys.Size() -1; // - 1
    Vector4 k1 = motionKeys[i];

    unsigned int next_i = i + 1;
    if (next_i >= motionKeys.Size()) 
        next_i = motionKeys.Size() -1;// - 1

//Log::Write(-1," motionKeys = " + String(motionKeys.Size()) +" i = " + String(i) + " next_i = " + String(next_i) +"\n");

    if (i == next_i) // странно, до конца первого цикла анимации не вызывается.
        return k1;

// Unity
// Mathf.Lerp(float a, float b, float t)		//Интерполяция
// Данная функция получает промежуточное значение (интерполирует) между a и b на основе t.
// При t равное 0 (или меньше) вернёт a. При t равное 1 (или больше) вернёт b. При t равное 0.5 вернёт среднее значение между a и b.
// Функция считает по формуле: a + (b-a) * t, где 0 <= t >= 1.

    Vector4 k2 = motionKeys[next_i];
//Log::Write(-1," k2 = " + String(k2) +"\n");

    // Проверка позволяет избежать возвращение персонажа в стартовую позицию по окончании анимации.
    if(k1 == Vector4::ZERO) //  || k2 == Vector4::ZERO
    {
      return k2;
    }
    else
    {
        // в теории от 0 до 1 будет степенью интерполяции. В теории может иметь фиксированное значение от 0 до 1 (без t и FRAME_PER_SEC)
        float a = t * FRAME_PER_SEC - float(i); 

        /*
        Log::Write(-1," k1 = " + String(k1) + "\n");
        Log::Write(-1," k2 = " + String(k2) + "\n");
        Log::Write(-1," a = " + String(a) + "\n");
        Log::Write(-1," k1 lerp = " + String(k1.Lerp(k2, a)) + "\n");
        Log::Write(-1," ----------------\n");
        */
        return k1.Lerp(k2, a);
    }
    //return k2; // без функции Lerp и проверки, не сбрасывает персонажа, но "дрожит" перемещение (не сглаженное)
}

Vector3 Motion::GetFuturePosition(Character* object, float t)
{
    Vector4 motionOut = GetKey(t);
    Node* _node = object->GetNode();
    if (looped)
        return _node->GetWorldRotation() * Vector3(motionOut.x_, motionOut.y_, motionOut.z_) + _node->GetWorldPosition() + object->motion_deltaPosition;
    else
        return Quaternion(0, object->motion_startRotation + object->motion_deltaRotation, 0) * Vector3(motionOut.x_, motionOut.y_, motionOut.z_) + object->motion_startPosition + object->motion_deltaPosition;
}

float Motion::GetFutureRotation(Character* object, float t)
{
    if (looped)
        return RADIO::AngleDiff(object->GetNode()->GetWorldRotation().EulerAngles().y_ + object->motion_deltaRotation + GetKey(t).w_);
    else
        return RADIO::AngleDiff(object->motion_startRotation +  object->motion_deltaRotation + GetKey(t).w_); //TODO:  + +  in script
        //return RADIO::AngleDiff(object->motion_startRotation + object->motion_deltaRotation + GetKey(t).w_);
        //return RADIO::AngleDiff(object->GetNode()->GetWorldRotation().EulerAngles().y_ + GetKey(t).w_);
}

void Motion::Start(Character* object, float localTime, float blendTime, float speed)
{
    object->PlayAnimation(animationName, LAYER_MOVE, looped, blendTime, localTime, speed);
    InnerStart(object);
}

float Motion::GetDockAlignTime()
{
// Если результатом первого операнда является значение true (1), оценивается второй операнд.
// Если результатом первого операнда является значение false (0), оценивается третий операнд.

    //return dockAlignBoneName.empty ? endTime : dockAlignTime;
    return (dockAlignBoneName == String::EMPTY) ? endTime : dockAlignTime;
}

Vector3 Motion::GetDockAlignPositionAtTime(Character* object, float targetRotation, float t)
{
    Node* _node = object->GetNode();
    Vector4 motionOut = GetKey(t);
    Vector3 motionPos = Quaternion(0, targetRotation, 0) * Vector3(motionOut.x_, motionOut.y_, motionOut.z_) + object->motion_startPosition + object->motion_deltaPosition;
    Vector3 offsetPos = Quaternion(0, targetRotation + motionOut.w_, 0) * dockAlignOffset;
    return motionPos + offsetPos;
}

void Motion::InnerStart(Character* object)
{
    object->motion_startPosition = object->GetNode()->GetWorldPosition();
    object->motion_startRotation = object->GetNode()->GetWorldRotation().EulerAngles().y_;
    object->motion_deltaRotation = 0;
    object->motion_deltaPosition = Vector3(0, 0, 0);
    object->motion_velocity = Vector3(0, 0, 0);
    object->motion_translateEnabled = true;
    object->motion_rotateEnabled = true;
    Log::Write(-1, " Motion: InnerStart - " + animationName + "\n");
//    Log::Write(-1, " motion_startPosition = " + object->motion_startPosition.ToString() + "\n");
//    Log::Write(-1, " motion_startRotation = " + String(object->motion_startRotation) + "\n");
}

int Motion::Move(Character* object, float dt)
{
    AnimationController* ctrl = object->animCtrl;
    Node* _node = object->GetNode(); 

    float localTime = ctrl->GetTime(animationName);
    float speed = ctrl->GetSpeed(animationName); // return 0.0f or current AnimationControl speed
    float absSpeed = Abs(speed);

//Log::Write(-1," MOTION::MOVE OBJECT_NODE_NAME =  " + _node->GetName() + "\n");
//Log::Write(-1," SPEED = " + String(speed) + "\n"); // всегда дает 1 кроме анимаций на месте (присесть, встать) которые ничего не дают.

    if (absSpeed < 0.001) // defaul AnimationControl speed is 1.0f (CrouchWalk is 0.5f Walk is 1.0f)
        return 0; // Motion not finished. Not call OnMotionFinished() in CharacterStates.cpp SingleMotionState

    dt *= absSpeed;

    if (looped || speed < 0)// CrouchWalk is 0.5f speed
    {   
        Vector4 motionOut = Vector4(0, 0, 0, 0);
        GetMotion(localTime, dt, looped, motionOut);

        if (!looped)
        {
            if (localTime < SEC_PER_FRAME)
                motionOut = Vector4(0, 0, 0, 0);
        }
// Log::Write(-1," motionOut.w_ = " + String(motionOut.w_) + "\n"); // Дает 0. Будет взято из motionKeys[] вектора (сохраненное из анимации значение)
        if (object->motion_rotateEnabled)
            _node->Yaw(motionOut.w_);

        if (object->motion_translateEnabled)
        {
            Vector3 tLocal(motionOut.x_, motionOut.y_, motionOut.z_);
            // Vector3 tLocal = tLocal * ctrl->GetWeight(animationName); // original comment

            if (object->physicsType == 0) // object->physicsType == 0     _node->GetName()!="player"
            {
                //Log::Write(-1," MOVE OBJECT_PHYSICS_TYPE = 0 " + _node->GetName() + "\n");
                Vector3 tWorld = _node->GetWorldRotation() * tLocal + _node->GetWorldPosition() + object->motion_velocity * dt;
                object->MoveTo(tWorld, dt);
            }
            else
            {
                //Log::Write(-1," MOVE OBJECT_PHYSICS_TYPE = 1 " + _node->GetName() + "\n");
                Vector3 tWorld = _node->GetWorldRotation() * tLocal;
                object->SetVelocity(tWorld / dt + object->motion_velocity);
            }

        }
        else
        {
            //Log::Write(-1," motion_translateEnabled = FALSE for " + _node->GetName() + "\n");
            object->SetVelocity(Vector3(0, 0, 0));
        }

        if (speed < 0 && localTime < 0.001)
            return 1; // Motion finished

        return 0;
     }
    else // Не looped или speed больше или равно 0
    {
        //Log::Write(-1," NO_LOOP OR SPEED >= 0 " + _node->GetName() + "\n");
        // motionOut берем из motionKeys[] вектора. (сохраненное из анимации значение)
        Vector4 motionOut = GetKey(localTime);
//Log::Write(-1," motionOut = " + String(motionOut) + "\n");
        // motion_startRotation = Вращение Узла компонента Character в градусах по Y оси (в Мировых координатах)
        // Получает значение в Motion: InnerStart
        // Вращаем только по Y
        if (object->motion_rotateEnabled)
            _node->SetWorldRotation(Quaternion(0, object->motion_startRotation + motionOut.w_ + object->motion_deltaRotation, 0));

        if (object->motion_translateEnabled)
        {
            if (object->physicsType == 0) // && _node->GetName() == "player")
            {
                object->motion_deltaPosition += object->motion_velocity * dt;
                Vector3 tWorld = Quaternion(0, object->motion_startRotation + object->motion_deltaRotation, 0) * Vector3(motionOut.x_, motionOut.y_, motionOut.z_) + object->motion_startPosition + object->motion_deltaPosition;
                object->MoveTo(tWorld, dt);
            }
            else
            {
                Vector3 tWorld1 = Quaternion(0, object->motion_startRotation + object->motion_deltaRotation, 0) * Vector3(motionOut.x_, motionOut.y_, motionOut.z_);
                motionOut = GetKey(localTime + dt);
                Vector3 tWorld2 = Quaternion(0, object->motion_startRotation + object->motion_deltaRotation, 0) * Vector3(motionOut.x_, motionOut.y_, motionOut.z_);
                Vector3 vel = (tWorld2 - tWorld1) / dt;
                object->SetVelocity(vel + object->motion_velocity);
            }
        }
        else
        {
            //Log::Write(-1," motion_translateEnabled = FALSE for " + _node->GetName() + "\n");
            object->SetVelocity(Vector3(0, 0, 0));
        }

        if (!(dockAlignBoneName == String::EMPTY))
        {
            if (localTime < dockAlignTime && (localTime + dt) > dockAlignTime)
                return 2;
        }

        bool bFinished = (speed > 0) ? localTime >= endTime : (localTime < 0.001);
        //if (bFinished)
        //    object.SetVelocity(Vector3(0, 0, 0));
        return bFinished ? 1 : 0;
    }
}

void Motion::DebugDraw(DebugRenderer* debug, Character* object)
{
//Log::Write(-1," Motion: DebugDraw\n");
    Node* _node = object->GetNode();
// TODO: This is Original comments
/*    
    if (looped) {
        Vector4 tFinnal = GetKey(endTime);
        Vector3 tLocal(tFinnal.x_, tFinnal.y_, tFinnal.z_);
        debug->AddLine(_node->GetWorldRotation() * tLocal + _node->GetWorldPosition(), _node->GetWorldPosition(), Color(0.5f, 0.5f, 0.7f), false);
    }
    else {
        Vector4 tFinnal = GetKey(endTime);
        Vector3 tMotionEnd = Quaternion(0, object->motion_startRotation + object->motion_deltaRotation, 0) * Vector3(tFinnal.x_, tFinnal.y_, tFinnal.z_);
        debug->AddLine(tMotionEnd + object->motion_startPosition,  object->motion_startPosition, Color(0.5f, 0.5f, 0.7f), false);
        RADIO::g_mm->DebugDrawDirection(debug, _node, object->motion_startRotation + object->motion_deltaRotation + tFinnal.w_, Color::RED, 2.0);
    }
*/    

    if (!(dockAlignBoneName == String::EMPTY))
    {
        //Vector3 v = _node.LocalToWorld(dockAlignOffset);
        //debug.AddLine(_node.worldPosition, v, BLUE, false);
        //debug.AddCross(v, 0.5f, GREEN, false);
        debug->AddCross(_node->GetChild(dockAlignBoneName, true)->GetWorldPosition(), 0.25f, Color::GREEN, false);
        //debug.AddNode(_node.GetChild(dockAlignBoneName, true), 0.25f, false);
    }
}

Vector3 Motion::GetStartPos()
{
    return Vector3(startFromOrigin.x_, startFromOrigin.y_, startFromOrigin.z_);
}

float Motion::GetStartRot()
{
    return -rotateAngle;
}

// ==================================
// AttackMotion
// ==================================

AttackMotion::AttackMotion(Context* context, const String& name, int impactFrame, int _type, const String& bName):
    Object(context)
{
    MotionManager* mm = RADIO::g_mm;
    motion = mm->FindMotion(name);
    if (!motion)
        return;
    impactTime = impactFrame * SEC_PER_FRAME;
    Vector4 k = motion->motionKeys[impactFrame];
    impactPosition = Vector3(k.x_, k.y_, k.z_);
    impactDist = impactPosition.Length();
    type = _type;
    boneName = bName;
}

AttackMotion::~AttackMotion()
{}

int AttackMotion::opCmp(const AttackMotion& obj)
{
    if (impactDist > obj.impactDist)
        return 1;
    else if (impactDist < obj.impactDist)
        return -1;
    else
        return 0;
}




















