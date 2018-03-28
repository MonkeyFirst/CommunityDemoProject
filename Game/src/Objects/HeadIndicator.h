#pragma once

#include <Urho3D/Urho3DAll.h>
//#include "Motions.h"

enum StateIndicator
{
    STATE_INDICATOR_HIDE,
    STATE_INDICATOR_ATTACK,
};

class HeadIndicator : public LogicComponent
{
    URHO3D_OBJECT(HeadIndicator, LogicComponent);

public:
    HeadIndicator(Context* context);
    ~HeadIndicator();
    
    // Called when the component is added to a scene node. Other components may not yet exist.
    virtual void Start();
    // Called before the first update. At this point all other components of the node should exist. 
    // Will also be called if update events are not wanted; in that case the event is immediately unsubscribed afterward.
    virtual void DelayedStart();
    // Called when the component is detached from a scene node, usually on destruction. 
    // Note that you will no longer have access to the node and scene at that point.
    virtual void Stop();
    // Called on scene update, variable timestep.
    virtual void Update(float dt);

    void ChangeState(int newState);

    Vector3 offset;
    unsigned int headNodeId;
    int state;
    Vector<Texture2D*> textures;
    WeakPtr<Sprite> sprite;
    //Sprite* sprite;
};