#include <Urho3D/Urho3DAll.h>

#include "Objects/GameObject.h"

// GLOBAL FUNCTION
void SetWorldTimeScale(Scene* _scene, float scale)
{
    Log::Write(-1," SetWorldTimeScale: " + String(scale) + "\n");
    
    /// Return child scene nodes with a specific component.
//    void GetChildrenWithComponent(PODVector<Node*>& dest, StringHash type, bool recursive = false)
    /// Return child scene nodes with a specific component.
//    PODVector<Node*> GetChildrenWithComponent(StringHash type, bool recursive = false)
    /// Return child scene nodes with a specific tag.
//    void GetChildrenWithTag(PODVector<Node*>& dest, const String& tag, bool recursive = false)
    /// Return child scene nodes with a specific tag.
//    PODVector<Node*> GetChildrenWithTag(const String& tag, bool recursive = false)

//void GetChildrenWithComponent(PODVector<Node*>& dest, bool recursive = false)
    
    //Vector<Node*> nodes = _scene->GetChildrenWithScript(false);
    PODVector<Node*> nodes;
    _scene->GetChildrenWithComponent<GameObject>(nodes, false);
    
    for (unsigned int i = 0; i < nodes.Size(); ++i) // length; ++i)
    {
        //GameObject* object = cast<GameObject>(nodes[i].scriptObject);
        GameObject* object = nodes[i]->GetComponent<GameObject>();
        if (!object)
            continue;
        object->SetTimeScale(scale);
    }
}

GameObject::GameObject(Context* context):
    LogicComponent(context)
{
    duration = -1;
    flags = 0;
    side = 0;
    timeScale = 1.0f;
    
    finalized = false;

//    Log::Write(-1," GameObject: Constructor\n");
}

GameObject::~GameObject()
{
//    Log::Write(-1," GameObject: Destructor\n");
}

void GameObject::SetTimeScale(float scale)
{
    timeScale = scale;
    Log::Write(-1," " + GetName() + " GameObject: SetTimeScale: " + String(scale) + "\n");
}

void GameObject::FixedUpdate(float timeStep)
{
    timeStep *= timeScale;
    CheckDuration(timeStep);
}

void GameObject::CheckDuration(float timeStep)
{
    // Disappear when duration expired
    if (duration >= 0)
    {
        duration -= timeStep;
        if (duration <= 0) // <= 0
        {
            //Log::Write(-1, " KILL GAME OBJECT\n");
            Kill(); // GetNode()->Remove(); Remove() In Component class. Only Remove() funtion call in script
        }
    }
}

// Remove from the scene node. If no other shared pointer references exist, causes immediate deletion.
void GameObject::Kill()
{
    Log::Write(-1, " GameObject: Kill Node " + GetNode()->GetName() + "\n");
//    GetNode()->Remove();
//    GetNode()->RemoveComponent(this);
    Node* n = GetNode();
    if(n)
    {
        n->RemoveAllComponents();
        Log::Write(-1, " GameObject: RemoveAllComponents end\n");
        n->RemoveAllChildren();
        Log::Write(-1, " GameObject: RemoveAllChildren end\n");
        n->Remove(); // remove from parent node.
    }
}

void GameObject::Update(float timeStep)
{
    timeStep *= timeScale;
}

void GameObject::PlaySound(const String& soundName)
{
    Log::Write(-1," " + GetName() + " GameObject: PlaySound " + soundName + "\n");

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Node* nd = GetNode();
    if(nd)
    {
        // Create the sound channel
        SoundSource3D* source = GetNode()->CreateComponent<SoundSource3D>(); //("SoundSource3D");
        //SoundSource* source = GetNode().CreateComponent("SoundSource");
        Sound* sound = cache->GetResource<Sound>(soundName); //("Sound", soundName);
        source->SetDistanceAttenuation(5, 50, 2);
        source->Play(sound);
        source->SetSoundType(SOUND_EFFECT); // soundType = SOUND_EFFECT;
        //frequency = source->frequency * GetNode().scene.timeScale; // * timeScale;
        source->SetFrequency(source->GetFrequency() * GetNode()->GetScene()->GetTimeScale()); 
        // Subscribe to sound finished for cleaning up the source
        //SubscribeToEvent(node, "SoundFinished", "HandleSoundFinished");
        SubscribeToEvent(GetNode(), E_SOUNDFINISHED, URHO3D_HANDLER(GameObject, HandleSoundFinished));
        return;
    }
    Log::Write(-1," GameObject: PlaySound Node not exist\n");
}

void GameObject::HandleSoundFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace SoundFinished;
    SoundSource3D* source = static_cast<SoundSource3D*>(eventData[P_SOUNDSOURCE].GetPtr());
    //SoundSource3D* source = eventData[P_SOUNDSOURCE].GetPtr();
    //WeakPtr<SoundSource> source = eventData[P_SOUNDSOURCE].GetPtr();
    if(source)
        source->Remove();
    Log::Write(-1," GameObject: HandleSoundFinished end\n");
}

void GameObject::DebugDraw(DebugRenderer* debug)
{

}

String GameObject::GetDebugText()
{
    return "";
}

String GameObject::GetName()
{
    return "";
}

void GameObject::AddFlag(int flag)
{
    flags |= flag;
}

void GameObject::RemoveFlag(int flag)
{
    flags &= ~flag;
}

bool GameObject::HasFlag(int flag)
{
    return (flags & flag);// != 0;
}

void GameObject::ResetObject()
{

}

bool GameObject::OnDamage(GameObject* attacker, const Vector3& position, const Vector3& direction, int damage, bool weak)
{
    return true;
}
/*
Node* GameObject::GetNode()
{
    Component::GetNode();
    //return NULL;
}
*/
/*
Scene* GameObject::GetScene()
{
    Node* _node = GetNode();
    if (!_node)
        return NULL;
    return _node->GetScene(); //.scene;
}
*/
void GameObject::SetSceneTimeScale(float scale)
{
    Scene* _scene = GetScene();
    if (!_scene)
        return;
    if (_scene->GetTimeScale() == scale)
        return;
    _scene->SetTimeScale(scale); //.timeScale = scale;
// Ничего не реализует. В gGame классе закоментировано
//    gGame.OnSceneTimeScaleUpdated(_scene, scale);
    Log::Write(-1," " + GetName() + " SetSceneTimeScale:" + String(scale));
}

void GameObject::Transform(const Vector3& pos, const Quaternion& qua)
{
    Node* _node = GetNode();
    _node->SetWorldPosition(pos); //.worldPosition = pos;
    _node->SetWorldRotation(qua); //.worldRotation = qua;
}
/*
void GameObject::RemoveMy()
{
    Log::Write(-1," GameObject: RemoveMy\n");
    
    if(!GetNode())
        Log::Write(-1," GameObject: ERROR GET NODE\n");
    
    Log::Write(-1, GetNode()->GetName() + " RemoveComponent and Node\n");// (node.name + ".Remove()");

//    Remove(); // (if node_) node_->RemoveComponent(this) in Component class Удалили компонент.
    GetNode()->Remove(); // Удалили Узел.
}
*/
bool GameObject::IsVisible()
{
    return true;
}


