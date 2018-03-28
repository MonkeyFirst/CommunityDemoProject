
#include <Urho3D/Urho3DAll.h>

#include "Character/Player.h"
#include "Character/EnemyManager.h"
#include "Character/Bruce.h"
#include "Character/Ragdoll.h"
#include "Character/Thug.h"

#include "Objects/HeadIndicator.h"

#include "radio.h"

#include "States/GameState.h"

GameState::GameState(Context* context):
    Object(context) 
{}

GameState::~GameState()
{}

void GameState::Enter(GameState* lastState)
{
    timeInState = 0;
}

void GameState::Exit(GameState* nextState)
{
    timeInState = 0;
}

void GameState::Update(float dt)
{
    timeInState += dt;
}

void GameState::FixedUpdate(float dt)
{

}

void GameState::DebugDraw(DebugRenderer* debug)
{

}

String GameState::GetDebugText()
{
    return " name= " + name + " timeInState= " + String(timeInState) + "\n";
}

void GameState::SetName(const String& s)
{
    name = s;
    nameHash = StringHash(s);
}

bool GameState::CanReEntered()
{
    return false;
}
// -----------------------------------------------

Player* GameState::GetPlayer()
{
    return NULL;
}

Scene* GameState::GetScene()
{
    return NULL;
}

void GameState::OnCharacterKilled(Character* killer, Character* dead)
{
    
}

void GameState::OnSceneLoadFinished(Scene* _scene)
{
    
}

void GameState::OnAsyncLoadProgress(Scene* _scene, float progress, int loadedNodes, int totalNodes, int loadedResources, int totalResources)
{
    
}

void GameState::OnKeyDown(int key)
{
//    if (key == KEY_ESCAPE)
//    {
//         if (!console.visible)
//            OnESC();
//        else
//            console.visible = false;
//    }
}

void GameState::OnPlayerStatusUpdate(Player* player)
{
    
}

void GameState::OnESC()
{
//    engine.Exit();
}

void GameState::OnSceneTimeScaleUpdated(Scene* scene, float newScale)
{
    
}






