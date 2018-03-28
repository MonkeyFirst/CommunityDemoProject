
#include <Urho3D/Urho3DAll.h>

#include "Objects/TextMenu.h"


TextMenu::TextMenu(Context* context, const String& fName, int fSize) :
    Object(context)
{
    Log::Write(-1," TextMenu: Constructor\n");

    selection = 0;
    highLightColor = Color(1, 1, 0);
    normalColor = Color(1, 0, 0);
    size = IntVector2(400, 100);
    lastDirectionKeyTime = 0;

    fontName = fName;
    fontSize = fSize;
}

TextMenu::~TextMenu()
{}

void TextMenu::Add()
{
    if (menuroot) // UIElement* если уже существует.
        return;

    UI* ui = GetSubsystem<UI>();
    Graphics* graphics = GetSubsystem<Graphics>();
    
    menuroot = ui->GetRoot()->CreateChild<UIElement>(); ///("UIElement");
    if (!GetSubsystem<Engine>()->IsHeadless()) //  engine.headless)
    {
        int height = graphics->GetHeight() / 22; //.height / 22;
        if (height > 64)
            height = 64;

        menuroot->SetAlignment(HA_CENTER, VA_CENTER);
        menuroot->SetPosition(0, -height * 2);
    }

    menuroot->SetLayout(LM_VERTICAL, 8);
    menuroot->SetFixedSize(size.x_, size.y_);

    for (unsigned int i = 0; i < texts.Size(); ++i) //length; ++i)
    {
        AddText(texts[i]);
    }

    items[selection]->SetColor(highLightColor); // .color = highLightColor;
    lastDirectionKeyTime = GetSubsystem<Time>()->GetSystemTime(); //time.systemTime;
    
    Log::Write(-1," TextMenu: Add Complete\n");
}

void TextMenu::Destroy()
{
//    Log::Write(-1," TextMenu: Destroy menuroot UIElement\n");

    if (!menuroot)
    {
//        Log::Write(-1," TextMenu: Destroy END\n");
        return;
    }
    else
    {
        items.Clear(); // Vector<Text*> items
//        Log::Write(-1," TextMenu: Before Remove menuroot\n");
        menuroot->Remove();
        menuroot = NULL;
        
//        Log::Write(-1," TextMenu: Destroy menuroot UIElement END\n");
    }
}

void TextMenu::AddText(const String& str)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    //Text* text = menuroot->CreateChild("Text");
    Text* text = new Text(context_);
    menuroot->AddChild(text);
    text->SetFont(cache->GetResource<Font>(fontName), fontSize);
    text->SetText(str); //text = str;
    text->SetColor(normalColor);//color = normalColor;
    items.Push(text);
}

int TextMenu::Update(float dt)
{
/*
    int selIndex = selection;
    int inputDirection = gInput.GetDirectionPressed();
    if (inputDirection >= 0)
    {
        unsigned int time_diff = time.systemTime - lastDirectionKeyTime;
        if (time_diff < 200)
            inputDirection = -1;
        else
            lastDirectionKeyTime = time.systemTime;
    }

    if (inputDirection == 0)
        selIndex --;
    if (inputDirection == 1)
        selIndex ++;
    if (inputDirection == 2)
        selIndex ++;
    if (inputDirection == 3)
        selIndex --;

    if (selIndex >= int(items.length))
        selIndex = 0;
    if (selIndex < 0)
        selIndex = int(items.length) - 1;

    ChangeSelection(selIndex);
    return gInput.IsEnterPressed() ? selection : -1;
    */
}

void TextMenu::ChangeSelection(int index)
{
    if (selection == index)
        return;

    if (selection >= 0)
        items[selection]->SetColor(normalColor); //color = normalColor;

    selection = index;
    if (selection >= 0)
        items[selection]->SetColor(highLightColor); //color = highLightColor;
}



