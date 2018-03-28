#pragma once

#include <Urho3D/Urho3DAll.h>

class InputManager : public Object
{
    URHO3D_OBJECT(InputManager, Object);

public:
    InputManager(Context* context);
    ~InputManager();

    void Initialize();
    void Finalize();

    void Update(float dt);
    Vector3 GetLeftAxis();
    Vector3 GetRightAxis();
    float GetLeftAxisAngle();
    int GetLeftAxisHoldingFrames();
    float GetLeftAxisHoldingTime();
    Vector2 GetLeftStick();
    Vector2 GetRightStick();
    JoystickState* GetJoystick();
    bool HasLeftStickBeenStationary(float value);
    bool IsLeftStickStationary();
    bool IsLeftStickInDeadZone();
    bool IsRightStickInDeadZone();
    bool IsAttackPressed();
    bool IsCounterPressed();
    bool IsEvadePressed();
    bool IsEnterPressed();
    bool IsDistractPressed();
    bool IsCrouchPressed();
    bool IsCrouchDown();
    bool IsActionPressed();
    int GetDirectionPressed();
    bool IsRunHolding();
    String GetDebugText();

    Input* input_;

    bool freezeInput_;
//    int tmpdebugvalue_;

    float m_leftStickX;
    float m_leftStickY;
    float m_leftStickMagnitude;
    float m_leftStickAngle;

    float m_rightStickX;
    float m_rightStickY;
    float m_rightStickMagnitude;

    float m_lastLeftStickX;
    float m_lastLeftStickY;
    float m_leftStickHoldTime;

    float m_smooth;

    float mouseSensitivity;
    float joySensitivity;
    float joyLookDeadZone;

    int   m_leftStickHoldFrames;
    unsigned int  lastMiddlePressedTime;

    bool  flipRightStick;
};