
#include <Urho3D/Urho3DAll.h>

#include "Character/PhysicsSensor.h"
#include "Character/CharacterStates.h"

#include "Motions/MotionManager.h"

#include "radio.h"

PhysicsSensor::PhysicsSensor(Context* context, Node* n) :
    Object(context)
{
    grounded = true;
    inAirHeight = 0.0f;
    inAirFrames = 0;
    halfHeight = CHARACTER_HEIGHT / 2;

    sceneNode = n;
    sensorNode = sceneNode->CreateChild("SensorNode");
    shape = sensorNode->CreateComponent<CollisionShape>(); // ("CollisionShape");
    shape->SetCapsule(COLLISION_RADIUS, CHARACTER_HEIGHT, Vector3(0.0f, CHARACTER_HEIGHT / 2, 0.0f));
    verticalShape = sensorNode->CreateComponent<CollisionShape>(); // ("CollisionShape");
    verticalShape->SetBox(Vector3(0.2f, CHARACTER_HEIGHT, 0));
    horinzontalShape = sensorNode->CreateComponent<CollisionShape>(); // ("CollisionShape");
    horinzontalShape->SetBox(Vector3(COLLISION_RADIUS * 2, 0.2f, 0));
}

PhysicsSensor::~PhysicsSensor()
{
    sensorNode->Remove();
}

void PhysicsSensor::Update(float dt)
{
    start = sceneNode->GetWorldPosition(); // worldPosition;
    end = start;
    start.y_ += halfHeight;
    float addlen = 30.0f;
    end.y_ -= addlen;

    /// Perform a physics world swept convex test using a user-supplied collision shape and return the first hit.
//    void ConvexCast(PhysicsRaycastResult& result, CollisionShape* shape, const Vector3& startPos, const Quaternion& startRot,
//        const Vector3& endPos, const Quaternion& endRot, unsigned collisionMask = M_MAX_UNSIGNED);

    /// Perform a physics world swept convex test using a user-supplied Bullet collision shape and return the first hit.
//    void ConvexCast(PhysicsRaycastResult& result, btCollisionShape* shape, const Vector3& startPos, const Quaternion& startRot,
//        const Vector3& endPos, const Quaternion& endRot, unsigned collisionMask = M_MAX_UNSIGNED);

//    PhysicsRaycastResult result = sceneNode.scene.physicsWorld.ConvexCast(shape, start, Quaternion(), end, Quaternion(), COLLISION_LAYER_LANDSCAPE | COLLISION_LAYER_PROP);
    PhysicsRaycastResult result;
    sceneNode->GetScene()->GetComponent<PhysicsWorld>()->ConvexCast(result, shape, start, Quaternion(), end, Quaternion(), COLLISION_LAYER_LANDSCAPE | COLLISION_LAYER_PROP);

    if (result.body_)
    {
        end = result.position_;

        float h = start.y_ - end.y_;
        if (h > halfHeight + 0.5f)
            grounded = false;
        else
            grounded = true;
        inAirHeight = h;
    }
    else
    {
        grounded = false;
        inAirHeight = addlen + 1;
    }

    if (grounded)
    {
        inAirFrames = 0;
        inAirHeight = 0.0f;
    }
    else
    {
        inAirFrames ++;
        
//        Log::Write(-1," In Air! = " + String(inAirFrames) + "\n");
    }
}

void PhysicsSensor::DebugDraw(DebugRenderer* debug)
{
    debug->AddLine(start, end, grounded ? Color::GREEN : Color::RED, false);
}

Vector3 PhysicsSensor::GetGround(const Vector3& pos)
{
    Vector3 start = pos;
    start.y_ += 1.0f;
    Ray ray;
    ray.Define(start, Vector3(0, -1, 0));

    /// Perform a physics world raycast and return the closest hit.
//    void RaycastSingle(PhysicsRaycastResult& result, const Ray& ray, float maxDistance, unsigned collisionMask = M_MAX_UNSIGNED);

    PhysicsRaycastResult result;
    sceneNode->GetScene()->GetComponent<PhysicsWorld>()->RaycastSingle(result, ray, 30.0f, COLLISION_LAYER_LANDSCAPE);

    return (result.body_) ? result.position_ : end;
}

int PhysicsSensor::DetectWallBlockingFoot(float dist)
{
    int ret = 0;
    Node* footLeft = sceneNode->GetChild(L_FOOT, true);
    Node* foootRight = sceneNode->GetChild(R_FOOT, true);
    PhysicsWorld* world = sceneNode->GetScene()->GetComponent<PhysicsWorld>(); // .scene.physicsWorld;

    Vector3 dir = sceneNode->GetWorldRotation() * Vector3(0, 0, 1);
    Ray ray;
    ray.Define(footLeft->GetWorldPosition(), dir);
    PhysicsRaycastResult result;

    world->RaycastSingle(result, ray, dist, COLLISION_LAYER_LANDSCAPE);
    if (result.body_)
        ret ++;

    ray.Define(foootRight->GetWorldPosition(), dir);

    world->RaycastSingle(result, ray, dist, COLLISION_LAYER_LANDSCAPE);
    if (result.body_)
        ret ++;

    return ret;
}













