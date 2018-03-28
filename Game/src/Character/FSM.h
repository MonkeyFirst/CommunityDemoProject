#pragma once

#include <Urho3D/Urho3DAll.h>

class State : public Object
{
    URHO3D_OBJECT(State, Object);

public:
    State(Context* context);
    virtual ~State();

    virtual void Enter(State* lastState);
    virtual void Exit(State* nextState);
    virtual void Update(float dt);
    virtual void FixedUpdate(float dt);
    virtual void DebugDraw(DebugRenderer* debug);
    virtual String GetDebugText();
    virtual void SetName(const String& s);
    virtual bool CanReEntered();

    String name;
    StringHash nameHash;
    float timeInState;
};

class FSM : public Object
{
    URHO3D_OBJECT(FSM, Object);

public:
    FSM(Context* context);
    ~FSM();

    void AddState(State* state);
    State* FindState(const String& name);
    State* FindState_Hash(const StringHash& nameHash);
    bool ChangeState_Hash(const StringHash& nameHash);
    bool ChangeState(const String& name);
    void ChangeStateQueue(const StringHash& name);
    void Update(float dt);
    void FixedUpdate(float dt);
    void DebugDraw(DebugRenderer* debug);
    String GetDebugText();

    //Vector<State*> states;
    Vector<SharedPtr<State> > states;
    WeakPtr<State> currentState;
    StringHash queueState;
};