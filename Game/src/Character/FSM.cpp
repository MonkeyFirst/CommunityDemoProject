
#include "Character/FSM.h"
//#include "Levels/GameManager.h"

#include "radio.h"

State::State(Context* context) :
    Object(context)
{
//    Log::Write(-1," State: Constructor\n");
}

State::~State()
{
//    Log::Write(-1," State: Desstructor\n");
}

void State::Enter(State* lastState)
{
//    Log::Write(-1," State: Enter\n");
    timeInState = 0;
}

void State::Exit(State* nextState)
{
//    Log::Write(-1," State: Exit\n");
    timeInState = 0;
}

void State::Update(float dt)
{
    timeInState += dt;
}

void State::FixedUpdate(float dt)
{

}

void State::DebugDraw(DebugRenderer* debug)
{

}

String State::GetDebugText()
{
    return " name=" + name + " timeInState=" + String(timeInState) + "\n";
}

void State::SetName(const String& s)
{
    name = s;
    nameHash = StringHash(s);
}

bool State::CanReEntered()
{
    return false;
}
// ============================================

FSM::FSM(Context* context) :
    Object(context)
{
    //URHO3D_LOGINFO("FSM()");
}

FSM::~FSM()
{
    /*
    if (currentState)
        URHO3D_LOGINFO("~FSM() currentState=" + currentState.name);
    else
        URHO3D_LOGINFO("~FSM()");
    */
    currentState = NULL;
    
    for (unsigned int i = 0; i < states.Size(); ++i)
    {
        if (states[i])
            states[i].Reset();
    }
    
    states.Clear();
}

void FSM::AddState(State* state)
{
    SharedPtr<State> stateShared(state);
    states.Push(stateShared);
}

State* FSM::FindState(const String& name)
{
    return FindState_Hash(StringHash(name));
}

State* FSM::FindState_Hash(const StringHash& nameHash)
{
    for (unsigned int i = 0; i < states.Size(); ++i)
    {
        if (states[i]->nameHash == nameHash)
            return states[i];
    }
    //Log::Write(-1," FSM: FindState_Hash - State " + nameHash.ToString() + " not found in states vector\n");
    return NULL;
}

bool FSM::ChangeState_Hash(const StringHash& nameHash)
{
//Log::Write(-1," zzzzzzzzzzzzzzzzzzzz\n");
    State* newState = FindState_Hash(nameHash);

    if (!newState)
    {
//        Log::Write(-1," FSM: new-state not found " + nameHash.ToString()+ "\n");
        return false;
    }

    if (currentState == newState) 
    {
        // URHO3D_LOGINFO("same state !!!");
        if (!currentState->CanReEntered())
            return false;
        currentState->Exit(newState);
        currentState->Enter(newState);
    }

    State* oldState = currentState;
    if (oldState)
        oldState->Exit(newState);

    if (newState)
        newState->Enter(oldState);

    currentState = newState;

    if (RADIO::d_log)
    {
        String oldStateName = "NULL";
        if (oldState)
            oldStateName = oldState->name;

        String newStateName = "NULL";
        if (newState)
            newStateName = newState->name;
        
        Log::Write(-1," FSM: Change State from " +oldStateName+ " to " +newStateName+ "\n");
    }

    return true;
}

bool FSM::ChangeState(const String& name)
{
    return ChangeState_Hash(StringHash(name));
}

void FSM::ChangeStateQueue(const StringHash& name)
{
    queueState = name;
}

void FSM::Update(float dt)
{
    if (currentState)
        currentState->Update(dt);

    if (queueState != StringHash::ZERO)
    {
        ChangeState_Hash(queueState);
        queueState = StringHash::ZERO; //0;
    }
}

void FSM::FixedUpdate(float dt)
{
    if (currentState)
        currentState->FixedUpdate(dt);
}

void FSM::DebugDraw(DebugRenderer* debug)
{
    if (currentState)
        currentState->DebugDraw(debug);
}

String FSM::GetDebugText()
{
    String ret = " current-state: ";
    if (currentState)
        ret += currentState->GetDebugText();
    else
        ret += " NULL\n";
    return ret;
}
