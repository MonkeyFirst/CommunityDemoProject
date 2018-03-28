#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Character/Character.h"

#include "Character/CharacterStates.h"

// -- NON CONST
const float KEEP_DIST_WITH_PLAYER = -0.25f;

class Enemy : public Character
{
    URHO3D_OBJECT(Enemy, Character);

public:
    Enemy(Context* context);
    virtual ~Enemy();

//    virtual void ObjectStart(){}

    void Start();
    void Kill(); // Remove()
    String GetDebugText();
    bool IsTargetSightBlocked();

    // Realization in Thug class
    virtual bool KeepDistanceWithEnemy();
    virtual bool KeepDistanceWithPlayer(float max_dist = KEEP_DIST_WITH_PLAYER);

    float requiredDistanceFromNeighbors;

};