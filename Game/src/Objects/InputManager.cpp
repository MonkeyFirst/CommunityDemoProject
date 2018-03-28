// ==============================================
//
//    Input Processing Class
//
//
//    Joystick: 0 -> A 1 -> B 2 -> X 3 -> Y
//
//
// ==============================================

#include <Urho3D/Urho3DAll.h>

#include "Objects/InputManager.h"

#include "radio.h"

bool  freezeInput = false;

InputManager::InputManager(Context* context):
    Object(context)
{

}

InputManager::~InputManager()
{}

void InputManager::Initialize()
{
    input_ = GetSubsystem<Input>();

    m_smooth = 0.9f;
    mouseSensitivity = 0.125f;
    joySensitivity = 0.75;
    joyLookDeadZone = 0.05;
    m_leftStickHoldFrames = 0;
    flipRightStick = false;
    freezeInput_ = false;
    
    lastMiddlePressedTime = 0;
    
    JoystickState* js = GetJoystick();
    if (js)
    {
        URHO3D_LOGINFO("found a joystick " + js->name_ + " numHats=" + String(js->GetNumHats()) + " numAxes=" + String(js->GetNumAxes()) + " numButtons=" + String(js->GetNumButtons()));
        if (js->GetNumHats() == 1)
            flipRightStick = true;
    }
}

void InputManager::Finalize()
{
    input_ = NULL;
}

void InputManager::Update(float dt)
{

    if(RADIO::tmpdebugvalue <= 3)
    {
        Log::Write(-1," InputManager: Update " + String(RADIO::tmpdebugvalue) + " (3)\n");
    }

    if (input_->IsMouseVisible())   // mouseVisible)
        return;

    m_lastLeftStickX = m_leftStickX;
    m_lastLeftStickY = m_leftStickY;

    Vector2 leftStick = GetLeftStick();
    Vector2 rightStick = GetRightStick();

    m_leftStickX = Lerp(m_leftStickX, leftStick.x_, m_smooth);
    m_leftStickY = Lerp(m_leftStickY, leftStick.y_, m_smooth);
    m_rightStickX = rightStick.x_; //Lerp(m_rightStickX, rightStick.x, m_smooth);
    m_rightStickY = rightStick.y_; //Lerp(m_rightStickY, rightStick.y, m_smooth);

    m_leftStickMagnitude = m_leftStickX * m_leftStickX + m_leftStickY * m_leftStickY;
    m_rightStickMagnitude = m_rightStickX * m_rightStickX + m_rightStickY * m_rightStickY;

    m_leftStickAngle = Atan2(m_leftStickX, m_leftStickY);

    float diffX = m_lastLeftStickX - m_leftStickX;
    float diffY = m_lastLeftStickY - m_leftStickY;
    float stickDifference = diffX * diffX + diffY * diffY;

    if(stickDifference < 0.1f)
    {
        m_leftStickHoldTime += dt;
        ++m_leftStickHoldFrames;
    }
    else
    {
        m_leftStickHoldTime = 0;
        m_leftStickHoldFrames = 0;
    }

    if (input_->GetMouseButtonPress(MOUSEB_MIDDLE))//.mouseButtonPress[MOUSEB_MIDDLE])
        lastMiddlePressedTime = Time::GetSystemTime();
    // URHO3D_LOGINFO("m_leftStickX=" + String(m_leftStickX) + " m_leftStickY=" + String(m_leftStickY));
}

Vector3 InputManager::GetLeftAxis()
{
    return Vector3(m_leftStickX, m_leftStickY, m_leftStickMagnitude);
}

Vector3 InputManager::GetRightAxis()
{
    return Vector3(m_rightStickX, m_rightStickY, m_rightStickMagnitude);
}

float InputManager::GetLeftAxisAngle()
{
    return m_leftStickAngle;
}

int InputManager::GetLeftAxisHoldingFrames()
{
    return m_leftStickHoldFrames;
}

float InputManager::GetLeftAxisHoldingTime()
{
    return m_leftStickHoldTime;
}

Vector2 InputManager::GetLeftStick()
{
    Vector2 ret;
    JoystickState* joystick = GetJoystick();
    if (joystick)
    {
        if (joystick->GetNumAxes() >= 2)
        {
            ret.x_ = joystick->GetAxisPosition(0); //axisPosition[0];
            ret.y_ = -joystick->GetAxisPosition(1); //axisPosition[1];

            if (Abs(ret.x_) < 0.01)
                ret.x_ = 0.0f;
            if (Abs(ret.y_) < 0.01)
                ret.y_ = 0.0f;
        }
    }
    else
    {
        if (input_->GetKeyDown(KEY_W)) // keyDown['W'])
            ret.y_ += 1.0f;
        if (input_->GetKeyDown(KEY_S)) // keyDown['S'])
            ret.y_ -= 1.0f;
        if (input_->GetKeyDown(KEY_D)) // keyDown['D'])
            ret.x_ += 1.0f;
        if (input_->GetKeyDown(KEY_A)) // keyDown['A'])
            ret.x_ -= 1.0f;
    }
    return ret;
}

Vector2 InputManager::GetRightStick()
{
    JoystickState* joystick = GetJoystick();
    Vector2 rightAxis = Vector2(m_rightStickX, m_rightStickY);

    if (joystick)
    {
        if (joystick->GetNumAxes() >= 4)
        {
            float lookX = joystick->GetAxisPosition(2); //axisPosition[2];
            float lookY = joystick->GetAxisPosition(3); //axisPosition[3];
            if (flipRightStick)
            {
                lookX = joystick->GetAxisPosition(3); //axisPosition[3];
                lookY = joystick->GetAxisPosition(2); //axisPosition[2];
            }

            if (lookX < -joyLookDeadZone)
                rightAxis.x_ -= joySensitivity * lookX * lookX;
            if (lookX > joyLookDeadZone)
                rightAxis.x_ += joySensitivity * lookX * lookX;
            if (lookY < -joyLookDeadZone)
                rightAxis.y_ -= joySensitivity * lookY * lookY;
            if (lookY > joyLookDeadZone)
                rightAxis.y_ += joySensitivity * lookY * lookY;
        }
    }
    else
    {
        rightAxis.x_ += mouseSensitivity * input_->GetMouseMoveX(); // mouseMoveX;
        rightAxis.y_ += mouseSensitivity * input_->GetMouseMoveY(); // mouseMoveY;
    }
    return rightAxis;
}

JoystickState* InputManager::GetJoystick()
{
    if (input_->GetNumJoysticks() > 0)
    {
        return input_->GetJoystickByIndex(0); // joysticksByIndex[0];
    }
    return NULL;
}

// Returns true if the left game stick hasn't moved in the given time frame
bool InputManager::HasLeftStickBeenStationary(float value)
{
    return m_leftStickHoldTime > value;
}

// Returns true if the left game pad hasn't moved since the last update
bool InputManager::IsLeftStickStationary()
{
    return HasLeftStickBeenStationary(0.01f);
}

// Returns true if the left stick is the dead zone, false otherwise
bool InputManager::IsLeftStickInDeadZone()
{
    return m_leftStickMagnitude < 0.1;
}

// Returns true if the right stick is the dead zone, false otherwise
bool InputManager::IsRightStickInDeadZone()
{
    return m_rightStickMagnitude < 0.1;
}

bool InputManager::IsAttackPressed()
{
    if (freezeInput)
        return false;

    JoystickState* joystick = GetJoystick();
    if (joystick)
        return joystick->GetButtonPress(2); // buttonPress[2];
    else
        return input_->GetMouseButtonPress(MOUSEB_LEFT); // mouseButtonPress[MOUSEB_LEFT];
}

bool InputManager::IsCounterPressed()
{
    if (freezeInput)
        return false;

    JoystickState* joystick = GetJoystick();
    if (joystick)
        return joystick->GetButtonPress(3); // buttonPress[3];
    else
        return input_->GetMouseButtonPress(MOUSEB_RIGHT); // mouseButtonPress[MOUSEB_RIGHT];
}

bool InputManager::IsEvadePressed()
{
    if (freezeInput)
        return false;

    JoystickState* joystick = GetJoystick();
    if (joystick)
        return joystick->GetButtonPress(0); // buttonPress[0];
    else
        return input_->GetKeyPress(KEY_SPACE); // keyPress[KEY_SPACE];
}

bool InputManager::IsEnterPressed()
{
    JoystickState* joystick = GetJoystick();
    if (joystick)
    {
        if (joystick->GetButtonPress(2)) // buttonPress[2])
            return true;
    }
    return input_->GetKeyPress(KEY_RETURN) || input_->GetKeyPress(KEY_SPACE) || input_->GetMouseButtonPress(MOUSEB_LEFT); // mouseButtonPress[MOUSEB_LEFT];
}

bool InputManager::IsDistractPressed()
{
    JoystickState* joystick = GetJoystick();
    if (joystick)
        return joystick->GetButtonPress(1); // buttonPress[1];
    else
        return input_->GetMouseButtonPress(MOUSEB_MIDDLE);
}

bool InputManager::IsCrouchPressed()
{
    JoystickState* joystick = GetJoystick();
    if (joystick)
        return joystick->GetButtonPress(1); // buttonPress[1];
    else
        return input_->GetKeyPress(KEY_LCTRL); // keyPress[KEY_LCTRL];
}

bool InputManager::IsCrouchDown()
{
    JoystickState* joystick = GetJoystick();
    if (joystick)
        return joystick->GetButtonDown(1); // buttonDown[1];
    else
        return input_->GetKeyDown(KEY_LCTRL); // keyDown[KEY_LCTRL];
}

bool InputManager::IsActionPressed()
{
    if (freezeInput)
        return false;

    JoystickState* joystick = GetJoystick();
    if (joystick)
        return joystick->GetButtonPress(0); // buttonPress[0];
    else
        return input_->GetKeyPress(KEY_SPACE); // keyPress[KEY_SPACE];
}

int InputManager::GetDirectionPressed()
{
    JoystickState* joystick = GetJoystick();
    if (joystick)
    {
        if (m_lastLeftStickY > 0.333f)
            return 0;
        else if (m_lastLeftStickX > 0.333f)
            return 1;
        else if (m_lastLeftStickY < -0.333f)
            return 2;
        else if (m_lastLeftStickX < -0.333f)
            return 3;
    }

    if (input_->GetKeyDown(KEY_UP)) // keyDown[KEY_UP])
        return 0;
    else if (input_->GetKeyDown(KEY_RIGHT)) // keyDown[KEY_RIGHT])
        return 1;
    else if (input_->GetKeyDown(KEY_DOWN)) // keyDown[KEY_DOWN])
        return 2;
    else if (input_->GetKeyDown(KEY_LEFT)) // keyDown[KEY_LEFT])
        return 3;

    return -1;
}

bool InputManager::IsRunHolding()
{
    JoystickState* joystick = GetJoystick();
    if (joystick)
        return joystick->GetButtonDown(4); // buttonDown[4];
    return input_->GetKeyDown(KEY_LSHIFT); // keyDown[KEY_LSHIFT];
}

String InputManager::GetDebugText()
{
    String ret =   "leftStick:(" + String(m_leftStickX) + "," + String(m_leftStickY) + ")" +
                   " left-angle=" + String(m_leftStickAngle) + " hold-time=" + String(m_leftStickHoldTime) + " hold-frames=" + String(m_leftStickHoldFrames) + " left-magnitude=" + String(m_leftStickMagnitude) +
                   " rightStick:(" + String(m_rightStickX) + "," + String(m_rightStickY) + ")\n";

    JoystickState* joystick = GetJoystick();
    if (joystick)
    {
        ret += "joystick button--> 0=" + String(joystick->GetButtonDown(0)) + " 1=" + String(joystick->GetButtonDown(1)) + " 2=" + String(joystick->GetButtonDown(2)) + " 3=" + String(joystick->GetButtonDown(3)) + "\n";
        ret += "joystick axis--> 0=" + String(joystick->GetAxisPosition(0)) + " 1=" + String(joystick->GetAxisPosition(1)) + " 2=" + String(joystick->GetAxisPosition(2)) + " 3=" + String(joystick->GetAxisPosition(3)) + "\n";
    }

    return ret;
}

