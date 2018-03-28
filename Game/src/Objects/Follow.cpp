#include <Urho3D/Urho3DAll.h>

#include "Objects/Follow.h"


Follow::Follow(Context* context):
    LogicComponent(context)
{
    Log::Write(-1," Follow constructor\n");
    toFollow = M_MAX_UNSIGNED;
    speed = 0.5f;
    offset = Vector3(0, 5, 0);
}

Follow::~Follow()
{
    Log::Write(-1," Follow destructor\n");
}

void Follow::Update(float dt)
{
    Node* n = GetScene()->GetNode(toFollow);
    if (!n)
        return;

    Vector3 myPos = GetNode()->GetWorldPosition();
    Vector3 targetPos = n->GetWorldPosition() + offset;
    GetNode()->SetWorldPosition(myPos + (targetPos - myPos) * dt * speed);
}








