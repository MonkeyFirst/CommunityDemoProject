
#include <Urho3D/Urho3DAll.h>

#include "States/GameManager.h"
#include "States/LoadingState.h"
#include "States/LevelState.h"
#include "States/MainState.h"

#include "Character/Bruce.h"
#include "Character/Thug.h"
#include "Character/Ragdoll.h"
#include "Objects/HeadIndicator.h"

#include "radio.h"

GameManager::GameManager(Context* context):
    Object(context)
{
    Log::Write(-1," GameManager: Constructor\n");
    finalized = false;
    initialized = false;
}

GameManager::~GameManager()
{
    Log::Write(-1," GameManager: Destructor\n");
    if(!finalized)
        Finalize();
}

// Вызывает объект Go после создания GameManager объекта.
bool GameManager::Initialize()
{
    Log::Write(-1," GameManager: Initialize\n");

    InitAudio();
    AddState(new MainState(context_));
    AddState(new LoadingState(context_));
    AddState(new LevelState(context_));

    initialized = true;
    return true;
}
/*

Node* myNode = scene_->CreateChild();

WeakPtr<StaticModel> gridComp =  WeakPtr<StaticModel>(myNode->CreateComponent<StaticModel>());

Log::Write(-1, String(gridComp.Refs())); // 1
gridComp->GetNode()->Remove();

Log::Write(-1,String(gridComp.Refs())); // 0

if (!gridComp.Expired()) // I should be using Expired() instead of NotNull()
{
   gridComp->GetNode()->Remove();
}

*/
// Вызывать ее до уничтожения объекта GameManager (как подсистемы) иначе крашится при выходе из приложения.
// Вызывает объект Go перед уничтожением GameManager объекта.
bool GameManager::Finalize()
{
    Log::Write(-1," GameManager: Finalize\n");
    
    for (unsigned int i = 0; i < states.Size(); ++i)
    {
        if (states[i])
            states[i].Reset();
    }
    //currentState = null;
    states.Clear();
    
    if(musicNode)
        musicNode.Reset();

    musicNode = NULL;

    finalized = true;
    return true;
}

Player* GameManager::GetPlayer() // virtual in GameState
{
    if (currentState)
        return currentState->GetPlayer();
    return NULL;
}

Scene* GameManager::GetScene()
{
//    Log::Write(-1," GameManager::GetScene() "+currentState->name+"\n");
    if (currentState)
        return currentState->GetScene();
    return NULL;
}

Node* GameManager::CreateCharacter(const String& name, const String& objectFile, const String& scriptClass, const Vector3& position, const Quaternion& rotation)
{
    Log::Write(-1,"\n CHARACTER: " + objectFile + " CLASS: " + scriptClass + "\n");

    Scene* scene = GetScene();
    if(!scene)
        return NULL;
    
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Node* p_node = scene->InstantiateXML(*cache->GetFile("Objects/" + objectFile), position, rotation);
    p_node->SetName(name);

    if(scriptClass == "Bruce")
        p_node->CreateComponent<Bruce>();
    else
        p_node->CreateComponent<Thug>();

    p_node->CreateComponent<Ragdoll>();
    p_node->CreateComponent<HeadIndicator>();
  
    return p_node;
}

void GameManager::Start()
{
//    InitAudio();
//    AddState(new LoadingState(context_));
//    AddState(new LevelState(context_));
}

void GameManager::InitAudio()
{
    if (GetSubsystem<Engine>()->IsHeadless())
        return;

    Audio* audio = GetSubsystem<Audio>();
//    audio.masterGain[SOUND_MASTER] = 0.5f;
//    audio.masterGain[SOUND_MUSIC] = 0.5f;
//    audio.masterGain[SOUND_EFFECT] = 1.0f;
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    if (!RADIO::nobgm)
    {
        Sound* musicFile = cache->GetResource<Sound>("Sfx/bgm.ogg");
        musicFile->SetLooped(true);

        RADIO::BGM_BASE_FREQ = musicFile->GetFrequency();

        // Note: the non-positional sound source component need to be attached to a node to become effective
        // Due to networked mode clearing the scene on connect, do not attach to the scene itself
        musicNode = new Node(context_);
        SoundSource* musicSource = musicNode->CreateComponent<SoundSource>();
        musicSource->SetSoundType(SOUND_MUSIC);
        musicSource->SetGain(0.5f);
        musicSource->Play(musicFile);
    }
}

void GameManager::AddState(GameState* state)
{
    SharedPtr<GameState> stateShared(state);
    states.Push(stateShared);
//    states.Push(state);
}

GameState* GameManager::FindState(const String& name)
{
    return FindState_Hash(StringHash(name));
}

GameState* GameManager::FindState_Hash(const StringHash& nameHash)
{
    for (unsigned int i = 0; i < states.Size(); ++i)
    {
        if (states[i]->nameHash == nameHash)
            return states[i];
    }
    return NULL;
}

bool GameManager::ChangeState(const String& name)
{
    Log::Write(-1," GameManager: ChangeState(" + name + ")\n");
    return ChangeState_Hash(StringHash(name));
}

bool GameManager::ChangeState_Hash(const StringHash& nameHash)
{
 //   bool b = FSM::ChangeState(nameHash);
 
    GameState* newState = FindState_Hash(nameHash);
    
    if (!newState)
    {
        Log::Write(-1," GameManager: new-game-state not found " + nameHash.ToString() + "\n");
        return false;
    }

    if (currentState == newState)
    {
        Log::Write(-1," GameManager: currentState == newState !!!\n");
        if (!currentState->CanReEntered())
            return false;
        currentState->Exit(newState);
        currentState->Enter(newState);
    }

    GameState* oldState = currentState;
    if (oldState)
        oldState->Exit(newState);

    if (newState)
    {
        currentState = newState;
        newState->Enter(oldState);
        return true;
    }

    return false;
//    currentState = newState;
/*
    if (RADIO::d_log)
    {
        String oldStateName = "NULL";
        if (oldState)
            oldStateName = oldState->name;

        String newStateName = "NULL";
        if (newState)
            newStateName = newState->name;

        Log::Write(-1," GameManager Change GameState from " +oldStateName+ " to " +newStateName+"\n");        
    }
*/
//    return true;
}

void GameManager::ChangeStateQueue(const StringHash& name)
{
    queueState = name;
}

// INFO: Call Go class
void GameManager::Update(float dt)
{
    if (currentState)
        currentState->Update(dt);

    if (queueState != StringHash::ZERO)
    {
        ChangeState_Hash(queueState);
        queueState = StringHash::ZERO;
    }
}

void GameManager::FixedUpdate(float dt)
{
    if (currentState)
        currentState->FixedUpdate(dt);
}

void GameManager::DebugDraw(DebugRenderer* debug)
{
    if (currentState)
        currentState->DebugDraw(debug);
}

String GameManager::GetDebugText()
{
    String ret = "current-state: ";
    if (currentState)
        ret += currentState->GetDebugText();
    else
        ret += "NULL\n";
    return ret;
}


// INFO: Call Go class
void GameManager::OnSceneLoadFinished(Scene* _scene)
{
    Log::Write(-1," GameManager: OnSceneLoadFinished\n");
    if (currentState)
        currentState->OnSceneLoadFinished(_scene);
}

// INFO: Call Go class
void GameManager::OnAsyncLoadProgress(Scene* _scene, float progress, int loadedNodes, int totalNodes, int loadedResources, int totalResources)
{
    if (currentState)
        currentState->OnAsyncLoadProgress(_scene, progress, loadedNodes, totalNodes, loadedResources, totalResources);
}

// INFO: Call Go class
void GameManager::OnKeyDown(int key)
{
    if (currentState)
        currentState->OnKeyDown(key);
}

void GameManager::OnPlayerStatusUpdate(Player* player)
{
    if (currentState)
        currentState->OnPlayerStatusUpdate(player);
}

void GameManager::OnSceneTimeScaleUpdated(Scene* scene, float newScale)
{
    if (currentState)
        currentState->OnSceneTimeScaleUpdated(scene, newScale);
}

void GameManager::OnCharacterKilled(Character* killer, Character* dead)
{
    if (currentState)
        currentState->OnCharacterKilled(killer, dead);
}


