
#pragma once

#include "App.h"

//#include "Cfg/CfgManager.h"
/*
#include "Tools/Line.h"
#include "States/GameManager.h"
#include "Motions/MotionManager.h"
#include "Camera/CameraManager.h"
#include "Objects/InputManager.h"
#include "Character/EnemyManager.h"
*/
namespace Urho3D
{
class Node;
class Scene;
class Window;

}

class Go : public App
{
	URHO3D_OBJECT(Go, App);

public:
    Go(Context* context);
    ~Go();
	// Setup before engine initialization. Modifies the engine parameters.
	void Setup();
    // Setup after engine initialization and before running the main loop.
    void Start();
    // Cleanup after the main loop. Called by App.
    void Stop();
    
    bool SetupStepTwo();
    
    void CreateConsoleAndDebugHud();
    void SetWindowTitleAndIcon();
    void CreateUI();
    
    void CreateEnemy();
    
    void ExecuteCommand();

protected:
/*
    SharedPtr<GameManager> gm_;
    SharedPtr<MotionManager> mm_;
    SharedPtr<CameraManager> cm_;
//    SharedPtr<InputManager> im_;
    SharedPtr<EnemyManager> em_;
    SharedPtr<LineWorld> lw_;
*/
//    SharedPtr<Cfg> _cfg;
    SharedPtr<DebugHud> dHud_;
    SharedPtr<Console> console_;
    
    void SubscribeToEvents();
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
	void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
    void HandleAsyncLoadProgress(StringHash eventType, VariantMap& eventData);
    void HandleSceneLoadFinished(StringHash eventType, VariantMap& eventData);
    void HandleCameraEvent(StringHash eventType, VariantMap& eventData);

private:

};


