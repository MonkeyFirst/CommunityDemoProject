
#include <Urho3D/Urho3DAll.h>

#include "Character/EM_Helper.h"
#include "Character/Enemy.h"
#include "Character/Player.h"

#include "Motions/MotionManager.h"

#include "radio.h"

// EM_Helper  ===============================================

EM_Helper::EM_Helper(Context* context):
    LogicComponent(context)
{

}

EM_Helper::~EM_Helper()
{
    em_ = NULL;
}

// Called when the component is added to a scene node. Other components may not yet exist.
void EM_Helper::Start()
{
    em_ = RADIO::g_em;
    em_->Start();
}
// Called before the first update. At this point all other components of the node should exist. 
// Will also be called if update events are not wanted; in that case the event is immediately unsubscribed afterward.
void EM_Helper::DelayedStart()
{

}
// Called when the component is detached from a scene node, usually on destruction. 
// Note that you will no longer have access to the node and scene at that point.
void EM_Helper::Stop()
{
    em_->Stop();
}
// Called on scene update, variable timestep.
void EM_Helper::Update(float timeStep)
{
//    Log::Write(-1," EM_Helper::Update\n");
    em_->Update(timeStep);
}
// Called on scene post-update, variable timestep.
void EM_Helper::PostUpdate(float timeStep)
{

}
// Called on physics update, fixed timestep.
void EM_Helper::FixedUpdate(float timeStep)
{

}
// Called on physics post-update, fixed timestep.
void EM_Helper::FixedPostUpdate(float timeStep)
{

}















