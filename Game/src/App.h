
#pragma once

#include <Core/Context.h>
#include <Core/Object.h>

namespace Urho3D
{

class Engine;
//class Node;
//class Scene;

}

using namespace Urho3D;

/// Base class for creating applications which initialize the Urho3D engine and run a main loop until exited.
class App : public Object
{
    URHO3D_OBJECT(App, Object);
    
public:
    // Construct. Parse default engine parameters from the command line, 
    // and create the engine in an uninitialized state.
    App(Context* context);

    // Setup before engine initialization. This is a chance to eg. modify the engine parameters. 
    // Call ErrorExit() to terminate without initializing the engine. Called by App.
    virtual void Setup() {}
    // Setup after engine initialization and before running the main loop. 
    // Call ErrorExit() to terminate without running the main loop. Called by App.
    virtual void Start() {}
    // Cleanup after the main loop. Called by App.
    virtual void Stop() {}

    // Initialize the engine and run the main loop, then return the application exit code. 
    // Catch out-of-memory exceptions while running.
    int Run();
    // Show an error message (last log message if empty), 
    // terminate the main loop, and set failure exit code.
    void ErrorExit(const String& message = String::EMPTY);

protected:
    // Handle log message.
    void HandleLogMessage(StringHash eventType, VariantMap& eventData);
    
    // Urho3D engine.
    SharedPtr<Engine> engine_;
    // Engine parameters map.
    VariantMap engineParameters_;
    // Collected startup error log messages.
    String startupErrors_;
    // App exit code.
    int exitCode_;
};

