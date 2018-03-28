
#include <Urho3D/Urho3DAll.h>

#include "Character/Player.h"
#include "Character/EnemyManager.h"
#include "Character/PlayerMovement.h"
#include "Character/PlayerCombat.h"
#include "Character/Thug.h"

#include "States/GameManager.h"
#include "Objects/InputManager.h"
#include "Camera/CameraManager.h"

#include "Motions/MotionManager.h"

#include "radio.h"

Player::Player(Context* context) :
    Character(context)
{
    combo = 0;
    killed = 0; 
    lastAttackId = M_MAX_UNSIGNED;
    applyGravity = true;   

    if(RADIO::d_log)
        Log::Write(-1," Player Constructor\n");
}

Player::~Player()
{
    points.Clear();
    results.Clear();
}

void Player::ObjectStart()
{
    if(RADIO::d_log)
        Log::Write(-1," Player: ObjectStart\n");

    Character::ObjectStart();

    side = 1; // GameObject member
    sensor = new PhysicsSensor(context_, sceneNode); // Character member

    Node* tailNode = sceneNode->CreateChild("TailNode"); // sceneNode Character member
    ParticleEmitter* emitter = tailNode->CreateComponent<ParticleEmitter>();

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    emitter->SetEffect(cache->GetResource<ParticleEffect>("Particle/Tail.xml")); 
    tailNode->SetEnabled(false);

    AddStates(); // empty virtual in Player and realization in Batman class
    ChangeState("StandState"); // function in Character class
}

void Player::AddStates()
{
    if(RADIO::d_log)
        Log::Write(-1," Player: AddStates\n");
}

// virtual in Character. Not realized in Bruce class
bool Player::Counter()
{
    if (RADIO::game_type != 0)
        return false;

    PlayerCounterState* state = static_cast<PlayerCounterState*>(stateMachine->FindState("CounterState"));
    if (!state)
        return false;

    //old script. Player function.
    int len = PickCounterEnemy(state->counterEnemies);
    if (len == 0)
        return false;

/* // new script
    state->counterEnemy = PickCounterEnemy();
    if (!state->counterEnemy)
        return false;
*/
    ChangeState("CounterState");
    return true;
}

bool Player::Evade()
{
    ChangeState("EvadeState");
    return true;
}

void Player::CommonStateFinishedOnGroud()
{
    InputManager* gInput = RADIO::g_im;
    
    if (health <= 0)
        return;

    if (CheckFalling())
        return;

    bool bCrouch = gInput->IsCrouchDown();
    if (!gInput->IsLeftStickInDeadZone() && gInput->IsLeftStickStationary())
    {
        int index = RadialSelectAnimation(4);
        sceneNode->SetVar(ANIMATION_INDEX, index -1);

        if(RADIO::d_log)
            Log::Write(-1," " + GetName() + "CommonStateFinishedOnGroud crouch=" + String(bCrouch) + "To->Move|Turn hold-frames=" 
            + String(gInput->GetLeftAxisHoldingFrames()) + " hold-time=" + String(gInput->GetLeftAxisHoldingTime())+"\n");
        if (index != 0)
        {
            if (bCrouch)
            {
                if (ChangeState("CrouchTurnState"))
                    return;
            }
            else
            {
                if (ChangeState( gInput->IsRunHolding() ? "StandToRunState" : "StandToWalkState"))
                    return;
            }
        }

        if (bCrouch)
            ChangeState("CrouchState");
        else
            ChangeState(gInput->IsRunHolding() ? "RunState" : "WalkState");
    }
    else
        ChangeState(bCrouch ? "CrouchState" : "StandState");
}

float Player::GetTargetAngle()
{
    
    return RADIO::g_im->GetLeftAxisAngle() + RADIO::g_cm->GetCameraAngle();
}

bool Player::OnDamage(GameObject* attacker, const Vector3& position, const Vector3& direction, int damage, bool weak)
{
    if (!CanBeAttacked()) 
    {
        if (RADIO::d_log)
            Log::Write(-1," " + GetName() + " OnDamage failed because I can no be attacked \n");
        return false;
    }

    health -= damage;
    health = Max(0, health);
    combo = 0;

    SetHealth(health);

    int index = RadialSelectAnimation(attacker->GetNode(), 4);

    if(RADIO::d_log)
        Log::Write(-1," Player: " + GetName() + " OnDamage RadialSelectAnimation index=" + String(index)+"\n");

    if (health <= 0)
        OnDead();
    else
    {
        sceneNode->SetVar(ANIMATION_INDEX, index);
        ChangeState("HitState");
    }

    StatusChanged();
    return true;
}

void Player::OnAttackSuccess(Character* target)
{
    if (!target)
    {
        Log::Write(-1," Player: " + GetName() + " OnAttackSuccess target is null\n");
        return;
    }

    combo ++;

    if (target->health == 0)
    {
        killed ++;
        if(RADIO::d_log)
            Log::Write(-1," Player: " + GetName() + " killed add to " + String(killed)+"\n");
        RADIO::g_gm->OnCharacterKilled(this, target);
    }

    StatusChanged();
}

void Player::OnCounterSuccess()
{
    combo ++;
    
    if(RADIO::d_log)
        Log::Write(-1," Player: " + GetName() + " OnCounterSuccess combo add to " + String(combo)+"\n");
    StatusChanged();
}

void Player::StatusChanged()
{
    const int speed_up_combo = 10;
    float fov = RADIO::BASE_FOV;

    if (combo < speed_up_combo)
    {
        SetTimeScale(1.0f);
    }
    else
    {
        int max_comb = 80;
        int c = Min(combo, max_comb);
        float a = float(c)/float(max_comb);
        const float max_time_scale = 1.35f;
        float time_scale = Lerp(1.0f, max_time_scale, a);
        SetTimeScale(time_scale);
        const float max_fov = 75;
        fov = Lerp(RADIO::BASE_FOV, max_fov, a);
    }
    VariantMap data;
    data[TARGET_FOV] = fov;
    SendEvent("CameraEvent", data);
    RADIO::g_gm->OnPlayerStatusUpdate(this);
}

//====================================================================
//      SMART ENEMY PICK FUNCTIONS
//====================================================================
int Player::PickCounterEnemy(Vector<Enemy*>& counterEnemies)
{
    EnemyManager* em = RADIO::g_em;
    if (!em)
        return 0;

    counterEnemies.Clear();
    Vector3 myPos = sceneNode->GetWorldPosition();
    for (unsigned int i = 0; i < em->enemyList.Size(); ++i)
    {
        Enemy* e = em->enemyList[i];
        if (!e->CanBeCountered())
        {
            if (RADIO::d_log)
                Log::Write(-1," " + e->GetName() + " can not be countered\n");
            continue;
        }
        Vector3 posDiff = e->sceneNode->GetWorldPosition() - myPos;
        posDiff.y_ = 0;
        float distSQR = posDiff.LengthSquared();
        if (distSQR > MAX_COUNTER_DIST * MAX_COUNTER_DIST)
        {
            if (RADIO::d_log)
                Log::Write(-1," " + e->GetName() + " counter distance too long " + String(distSQR)+"\n");
            continue;
        }
        counterEnemies.Push(e);
    }

    return counterEnemies.Size();
}

Enemy* Player::PickRedirectEnemy()
{
    EnemyManager* em = RADIO::g_em;
    if (!em)
        return NULL;

    Enemy* redirectEnemy = NULL;
    const float bestRedirectDist = 5;
    const float maxRedirectDist = 7;
    const float maxDirDiff = 45;

    float myDir = GetCharacterAngle();
    float bestDistDiff = 9999;

    for (unsigned int i = 0; i < em->enemyList.Size(); ++i)
    {
        Enemy* e = em->enemyList[i];
        if (!e->CanBeRedirected()) 
        {
            if(RADIO::d_log)
                Log::Write(-1," Enemy " + e->GetName() + " can not be redirected. (Player: PickRedirectEnemy)\n");
            continue;
        }

        float enemyDir = e->GetCharacterAngle();
        float totalDir = Abs(RADIO::AngleDiff(myDir - enemyDir));
        float dirDiff = Abs(totalDir - 180);
        
        if(RADIO::d_log)
            Log::Write(-1," " + GetName() + " Evade-- myDir=" + String(myDir) + " enemyDir=" + String(enemyDir) 
            + " totalDir=" + String(totalDir) + " dirDiff=" + String(dirDiff)+"\n");

        if (dirDiff > maxDirDiff)
            continue;

        float dist = GetTargetDistance(e->sceneNode);

        if (dist > maxRedirectDist)
            continue;

        dist = Abs(dist - bestRedirectDist);

        if (dist < bestDistDiff)
        {
            redirectEnemy = e;
            dist = bestDistDiff;
        }
    }

    return redirectEnemy;
}

Enemy* Player::CommonPickEnemy(float maxDiffAngle, float maxDiffDist, int flags, bool checkBlock, bool checkLastAttack)
{
    unsigned int t = GetSubsystem<Time>()->GetSystemTime();
    Scene* _scene = GetScene(); // Component function
    EnemyManager* em = RADIO::g_em;
    if (!em)
        return NULL;

    // Find the best enemy
    Vector3 myPos = sceneNode->GetWorldPosition();
    Vector3 myDir = sceneNode->GetWorldRotation() * Vector3(0, 0, 1);
    float myAngle = Atan2(myDir.x_, myDir.z_);
    float targetAngle = GetTargetAngle();
    em->scoreCache.Clear();

    Enemy* attackEnemy = NULL;
    for (unsigned int i = 0; i < em->enemyList.Size(); ++i)
    {
        Enemy* e = em->enemyList[i];
        if (!e->HasFlag(flags))
        {
            if (RADIO::d_log)
                Log::Write(-1," Enemy " + e->GetName() + " no flag: " + String(flags)+" (Player::CommonPickEnemy)\n");
            em->scoreCache.Push(-1);
            continue;
        }

        Vector3 posDiff = e->GetNode()->GetWorldPosition() - myPos;
        posDiff.y_ = 0;
        int score = 0;
        float dist = posDiff.Length() - PLAYER_COLLISION_DIST;

        if (dist > maxDiffDist)
        {
            if (RADIO::d_log)
                Log::Write(-1," Enemy " + e->GetName() + " far way from player (Player::CommonPickEnemy)\n");
            em->scoreCache.Push(-1);
            continue;
        }

        float enemyAngle = Atan2(posDiff.x_, posDiff.z_);
        float diffAngle = targetAngle - enemyAngle;
        diffAngle = RADIO::AngleDiff(diffAngle);

        if (Abs(diffAngle) > maxDiffAngle)
        {
            if (RADIO::d_log)
                Log::Write(-1," Enemy " + e->GetName() + " diffAngle=" + String(diffAngle) + " too large (Player::CommonPickEnemy)\n");
            em->scoreCache.Push(-1);
            continue;
        }

        if (RADIO::d_log)
            Log::Write(-1," " + GetName() + " enemyAngle=" + String(enemyAngle) + " targetAngle=" + String(targetAngle) 
                + " diffAngle=" + String(diffAngle)+"\n");

        int threatScore = 0;
        if (dist < 1.0f + COLLISION_SAFE_DIST)
        {
            CharacterState* state = static_cast<CharacterState*>(e->GetState());
            threatScore += int(state->GetThreatScore() * THREAT_SCORE);
        }
        int angleScore = int((180.0f - Abs(diffAngle))/180.0f * ANGLE_SCORE);
        int distScore = int((maxDiffDist - dist) / maxDiffDist * DIST_SCORE);
        score += distScore;
        score += angleScore;
        score += threatScore;

        if (checkLastAttack)
        {
            if (lastAttackId == e->sceneNode->GetID())
            {
                if (diffAngle <= LAST_ENEMY_ANGLE)
                    score += LAST_ENEMY_SCORE;
            }
        }

        em->scoreCache.Push(score);

        if (RADIO::d_log)
            Log::Write(-1," Enemy " + e->sceneNode->GetName() + " dist=" + String(dist) + " diffAngle=" 
                + String(diffAngle) + " score=" + String(score) + " (Player::CommonPickEnemy)\n");
    }

    int bestScore = 0;
    for (unsigned int i = 0; i < em->scoreCache.Size(); ++i)
    {
        int score = em->scoreCache[i];
        if (score >= bestScore) 
        {
            bestScore = score;
            attackEnemy = em->enemyList[i];
        }
    }

    if (attackEnemy && checkBlock)
    {
        if(RADIO::d_log)
            Log::Write(-1," " + GetName() + " CommonPicKEnemy-> attackEnemy is " + attackEnemy->GetName()+"\n");

        Vector3 v_pos = sceneNode->GetWorldPosition();
        v_pos.y_ = CHARACTER_HEIGHT / 2;
        Vector3 e_pos = attackEnemy->GetNode()->GetWorldPosition();
        e_pos.y_ = v_pos.y_;
        Vector3 dir = e_pos - v_pos;
        float len = dir.Length();
        dir.Normalize();
        Ray ray;
        ray.Define(v_pos, dir);
        PhysicsRaycastResult result;
        sceneNode->GetScene()->GetComponent<PhysicsWorld>()->RaycastSingle(result, ray, len, COLLISION_LAYER_CHARACTER);
        if (result.body_)
        {
            Node* n = result.body_->GetNode()->GetParent(); 
            Enemy* e = static_cast<Enemy*>(n->GetComponent<Thug>());
            if (e && e != attackEnemy && e->HasFlag(FLAGS_ATTACK))
            {
                if(RADIO::d_log)
                    Log::Write(-1," " + GetName() + " Find a block enemy " + e->GetName() + " before " + attackEnemy->GetName()+"\n");
                attackEnemy = e;
            }
        }
    }

    if(RADIO::d_log)
        Log::Write(-1," " + GetName() + " CommonPicKEnemy() time-cost = " + String(GetSubsystem<Time>()->GetSystemTime() - t) + " ms\n");

    return attackEnemy;
}

void Player::CommonCollectEnemies(Vector<Enemy*>& enemies, float maxDiffAngle, float maxDiffDist, int flags)
{
    enemies.Clear();

    unsigned int t = GetSubsystem<Time>()->GetSystemTime();
    Scene* _scene = GetScene(); // Component function.
    EnemyManager* em = RADIO::g_em;
    if (!em)
        return;

    Vector3 myPos = sceneNode->GetWorldPosition();
    Vector3 myDir = sceneNode->GetWorldRotation() * Vector3(0, 0, 1);
    float myAngle = Atan2(myDir.x_, myDir.z_);
    float targetAngle = GetTargetAngle();

    for (unsigned int i = 0; i < em->enemyList.Size(); ++i)
    {
        Enemy* e = em->enemyList[i];
        if (!e->HasFlag(flags))
            continue;
        Vector3 posDiff = e->GetNode()->GetWorldPosition() - myPos;
        posDiff.y_ = 0;
        int score = 0;
        float dist = posDiff.Length() - PLAYER_COLLISION_DIST;
        if (dist > maxDiffDist)
            continue;
        float enemyAngle = Atan2(posDiff.x_, posDiff.z_);
        float diffAngle = targetAngle - enemyAngle;
        diffAngle = RADIO::AngleDiff(diffAngle);
        if (Abs(diffAngle) > maxDiffAngle)
            continue;
        enemies.Push(e);
    }

    if(RADIO::d_log)
        Log::Write(-1," " + GetName() + " CommonCollectEnemies() len=" + String(enemies.Size()) + " time-cost = " 
        + String(GetSubsystem<Time>()->GetSystemTime() - t) + " ms\n");
}

String Player::GetDebugText()
{
    return Character::GetDebugText() +  " health=" + String(health) + " flags=" + String(flags) +
          " combo=" + String(combo) + " killed=" + String(killed) + " timeScale=" + String(timeScale) + " tAngle=" + String(GetTargetAngle()) +
          " grounded=" + String(sensor->grounded) + " inAirHeight=" + String(sensor->inAirHeight) + "\n";
}

void Player::ResetObject() // Reset()
{
    SetSceneTimeScale(1.0f);
    Character::ResetObject();
    combo = 0;
    killed = 0;
    RADIO::g_gm->OnPlayerStatusUpdate(this);

    VariantMap data;
    data[TARGET_FOV] = RADIO::BASE_FOV;
    SendEvent("CameraEvent", data);
}

bool Player::ActionCheck(bool bAttack, bool bDistract, bool bCounter, bool bEvade)
{
    InputManager* gInput = RADIO::g_im;

    if (bAttack && gInput->IsAttackPressed())
        return Attack();

    if (bDistract && gInput->IsDistractPressed())
        return Distract();

    if (bCounter && gInput->IsCounterPressed())
        return Counter();

    if (bEvade && gInput->IsEvadePressed())
        return Evade();

    return false;
}

bool Player::Attack()
{
    if (RADIO::game_type != 0)
        return false;

    if(RADIO::d_log)
        Log::Write(-1," Player: " + GetName() + " Do--Attack--->\n");

    Enemy* e = CommonPickEnemy(90, RADIO::MAX_ATTACK_DIST, FLAGS_ATTACK, true, true);
    SetTarget(e);
// Отсутствует состояние в коде. Нужно его брать из скриптов ревизии 534.
// FLAGS_STUN - противник в шоковом состояниии (оглушён).
    if (e && e->HasFlag(FLAGS_STUN))
        ChangeState("BeatDownHitState");
    else
        ChangeState("AttackState");
    return true;
}

bool Player::Distract()
{
    if (RADIO::game_type != 0)
        return false;

    if(RADIO::d_log)
        Log::Write(-1," Player: " + GetName() + " Do--Distract--->\n");
    Enemy* e = CommonPickEnemy(45, RADIO::MAX_ATTACK_DIST, FLAGS_ATTACK | FLAGS_STUN, true, true);
    if (!e)
        return false;
    SetTarget(e);
    ChangeState("BeatDownHitState");
    return true;
}

bool Player::CheckLastKill()
{
    EnemyManager* em = RADIO::g_em;
    if (!em)
        return false;

    int alive = em->GetNumOfEnemyAlive();

    if(RADIO::d_log)
        Log::Write(-1," Player: CheckLastKill() alive=" + String(alive)+"\n");

    if (alive == 1)
    {
        VariantMap data;
        data[NODE] = target->GetNode()->GetID();
        data[NAME] = CHANGE_STATE;
        data[VALUE] = StringHash("Death");
        SendEvent("CameraEvent", data);
        return true;
    }
    return false;
}

void Player::SetTarget(Character* t)
{
    if(RADIO::d_log)
        Log::Write(-1," Player: SetTarget\n");

    if (target == t)
        return;
    if (target)
        target->RemoveFlag(FLAGS_NO_MOVE);
    Character::SetTarget(t);
}

void Player::DebugDraw(DebugRenderer* debug)
{

    Character::DebugDraw(debug);
    // debug.AddCircle(sceneNode.worldPosition, Vector3(0, 1, 0), COLLISION_RADIUS, YELLOW, 32, false);
    // sensor.DebugDraw(debug);

    debug->AddNode(sceneNode->GetChild(TranslateBoneName, true), 0.5f, false);

// ERROR: crash
//    if (dockLine)
//        debug->AddLine(dockLine->ray.origin_, dockLine->end, Color::YELLOW, false);

    if (points.Size() > 1)
    {
        for (unsigned int i = 0; i < points.Size() - 1; ++i)
        {
            debug->AddLine(points[i], points[i+1], Color(0.5, 0.45, 0.75), false);
        }

        for (unsigned int i = 0; i < results.Size(); ++i)
        {
            if (results[i].body_)
                debug->AddCross(results[i].position_, 0.25f, Color(0.1f, 0.7f, 0.25f), false);
        }
    }

    debug->AddBoundingBox(box, Color(0.25, 0.75, 0.25), false);
}

void Player::Update(float dt)
{
    if(sensor)
        sensor->Update(dt);
    Character::Update(dt);
}

void Player::SetVelocity(const Vector3& vel)
{
    if (!sensor->grounded && applyGravity)
        Character::SetVelocity(vel + Vector3(0, -9.8f, 0));
    else
        Character::SetVelocity(vel);
}

bool Player::CheckFalling()
{
    if (!sensor->grounded && sensor->inAirHeight > 1.5f && sensor->inAirFrames > 2 && applyGravity)
    {
        ChangeState("FallState");
        return true;
    }
    return false;
}

bool Player::CheckDocking(float distance)
{
    Vector3 charPos = sceneNode->GetWorldPosition();
    float charAngle = GetCharacterAngle();
    Line* l = RADIO::g_lw->GetNearestLine(charPos, charAngle, distance);

    if (!l)
        return false;

    String stateToChange;

    if (l->type == LINE_COVER)
        stateToChange = "CoverState";
    else if (l->type == LINE_RAILING)
        stateToChange = "RailUpState";
    else if (l->type == LINE_EDGE)
    {
        Vector3 proj = l->Project(charPos);
        float lineToMe = proj.y_ - charPos.y_;
        // URHO3D_LOGINFO("lineToMe_Height=" + lineToMe);

        if (lineToMe < 0.1f)
        {
            // move down case
            float distSQR = (proj- charPos).LengthSquared();
            const float minDownDist = 1.5f;
            if (distSQR < minDownDist * minDownDist)
                stateToChange = "ClimbDownState";
        }
        else if (lineToMe > HEIGHT_128 / 4)
        {
            // ClimbUpRaycasts(l);
            Line* line = FindForwardUpDownLine(l);

            bool hitUp = results[0].body_;
            bool hitForward = results[1].body_;
            bool hitDown = results[2].body_;
            bool hitBack = results[3].body_;

            float lineToGround = l->end.y_ - results[2].position_.y_;
            bool isWallTooShort = lineToMe < (HEIGHT_128 + HEIGHT_256) / 2;

            // URHO3D_LOGINFO("CheckDocking hitUp=" + hitUp + " hitForward=" + hitForward + " hitDown=" + hitDown + " hitBack=" + hitBack + " lineToGround=" + lineToGround + " isWallTooShort=" + isWallTooShort);

            if (!hitUp)
            {
                if (!hitForward)
                {
                    if (hitDown && lineToGround < 0.25f)
                    {
                        if (!l->HasFlag(LINE_SHORT_WALL))
                            stateToChange = "ClimbUpState";
                    }
                    else
                    {
                        // TODO
                        if (isWallTooShort && l->HasFlag(LINE_THIN_WALL))
                        {
                            stateToChange = "ClimbOverState";
                            PlayerClimbOverState* s = static_cast<PlayerClimbOverState*>(FindState(stateToChange));
                            if (s)
                                s->downLine = line;
                        }
                        else
                            stateToChange = "HangUpState"; // "ClimbOverState";
                    }
                }
                else
                {
                    stateToChange = "HangUpState";
                }
            }
        }
    }

    if (stateToChange == String::EMPTY)
        return false;

    AssignDockLine(l);
    ChangeState(stateToChange);
    return true;
}

void Player::ClimbUpRaycasts(Line* line)
{
    results.Resize(4);
    points.Resize(7);

    PhysicsWorld* world = GetScene()->GetComponent<PhysicsWorld>();
    Vector3 charPos = GetNode()->GetWorldPosition();
    Vector3 proj = line->Project(charPos);
    float h_diff = proj.y_ - charPos.y_;
    float above_height = 1.0f;
    Vector3 v1, v2, v3, v4, v5, v6, v7;
    Vector3 dir = (line->type != LINE_RAILING) ? (proj - charPos) : (GetNode()->GetWorldRotation() * Vector3(0, 0, 1));
    dir.y_ = 0;
    float fowardDist = dir.Length() + COLLISION_RADIUS * 1.5f;

    // up test
    v1 = charPos;
    Ray ray;
    ray.Define(v1, Vector3(0, 1, 0));
    float dist = h_diff + above_height;
    v2 = ray.origin_ + ray.direction_ * dist;
    world->RaycastSingle(results[0], ray, dist, COLLISION_LAYER_LANDSCAPE);

    // forward test
    ray.Define(v2, dir);
    dist = fowardDist;
    v3 = ray.origin_ + ray.direction_ * dist;
    world->RaycastSingle(results[1], ray, dist, COLLISION_LAYER_LANDSCAPE);

    // down test
    v4 = v3;
    v4.y_ = line->end.y_;
    v4.y_ += HEIGHT_384;
    ray.Define(v4, Vector3(0, -1, 0));
    dist = HEIGHT_384 + HEIGHT_256;
    v5 = ray.origin_ + ray.direction_ * dist;
     world->RaycastSingle(results[2], ray, dist, COLLISION_LAYER_LANDSCAPE);

    // here comes the tricking part
    v6 = v5;
    v6.y_ = line->end.y_;
    v6.y_ -= HEIGHT_128;
    dir *= -1;
    dir.Normalize();
    dist = fowardDist;
    v7 = v6 + dir * dist;
    world->ConvexCast(results[3], sensor->verticalShape, v6, Quaternion(), v7, Quaternion(), COLLISION_LAYER_LANDSCAPE);

    points[0] = v1;
    points[1] = v2;
    points[2] = v3;
    points[3] = v4;
    points[4] = v5;
    points[5] = v6;
    points[6] = v7;
}

void Player::ClimbDownRaycasts(Line* line)
{
    results.Resize(3);
    points.Resize(5);

    PhysicsWorld* world = GetScene()->GetComponent<PhysicsWorld>();
    Vector3 charPos = GetNode()->GetWorldPosition();
    Vector3 proj = line->Project(charPos);
    float h_diff = proj.y_ - charPos.y_;
    float above_height = 1.0f;
    Vector3 v1, v2, v3, v4, v5;
    Vector3 dir = (line->type != LINE_RAILING) ? (proj - charPos) : (GetNode()->GetWorldRotation() * Vector3(0, 0, 1));
    dir.y_ = 0;
    float fowardDist = dir.Length() + COLLISION_RADIUS * 1.5f;
    float dist;

    // forward test
    v1 = charPos + Vector3(0, above_height, 0);
    Ray ray;
    ray.Define(v1, dir);
    dist = fowardDist;
    v2 = ray.origin_ + ray.direction_ * dist;
    world->RaycastSingle(results[0], ray, dist, COLLISION_LAYER_LANDSCAPE);

    // down test
    ray.Define(v2, Vector3(0, -1, 0));
    dist = above_height + HEIGHT_256;
    v3 = ray.origin_ + ray.direction_ * dist;
    world->RaycastSingle(results[1], ray, dist, COLLISION_LAYER_LANDSCAPE);

    // here comes the tricking part
    v4 = v3;
    v4.y_ = line->end.y_;
    v4.y_ -= HEIGHT_128;
    dir *= -1;
    dir.Normalize();
    dist = fowardDist;
    v5 = v4 + dir * dist;

    world->ConvexCast(results[2], sensor->verticalShape, v4, Quaternion(), v5, Quaternion(), COLLISION_LAYER_LANDSCAPE);

    points[0] = v1;
    points[1] = v2;
    points[2] = v3;
    points[3] = v4;
    points[4] = v5;
}

void Player::ClimbLeftOrRightRaycasts(Line* line, bool bLeft)
{
    results.Resize(3);
    points.Resize(4);

    PhysicsWorld* world = GetScene()->GetComponent<PhysicsWorld>();

    Vector3 myPos = sceneNode->GetWorldPosition();
    Vector3 proj = line->Project(myPos);
    Vector3 dir;
    dir = proj - myPos;
    if (line->type == LINE_RAILING)
    {
        dir = bLeft ? Vector3(-1, 0, 0) : Vector3(1, 0, 0);
        dir = GetNode()->GetWorldRotation() * dir;
    }
    else
        dir = proj - myPos;

    Quaternion q(0, Atan2(dir.x_, dir.z_), 0);

    Vector3 towardDir = bLeft ? Vector3(-1, 0, 0) : Vector3(1, 0, 0);
    towardDir = q * towardDir;
    Vector3 linePt = line->GetLinePoint(towardDir);

    Vector3 v1, v2, v3, v4;

    v1 = myPos;
    v1.y_ = sceneNode->GetChild(L_HAND, true)->GetWorldPosition().y_;

    Vector3 v = linePt - v1;
    v.y_ = 0;
    dir = towardDir;
    dir.y_ = 0;

    float len = v.Length() + COLLISION_RADIUS;

    Ray ray;
    ray.Define(v1, dir);
    v2 = ray.origin_ + ray.direction_ * len;

    world->ConvexCast(results[0], sensor->verticalShape, v1, Quaternion(), v2, Quaternion(), COLLISION_LAYER_LANDSCAPE);

    dir = q * Vector3(0, 0, 1);
    len = COLLISION_RADIUS * 2;
    ray.Define(v2, dir);
    v3 = v2 + ray.direction_ * len;
    world->ConvexCast(results[1], sensor->verticalShape, v2, Quaternion(), v3, Quaternion(), COLLISION_LAYER_LANDSCAPE);

    dir = bLeft ? Vector3(1, 0, 0) : Vector3(-1, 0, 0);
    dir = q * dir;
    ray.Define(v3, dir);
    v4 = v3 + ray.direction_ * len;
    world->ConvexCast(results[2], sensor->verticalShape, v3, Quaternion(), v4, Quaternion(), COLLISION_LAYER_LANDSCAPE);

    points[0] = v1;
    points[1] = v2;
    points[2] = v3;
    points[3] = v4;
}

Line* Player::FindCrossLine(bool left, int& convexIndex)
{
    Line* oldLine = dockLine;
    ClimbLeftOrRightRaycasts(oldLine, left);

    bool hit1 = results[0].body_;
    bool hit2 = results[1].body_;
    bool hit3 = results[2].body_;

    if(RADIO::d_log)
        Log::Write(-1," " + GetName() + " FindCrossLine hit1=" + String(hit1) + " hit2=" + String(hit2) + " hit3=" + String(hit3)+"\n");
    convexIndex = 1;
    Vector<Line*> lines = RADIO::g_lw->cacheLines; // Array<Line*>* lines = gLineWorld.cacheLines;

    lines.Clear();

    if (hit1)
    {
        convexIndex = 2;
        RADIO::g_lw->CollectLinesByNode(results[0].body_->GetNode(), lines);
    }
    else if (!hit2 && hit3)
    {
        convexIndex = 1;
        RADIO::g_lw->CollectLinesByNode(results[2].body_->GetNode(), lines);
    }
    else
        return NULL;

    if (lines.Empty())
        return NULL;

    Line* bestLine = NULL;
    float maxHeightDiff = 1.0f;
    float maxDistSQR = 999999;
    Vector3 comparePot = (convexIndex == 1) ? points[1] : points[2];

    for (unsigned int i = 0; i < lines.Size(); ++i)
    {
        Line* l = lines[i];
        if (!l->TestAngleDiff(oldLine, 90))
            continue;
        if (Abs(l->end.y_ - oldLine->end.y_) > maxHeightDiff)
            continue;
        Vector3 proj = l->Project(comparePot);
        proj.y_ = comparePot.y_;
        float distSQR = (proj - comparePot).LengthSquared();
        if (distSQR < maxDistSQR)
        {
            bestLine = l;
            maxDistSQR = distSQR;
        }
    }
    return bestLine;
}

Line* Player::FindParalleLine(bool left, float& outDistErrorSQR)
{
    Line* oldLine = dockLine;
    Node* n = GetNode();
    Vector3 myPos = n->GetWorldPosition();

    Vector3 towardDir = left ? Vector3(-1, 0, 0) : Vector3(1, 0, 0);
    towardDir = n->GetWorldRotation() * towardDir;
    Vector3 linePt = oldLine->GetLinePoint(towardDir);

    towardDir = linePt - oldLine->Project(myPos);

    float myAngle = GetCharacterAngle();
    float angle = Atan2(towardDir.x_, towardDir.z_);

    float w = 6.0f;
    float h = 2.0f;
    float l = 2.0f;
    Vector3 halfSize(w/2, h/2, l/2);
    Vector3 min = halfSize * -1;
    Vector3 max = halfSize;
    box.Define(min, max);

    Quaternion q(0, angle + 90, 0);
    Vector3 center = towardDir.Normalized() * halfSize.x_ + linePt;
    Matrix3x4 m;
    m.SetTranslation(center);
    m.SetRotation(q.RotationMatrix());
    box.Transform(m);

    Vector<Line*> lines = RADIO::g_lw->cacheLines; // Array<Line*>* lines = gLineWorld.cacheLines;
    lines.Clear();

    int num = RADIO::g_lw->CollectLinesInBox(GetScene(), box, oldLine->nodeId, lines);
    if (num == 0)
        return NULL;

    if(RADIO::d_log)
        Log::Write(-1," " + GetName() + " FindParalleLine lines.num=" + String(num)+"\n");

    Line* bestLine = NULL;
    float maxHeightDiff = 1.0f;
    float maxDistSQR = 5.0f * 5.0f;
    Vector3 comparePot = myPos;

    for (unsigned int i = 0; i < lines.Size(); ++i)
    {
        Line* line = lines[i];
        if (!line->TestAngleDiff(oldLine, 0) && !line->TestAngleDiff(oldLine, 180))
            continue;
        if (Abs(line->end.y_ - oldLine->end.y_) > maxHeightDiff)
            continue;
        if (!line->IsAngleValid(myAngle))
            continue;

        Vector3 v = line->GetNearPoint(comparePot);
        v -= comparePot;
        v.y_ = 0;
        float distSQR = v.LengthSquared();
        if (distSQR < maxDistSQR)
        {
            bestLine = line;
            maxDistSQR = distSQR;
        }
    }

    outDistErrorSQR = maxDistSQR;
    return bestLine;
}
// Line* Player::FindDownLine(Array<Line*>* lines, Line* oldLine)
Line* Player::FindDownLine(Vector<Line*> lines, Line* oldLine)
{
    Vector3 comparePot = oldLine->end;
    float maxDistSQR = COLLISION_RADIUS * COLLISION_RADIUS;
    Line* bestLine;

    if (lines.Empty())
        return NULL;

    // URHO3D_LOGINFO("FindDownLine lines.num=" + lines.length);

    for (unsigned int i = 0; i < lines.Size(); ++i)
    {
        Line* l = lines[i];
        if (l == oldLine)
            continue;

        if (!l->TestAngleDiff(oldLine, 0) && !l->TestAngleDiff(oldLine, 180))
            continue;

        float heightDiff = oldLine->end.y_ - l->end.y_;
        float diffTo128 = Abs(heightDiff - HEIGHT_128);
        
        if(RADIO::d_log)
            Log::Write(-1," " + GetName() + " heightDiff= " + String(heightDiff) + " diffTo128 = " + String(diffTo128)+"\n");

        if (diffTo128 > HEIGHT_128/2)
            continue;

        Vector3 tmpV = l->Project(comparePot);
        tmpV.y_ = comparePot.y_;
        float distSQR = (tmpV - comparePot).LengthSquared();
        
        if(RADIO::d_log)
            Log::Write(-1," " + GetName() + " distSQR=" + String(distSQR)+"\n");

        if (distSQR < maxDistSQR)
        {
            bestLine = l;
            maxDistSQR = distSQR;
        }
    }

    return bestLine;
}

Line* Player::FindDownLine(Line* oldLine)
{
    ClimbDownRaycasts(oldLine);
    if (!results[2].body_)
        return NULL;
    Vector<Line*> lines = RADIO::g_lw->cacheLines; // Array<Line*>* lines = gLineWorld.cacheLines;
    lines.Clear();
    Vector3 myPos = sceneNode->GetWorldPosition();
    RADIO::g_lw->CollectLinesByNode(results[2].body_->GetNode(), lines);
    return FindDownLine(lines, oldLine);
}

Line* Player::FindForwardUpDownLine(Line* oldLine)
{
    ClimbUpRaycasts(oldLine);
    if (!results[3].body_)
        return NULL;
    Vector<Line*> lines = RADIO::g_lw->cacheLines; // Array<Line*>* lines = gLineWorld.cacheLines;
    lines.Clear();
    Vector3 myPos = sceneNode->GetWorldPosition();
    RADIO::g_lw->CollectLinesByNode(results[3].body_->GetNode(), lines);
    return FindDownLine(lines, oldLine);
}

void Player::ClearPoints()
{
    points.Clear();
    results.Clear();
}



