#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Character/Thug.h"


class EnemyManager : public Object
{
    URHO3D_OBJECT(EnemyManager, Object);

public:
    EnemyManager(Context* context);
    ~EnemyManager();

    void Initialize();
    void Finalize();

    void Start(); // LogicComponent
    void Stop(); // LogicComponent
    void Update(float dt); // LogicComponent

    void DoUpdate();
    void RemoveAll();
    void ResetEM();
    void CreateEnemies();
    Node* CreateEnemy(const Vector3& position, const Quaternion& rotation, const String& type, const String& name = "");
    void RegisterEnemy(Enemy* e);
    void UnRegisterEnemy(Enemy* e);
    int GetNumOfEnemyInState(const StringHash& nameHash);
    int GetNumOfEnemyAttackValid();
    int GetNumOfEnemyHasFlag(int flag);
    int GetNumOfEnemyAlive();
    int GetNumOfEnemyWithinDistance(float dist);
    void DebugDraw(DebugRenderer* debug);
    Node* CreateThug(const String& name, const Vector3& position, const Quaternion& rotation);

    Vector<Vector3> enemyResetPositions;
    Vector<Quaternion> enemyResetRotations;
    Vector<Enemy*> enemyList;
    Vector<int> scoreCache;
    
    int thugId = 0;
    float updateTimer = 0.0f;
    float updateTime = 0.25f;
    float attackValidDist = 6.0f;
};