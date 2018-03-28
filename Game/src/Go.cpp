// $(SolutionDir)..\..\Urho3D\Build\include
// $(SolutionDir)..\src
#include <Urho3D/Urho3DAll.h>

#include "Tools/Line.h"
#include "States/GameManager.h"
#include "Motions/MotionManager.h"
#include "Camera/CameraManager.h"
#include "Objects/InputManager.h"
#include "Character/EnemyManager.h"
#include "Objects/InputManager.h"

#include "Cfg/Cfg.h"
#include "Cfg/CfgManager.h"

#include <IMUI/imgui/imgui.h>
#include "IMUI/IMUI.h"

//#include "Levels/LevelEvents.h"
#include "Objects/Follow.h"
#include "Objects/HeadIndicator.h"
#include "Objects/ObjectKiller.h"
#include "Character/EM_Helper.h"
#include "Character/Ragdoll.h"
#include "Character/Bruce.h"
#include "Character/Thug.h"

#include "Go.h"

#include "radio.h"

// for main function
#include <Urho3D/Core/ProcessUtils.h>
#ifdef WIN32
#include <windows.h>
#endif

//SetRandomSeed(Random(1, M_MAX_INT)); // или SetRandomSeed(Time::GetSystemTime());
//int chance = Rand(); //General Random
//int chance2 = Random(0, 100); //Random between 0 - 99
//int chance3 = Random(100); // Random between 0 - 99

int RunApp()
{
    Urho3D::SharedPtr<Urho3D::Context> context(new Urho3D::Context());
    Urho3D::SharedPtr<Go> application(new Go(context));
    return application->Run();
}
// extern "C" int SDL_main(int argc, char** argv);
// int SDL_main(int argc, char** argv)
int main(int argc, char** argv)
{
    Urho3D::ParseArguments(argc, argv);
    return RunApp();
}

Go::Go(Context* context) :
    App(context)

{
}

Go::~Go()
{
    Log::Write(-1," GO: DESTRUCTOR\n\n");
}

void Go::Stop()
{
    Log::Write(-1,"\n GO: STOP\n");
    if(RADIO::g_mm)
    {
        //mm_->Stop(); TODO: mm_->Stop()
        RADIO::g_mm->Finalize();
        RADIO::g_mm.Reset();
    }
    if(RADIO::g_gm)
        RADIO::g_gm->Finalize();
        RADIO::g_gm.Reset();

    if(RADIO::g_cm)
        RADIO::g_cm->Finalize();
        RADIO::g_cm.Reset();

    if(RADIO::g_im)
        RADIO::g_im->Finalize();
        RADIO::g_im.Reset();

    if(RADIO::g_em)
        RADIO::g_em->Finalize();
        RADIO::g_em.Reset();

    if(RADIO::g_lw)
        RADIO::g_lw->Finalize();
        RADIO::g_lw.Reset();

    if(RADIO::g_cfg)
        RADIO::g_cfg->Finalize();
        RADIO::g_cfg.Reset();

//    if(_cfg)
//        _cfg.Reset();

    if(dHud_)
        dHud_.Reset();
    
    if(console_)
        console_.Reset();
}

void Go::Setup()
{
    RADIO::g_cfg = new CfgManager(context_);
	RADIO::g_cfg->Initialize();
    Cfg* cfg = RADIO::g_cfg->GetCfg("go.cfg", CFG_GAME); // GetCfg("go.cfg");

//    _cfg = new Cfg(context_);
//    _cfg->Load("go.cfg");

Urho3D::Variant df = cfg->SectorsCount();
URHO3D_LOGINFO("Config file - Sectors - " + df.ToString());

Urho3D::Variant sf = cfg->KeysCount("OS");
URHO3D_LOGINFO("Config file - Keys in OS - " + sf.ToString());

	// Modify engine startup parameters
//	engineParameters_["WindowTitle"] = GetTypeName();
//	engineParameters_["LogName"] = GetTypeName() + ".log";
//	engineParameters_["FullScreen"] = false;
//	engineParameters_["Headless"] = false;
//	engineParameters_["Sound"] = false;


    // ENGINE
    engineParameters_["Headless"]                  = cfg->GetBool("ENGINE", "headless", false);
    engineParameters_["LogLevel"]                  = cfg->GetInt("ENGINE", "logLevel", 2); // LOG_DEBUG = 0 LOG_INFO = 1 LOG_WARNING = 2 LOG_ERROR = 3 LOG_NONE = 4
    engineParameters_["LogQuiet"]                  = cfg->GetBool("ENGINE", "logQuiet", false);
    engineParameters_["LogName"]                  = cfg->GetString("ENGINE", "logName", "log.log");
    engineParameters_["FrameLimiter"]             = cfg->GetBool("ENGINE", "fpsLimiter", true);
    engineParameters_["WorkerThreads"]         = cfg->GetBool("ENGINE", "threads", true);
    engineParameters_["AutoloadPaths"]          = cfg->GetString("ENGINE", "autoPaths");
    engineParameters_["ResourcePrefixPaths"] = cfg->GetString("ENGINE", "prefixPaths");
    engineParameters_["ResourcePaths"]          = cfg->GetString("ENGINE", "paths", "CoreData;Data");
    engineParameters_["ResourcePackages"]    = cfg->GetString("ENGINE", "packages");
    engineParameters_["EventProfiler"]             = cfg->GetBool("ENGINE", "eventProfiler", true);
// /*    
    // GRAPHICS
    engineParameters_["WindowTitle"]          = cfg->GetString("VIDEO", "windowTitle", "radio");
    engineParameters_["WindowWidth"]       = cfg->GetInt("VIDEO", "width", 800);
    engineParameters_["WindowHeight"]      = cfg->GetInt("VIDEO", "height", 600);
    engineParameters_["WindowResizable"] = cfg->GetBool("VIDEO", "resizable", false);
    engineParameters_["Borderless"]            = cfg->GetBool("VIDEO", "borderless", false);
    engineParameters_["WindowIcon"]         = cfg->GetString("VIDEO", "windowIcon");
    engineParameters_["ForceGL2"]              = cfg->GetBool("VIDEO", "forceGL2", false);
    engineParameters_["FullScreen"]             = cfg->GetBool("VIDEO", "fullscreen", false);
    engineParameters_["TripleBuffer"]           = cfg->GetBool("VIDEO", "tripleBuffer", false);
    engineParameters_["VSync"]                    = cfg->GetBool("VIDEO", "vsync", false);
    engineParameters_["Multisample"]           = cfg->GetInt("VIDEO", "multiSample", 1);
    // engineParameters_["WindowPositionX"]      = cfg->GetInt("VIDEO", "winPosX", 4);
    // engineParameters_["WindowPositionY"]      = cfg->GetInt("VIDEO", "winPosY", 4);
//*/    
    // RENDERER
    engineParameters_["RenderPath"]          = cfg->GetString("VIDEO", "renderPath", "RenderPaths/Forward.xml");
    engineParameters_["Shadows"]              = cfg->GetBool("VIDEO", "shadows", true);
    engineParameters_["MaterialQuality"]     = cfg->GetInt("VIDEO", "materialQuality", 2);
    engineParameters_["TextureQuality"]      = cfg->GetInt("VIDEO", "textureQuality", 2);
    engineParameters_["TextureFilterMode"] = cfg->GetInt("VIDEO", "textureFilterMode", 1);
    engineParameters_["TextureAnisotropy"] = cfg->GetInt("VIDEO", "textureAnisotropy", 4);
    
    // AUDIO
    engineParameters_["Sound"]                     = cfg->GetBool("AUDIO", "sound", true);
    engineParameters_["SoundBuffer"]            = cfg->GetInt("AUDIO", "buffer", 100);
    engineParameters_["SoundMixRate"]         = cfg->GetInt("AUDIO", "mixRate", 44100);
    engineParameters_["SoundStereo"]           = cfg->GetBool("AUDIO", "stereo", true);
    engineParameters_["SoundInterpolation"] = cfg->GetBool("AUDIO", "interpolation", true);
 /*   
    
    // Construct a search path to find the resource prefix with two entries:
    // The first entry is an empty path which will be substituted with program/bin directory -- this entry is for binary when it is still in build tree
    // The second and third entries are possible relative paths from the installed program/bin directory to the asset directory -- these entries are for binary when it is in the Urho3D SDK installation location
    if (!engineParameters_.Contains("ResourcePrefixPaths"))
        engineParameters_["ResourcePrefixPaths"] = ";../share/Resources;../share/Urho3D/Resources";
*/
}

bool Go::SetupStepTwo()
{
    Cfg* cfg = RADIO::g_cfg->GetCfg("go.cfg", CFG_GAME);
    
    if( cfg->GetBool("ENGINE", "fpsLimiter", true) && cfg->GetInt("ENGINE", "fpsLimit", 25) > 4 )
    {
        GetSubsystem<Engine>()->SetMaxFps( cfg->GetInt("ENGINE", "fpsLimit", 25) );
    }
    else 
    {
        GetSubsystem<Engine>()->SetMaxFps(0);// no limit
    }

    GetSubsystem<Engine>()->SetPauseMinimized( cfg->GetBool("ENGINE", "pauseMinimized", false) );

    // AUDIO
    Audio* audio = GetSubsystem<Audio>();
    audio->SetMasterGain(SOUND_MASTER, cfg->GetFloat("AUDIO", "masterVolume", 0.75) );
    audio->SetMasterGain(SOUND_MUSIC, cfg->GetFloat("AUDIO", "musicVolume", 0.5) );
    audio->SetMasterGain(SOUND_EFFECT, cfg->GetFloat("AUDIO", "effectsVolume", 0.5) );
    audio->SetMasterGain(SOUND_AMBIENT, cfg->GetFloat("AUDIO", "ambientVolume", 0.5) );
    audio->SetMasterGain(SOUND_VOICE, cfg->GetFloat("AUDIO", "voiceVolume", 0.5) );

    // Graphics
    // Renderer
    Renderer* render = GetSubsystem<Renderer>();
//    render->SetTextureQuality( cfg->GetInt("VIDEO", "textureQuality", 2) );
//    render->SetMaterialQuality( cfg->rGetInt("VIDEO", "materialQuality", 2) );
    render->SetSpecularLighting( cfg->GetBool("VIDEO", "specularLighting", true) );
    render->SetMaxOccluderTriangles( cfg->GetInt("VIDEO", "maxOccluderTriangles", 4000) );
    render->SetDynamicInstancing( cfg->GetBool("VIDEO", "dynamicInstancing", true) );
    render->SetShadowMapSize( cfg->GetInt("VIDEO", "shadowMapSize", 512) );
    render->SetShadowQuality((ShadowQuality) cfg->GetInt("VIDEO", "shadowQuality", 1) ); // SHADOWQUALITY_SIMPLE_24BIT
    // Input

cfg->SetFileName("tmp.cfg");
cfg->DeleteSector("OS");
cfg->CreateSector("NEW", "new sector comment");
cfg->SetVar("test", 3, "comment", "NEW");
cfg->Save();

    return true;    
}

void Go::Start()
{
    //GetSubsystem<Log>()->SetLevel(LOG_DEBUG);
    // Execute base class startup
 //   App::Start();

    SetupStepTwo();

    if (RADIO::game_type == 1)
    {
        RADIO::collision_type = 1;
        RADIO::reflection = false;
    }
    if (!GetSubsystem<Engine>()->IsHeadless() && GetSubsystem<Graphics>()->GetWidth() < 640)
        RADIO::render_features = RF_NONE;
    
//    if (renderer !is null && (render_features & RF_HDR != 0))
//        renderer.hdrRendering = true;
    
    GetSubsystem<Engine>()->SetPauseMinimized(true); // Уже в конфиге делается.    
    GetSubsystem<ResourceCache>()->SetAutoReloadResources(true);
    SetRandomSeed(Time::GetSystemTime());
    // SetRandomSeed(Random(1, M_MAX_INT)); // или SetRandomSeed(Time::GetSystemTime());
    //int chance = Rand(); //General Random
    //int chance2 = Random(0, 100); //Random between 0 - 99
    //int chance3 = Random(100); // Random between 0 - 99

    context_->RegisterSubsystem(new IMUIContext(context_));

    // Enable OS cursor
//    GetSubsystem<Input>()->SetMouseVisible(true);

//	context_->RegisterSubsystem(new GameManager(context_));
//	gm_ = GetSubsystem<GameManager>();

    RADIO::g_mm = new MotionManager(context_);
	RADIO::g_mm->Initialize();

    RADIO::g_cm = new CameraManager(context_);
	RADIO::g_cm->Initialize();

    RADIO::g_im = new InputManager(context_);
    RADIO::g_im->Initialize();

    RADIO::g_em = new EnemyManager(context_);
	RADIO::g_em->Initialize();

    RADIO::g_lw = new LineWorld(context_);
	RADIO::g_lw->Initialize();

    RADIO::g_gm = new GameManager(context_);
	RADIO::g_gm->Initialize();

    context_->RegisterFactory<ObjectKiller>();
    context_->RegisterFactory<EM_Helper>();
    context_->RegisterFactory<Ragdoll>();
    context_->RegisterFactory<Follow>();
    context_->RegisterFactory<HeadIndicator>();
    context_->RegisterFactory<Bruce>();
    context_->RegisterFactory<Thug>();
    
    Log::Write(-1," Go: RegisterFactory Ragdoll Follow HeadIndicator Bruce Thug\n");

    if (!GetSubsystem<Engine>()->IsHeadless())
    {
        SetWindowTitleAndIcon();
        CreateConsoleAndDebugHud();
        CreateUI();
//        InitAudio(); // do it in GameManager
    }
    
    SubscribeToEvents();

//    RADIO::g_gm->Start(); // add gamestates to gamemanager.
    RADIO::g_gm->ChangeState("LoadingState");
}

void Go::CreateConsoleAndDebugHud()
{
    XMLFile* xmlFile = GetSubsystem<ResourceCache>()->GetResource<XMLFile>("UI/DefaultStyle.xml");
    if (!xmlFile)
        return;

    console_ = GetSubsystem<Engine>()->CreateConsole();
    console_->SetDefaultStyle(xmlFile);
    console_->GetBackground()->SetOpacity(0.8f);

    // Create debug HUD
    dHud_ = GetSubsystem<Engine>()->CreateDebugHud();
    dHud_->SetDefaultStyle(xmlFile);
}

void Go::SetWindowTitleAndIcon()
{
    Image* icon = GetSubsystem<ResourceCache>()->GetResource<Image>("Textures/UrhoIcon.png");
    GetSubsystem<Graphics>()->SetWindowIcon(icon);
    GetSubsystem<Graphics>()->SetWindowTitle("test");
}

void Go::CreateUI()
{
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    
    ui->GetRoot()->SetDefaultStyle(xmlFile);
    // Create a Cursor UI element because we want to be able to hide and show it at will. When hidden, the mouse cursor will
    // control the camera, and when visible, it will point the raycast target
    Cursor* cursor = new Cursor(context_);
    cursor->SetStyleAuto(xmlFile);
    ui->SetCursor(cursor);
    cursor->SetVisible(false);

    // debug text can be created in testgamestate.
    // Set starting position of the cursor at the rendering window center
    //cursor.SetPosition(graphics.width / 2, graphics.height / 2);
    //input.SetMouseVisible(true);
    Text* text = ui->GetRoot()->CreateChild<Text>("debug");
    text->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 12);
    text->SetAlignment(HA_LEFT, VA_TOP);
    text->SetPosition(5, (RADIO::game_type == 0) ? 50 : 0);
    text->SetColor(Color(0, 0, 1));
    text->SetPriority(-99999);
    // text.textEffect = TE_SHADOW;
}

void Go::CreateEnemy()
{
    Scene* scene = RADIO::g_gm->GetScene();
    if (!scene)
        return;

    if (!RADIO::g_em)
        return;

    IntVector2 pos = GetSubsystem<UI>()->GetCursorPosition();
    // Check the cursor is visible and there is no UI element in front of the cursor
    if (GetSubsystem<UI>()->GetElementAt(pos, true))
            return;

    Camera* camera = RADIO::g_cm->GetCamera();
    if (!camera)
        return;

    Graphics* gfx = GetSubsystem<Graphics>();
    Ray cameraRay = camera->GetScreenRay(float(pos.x_) / gfx->GetWidth(), float(pos.y_) / gfx->GetHeight());
    float rayDistance = 100.0f;
    PhysicsRaycastResult result;
    scene->GetComponent<PhysicsWorld>()->RaycastSingle(result, cameraRay, rayDistance, COLLISION_LAYER_LANDSCAPE);
    if (!result.body_)
        return;

    if (result.body_->GetNode()->GetName() != "floor")
        return;

    RADIO::g_em->CreateEnemy(result.position_, Quaternion(0, Random(360), 0), "Thug");
}

void Go::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Go, HandleUpdate));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Go, HandlePostRenderUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Go, HandleKeyDown));
//    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(Go, HandleMouseButtonDown));
    SubscribeToEvent(E_ASYNCLOADFINISHED, URHO3D_HANDLER(Go, HandleSceneLoadFinished));
    SubscribeToEvent(E_ASYNCLOADPROGRESS, URHO3D_HANDLER(Go, HandleAsyncLoadProgress));
    SubscribeToEvent("CameraEvent", URHO3D_HANDLER(Go, HandleCameraEvent));
//    SubscribeToEvent("SliderChanged", URHO3D_HANDLER(Go, HandleSliderChanged));
}

void Go::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    float timeStep = eventData[P_TIMESTEP].GetFloat();

    RADIO::g_im->Update(timeStep);
    RADIO::g_cm->Update(timeStep);
    RADIO::g_gm->Update(timeStep);
    
    if(RADIO::tmpdebugvalue < 4)
        RADIO::tmpdebugvalue ++;

    if (GetSubsystem<Engine>()->IsHeadless())
        ExecuteCommand();

    Scene* scn = RADIO::g_gm->GetScene();
    if (!scn)
        return;    

    if (RADIO::drawDebug > 0)
    {
        String seperator = "----------------------------------------------------------------\n";
        String debugText = "";// = seperator;
        debugText += RADIO::g_gm->GetDebugText();
        debugText += seperator;
        debugText += "current LUT: " + RADIO::LUT + "\n";
        debugText += RADIO::g_cm->GetDebugText();
        debugText += RADIO::g_im->GetDebugText();
        debugText += seperator;
        Player* player = RADIO::g_gm->GetPlayer();
        if (player)
            debugText += player->GetDebugText();
        debugText += seperator;

        if (RADIO::drawDebug > 1)
        {
            EnemyManager* em = RADIO::g_em;
            if (em && !em->enemyList.Empty())
            {
                debugText += em->enemyList[0]->GetDebugText();
                debugText += seperator;
            }
        }
        // возможно нужно хранить текст элемент.
        Text* text = static_cast <Text*>(GetSubsystem<UI>()->GetRoot()->GetChild("debug", true));
        if (text)
            text->SetText(debugText);
    }
}

void Go::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
//    Log::Write(-1," Go::HandlePostRenderUpdate 1\n");
///*
    Scene* scene = RADIO::g_gm->GetScene();
    if (!scene)
        return;

    DebugRenderer* debug = scene->GetComponent<DebugRenderer>();
    if (!debug || RADIO::drawDebug == 0)
        return;

    if (RADIO::drawDebug > 0)
    {
        //RADIO::g_cm->DebugDraw(debug);
        debug->AddNode(scene, 1.0f, false);
        RADIO::g_lw->DebugDraw(debug);
        Player* player = RADIO::g_gm->GetPlayer();
        if (player)
            player->DebugDraw(debug);
    }
    if (RADIO::drawDebug > 1)
    {
        if (RADIO::g_em)
            RADIO::g_em->DebugDraw(debug);
    }
    if (RADIO::drawDebug > 2)
        scene->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
//*/    
//    Log::Write(-1," Go::HandlePostRenderUpdate 2\n");
}

void Go::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;

    int key = eventData[P_KEY].GetInt();
    
    RADIO::g_gm->OnKeyDown(key);

    if (key == KEY_ESCAPE)
    {
        if(!console_->IsVisible())
            engine_->Exit();
        else
            console_->SetVisible(false);
    }
//
    Scene* scene = RADIO::g_gm->GetScene();

    if (key == KEY_F1)
    {
        RADIO::drawDebug++;
        if (RADIO::drawDebug > 3)
            RADIO::drawDebug = 0;
        
        // нужно хранить элемент или просто взять как UIElement.
        //Text* txt = static_cast <Text*>(GetSubsystem<UI>()->GetRoot()->GetChild("debug", true));
        UIElement* txt = GetSubsystem<UI>()->GetRoot()->GetChild("debug", true);
        if (txt)
            txt->SetVisible(RADIO::drawDebug != 0);
    }

    else if (key == KEY_F2)
        dHud_->ToggleAll();
    else if (key == KEY_F3)
        console_->Toggle();
    else if (key == KEY_F4)
    {
        Camera* cam = RADIO::g_cm->GetCamera();
        if (cam)
            cam->SetFillMode((cam->GetFillMode() == FILL_SOLID) ? FILL_WIREFRAME : FILL_SOLID);
    }
/*
    else if (key == KEY_F5)
        ToggleDebugWindow();
    else if (key == KEY_1)
        ShootSphere(scene_);
    else if (key == KEY_2)
        ShootBox(scene_);
*/
    else if (key == KEY_3)
        CreateEnemy();//(scene_);

    else if (key == KEY_4)
    {
        CameraController* cc = RADIO::g_cm->currentController;
        if(!cc)
            return;
        if (cc->nameHash == StringHash("Debug"))
        {
            GetSubsystem<UI>()->GetCursor()->SetVisible(false);
            RADIO::g_cm->SetCameraController("ThirdPerson");
        }
        else
        {
            GetSubsystem<UI>()->GetCursor()->SetVisible(true);
            RADIO::g_cm->SetCameraController("Debug");
        }
    }
    else if (key == KEY_5)
    {
        VariantMap data;
        data[TARGET_FOV] = 60;
        SendEvent("CameraEvent", data);
    }
/*
    else if (key == KEY_6)
    {
        colorGradingIndex ++;
        SetColorGrading(colorGradingIndex);
    }
    else if (key == KEY_7)
    {
        colorGradingIndex --;
        SetColorGrading(colorGradingIndex);
    }
    else if (key == 'R')
        scene_.updateEnabled = !scene_.updateEnabled;
    else if (key == 'T')
    {
        if (scene_.timeScale >= 0.999f)
            scene_.timeScale = 0.1f;
        else
            scene_.timeScale = 1.0f;
    }
    else if (key == 'Q')
        engine.Exit();
    else if (key == 'J')
        TestAnimations_Group_2();
    else if (key == 'K')
        TestAnimations_Group_3();
    else if (key == 'L')
        TestAnimations_Group_4();
    else if (key == 'H')
        TestAnimations_Group_Beat();
    else if (key == 'E')
    {
        Array<String> testAnimations;
        //String testName = "TG_Getup/GetUp_Back";
        //String testName = "TG_BM_Counter/Counter_Leg_Front_01";
        //String testName = "TG_HitReaction/Push_Reaction";
        //String testName = "BM_TG_Beatdown/Beatdown_Strike_End_01";
        //String testName = "TG_HitReaction/HitReaction_Back_NoTurn";
        //String testName = "BM_Attack/Attack_Far_Back_04";
        //String testName = "TG_BM_Counter/Double_Counter_2ThugsB_01";
        //String testName = "BM_Attack/Attack_Far_Back_03";
        //String testName = "BM_Climb/Stand_Climb_Up_256_Hang";
        //String testName = GetAnimationName("BM_Railing/Railing_Idle");
        //String testName = ("BM_Railing/Railing_Climb_Down_Forward");
        //String testName = "BM_Climb/Stand_Climb_Up_256_Hang";
        String testName = "BM_Climb/Dangle_To_Hang"; //"BM_Climb/Walk_Climb_Down_128"; //"BM_Climb/Stand_Climb_Up_256_Hang";
        Player@ player = GetPlayer();
        testAnimations.Push(testName);
        //testAnimations.Push("BM_Climb/Dangle_Right");
        // testAnimations.Push(GetAnimationName("BM_Railing/Railing_Run_Forward_Idle"));
        if (player !is null)
            player.TestAnimation(testAnimations);
    }
    else if (key == 'F')
    {
        scene_.timeScale = 1.0f;
        // SetWorldTimeScale(scene_, 1);
    }
    else if (key == 'O')
    {
        Node@ n = scene_.GetChild("thug2");
        if (n !is null)
        {
            n.vars[ANIMATION_INDEX] = RandomInt(4);
            Thug@ thug = cast<Thug>(n.scriptObject);
            thug.ChangeState("HitState");
        }
    }
    else if (key == 'I')
    {
        Player@ p = GetPlayer();
        if (p !is null)
            p.SetPhysicsType(1 - p.physicsType);
    }
    else if (key == 'M')
    {
        Player@ p = GetPlayer();
        if (p !is null)
        {
            Print("------------------------------------------------------------");
            for (uint i=0; i<p.stateMachine.states.length; ++i)
            {
                State@ s = p.stateMachine.states[i];
                Print("name=" + s.name + " nameHash=" + s.nameHash.ToString());
            }
            Print("------------------------------------------------------------");
        }
    }
    else if (key == 'U')
    {
        Player@ p = GetPlayer();
        if (p.timeScale > 1.0f)
            p.timeScale = 1.0f;
        else
            p.timeScale = 1.25f;
    }
*/
}

void Go::HandleAsyncLoadProgress(StringHash eventType, VariantMap& eventData)
{
    Log::Write(-1," Go::HandleAsyncLoadProgress 1\n");

    using namespace AsyncLoadProgress;
    
    // Node* nodeA = static_cast<Node*>(eventData[P_NODEA].GetPtr());
    Scene* _scene = static_cast<Scene*>(eventData[P_SCENE].GetPtr());
    
    float progress = eventData[P_PROGRESS].GetFloat();
    int loadedNodes = eventData[P_LOADEDNODES].GetInt();
    int totalNodes = eventData[P_TOTALNODES].GetInt();
    int loadedResources = eventData[P_LOADEDRESOURCES].GetInt();
    int totalResources = eventData[P_TOTALRESOURCES].GetInt();

    RADIO::g_gm->OnAsyncLoadProgress(_scene, progress, loadedNodes, totalNodes, loadedResources, totalResources);

    Log::Write(-1," Go::HandleAsyncLoadProgress 2\n");
}

void Go::HandleSceneLoadFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace AsyncLoadFinished;
    
    // Node* nodeA = static_cast<Node*>(eventData[P_NODEA].GetPtr());
    Scene* _scene = static_cast<Scene*>(eventData[P_SCENE].GetPtr());
    RADIO::g_gm->OnSceneLoadFinished(_scene);
}

void Go::HandleCameraEvent(StringHash eventType, VariantMap& eventData)
{
    Log::Write(-1," Go: HandleCameraEvent\n");
    RADIO::g_cm->OnCameraEvent(eventData);
}

void Go::ExecuteCommand()
{
    
}


