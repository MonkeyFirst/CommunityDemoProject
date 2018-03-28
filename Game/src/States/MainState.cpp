
#include <Urho3D/Urho3DAll.h>

#include "States/GameState.h"
//#include "Levels/LevelEvents.h"
#include "States/MainState.h"
#include "States/GameManager.h"

//#include "Game/TextMenu.h"
#include "Camera/CameraManager.h"
#include "Character/EnemyManager.h"
#include "Motions/MotionManager.h"
#include "Objects/InputManager.h"
#include "Objects/Follow.h"

#include "Tools/Line.h"

#include "Character/EM_Helper.h"
#include "Character/Bruce.h"

#include "radio.h"

// временно для enum GameSubState и String CAMERA_NAME
#include "States/LevelState.h"

MainState::MainState(Context* context) :
    GameState(context)
{
    Log::Write(-1," MainState: Constructor \n");

    sub_state = -1;
    pauseState = -1;
    fadeInDuration = 2.0f;
    restartDuration = 5.0f;
    postInited = false;
    RADIO::maxKilled = 5;

    GameState::SetName("MainState");

    pauseMenu = new TextMenu(context_, UI_FONT, UI_FONT_SIZE);
    fullscreenUI = new BorderImage(context_);//("FullScreenImage");
    fullscreenUI->SetVisible(false);//.visible = false;
    fullscreenUI->SetPriority(-9999);//.priority = -9999;
    fullscreenUI->SetOpacity(1.0f);//.opacity = 1.0f;
    Texture* texture = GetSubsystem<ResourceCache>()->GetResource<Texture2D>("Textures/fade.png");
    fullscreenUI->SetTexture(texture);//.texture = cache.GetResource("Texture2D", "Textures/fade.png");
    fullscreenUI->SetFullImageRect();
    if (!GetSubsystem<Engine>()->IsHeadless())
        fullscreenUI->SetFixedSize(GetSubsystem<Graphics>()->GetWidth(), GetSubsystem<Graphics>()->GetHeight());
    GetSubsystem<UI>()->GetRoot()->AddChild(fullscreenUI);
    pauseMenu->texts.Push("RESUME");
    pauseMenu->texts.Push("EXIT");    

}

MainState::~MainState()
{
    Log::Write(-1," MainState: Destructor \n");

    if(pauseMenu)
        pauseMenu.Reset();

    if(gameScene)
        gameScene->Remove();
    gameScene.Reset();

    if(fullscreenUI)
        fullscreenUI->Remove();
    fullscreenUI.Reset();

    if(messageText)
        messageText->Remove();
    messageText.Reset();
    
    if(statusText)
        statusText->Remove();
    statusText.Reset();
/*    
    if(inputText)
        inputText->Remove();
    inputText.Reset();
    
    if(debug_txt)
        debug_txt->Remove();
    debug_txt.Reset();
*/   
}

void MainState::Enter(GameState* lastState)
{
    Log::Write(-1," MainState::Enter\n");
    sub_state = -1;
    GameState::Enter(lastState);
    CreateScene();
    if (GetSubsystem<Engine>()->IsHeadless())
    {
        return;
    }
        
    CreateViewPort();
    CreateUI();
    PostCreate();
    
    ChangeSubState(GAME_FADING);
}

void MainState::Exit(GameState* nextState)
{
    Log::Write(-1," MainState::Exit\n");
    GameState::Exit(nextState);

// INFO: Добавлено удаление того что создается при Входе в состояние. 
// Кроме создаваемого в конструкторе и существующего пока есть объект.
// pauseMenu и fullscreenUI
    if(gameScene)
        gameScene->Remove();
    gameScene.Reset();

    if(messageText)
        messageText->Remove();
    messageText.Reset();
    
    if(statusText)
        statusText->Remove();
    statusText.Reset();
}

void MainState::PostCreate()
{
// INFO: Здесь исключил создание reflection    

    Node* zoneNode = gameScene->GetChild("zone", true);
    Zone* zone = zoneNode->GetComponent<Zone>();
    // zone.heightFog = false;
}

void MainState::CreateUI()
{
    if(RADIO::game_type != 0)
        return;
    
    Graphics* graphics = GetSubsystem<Graphics>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();

    int height = graphics->GetHeight() / 22;
    if (height > 64)
        height = 64;

    messageText = ui->GetRoot()->CreateChild<Text>("message");
    messageText->SetFont(cache->GetResource<Font>(RADIO::UI_FONT), RADIO::UI_FONT_SIZE);
    messageText->SetAlignment(HA_CENTER, VA_CENTER);
    messageText->SetPosition(0, -height * 2 + 100);
    messageText->SetColor(Color(1, 0, 0));
    messageText->SetVisible(false);

    statusText = ui->GetRoot()->CreateChild<Text>("status");
    statusText->SetFont(cache->GetResource<Font>(RADIO::UI_FONT), RADIO::UI_FONT_SIZE);
    statusText->SetAlignment(HA_LEFT, VA_TOP);
    statusText->SetPosition(0, 0);
    statusText->SetColor(Color(1, 1, 0)); 
    statusText->SetVisible(true);

    OnPlayerStatusUpdate(GetPlayer());
}

void MainState::Update(float dt)
{
    //Log::Write(-1," UPDATE 2\n");

    switch (sub_state)
    {
    case GAME_FADING:
        {
            float t = fullscreenUI->GetAttributeAnimationTime("Opacity");
            if (t + 0.05f >= fadeTime)
            {
                fullscreenUI->SetVisible(false);
                ChangeSubState(GAME_RUNNING);
            }
        }
        break;

    case GAME_RESTARTING:
        {
            EnemyManager* em = RADIO::g_em;
            
            if (fullscreenUI->GetOpacity() > 0.95f && em->enemyList.Empty())
            {
                RADIO::g_im->m_rightStickX = 20;
                RADIO::g_im->m_rightStickY = 30;
                em->CreateEnemies();
            }

            float t = fullscreenUI->GetAttributeAnimationTime("Opacity");
            if (t + 0.05f >= fadeTime)
            {
                fullscreenUI->SetVisible(false);
                ChangeSubState(GAME_RUNNING);
            }
        }
        break;

    case GAME_FAIL:
    case GAME_WIN:
        {
            if (RADIO::g_im->IsAttackPressed())
            {
                ChangeSubState(GAME_RESTARTING);
                ShowMessage("", false);
            }
        }
        break;

    case GAME_PAUSE:
        {
            int selection = pauseMenu->Update(dt);
            if (selection == 0)
                ChangeSubState(pauseState);
            else if (selection == 1)
                GetSubsystem<Engine>()->Exit();
        }
        break;

    case GAME_RUNNING:
        {
            if (!postInited) {
                if (timeInState > 2.0f) 
                {
                    postInit();
                    postInited = true;
                }
            }
        }
        break;
    }
    GameState::Update(dt);
}

void MainState::postInit()
{
//    if (bHdr && graphics !is null)
//        renderer.viewports[0].renderPath.shaderParameters["AutoExposureAdaptRate"] = 0.6f;
}

Player* MainState::GetPlayer()
{
    if (!gameScene)
        return NULL;
    Node* characterNode = gameScene->GetNode(RADIO::playerId);
    if (!characterNode)
        return NULL;
    return static_cast<Player*>(characterNode->GetComponent<Bruce>());
}

Scene* MainState::GetScene()
{
    if (!gameScene)
        return NULL;
    
    return gameScene;
}

void MainState::ChangeSubState(int newState)
{
    if (sub_state == newState)
        return;

    int oldState = sub_state;
    Log::Write(-1," MainState: Switch State from " + String(oldState) + " to " + String(newState) + "\n");

    sub_state = newState;
    timeInState = 0.0f;

    gameScene->SetUpdateEnabled(!(newState == GAME_PAUSE));

    if (newState == GAME_PAUSE)
        pauseMenu->Add();
    else
        pauseMenu->Destroy();

    Player* player = GetPlayer();
    EnemyManager* em = RADIO::g_em;

    switch (newState)
    {
    case GAME_RUNNING:
        {
            if (player)
                player->RemoveFlag(FLAGS_INVINCIBLE);

            RADIO::g_im->freezeInput_ = false;
            VariantMap data;
            data[NAME] = CHANGE_STATE;
            data[VALUE] = StringHash("ThirdPerson");
            SendEvent("CameraEvent", data);
        }
        break;

    case GAME_FADING:
        {
            if (oldState != GAME_PAUSE)
            {
                ValueAnimation* alphaAnimation = new ValueAnimation(context_);
                alphaAnimation->SetKeyFrame(0.0f, Variant(1.0f));
                alphaAnimation->SetKeyFrame(fadeInDuration, Variant(0.0f));
                fadeTime = fadeInDuration;
                fullscreenUI->SetVisible(true);
                fullscreenUI->SetAttributeAnimation("Opacity", alphaAnimation, WM_ONCE);
            }

            RADIO::g_im->freezeInput_ = true;
            if (player)
                player->AddFlag(FLAGS_INVINCIBLE);
                
        }
        break;

    case GAME_RESTARTING:
        {
            if (oldState != GAME_PAUSE)
            {
                ValueAnimation* alphaAnimation = new ValueAnimation(context_);
                alphaAnimation->SetKeyFrame(0.0f, Variant(0.0f));
                alphaAnimation->SetKeyFrame(restartDuration/2, Variant(1.0f));
                alphaAnimation->SetKeyFrame(restartDuration, Variant(0.0f));
                fadeTime = restartDuration;
                fullscreenUI->SetOpacity(0.0f);
                fullscreenUI->SetVisible(true);
                fullscreenUI->SetAttributeAnimation("Opacity", alphaAnimation, WM_ONCE);
            }

            RADIO::g_im->freezeInput_ = true;
            if (em)
                em->RemoveAll();

            if (player)
            {
                player->ResetObject();
                player->AddFlag(FLAGS_INVINCIBLE);
            }
        }
        break;

    case GAME_PAUSE:
        {
            // ....
        }
        break;

    case GAME_WIN:
        {
            ShowMessage(" You Win!\n Press Stride to restart!", true);
            if (player)
                player->SetTarget(NULL);
        }
        break;

    case GAME_FAIL:
        {
            ShowMessage(" You Died!\n Press Stride to restart!", true);
            if (player)
                player->SetTarget(NULL);
        }
        break;
    }
}

void MainState::CreateViewPort()
{
    Viewport* viewport = new Viewport(context_, gameScene, RADIO::g_cm->GetCamera());
    GetSubsystem<Renderer>()->SetViewport(0, viewport);
    //renderer.viewports[0] = viewport;

/*
        RenderPath@ renderpath = viewport.renderPath.Clone();
        if (render_features & RF_HDR != 0)
        {
            // if (reflection)
            //    renderpath.Load(cache.GetResource("XMLFile","RenderPaths/ForwardHWDepth.xml"));
            // else
            renderpath.Load(cache.GetResource("XMLFile","RenderPaths/ForwardDepth.xml"));
            renderpath.Append(cache.GetResource("XMLFile","PostProcess/AutoExposure.xml"));
            renderpath.Append(cache.GetResource("XMLFile","PostProcess/BloomHDR.xml"));
            renderpath.Append(cache.GetResource("XMLFile","PostProcess/Tonemap.xml"));
            renderpath.SetEnabled("TonemapReinhardEq3", false);
            renderpath.SetEnabled("TonemapUncharted2", true);
            renderpath.shaderParameters["TonemapMaxWhite"] = 1.8f;
            renderpath.shaderParameters["TonemapExposureBias"] = 2.5f;
            renderpath.shaderParameters["AutoExposureAdaptRate"] = 2.0f;
            renderpath.shaderParameters["BloomHDRMix"] = Variant(Vector2(0.9f, 0.6f));
        }
        renderpath.Append(cache.GetResource("XMLFile", "PostProcess/FXAA2.xml"));
        renderpath.Append(cache.GetResource("XMLFile","PostProcess/ColorCorrection.xml"));
        viewport.renderPath = renderpath;
        SetColorGrading(colorGradingIndex);
*/
}

void MainState::CreateScene()
{
    Log::Write(-1," MainState: CreateScene \n");

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Time* time = GetSubsystem<Time>();
    unsigned int t = time->GetSystemTime();

    gameScene = new Scene(context_);
    
    // HACK: Update helper for EnemyManager
    Node* em_helper = gameScene->CreateChild("EM_Helper");
    em_helper->CreateComponent<EM_Helper>(); // нельзя прямо в Сцену добавлять компоненты.

    String scnFile = "Scenes/1.xml";
    if (RADIO::game_type == 1)
        scnFile = "Scenes/2.xml";

    gameScene->LoadXML(*cache->GetFile(scnFile));

    Log::Write(-1," loading-scene XML --> time-cost " + String(time->GetSystemTime() - t) + " ms\n");

    Node* cameraNode = gameScene->CreateChild(RADIO::CAMERA_NAME);
    Camera* cam = cameraNode->CreateComponent<Camera>();
    cam->SetFov(RADIO::BASE_FOV);
    RADIO::cameraId = cameraNode->GetID(); // In LevelBase

    Node* tmpPlayerNode = gameScene->GetChild("player", true);
    Vector3 playerPos;
    Quaternion playerRot;
    if (tmpPlayerNode)
    {
        playerPos = tmpPlayerNode->GetWorldPosition();
        playerRot = tmpPlayerNode->GetWorldRotation();
        if (RADIO::collision_type == 0)
            playerPos.y_ = 0;
        tmpPlayerNode->Remove();
    }

    EnemyManager* em = RADIO::g_em;
    Audio* audio = GetSubsystem<Audio>();

    // INFO: removed playerType check before create Character.
    Node* playerNode = RADIO::g_gm->CreateCharacter("player", "BATMAN/bruce/bruce.xml", "Bruce", playerPos, playerRot);

    if(!playerNode)
        Log::Write(-1," ERROR: CREATE CHARACTER IMPOSIBLE\n");
    
    audio->SetListener(playerNode->GetChild(HEAD, true)->CreateComponent<SoundListener>());
    RADIO::playerId = playerNode->GetID();

    // preprocess current scene
    Vector<unsigned int> nodes_to_remove;
    int enemyNum = 0;
    
    Log::Write(-1,"\n Parse Top Nodes from Scene " + scnFile + ":\n");
    
    for (unsigned int i = 0; i < gameScene->GetNumChildren(false); ++i)
    {
        Node* _node = gameScene->GetChildren()[i];

        Log::Write(-1," _node.name = " + _node->GetName() + "\n");

        if (_node->GetName().StartsWith("thug"))
        {
            nodes_to_remove.Push(_node->GetID());
            if (RADIO::test_enemy_num_override > 0 && enemyNum >= RADIO::test_enemy_num_override)
                continue;
            if (RADIO::game_type != 0)
                continue;
            Vector3 v = _node->GetWorldPosition();
            v.y_ = 0;

            em->enemyResetPositions.Push(v);
            em->enemyResetRotations.Push(_node->GetWorldRotation());
            ++enemyNum;
        }

        else if (_node->GetName().StartsWith("preload_"))
            nodes_to_remove.Push(_node->GetID());

        else if (_node->GetName().StartsWith("light"))
        {
            Light* light = _node->GetComponent<Light>();
//            if (!render_features & RF_SHADOWS)
//                light->SetCastShadows(false); //castShadows = false;
//            light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
//            light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
        }
    }

    for (unsigned int i = 0; i < nodes_to_remove.Size(); ++i)
        gameScene->GetNode(nodes_to_remove[i])->Remove();

    em->CreateEnemies();

    RADIO::maxKilled = em->enemyResetRotations.Size();

    Vector3 v_pos = playerNode->GetWorldPosition();
    cameraNode->SetPosition(Vector3(v_pos.x_, 10.0f, -10));
    cameraNode->LookAt(Vector3(v_pos.x_, 4, 0));
    
    CameraManager* cm = RADIO::g_cm;
    cm->Start(cameraNode);
    cm->SetCameraController("ThirdPerson");
    //cm->SetCameraController("Debug");

    //gameScene = scene_;
    
    Node* lightNode = gameScene->GetChild("light");
    if (lightNode)
    {
        //Follow* flw = cast<Follow>(lightNode.CreateScriptObject(scriptFile, "Follow"));
        Follow* flw = lightNode->CreateComponent<Follow>();
        flw->toFollow = RADIO::playerId;
        flw->offset = Vector3(0, 10, 0);
    }

    if (RADIO::game_type == 1)
        RADIO::g_lw->Process(gameScene);

    //DumpSkeletonNames(playerNode);
    Log::Write(-1," CreateScene() --> total time-cost " + String(time->GetSystemTime() - t) + " ms.\n");

}

void MainState::ShowMessage(const String& msg, bool show)
{
    //Text* messageText = ui.root.GetChild("message", true);
    if (messageText)
    {
        messageText->SetText(msg);
        messageText->SetVisible(true);
    }
}

void MainState::OnCharacterKilled(Character* killer, Character* dead)
{
    if (dead->side == 1)
    {
        Log::Write(-1," OnPlayerDead !!!!!!!!\n");
        ChangeSubState(GAME_FAIL);
    }

    if (killer)
    {
        if (killer->side == 1)
        {
            Player* player = static_cast<Player*>(killer);
            if (player)
            {
                if (player->killed >= RADIO::maxKilled)
                {
                    Log::Write(-1," WIN !!!!!!!!\n");
                    ChangeSubState(GAME_WIN);
                }
            }
        }
    }
}

void MainState::OnKeyDown(int key)
{
    if (key == KEY_Q)
    {
        if (RADIO::game_type == 1)
        {
//            engine.Exit();
            return;
        }
        int oldState = sub_state;
        if (oldState == GAME_PAUSE)
            ChangeSubState(pauseState);
        else
        {
            ChangeSubState(GAME_PAUSE);
            pauseState = oldState;
        }
        return;
    }

    GameState::OnKeyDown(key);
}

void MainState::OnPlayerStatusUpdate(Player* player)
{
    if (!player)
        return;

    if (statusText)
    {
        statusText->SetText(" HP: " + String(player->health) + " COMBO: " + String(player->combo) + " KILLED:" + String(player->killed));
    }

    // ApplyBGMScale(gameScene.timeScale *  GetPlayer().timeScale);
}

void MainState::OnSceneTimeScaleUpdated(Scene* scene, float newScale)
{
    if (gameScene != scene)
        return;
    // ApplyBGMScale(newScale *  GetPlayer().timeScale);
}

void MainState::ApplyBGMScale(float scale)
{
    if (!RADIO::g_gm->musicNode)
        return;
    Log::Write(-1," Game::ApplyBGMScale " + String(scale) + "\n");
    SoundSource* s = RADIO::g_gm->musicNode->GetComponent<SoundSource>();
    if (!s)
        return;
    s->SetFrequency(RADIO::BGM_BASE_FREQ * scale);
}

String MainState::GetDebugText()
{
    return  " name=" + name + " timeInState=" + String(timeInState) + " state=" + String(sub_state) + " pauseState=" + String(pauseState) + "\n";
}



