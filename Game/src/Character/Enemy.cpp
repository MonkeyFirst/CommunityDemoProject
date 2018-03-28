#include <Urho3D/Urho3DAll.h>

#include "Character/Enemy.h"
#include "Character/CharacterStates.h"
#include "Character/EnemyManager.h"

#include "States/GameManager.h"

#include "radio.h"

// Enemy  ===============================================

Enemy::Enemy(Context* context):
    Character(context)
{
    requiredDistanceFromNeighbors = 1.25f;

}

Enemy::~Enemy()
{}


void Enemy::Start()
{
    Character::Start();
    EnemyManager* em = RADIO::g_em;
    if (em)
        em->RegisterEnemy(this);
    SetTarget(RADIO::g_gm->GetPlayer());
}

void Enemy::Kill() // Remove()
{
    if(RADIO::d_log)
        Log::Write(-1," Enemy: Kill " + GetNode()->GetName() + "\n");
    EnemyManager* em = RADIO::g_em;
    if (em)
        em->UnRegisterEnemy(this);
    Character::Kill();
}

String Enemy::GetDebugText()
{
    return Character::GetDebugText() + "health=" + String(health) +  " flags=" + String(flags) + " distToPlayer=" + String(GetTargetDistance()) + " timeScale=" + String(timeScale) + "\n";
}

bool Enemy::IsTargetSightBlocked()
{
    Vector3 my_mid_pos = sceneNode->GetWorldPosition(); 
    my_mid_pos.y_ += CHARACTER_HEIGHT/2;
    Vector3 target_mid_pos = target->sceneNode->GetWorldPosition();
    target_mid_pos.y_ += CHARACTER_HEIGHT/2;
    Vector3 dir = target_mid_pos - my_mid_pos;
    float rayDistance = dir.Length();
    Ray sightRay;
    sightRay.origin_ = my_mid_pos;
    sightRay.direction_ = dir.Normalized();

    PhysicsRaycastResult result;
    sceneNode->GetScene()->GetComponent<PhysicsWorld>()->RaycastSingle(result, sightRay, rayDistance, COLLISION_LAYER_CHARACTER);
    if (!result.body_)
        return false;
    return true;
}

bool Enemy::KeepDistanceWithEnemy()
{
    return false;
}

bool Enemy::KeepDistanceWithPlayer(float max_dist)
{
    if(RADIO::d_log)
        Log::Write(-1," === Enemy::KeepDistanceWithPlayer\n");
    return false;
}



