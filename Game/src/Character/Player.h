#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Character/Character.h"
#include "Character/CharacterStates.h"
#include "Character/Enemy.h"

#include "Objects/GameObject.h"
#include "Tools/Line.h"

class Player : public Character
{
	URHO3D_OBJECT(Player, Character);

public:
    Player(Context* context);
    virtual ~Player();

    void ObjectStart();
    virtual void AddStates(); // only this function exist in Batman class
/*
    // virtual form GameObject and Character
    void AddFlag(int flag){return GameObject::AddFlag(flag);}
    void RemoveFlag(int flag){return GameObject::RemoveFlag(flag);}
    bool HasFlag(int flag){return GameObject::HasFlag(flag);}
*/
    void StatusChanged();
    int PickCounterEnemy(Vector<Enemy*>& counterEnemies);
    Enemy* PickRedirectEnemy();
    Enemy* CommonPickEnemy(float maxDiffAngle, float maxDiffDist, int flags, bool checkBlock, bool checkLastAttack);
    // void CommonCollectEnemies(Array<Enemy*>* enemies, float maxDiffAngle, float maxDiffDist, int flags);
    void CommonCollectEnemies(Vector<Enemy*>& enemies, float maxDiffAngle, float maxDiffDist, int flags);
    bool CheckLastKill();
    void ClimbUpRaycasts(Line* line);
    void ClimbDownRaycasts(Line* line);
    void ClimbLeftOrRightRaycasts(Line* line, bool bLeft);
    Line* FindCrossLine(bool left, int& convexIndex);
    Line* FindParalleLine(bool left, float& outDistErrorSQR);
    Line* FindDownLine(Vector<Line*> lines, Line* oldLine); // Line* FindDownLine(Array<Line*>* lines, Line* oldLine);
    Line* FindDownLine(Line* oldLine);
    Line* FindForwardUpDownLine(Line* oldLine);
    void ClearPoints();
    
    // virtual
    bool ActionCheck(bool bAttack, bool bDistract, bool bCounter, bool bEvade);
    void Update(float dt);
    String GetDebugText();
    void SetVelocity(const Vector3& vel);
    bool Attack();
    bool Distract();
    void CommonStateFinishedOnGroud();
    void ResetObject();
    bool Counter();
    bool Evade();
    void DebugDraw(DebugRenderer* debug);
    float GetTargetAngle();
    bool OnDamage(GameObject* attacker, const Vector3& position, const Vector3& direction, int damage, bool weak = false);
    void OnAttackSuccess(Character* target);
    void OnCounterSuccess();
    void SetTarget(Character* t);
    bool CheckFalling();
    bool CheckDocking(float distance = 3);
    

    int combo;
    int killed;
    unsigned int lastAttackId;
    bool applyGravity;

    Vector<Vector3> points;
    Vector<PhysicsRaycastResult> results;
    BoundingBox box;
};
