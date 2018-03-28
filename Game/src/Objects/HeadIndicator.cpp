#include <Urho3D/Urho3DAll.h>

#include "Objects/HeadIndicator.h"

#include "Motions/MotionManager.h" // for const String HEAD = "Bip01_Head";
//#include "Levels/GameManager.h" // for GetCamera function
#include "Camera/CameraManager.h"
#include "radio.h"

HeadIndicator::HeadIndicator(Context* context):
    LogicComponent(context)
{
    Log::Write(-1," HeadIndicator: Constructor\n");
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    offset = Vector3(0, 1.5f, 0);
    state = -1;
    textures.Push(NULL); // для нулевого состояния (состояния hiden)
    textures.Push(cache->GetResource<Texture2D>("Textures/counter.tga"));
}

HeadIndicator::~HeadIndicator()
{
    Log::Write(-1," HeadIndicator: Destructor\n");
}

void HeadIndicator::Start()
{
    UI* ui = GetSubsystem<UI>();

    sprite = ui->GetRoot()->CreateChild<Sprite>("Indicator_" + GetNode()->GetName());
    sprite->SetBlendMode(BLEND_ADD); // .blendMode = BLEND_ADD;
    ChangeState(0);
}

void HeadIndicator::DelayedStart()
{
    headNodeId = GetNode()->GetChild(HEAD, true)->GetID();
}

void HeadIndicator::Stop()
{
    Log::Write(-1," HeadIndicator: Stop\n");
    textures.Clear();
    if(sprite)
        sprite->Remove();
}

void HeadIndicator::Update(float dt)
{
//    Log::Write(-1," HeadIndicator: Update\n");

    Graphics* graphics = GetSubsystem<Graphics>();

    if (GetSubsystem<Engine>()->IsHeadless())
        return;
    Node* headNode = GetNode()->GetScene()->GetNode(headNodeId);
    if (!headNode)
        return;
    Vector3 pos = headNode->GetWorldPosition() + offset;
    Vector2 pos_2d = RADIO::g_cm->GetCamera()->WorldToScreenPoint(pos);
    // sprite.position = Vector2(pos_2d.x_ * graphics->GetWidth(), pos_2d.y_ * graphics->GetHeight());
    sprite->SetPosition(Vector2(pos_2d.x_ * graphics->GetWidth(), pos_2d.y_ * graphics->GetHeight()));
}

void HeadIndicator::ChangeState(int newState)
{
    Log::Write(-1," HeadIndicator: ChangeState to " + String(newState) + "\n");

    if (state == newState)
        return;

    state = newState;

    if(newState != STATE_INDICATOR_HIDE)
    {
        if(!sprite)
        {
//            Log::Write(-1," HeadIndicator: NO SPRITE EXIST\n");
            return;
        }
        
        sprite->SetVisible(true); //visible = (newState != STATE_INDICATOR_HIDE);     
        sprite->SetTexture(textures[newState]); // texture = textures[newState];
        sprite->SetSize(IntVector2(64, 64)); // size = IntVector2(64, 64);
        sprite->SetHotSpot(IntVector2(sprite->GetSize().x_/2, sprite->GetSize().y_/2)); // hotSpot = IntVector2(sprite.size.x/2, sprite.size.y/2);
    }
    else
        sprite->SetVisible(false);
}
















