#pragma once

#include <Urho3D/Urho3DAll.h>
#include "Cfg/Cfg.h"

class CfgManager : public Object
{
    URHO3D_OBJECT(CfgManager, Object);

public:

    CfgManager(Context* context);
    ~CfgManager();

    void Initialize();
    void Finalize();
    void AddCfg(String path_file, ConfigType type);
    Cfg* GetCfg_Hash(const StringHash& fh, ConfigType type);
    Cfg* GetCfg(const String& file, ConfigType type);
    Cfg* GetCfg(const String& file);

protected:
    bool finalized;
    Vector<Cfg*> gameConfigs_;
    Vector<Cfg*> levelConfigs_;
    Vector<Cfg*> tmpConfigs_;

};
