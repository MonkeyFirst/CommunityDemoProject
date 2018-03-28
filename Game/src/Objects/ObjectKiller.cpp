#include <Urho3D/Urho3DAll.h>

#include "Objects/ObjectKiller.h"

#include "radio.h"


ObjectKiller::ObjectKiller(Context* context):
    GameObject(context)
{   
    Log::Write(-1," ObjectKiller: Constructor\n");
}

ObjectKiller::~ObjectKiller()
{
    Log::Write(-1," ObjectKiller: Destructor\n");
}




