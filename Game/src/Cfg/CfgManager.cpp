#include "Cfg/CfgManager.h"
#include "radio.h"

CfgManager::CfgManager(Context* context)
    : Object(context)
{

}

CfgManager::~CfgManager()
{
    if(!finalized)
        Finalize();
}

void CfgManager::Initialize()
{
	RADIO::CFG_ROOT = "";// DT/Cfg/
    AddCfg("go.cfg", CFG_GAME);
}

void CfgManager::Finalize()
{
    for (unsigned int i = 0; i < tmpConfigs_.Size(); ++i)
    {
        if (tmpConfigs_[i])
            delete tmpConfigs_[i]; // auto-save if dirty
    }
    tmpConfigs_.Clear();

    for (unsigned int i = 0; i < levelConfigs_.Size(); ++i)
    {
        if (levelConfigs_[i])
            delete levelConfigs_[i]; // auto-save if dirty
    }
    levelConfigs_.Clear();
    
    for (unsigned int i = 0; i < gameConfigs_.Size(); ++i)
    {
        if (gameConfigs_[i])
            delete gameConfigs_[i]; // auto-save if dirty
    }
    gameConfigs_.Clear();

    finalized = true;
}

void CfgManager::AddCfg(String path_file, ConfigType type)
{
    String fullName = RADIO::CFG_ROOT + path_file;
    // Auto load file from disk.
    Cfg* cfg = new Cfg(context_, fullName);
    cfg->fileName = fullName;
    cfg->fileHash = StringHash(fullName);

    switch(type) 
    {
        case CFG_TMP:
            tmpConfigs_.Push(cfg);
            break;
        case CFG_GAME:
            return gameConfigs_.Push(cfg);
            break;
        case CFG_LEVEL:
            return levelConfigs_.Push(cfg);
            break;
        case CFG_STATIC:
            Log::Write(-1," CFG_STATIC\n");
            break;
        default:
            Log::Write(-1," ERROR: CFG_ type out of range\n");
            break;
    }
}

Cfg* CfgManager::GetCfg_Hash(const StringHash& fh, ConfigType type)
{
    switch(type) 
    {
        case CFG_TMP:
            for (unsigned int i = 0; i < tmpConfigs_.Size(); ++i)
            {
                if (tmpConfigs_[i]->fileHash == fh)
                    return tmpConfigs_[i];
            }
            break;
        case CFG_GAME:
            for (unsigned int i = 0; i < gameConfigs_.Size(); ++i)
            {
                if (gameConfigs_[i]->fileHash == fh)
                    return gameConfigs_[i];
            }
            break;
        case CFG_LEVEL:
            for (unsigned int i = 0; i < levelConfigs_.Size(); ++i)
            {
                if (levelConfigs_[i]->fileHash == fh)
                    return levelConfigs_[i];
            }
            break;
        case CFG_STATIC:
            Log::Write(-1," ERROR: CFG_STATIC not realized type\n");
            return NULL;
            break;
        default:
            Log::Write(-1," ERROR: CFG_ type out of range\n");
            return NULL;
            break;
    }
}

Cfg* CfgManager::GetCfg(const String& file, ConfigType type)
{
    StringHash fh = StringHash(RADIO::CFG_ROOT + file);
    Cfg* cfg = GetCfg_Hash(fh, type);
    if (!cfg)
        Log::Write(-1," ERROR: CfgManager: Config not exist "+ RADIO::CFG_ROOT + file + " type = "+String(int(type))+"\n");
    return cfg;
}

Cfg* CfgManager::GetCfg(const String& file)
{
    StringHash fh = StringHash(RADIO::CFG_ROOT + file);
    Cfg* cfg = NULL;
    
    for (int tp = CFG_TMP; tp < CFG_END; tp++)
    {
        cfg = GetCfg_Hash(fh, ConfigType(tp));
        if(cfg)
            return cfg;
    }
    return NULL;
}




