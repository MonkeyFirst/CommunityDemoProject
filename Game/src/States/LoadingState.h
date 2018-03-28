#pragma once

#include <Urho3D/Urho3DAll.h>

#include "States/GameState.h"

enum LoadState
{
    LOADING_RESOURCES,
    LOADING_MOTIONS,
    LOADING_FINISHED,
    GAME_ENTER,
};

class LoadingState : public GameState
{
    URHO3D_OBJECT(LoadingState, GameState);
    
public:
    LoadingState(Context* context);
    ~LoadingState();

    void CreateLoadingUI();
    void Enter(GameState* lastState);
    void Exit(GameState* nextState);
    void Update(float dt);
    void ChangeSubState(int newState);

    void OnSceneLoadFinished(Scene* _scene);
    void OnAsyncLoadProgress(Scene* _scene, float progress, int loadedNodes, int totalNodes, int loadedResources, int totalResources);
    void OnESC();

    Player* GetPlayer();
    Scene* GetScene();
    
    unsigned int load_state;
    int numLoadedResources;
    
    SharedPtr<Scene> preloadScene;
    SharedPtr<Text> txt_;
};