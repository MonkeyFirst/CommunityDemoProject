#pragma once

#include <Urho3D/Urho3DAll.h>

const float HEIGHT_128 = 3.25f;
const float HEIGHT_256 = HEIGHT_128 * 2;
const float HEIGHT_384 = HEIGHT_128 * 3;

enum LineType
{
    LINE_COVER,
    LINE_RAILING,
    LINE_EDGE,
    LINE_TYPE_NUM
};

enum LineFlags
{
    LINE_THIN_WALL  = (1 << 0),
    LINE_SHORT_WALL = (1 << 1),
};

const float LINE_MIN_LENGTH = 2.0f;
const float LINE_MAX_HEIGHT_DIFF = 15;

class Line
{
public:
    Line();
    ~Line();

    bool HasFlag(int flag);
    Vector3 Project(const Vector3& pos);
    Vector3 Project(const Vector3& pos, float angle);
    bool IsProjectPositionInLine(const Vector3& proj, float bound = 0.5f);
    Vector3 FixProjectPosition(const Vector3& proj, float bound = 0.5f);
    int GetHead(const Quaternion& rot);
    float GetHeadDirection(const Quaternion& rot);
    int GetTowardHead(float towardAngle);
    float Test(const Vector3& pos, float angle);
    bool IsAngleValid(float theAngle);
    void DebugDraw(DebugRenderer* debug, const Color& color);
    float GetProjectFacingDir(const Vector3& pos, float angle);
    Vector3 GetNearPoint(const Vector3& pos);
    Vector3 GetCenter();
    Vector3 GetLinePoint(const Vector3& dir);
    bool TestAngleDiff(Line* l, float diff, float maxError = 5);
    float HeightDifference(const Vector3& pos);
    float GetTargetRotation(const Vector3& pos);

    float AngleDiff( float diff );
    void DebugDrawDirection(DebugRenderer* debug, const Vector3& start, float angle, const Color& color, float radius = 1.0);

    Ray ray;
    Vector3 end;
    Vector3 invDir;
    Vector3 size;

    float length;
    float lengthSquared;
    int type;
    float angle;
    float maxFacingDiff;
    float invalidAngleSide;

    unsigned int nodeId;
    int flags;
};

class LineWorld : public Object
{
	URHO3D_OBJECT(LineWorld, Object);

public:
    LineWorld(Context* context);
    ~LineWorld();

    void Initialize();
    void Finalize();

    void DebugDraw(DebugRenderer* debug);
    void AddLine(Line* l);
    Line* CreateLine(int type, const Vector3& start, const Vector3& end, const Vector3& size, Node* node);
    void CreateLine(int type, Node* n);
    void Process(Scene* scene);
    Line* GetNearestLine(const Vector3& pos, float angle, float maxDistance);
    void CollectLines(const Vector3& pos, float angle, float maxDistance);
    //int CollectLinesByNode(Node* node, Array<Line*>* outLines);
    int CollectLinesByNode(Node* node, Vector<Line*> outLines);
    //int CollectLinesInBox(Scene* scene, const BoundingBox& box, unsigned int nodeToIgnore, Array<Line*>* outLines);
    int CollectLinesInBox(Scene* scene, const BoundingBox& box, unsigned int nodeToIgnore, Vector<Line*> outLines);

//    Array<Line*> lines;
//    Array<Color> debugColors;
//    Array<Line*> cacheLines;

    Vector<Line*> lines;
    Vector<Color> debugColors;
    Vector<Line*> cacheLines;
    //Array<float> cacheError;
};


