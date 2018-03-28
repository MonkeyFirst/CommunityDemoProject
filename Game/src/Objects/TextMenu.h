#pragma once

#include <Urho3D/Urho3DAll.h>

const String UI_FONT = "Fonts/GAEN.ttf";
const int UI_FONT_SIZE = 20;

class TextMenu : public Object
{
    URHO3D_OBJECT(TextMenu, Object);

public:
    TextMenu(Context* context, const String& fName, int fSize);
    ~TextMenu();

    void Add();
    void Destroy();
    void AddText(const String& str);
    int Update(float dt);
    void ChangeSelection(int index);

    //UIElement* menuroot;
    SharedPtr<UIElement> menuroot;
    Vector<String> texts;
    Vector<Text*> items;
    String fontName;
    int fontSize;
    int selection;
    Color highLightColor;
    Color normalColor;
    IntVector2 size;
    unsigned int lastDirectionKeyTime;
};