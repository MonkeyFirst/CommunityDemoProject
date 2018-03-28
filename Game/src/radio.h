#pragma once

#include <Urho3D/Urho3DAll.h>

#include "Tools/Line.h"
#include "States/GameManager.h"
#include "Motions/MotionManager.h"
#include "Camera/CameraManager.h"
#include "Objects/InputManager.h"
#include "Character/EnemyManager.h"
#include "Cfg/CfgManager.h"

const unsigned int LAYER_MOVE = 0;
const unsigned int LAYER_ATTACK = 1;

enum AttackType
{
    ATTACK_PUNCH,
    ATTACK_KICK,
};

enum RenderFeature
{
    RF_NONE     = 0,
    RF_SHADOWS  = (1 << 0),
    RF_HDR      = (1 << 1),

    RF_FULL     = RF_SHADOWS | RF_HDR,
};

const float FULLTURN_THRESHOLD = 125;
const float COLLISION_RADIUS = 1.5f;
const float COLLISION_SAFE_DIST = COLLISION_RADIUS * 1.85f;
const float CHARACTER_HEIGHT = 5.0f;
const float START_TO_ATTACK_DIST = 6;
const float CHARACTER_CROUCH_HEIGHT = 3.0f;

const int MAX_NUM_OF_ATTACK = 3;
const int MAX_NUM_OF_MOVING = 3;
const int MAX_NUM_OF_NEAR = 4;

const int INITIAL_HEALTH = 100;


const float MAX_COUNTER_DIST = 4.0f;
const float PLAYER_COLLISION_DIST = COLLISION_RADIUS * 1.8f;
const float DIST_SCORE = 10.0f;
const float ANGLE_SCORE = 30.0f;
const float THREAT_SCORE = 30.0f;
const float LAST_ENEMY_ANGLE = 45.0f;
const int   LAST_ENEMY_SCORE = 5;
const int   MAX_WEAK_ATTACK_COMBO = 3;
const float MAX_DISTRACT_DIST = 4.0f;
const float MAX_DISTRACT_DIR = 90.0f;
const int   HIT_WAIT_FRAMES = 3;
const float LAST_KILL_SPEED = 0.35f;
const float COUNTER_ALIGN_MAX_DIST = 1.5f;
const float PLAYER_NEAR_DIST = 6.0f;
const float GOOD_COUNTER_DIST = 3.0f;
const float ATTACK_DIST_PICK_RANGE = 6.0f;

// no constants in Player.as
//const float MAX_ATTACK_DIST = 25.0f;
const float MAX_BEAT_DIST = 25.0f;

const String MOVEMENT_GROUP_THUG = "TG_Combat/";
const float MIN_TURN_ANGLE = 30;
const float MIN_THINK_TIME = 0.25f;
const float MAX_THINK_TIME = 1.0f;
// Юзается вроде только в ThugStandState. Не путать с KEEP_DIST_WITH_PLAYER 
const float KEEP_DIST_WITHIN_PLAYER = 20.0f; 
const float MAX_ATTACK_RANGE = 3.0f;
const float KEEP_DIST = 1.5f;
const Vector3 HIT_RAGDOLL_FORCE(25.0f, 10.0f, 0.0f);

namespace RADIO
{
    extern String CFG_ROOT;
    
    extern float MAX_ATTACK_DIST;
    extern float BRUCE_TRANSITION_DIST;

    extern float PUNCH_DIST;
    extern float KICK_DIST;
    extern float STEP_MAX_DIST;
    extern float STEP_MIN_DIST;
    //float KEEP_DIST_WITH_PLAYER = -0.25f;  // declared in Enemy.h
    extern float KEEP_DIST_WITH_PLR;

    extern Vector3 WORLD_HALF_SIZE;
    extern int num_of_sounds;
    extern int num_of_big_sounds;
    
    extern SharedPtr<CfgManager> g_cfg;
    extern SharedPtr<InputManager> g_im;
    extern SharedPtr<GameManager> g_gm;
    extern SharedPtr<MotionManager> g_mm;
    extern SharedPtr<CameraManager> g_cm;
    extern SharedPtr<EnemyManager> g_em;
    extern SharedPtr<LineWorld> g_lw;
    
    extern bool bHdr;
    extern int colorGradingIndex;
    extern int playerType;
    extern int render_features;
    extern bool reflection;
    
    const float BASE_FOV = 45.0f;
    
    extern int freeze_ai;
    extern int game_type;
    extern int collision_type;

    extern int tmpdebugvalue;
    extern unsigned int cameraId;
    extern unsigned int playerId;
    
    extern int drawDebug; 
    extern bool d_log;
    extern bool mm_d_log;
    extern bool updateEnable;

    extern int colorGradingIndex; // shader

    extern float BGM_BASE_FREQ;
    extern bool nobgm;
    
    extern String CAMERA_NAME;
    extern String LUT;
    extern String UI_FONT;
    extern int UI_FONT_SIZE;
    extern String LOAD_FONT;
    extern int LOAD_FONT_SIZE;

    extern int test_enemy_num_override;
    extern int test_beat_index;
    extern bool base_on_player;
    extern int test_counter_index;
    extern int test_double_counter_index;
    extern int test_triple_counter_index;
    extern int maxKilled;

    extern int playerType;
    
    extern float AngleDiff(float diff);
    extern float FaceAngleDiff(Node* thisNode, Node* targetNode);
    extern int DirectionMapToIndex(float directionDifference, int numDirections);
    extern Vector3 FilterPosition(const Vector3& position);
    extern void FillAnimationWithCurrentPose(Animation* anim, Node* _node);
    extern String GetAnimationName(const String& name);
    extern void SendAnimationTriger(Node* _node, const StringHash& nameHash, int value = 0);
    extern void DebugDrawDirection(DebugRenderer* debug, const Vector3& start, float angle, const Color& color, float radius = 1.0);
    extern Vector4 GetTargetTransform(Node* baseNode, Motion* alignMotion, Motion* baseMotion);
    extern Vector3 GetRagdollForce();
}


