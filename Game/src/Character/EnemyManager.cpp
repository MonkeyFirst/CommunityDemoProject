
#include <Urho3D/Urho3DAll.h>

#include "Character/EnemyManager.h"
#include "Character/Enemy.h"
#include "Character/Player.h"

#include "Motions/MotionManager.h"

#include "States/GameManager.h"
#include "radio.h"
// EnemyManager  ===============================================

EnemyManager::EnemyManager(Context* context):
    Object(context)
{
    Log::Write(-1," EnemyManager: Constructor\n");
    thugId = 0;
    updateTimer = 0.0f;
    updateTime = 0.25f;
    attackValidDist = 6.0f;
}

EnemyManager::~EnemyManager()
{
    Log::Write(-1," EnemyManager: Destructor\n");
}

void EnemyManager::Initialize()
{
    Log::Write(-1," EnemyManager: Initialize\n");
}

void EnemyManager::Finalize()
{
    Log::Write(-1," EnemyManager: Finalize\n");
}

void EnemyManager::Start()
{
    Log::Write(-1," EnemyManager: Start\n");
}

void EnemyManager::Stop()
{
    Log::Write(-1," EnemyManager: Stop\n");
    enemyList.Clear();
}

Node* EnemyManager::CreateEnemy(const Vector3& position, const Quaternion& rotation, const String& type, const String& name)
{
    if (type == "thug")
        return CreateThug(name, position, rotation);
    return NULL;
}

void EnemyManager::RegisterEnemy(Enemy* e)
{
    enemyList.Push(e);
}

void EnemyManager::UnRegisterEnemy(Enemy* e)
{
    Log::Write(-1," EnemyManager: UnRegisterEnemy!\n");
//    int i = enemyList.FindByRef(e);
//    if (i < 0)
//        return;
    for (unsigned int i = 0; i < enemyList.Size(); ++i)
    {
        if (enemyList[i] == e)
        {
            enemyList.Erase(i);
            return;
        }
    }
//    enemyList.Erase(i);
}

int EnemyManager::GetNumOfEnemyInState(const StringHash& nameHash)
{
    int ret = 0;
    for (unsigned int i = 0; i < enemyList.Size(); ++i)
    {
        if (enemyList[i]->IsInState_Hash(nameHash))
            ++ret;
    }
    return ret;
}

int EnemyManager::GetNumOfEnemyAttackValid()
{
    int ret = 0;
    for (unsigned int i = 0; i < enemyList.Size(); ++i)
    {
        Enemy* e = enemyList[i];
        if (e->IsInState_Hash(ATTACK_STATE) && e->GetTargetDistance() < attackValidDist)
            ++ret;
    }
    return ret;
}

int EnemyManager::GetNumOfEnemyHasFlag(int flag)
{
    int ret = 0;
    for (unsigned int i = 0; i < enemyList.Size(); ++i)
    {
        if (enemyList[i]->HasFlag(flag))
            ++ret;
    }
    return ret;
}

int EnemyManager::GetNumOfEnemyAlive()
{
    int ret = 0;
    for (unsigned int i = 0; i< enemyList.Size(); ++i)
    {
        if (enemyList[i]->health != 0)
            ++ret;
    }
    return ret;
}

int EnemyManager::GetNumOfEnemyWithinDistance(float dist)
{
    int ret = 0;
    for (unsigned int i = 0; i < enemyList.Size(); ++i)
    {
        if (enemyList[i]->GetTargetDistance() < dist)
            ++ret;
    }
    return ret;
}

void EnemyManager::DebugDraw(DebugRenderer* debug)
{
    if (enemyList.Empty())
        return;
    // Character* player = enemyList[0].target;
    // Vector3 v1 = player.GetNode().worldPosition;
    for (unsigned int i = 0; i < enemyList.Size(); ++i)
    {
        enemyList[i]->DebugDraw(debug);
        // debug.AddLine(v1, enemyList[i].GetNode().worldPosition, Color(0.25f, 0.55f, 0.65f), false);
    }
}

Node* EnemyManager::CreateThug(const String& name, const Vector3& position, const Quaternion& rotation)
{
    String thugName = name;
    if (thugName == "thug" || thugName == String::EMPTY)
        thugName = "thug_" + String(thugId);
    thugId ++;
    Log::Write(-1,"\n EnemyManager: CreateThug id = " + String(thugId));
    return RADIO::g_gm->CreateCharacter(thugName, "BATMAN/thug/thug.xml", "Thug", position, rotation);
}

void EnemyManager::RemoveAll()
{
    for (unsigned int i = 0; i < enemyList.Size(); ++i)
    {
        if(enemyList[i])
            enemyList[i]->GetNode()->Remove();
    }
    enemyList.Clear();
    ResetEM();
}

void EnemyManager::ResetEM()
{
    thugId = 0;
}

void EnemyManager::CreateEnemies()
{   
    if(enemyResetPositions.Size() <= 0)
    {
        Log::Write(-1,"\n EnemyManager: No Enemyes in scene?\n");
        return;
    }
    for (unsigned int i = 0; i < enemyResetPositions.Size(); ++i)
    {
        CreateEnemy(enemyResetPositions[i], enemyResetRotations[i], "thug");
    }
}

void EnemyManager::Update(float dt)
{
//    Log::Write(-1," EnemyManager::Update\n");
    updateTimer += dt;
    if (updateTimer >= updateTime)
    {
        DoUpdate();
        updateTimer -= updateTime;
    }
}

void EnemyManager::DoUpdate()
{
    int num_of_near = 0; // количество бандитов в этом обновлении рядом с игроком
    for (unsigned int i = 0; i < enemyList.Size(); ++i)
    {
        Enemy* e = enemyList[i];
        float dis = e->GetTargetDistance(); // получаем от бандита дистанцию до игрока

        if (dis <= PLAYER_NEAR_DIST) // если небольшая дистанция (PLAYER_NEAR_DIST = 6.0f в Player.h)
            num_of_near ++; // добавим в счетчик бандитов рядом

        if (num_of_near > MAX_NUM_OF_NEAR) // если счетчик достиг позволительного максимума (MAX_NUM_OF_NEAR = 4 in Enemy.h)
        {
            Log::Write(-1, " " + e->GetName() + " too close with player !!!\n");
            // если бандит стоит или разворачивается
            if (e->GetState()->nameHash == STAND_STATE || e->GetState()->nameHash == TURN_STATE)
                e->KeepDistanceWithPlayer(0); // виртуальная функция. Сработает в Thug классе.
        }
//        Log::Write(-1," EnemyManager::DoUpdate: distance_to_target = " + String(dis) + " num_of_near = " + String(num_of_near) + "\n");
    }
}














