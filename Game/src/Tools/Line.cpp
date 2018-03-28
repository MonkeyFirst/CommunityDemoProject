//
//
//  128 --> 3.25
//  256 --> 6.5
//  384 --> 9.75
//
//

#include <Urho3D/Urho3DAll.h>
#include "Tools/Line.h"
#include "Objects/GameObject.h"

// GLOBAL FUNCTIONS ===========================

float dot(const Vector3& u, const Vector3& v)
{
    return u.DotProduct(v);
}

float norm2( const Vector3& v )
{
    return v.LengthSquared();
}

Vector3 cross( const Vector3& b, const Vector3& c) // cross product
{
    return b.CrossProduct(c);
}

bool intersection(const Vector3& a_1, const Vector3& a_2, const Vector3& b_1, const Vector3& b_2, Vector3& ip)
{
    Vector3 da = a_2 - a_1;
    Vector3 db = b_2 - b_1;
    Vector3 dc = b_1 - a_1;

    if (dot(dc, cross(da,db)) != 0.0) // lines are not coplanar
        return false;

    float s = dot(cross(dc,db), cross(da,db)) / norm2(cross(da,db));
    if (s >= 0.0 && s <= 1.0)
    {
        ip = a_1 + da * Vector3(s,s,s);
        return true;
    }

    return false;
}

bool GetNodeSizeAndOffset(Node* n, Vector3& size, Vector3& offset)
{
    CollisionShape* shape = n->GetComponent<CollisionShape>(); // ("CollisionShape");
    if (shape)
    {
        size = shape->GetSize(); // .size;
        offset = shape->GetPosition(); // .position;
        return true;
    }

    StaticModel* staticModel = n->GetComponent<StaticModel>(); // ("StaticModel");
    if (staticModel)
    {
        size = staticModel->GetBoundingBox().Size(); //.boundingBox.size;
        offset = Vector3(0, 0, 0);
        return true;
    }

    AnimatedModel* animateModel = n->GetComponent<AnimatedModel>(); // ("AnimatedModel");
    if (animateModel)
    {
        size = animateModel->GetBoundingBox().Size(); // boundingBox.size;
        offset = Vector3(0, 0, 0);
        return true;
    }

    return false;
}

int GetCorners(Node* n, Vector3& outSize, Vector<Vector3>& points) // * points)
{
    Vector3 size, offset;
    if (!GetNodeSizeAndOffset(n, size, offset))
        return 0;

    Vector3 halfSize = size/2;
    float x = size.x_ * n->GetWorldScale().x_; // .worldScale.x;
    float z = size.z_ * n->GetWorldScale().z_; // .worldScale.z;

    if (x < LINE_MIN_LENGTH || z < LINE_MIN_LENGTH)
    {
        if (x > z)
        {
            points.Push(Vector3(halfSize.x_, halfSize.y_, 0));
            points.Push(Vector3(-halfSize.x_, halfSize.y_, 0));
        }
        else
        {
            points.Push(Vector3(0, halfSize.y_, halfSize.z_));
            points.Push(Vector3(0, halfSize.y_, -halfSize.z_));
        }
    }
    else
    {
        points.Push(Vector3(halfSize.x_, halfSize.y_, halfSize.z_));
        points.Push(Vector3(halfSize.x_, halfSize.y_, -halfSize.z_));
        points.Push(Vector3(-halfSize.x_, halfSize.y_, -halfSize.z_));
        points.Push(Vector3(-halfSize.x_, halfSize.y_, halfSize.z_));
    }

    for (unsigned int i = 0; i < points.Size(); ++i)
        points[i] = n->LocalToWorld(points[i] + offset);

    outSize = size * n->GetWorldScale(); // worldScale;
    return int(points.Size());
}

// LINE ===============================
Line::Line()
{
    maxFacingDiff = 45;
    invalidAngleSide = 999;
}

Line::~Line()
{}


bool Line::HasFlag(int flag)
{
    return flags & flag != 0;
}

Vector3 Line::Project(const Vector3& pos)
{
    return ray.Project(pos);
}

Vector3 Line::Project(const Vector3& pos, float angle)
{
    float radius = 30.0f;
    Vector3 start = pos;
    start.y_ = end.y_;
    Vector3 v = start + Vector3(Sin(angle) * radius, 0, Cos(angle) * radius);
    Vector3 vo;
    bool b = intersection(ray.origin_, end, start, v, vo);
    return b ? vo : Project(pos);
}

bool Line::IsProjectPositionInLine(const Vector3& proj, float bound)
{
    float l_to_start = (proj - ray.origin_).Length();
    float l_to_end = (proj - end).Length();
    // check if project is in line
    if (l_to_start > length || l_to_end > length)
        return false;
    // check if project is in bound
    return l_to_start >= bound && l_to_end >= bound;
}

Vector3 Line::FixProjectPosition(const Vector3& proj, float bound)
{
    float l_to_start = (proj - ray.origin_).Length();
    float l_to_end = (proj - end).Length();
    bool bFix = false;
    if (l_to_start > length || l_to_end > length)
        bFix = true;
    if (l_to_start < bound || l_to_end < bound)
        bFix = true;
    if (!bFix)
        return proj;
    // URHO3D_LOGINFO("FixProjectPosition");
    if (l_to_start > l_to_end)
        return end + invDir * bound;
    else
        return ray.origin_ + ray.direction_ * bound;
}

// Used in other files and classes
// clamps an angle to the rangle of [-2PI, 2PI]
float Line::AngleDiff( float diff )
{
    if (diff > 180)
        diff -= 360;
    if (diff < -180)
        diff += 360;
    return diff;
}

int Line::GetHead(const Quaternion& rot)
{
    float yaw = rot.EulerAngles().y_;
    float diff = AngleDiff(angle - AngleDiff(yaw));
    // URHO3D_LOGINFO("yaw=" + yaw + " angle=" + angle + " diff=" + diff);
    if (Abs(diff) > 90)
        return 1;
    return 0;
}

float Line::GetHeadDirection(const Quaternion& rot)
{
    int head = GetHead(rot);
    Vector3 dir;
    if (head == 1)
        dir = ray.origin_ - end;
    else
        dir = end - ray.origin_;
    return Atan2(dir.x_, dir.z_);
}

int Line::GetTowardHead(float towardAngle)
{
    float angleDiff = Abs(AngleDiff(angle - towardAngle));
    if (angleDiff > 90)
        return 0;
    return 1;
}

float Line::Test(const Vector3& pos, float angle)
{
    if (Abs(pos.y_ - end.y_) > LINE_MAX_HEIGHT_DIFF)
        return -1;

    Vector3 project = ray.Project(pos);
    if (!IsProjectPositionInLine(project))
        return -1;

    project.y_ = pos.y_;
    Vector3 dir = project - pos;
    float projDir = Atan2(dir.x_, dir.z_);

    // test facing angle
    float aDiff = Abs(AngleDiff(projDir - angle));
    if (aDiff > maxFacingDiff)
        return -1;

    /*
    // test invalid angle side
    if (invalidAngleSide < 360) {
        aDiff = Abs(AngleDiff(projDir + 180 - invalidAngleSide));
        if (aDiff < 90)
        {
            return -1;
        }
    }
    */

    return dir.Length();
}

bool Line::IsAngleValid(float theAngle)
{
    return invalidAngleSide > 360 ? true : (Abs(AngleDiff(theAngle - invalidAngleSide)) < 90);
}

void Line::DebugDrawDirection(DebugRenderer* debug, const Vector3& start, float angle, const Color& color, float radius)
{
    Vector3 end = start + Vector3(Sin(angle) * radius, 0, Cos(angle) * radius);
    debug->AddLine(start, end, color, false);
}

void Line::DebugDraw(DebugRenderer* debug, const Color& color)
{
    //debug->AddCross(ray.origin_, 0.25f, RED, false);
    //debug->AddCross(end, 0.25f, BLUE, false);
    debug->AddSphere(Sphere(ray.origin_, 0.15f), Color::YELLOW, false);
    debug->AddSphere(Sphere(end, 0.15f), Color::YELLOW, false);
    debug->AddLine(ray.origin_, end, color, false);
    if (invalidAngleSide < 360)
        DebugDrawDirection(debug, (ray.origin_ + end)/2, invalidAngleSide, Color::RED);
}

float Line::GetProjectFacingDir(const Vector3& pos, float angle)
{
    Vector3 proj = Project(pos);
    proj.y_ = pos.y_;
    Vector3 dir = proj - pos;
    float projDir = Atan2(dir.x_, dir.z_);
    float aDiff = AngleDiff(projDir - angle);
    return Abs(aDiff);
}

Vector3 Line::GetNearPoint(const Vector3& pos)
{
    float start_sqr = (pos - ray.origin_).LengthSquared();
    float end_sqr = (pos - end).LengthSquared();
    return (start_sqr < end_sqr) ? ray.origin_ : end;
}

Vector3 Line::GetCenter()
{
    return ray.direction_ * length / 2 + ray.origin_;
}

Vector3 Line::GetLinePoint(const Vector3& dir)
{
    int towardHead = GetTowardHead(Atan2(dir.x_, dir.z_));
    return (towardHead == 0) ? ray.origin_ : end;
}

bool Line::TestAngleDiff(Line* l, float diff, float maxError)
{
    float angle_diff = Abs(AngleDiff(l->angle - this->angle));
    return Abs(angle_diff - diff) < maxError;
}

float Line::HeightDifference(const Vector3& pos)
{
    return Project(pos).y_ - pos.y_;
}

float Line::GetTargetRotation(const Vector3& pos)
{
    Vector3 dir = Project(pos) - pos;
    return Atan2(dir.x_, dir.z_);
}

// LINE WORLD ===============================

LineWorld::LineWorld(Context* context) :
    Object(context)
{
    Log::Write(-1," LineWorld: Constructor\n");

    debugColors.Resize(LINE_TYPE_NUM);
    debugColors[LINE_COVER] = Color(0.75f, 0.15f, 0.15f);
    debugColors[LINE_EDGE] = Color(0.25f, 0.5f, 0.75f);
}

LineWorld::~LineWorld()
{
    Log::Write(-1," LineWorld: Destructor\n");
}

void LineWorld::Initialize()
{
    Log::Write(-1," LineWorld: Initialize\n");
}

void LineWorld::Finalize()
{
    Log::Write(-1," LineWorld: Finalize\n");
}

void LineWorld::DebugDraw(DebugRenderer* debug)
{
    for (unsigned int i = 0; i < lines.Size(); ++i)
    {
        Line* l = lines[i];
        l->DebugDraw(debug, debugColors[l->type]);
    }
}

void LineWorld::AddLine(Line* l)
{
    lines.Push(l);
}

Line* LineWorld::CreateLine(int type, const Vector3& start, const Vector3& end, const Vector3& size, Node* node)
{
    Vector3 dir = end - start;
    float lenSQR = dir.LengthSquared();
    if (lenSQR < LINE_MIN_LENGTH*LINE_MIN_LENGTH)
        return NULL;

    Line* l = new Line();
    l->ray.origin_ = start;
    l->end = end;
    l->type = type;
    l->ray.direction_ = dir.Normalized();
    l->invDir = (start - end).Normalized();
    l->length = dir.Length();
    l->lengthSquared = lenSQR;
    l->angle = Atan2(dir.x_, dir.z_);
    l->nodeId = node->GetID(); // .id;
    l->size = size;

    // if (type != LINE_RAILING)
    {
        if (size.x_ < LINE_MIN_LENGTH || size.z_ < LINE_MIN_LENGTH)
            l->flags |= LINE_THIN_WALL;
    }
    if (size.y_ < 1.0f)
        l->flags |= LINE_SHORT_WALL;

    AddLine(l);
    // URHO3D_LOGINFO("CreateLine type=" + type + " for node=" + node.name + " size=" + size.ToString() + " flags=" + l.flags);
    return l;
}

void LineWorld::CreateLine(int type, Node* n)
{
    Vector3 size;
    Vector<Vector3> points;

    if (GetCorners(n, size, points) < 2)
        return;



    if (points.Size() == 4)
    {
        Line* l1 = CreateLine(type, points[0], points[1], size, n);
        Line* l2 = CreateLine(type, points[1], points[2], size, n);
        Line* l3 = CreateLine(type, points[2], points[3], size, n);
        Line* l4 = CreateLine(type, points[3], points[0], size, n);
        Vector3 dir = points[2] - points[1];
        l1->invalidAngleSide = Atan2(dir.x_, dir.z_);
        dir = points[3] - points[2];
        l2->invalidAngleSide = Atan2(dir.x_, dir.z_);
        dir = points[0] - points[3];
        l3->invalidAngleSide = Atan2(dir.x_, dir.z_);
        dir = points[1] - points[0];
        l4->invalidAngleSide = Atan2(dir.x_, dir.z_);
    }
    else
    {
        for (unsigned int i = 0; i < points.Size()-1; ++i)
        {
            CreateLine(type, points[i], points[i+1], size, n);
        }
    }
}

void LineWorld::Process(Scene* scene)
{
    for (unsigned int i = 0; i < scene->GetNumChildren(false); ++i) // scene.numChildren
    {
        Node* _node = scene->GetChildren()[i]; // .children[i];
        if (_node->GetName().StartsWith("Cover"))
        {
            CreateLine(LINE_COVER, _node);
        }
        else if (_node->GetName().StartsWith("Edge"))
        {
            CreateLine(LINE_EDGE, _node);
        }
    }
    Log::Write(-1," LineWorld: Scene Process ok\n");
}

Line* LineWorld::GetNearestLine(const Vector3& pos, float angle, float maxDistance)
{
    Line* ret = NULL;
    for (unsigned int i = 0; i < lines.Size(); ++i)
    {
        Line* l = lines[i];
        float dist = l->Test(pos, angle);
        if (dist < 0)
            continue;

        if (dist < maxDistance)
        {
            // URHO3D_LOGINFO("dist=" + dist + " maxDistance=" + maxDistance);
            maxDistance = dist;
            ret = l;
        }
    }
    return ret;
}

void LineWorld::CollectLines(const Vector3& pos, float angle, float maxDistance)
{
    cacheLines.Clear();
    for (unsigned int i = 0; i < lines.Size(); ++i)
    {
        Line* l = lines[i];
        float dist = l->Test(pos, angle);
        if (dist < 0)
            continue;
        if (dist < maxDistance)
            cacheLines.Push(l);
    }
}

int LineWorld::CollectLinesByNode(Node* node, Vector<Line*> outLines)
{
    // outLines.Clear();

    for (unsigned int i = 0; i < lines.Size(); ++i)
    {
        if (lines[i]->nodeId == node->GetID())
            outLines.Push(lines[i]);
    }

    // URHO3D_LOGINFO("CollectLinesByNode " + node.name + " num=" + outLines.length);

    return outLines.Size();
}

int LineWorld::CollectLinesInBox(Scene* scene, const BoundingBox& box, unsigned int nodeToIgnore, Vector<Line*> outLines)
{
/// Return rigid bodies by a sphere query.
//void GetRigidBodies(PODVector<RigidBody*>& result, const Sphere& sphere, unsigned collisionMask = M_MAX_UNSIGNED);
/// Return rigid bodies by a box query.
//void GetRigidBodies(PODVector<RigidBody*>& result, const BoundingBox& box, unsigned collisionMask = M_MAX_UNSIGNED);
/// Return rigid bodies by contact test with the specified body. It needs to be active to return all contacts reliably.
//void GetRigidBodies(PODVector<RigidBody*>& result, const RigidBody* body);

    outLines.Clear();
    
    PODVector<RigidBody*> bodies;
    //Vector<RigidBody*> bodies = scene->GetComponent<PhysicsWorld>()->GetRigidBodies(box, COLLISION_LAYER_LANDSCAPE);
    scene->GetComponent<PhysicsWorld>()->GetRigidBodies(bodies, box, COLLISION_LAYER_LANDSCAPE);
    // URHO3D_LOGINFO("CollectLinesInBox bodies.num=" + bodies.length);
    if (bodies.Empty())
        return 0;

    for (unsigned int i = 0; i < bodies.Size(); ++i)
    {
        Node* n = bodies[i]->GetNode(); // node;
        if (n->GetID() == nodeToIgnore)
            continue;
        CollectLinesByNode(n, outLines);
    }

    return outLines.Size();
}



