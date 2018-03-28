#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Character/EnemyManager.h"

class EM_Helper : public LogicComponent
{
    URHO3D_OBJECT(EM_Helper, LogicComponent);

public:
    EM_Helper(Context* context);
    ~EM_Helper();

    // Called when the component is added to a scene node. Other components may not yet exist.
    void Start();
    // Called before the first update. At this point all other components of the node should exist. 
    // Will also be called if update events are not wanted; in that case the event is immediately unsubscribed afterward.
    void DelayedStart();
    // Called when the component is detached from a scene node, usually on destruction. 
    // Note that you will no longer have access to the node and scene at that point.
    void Stop();
    // Called on scene update, variable timestep.
    void Update(float timeStep);
    // Called on scene post-update, variable timestep.
    void PostUpdate(float timeStep);
    // Called on physics update, fixed timestep.
    void FixedUpdate(float timeStep);
    // Called on physics post-update, fixed timestep.
    void FixedPostUpdate(float timeStep);

    WeakPtr<EnemyManager> em_;
};