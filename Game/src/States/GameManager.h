#pragma once

#include <Urho3D/Urho3DAll.h>

#include "States/GameState.h"
#include "Motions/MotionManager.h"

class GameManager : public Object
{
    URHO3D_OBJECT(GameManager, Object);

public:
    GameManager(Context* context);
    ~GameManager();

    bool Initialize();
    bool Finalize();
    
    void Start();
    void AddState(GameState* state);
    GameState* FindState(const String& name);
    GameState* FindState_Hash(const StringHash& nameHash);
    bool ChangeState(const String& name);
    bool ChangeState_Hash(const StringHash& nameHash);
    void ChangeStateQueue(const StringHash& name);

    void InitAudio();
    
    void Update(float dt);
    void FixedUpdate(float dt);
    void DebugDraw(DebugRenderer* debug);
    String GetDebugText();
    
    Player* GetPlayer(); // virtual in GameState
    Scene* GetScene();

    Node* CreateCharacter(const String& name, const String& objectFile, const String& scriptClass, const Vector3& position, const Quaternion& rotation);
    
    void OnCharacterKilled(Character* killer, Character* dead);
    void OnSceneLoadFinished(Scene* _scene);
    void OnAsyncLoadProgress(Scene* _scene, float progress, int loadedNodes, int totalNodes, int loadedResources, int totalResources);
    void OnKeyDown(int key);
    void OnPlayerStatusUpdate(Player* player);
    void OnSceneTimeScaleUpdated(Scene* scene, float newScale);
    
    //Vector<GameState*> states;
    Vector<SharedPtr<GameState> > states;
    WeakPtr<GameState> currentState;
    SharedPtr<Node> musicNode;
    StringHash queueState;
    
    bool initialized;
    bool finalized;

};



