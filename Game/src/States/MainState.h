#pragma once

#include <Urho3D/Urho3DAll.h>

#include "States/GameState.h"

#include "Character/Character.h"
#include "Character/Player.h"

#include "Objects/TextMenu.h"
/*
const String CAMERA_NAME = "Camera";

enum GameSubState
{
    GAME_FADING,
    GAME_RUNNING,
    GAME_FAIL,
    GAME_RESTARTING,
    GAME_PAUSE,
    GAME_WIN,
};
*/
class MainState : public GameState
{
    URHO3D_OBJECT(MainState, GameState);
    
public:
    MainState(Context* context);
    ~MainState();

    void OnCharacterKilled(Character* killer, Character* dead);
    void OnKeyDown(int key);
    void OnPlayerStatusUpdate(Player* player);
    void OnSceneTimeScaleUpdated(Scene* scene, float newScale);

    void ApplyBGMScale(float scale);

    void ShowMessage(const String& msg, bool show);

    Player* GetPlayer(); // virtual in GameState
    Scene* GetScene();
    String GetDebugText();
    
    void Enter(GameState* lastState);
    void Exit(GameState* nextState);
    
    void PostCreate();
    void CreateUI();
    void Update(float dt);
    void ChangeSubState(int newState);

    void postInit();

    SharedPtr<Scene> gameScene;
    SharedPtr<TextMenu> pauseMenu;
    SharedPtr<BorderImage> fullscreenUI;

    int sub_state; // GameSubState
    int pauseState;
    float fadeTime;
    float fadeInDuration;
    float restartDuration;
    bool postInited;
    
    SharedPtr<Text> messageText;
    SharedPtr<Text> statusText;

    
private:
    
    void CreateScene();
    void CreateViewPort();
    
    void SubscribeToEvents();
    
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
    void HandleCameraEvent(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void GameUpdate(float dt);
};






