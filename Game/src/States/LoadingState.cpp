
#include <Urho3D/Urho3DAll.h>

#include "States/GameState.h"
//#include "Levels/LevelEvents.h"
#include "States/LoadingState.h"

//#include "IMUI/IMUI.h"

#include "States/GameManager.h"
#include "Motions/MotionManager.h"

#include "radio.h"

LoadingState::LoadingState(Context* context) :
GameState(context)
{
    Log::Write(-1," LoadingState: Constructor\n");
    load_state = -1;
    numLoadedResources = 0;
    GameState::SetName("LoadingState");
}

LoadingState::~LoadingState()
{
    Log::Write(-1," LoadingState: Destructor\n");
}

Player* LoadingState::GetPlayer()
{
    return NULL;
}

Scene* LoadingState::GetScene()
{
//    Log::Write(-1," LoadingState::GetScene()\n");
//    if (!preloadScene)
        return NULL;
//    else
//        return preloadScene;
}

void LoadingState::CreateLoadingUI()
{
    float alphaDuration = 1.0f;
    ValueAnimation* alphaAnimation = new ValueAnimation(context_);
    alphaAnimation->SetKeyFrame(0.0f, Variant(0.0f));
    alphaAnimation->SetKeyFrame(alphaDuration, Variant(1.0f));
    alphaAnimation->SetKeyFrame(alphaDuration * 2, Variant(0.0f));

    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Graphics* graphics = GetSubsystem<Graphics>();

    txt_ = ui->GetRoot()->CreateChild<Text>("loading_text");
    txt_->SetFont(cache->GetResource<Font>(RADIO::LOAD_FONT), RADIO::LOAD_FONT_SIZE);
    txt_->SetAlignment(HA_LEFT, VA_BOTTOM);
    txt_->SetPosition(2, 0);
    //txt_->SetStyleAuto();
    txt_->SetColor(Color(1, 1, 1));// Color(1, 1, 1);
    txt_->SetTextEffect(TE_STROKE);
    txt_->AddTag("TAG_LOADING");
    txt_->SetText("...");

    Texture2D* loadingTexture = cache->GetResource<Texture2D>("Textures/Loading.tga");
    Sprite* loadingSprite = ui->GetRoot()->CreateChild<Sprite>("loading_bg");
    loadingSprite->SetTexture(loadingTexture);

    int textureWidth = loadingTexture->GetImage()->GetWidth();
    int textureHeight = loadingTexture->GetImage()->GetHeight();
    loadingSprite->SetSize(textureWidth, textureHeight);

    // Get rendering window size as floats
    float width = (float)graphics->GetWidth();
    float height = (float)graphics->GetHeight();
    loadingSprite->SetPosition(width/2 - textureWidth/2, height/2 - textureHeight/2);
//    loadingSprite->priority = -100;
//    loadingSprite->opacity = 0.0f;
    loadingSprite->AddTag("TAG_LOADING");
    loadingSprite->SetAttributeAnimation("Opacity", alphaAnimation);
}

void LoadingState::Enter(GameState* lastState)
{
    Log::Write(-1," LoadingState: Enter\n");
    GameState::Enter(lastState);

    if (!GetSubsystem<Engine>()->IsHeadless())
        CreateLoadingUI();
    ChangeSubState(LOADING_RESOURCES);
}

void LoadingState::Exit(GameState* nextState)
{
    Log::Write(-1," LoadingState: Exit\n");
    GameState::Exit(nextState);

    PODVector<UIElement*> elements;
    GetSubsystem<UI>()->GetRoot()->GetChildrenWithTag(elements, "TAG_LOADING");
    for (unsigned int i = 0; i < elements.Size(); ++i)
        elements[i]->Remove();
    
    if(txt_) // removed in Update function.
        txt_.Reset();
}

void LoadingState::Update(float dt)
{
    if (load_state == LOADING_RESOURCES)
    {
// See OnAsyncLoadProgress function
//        if (txt_)
//            txt_->SetText(" Loading Resources ...\n");
    }
    else if (load_state == LOADING_MOTIONS)
    {
        if (txt_)
            txt_->SetText(" Loading Motions. Loaded: " + String(RADIO::g_mm->processedMotions));

//        if (RADIO::mm_d_log)
//            Log::Write(-1," ========== Motion Loading start ==========\n");

        if (RADIO::g_mm->Update(dt)) // return true when finish
        {
            RADIO::g_mm->Finish();
            ChangeSubState(LOADING_FINISHED);
            if (txt_) // TODO: пустое и ненужное
                txt_->SetText(" Loading Scene Resources ...");
        }

//        if (RADIO::mm_d_log)
//            Log::Write(-1," ========== Motion Loading end ==========\n");
    }
    else if (load_state == LOADING_FINISHED)
    {
        if (preloadScene)
            preloadScene->Remove();
        preloadScene = NULL;

        if (txt_)
            txt_->Remove();
        txt_ = NULL;
        
        RADIO::g_gm->ChangeState("LevelState");
    }
}

void LoadingState::ChangeSubState(int newState)
{
    if (load_state == newState)
        return;

    Log::Write(-1," LoadingState ChangeSubState from " + String(load_state) + " to " + String(newState) + "\n");
    load_state = newState;

    if (newState == LOADING_RESOURCES)
    {
        preloadScene = new Scene(context_);
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        preloadScene->LoadAsyncXML(cache->GetFile("Scenes/animation.xml"), LOAD_RESOURCES_ONLY);
    }
    else if (newState == LOADING_MOTIONS)
    {
        RADIO::g_mm->Run();
    }
}

void LoadingState::OnSceneLoadFinished(Scene* _scene)
{
    if (load_state == LOADING_RESOURCES)
    {
        ChangeSubState(LOADING_MOTIONS);
    }
}

void LoadingState::OnAsyncLoadProgress(Scene* _scene, float progress, int loadedNodes, int totalNodes, int loadedResources, int totalResources)
{
    //txt_ = ui.root.GetChild("loading_text");
    if (txt_)
        txt_->SetText("Loading Scene Resources. Total:" + String(progress) + " Resources:" + String(loadedResources) + "/" + String(totalResources) + "\n");
}

void LoadingState::OnESC()
{
//    if (load_state == LOADING_RESOURCES)
//        preloadScene.StopAsyncLoading();
//    engine.Exit();
}








