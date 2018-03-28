
//#include "Precompiled.h"
#include "App.h"
#include <Engine/Engine.h>
#include <IO/IOEvents.h>
#include <IO/Log.h>
#include <Core/ProcessUtils.h>

#include <exception>

#include <DebugNew.h>

using namespace Urho3D;
    
App::App(Context* context) :
    Object(context),
    exitCode_(EXIT_SUCCESS)
{
    engineParameters_ = Engine::ParseParameters(GetArguments());

    // Create the Engine, but do not initialize it yet. Subsystems except Graphics & Renderer are registered at this point
    engine_ = new Engine(context);

    // Subscribe to log messages so that can show errors if ErrorExit() is called with empty message
    SubscribeToEvent(E_LOGMESSAGE, URHO3D_HANDLER(App, HandleLogMessage));
}

int App::Run()
{
    try
    {
        Setup();
        if (exitCode_)
            return exitCode_;

        if (!engine_->Initialize(engineParameters_))
        {
            ErrorExit();
            return exitCode_;
        }

        Start();
        if (exitCode_)
            return exitCode_;

        // Platforms other than iOS run a blocking main loop
        while (!engine_->IsExiting())
            engine_->RunFrame();

        Stop();

        return exitCode_;
    }
    catch (std::bad_alloc&)
    {
        ErrorDialog(GetTypeName(), "An out-of-memory error occurred. The application will now exit.");
        return EXIT_FAILURE;
    }
}

void App::ErrorExit(const String& message)
{
    engine_->Exit(); // Close the rendering window
    exitCode_ = EXIT_FAILURE;

    // Only for WIN32, otherwise the error messages would be double posted on Mac OS X and Linux platforms
    if (!message.Length())
    {
        #ifdef WIN32
        ErrorDialog(GetTypeName(), startupErrors_.Length() ? startupErrors_ :
            "App has been terminated due to unexpected error.");
        #endif
    }
    else
        ErrorDialog(GetTypeName(), message);
}

void App::HandleLogMessage(StringHash eventType, VariantMap& eventData)
{
    using namespace LogMessage;
    
    if (eventData[P_LEVEL].GetInt() == LOG_ERROR)
    {
        // Strip the timestamp if necessary
        String error = eventData[P_MESSAGE].GetString();
        unsigned bracketPos = error.Find(']');
        if (bracketPos != String::NPOS)
            error = error.Substring(bracketPos + 2);
        
        startupErrors_ += error + "\n";
    }
}

