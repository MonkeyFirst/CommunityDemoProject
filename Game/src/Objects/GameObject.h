#pragma once

#include <Urho3D/Urho3DAll.h>

const int FLAGS_ATTACK  = (1 << 0);
const int FLAGS_COUNTER = (1 << 1);
const int FLAGS_REDIRECTED = (1 << 2);
const int FLAGS_NO_MOVE = (1 << 3);
const int FLAGS_MOVING = (1 << 4);
const int FLAGS_INVINCIBLE = (1 << 5);
const int FLAGS_STUN = (1 << 6);
const int FLAGS_RUN  = (1 << 7);

const int COLLISION_LAYER_LANDSCAPE = (1 << 0);
const int COLLISION_LAYER_CHARACTER = (1 << 1);
const int COLLISION_LAYER_PROP      = (1 << 2);
const int COLLISION_LAYER_RAGDOLL   = (1 << 3);
const int COLLISION_LAYER_AI        = (1 << 4);

class GameObject : public LogicComponent
{
    URHO3D_OBJECT(GameObject, LogicComponent);

public:
    GameObject(Context* context);
    virtual ~GameObject();

// Наверное, виртуальными нужно объявлять только что хотим переопределить здесь или будут переопределятся в дочерних классах

    virtual void Update(float timeStep); // LogicComponent (virtual)
    virtual void FixedUpdate(float timeStep); // LogicComponent (virtual)
    virtual void Kill();

//    virtual Node* GetNode(); // Exist in Component class
//    virtual Scene* GetScene(); // Exist in Component class
//    virtual void RemoveMy(); // Remove Exist in Component class

// GameObject functions
// Витруальные для возможности дочерним классам переопределить.
    virtual void SetTimeScale(float scale);
    virtual void DebugDraw(DebugRenderer* debug);
    virtual String GetDebugText();
    virtual String GetName();
    virtual void ResetObject();
    virtual bool OnDamage(GameObject* attacker, const Vector3& position, const Vector3& direction, int damage, bool weak = false);
    virtual bool IsVisible();

    // Функции самого класса. Дочерние могут их юзать в любое время, но не могут их переопределять.
    void SetSceneTimeScale(float scale);
    void CheckDuration(float timeStep);
    void PlaySound(const String& soundName);
    void HandleSoundFinished(StringHash eventType, VariantMap& eventData);
    void Transform(const Vector3& pos, const Quaternion& qua);

    void AddFlag(int flag);
    void RemoveFlag(int flag);
    bool HasFlag(int flag);

    float duration = -1;
    int flags = 0;
    int side = 0;
    float timeScale = 1.0f;

    bool finalized;
};