#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Character/Player.h"
#include "Character/Character.h"

class GameState : public Object
{
    URHO3D_OBJECT(GameState, Object);

public:
    GameState(Context* context);
    virtual ~GameState();

    virtual void Enter(GameState* lastState);
    virtual void Exit(GameState* nextState);
    virtual void Update(float dt);
    virtual void FixedUpdate(float dt);
    virtual void DebugDraw(DebugRenderer* debug);
    virtual String GetDebugText();
    virtual void SetName(const String& s);
    virtual bool CanReEntered();

    virtual Player* GetPlayer();
    virtual Scene* GetScene();
    
    String name;
    StringHash nameHash;
    float timeInState;
    
    virtual void OnCharacterKilled(Character* killer, Character* dead);
    virtual void OnSceneLoadFinished(Scene* _scene);
    virtual void OnAsyncLoadProgress(Scene* _scene, float progress, int loadedNodes, int totalNodes, int loadedResources, int totalResources);
    virtual void OnKeyDown(int key);
    virtual void OnPlayerStatusUpdate(Player* player);
    virtual void OnESC();
    virtual void OnSceneTimeScaleUpdated(Scene* scene, float newScale);


};