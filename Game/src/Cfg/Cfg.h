#pragma once

#include <Urho3D/Urho3D.h>
//#include <Urho3D/Resource/Resource.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Core/StringUtils.h>
#include <Urho3D/Core/Object.h>

#include <Urho3D/IO/Deserializer.h>
#include <Urho3D/IO/Serializer.h>

namespace Urho3D
{
    class File;
    class Variant;
}

using namespace Urho3D;

enum ConfigType
{
    CFG_TMP,
    CFG_GAME,
    CFG_LEVEL,
    CFG_STATIC,
    CFG_END,
};

typedef struct st_key
{
	String		key;
	String		value;
	String		comment;

	st_key()
	{
		key = String("");
		value = String("");
		comment = String("");
	}

} t_Key;

typedef Vector<t_Key> KeyList;
typedef KeyList::Iterator KeyItor;


typedef struct st_Sector
{
	String		sectorName;
	String		comment;
	KeyList		Keys;

	st_Sector()
	{
		sectorName = String("");
		comment = String("");
		Keys.Clear();
	}

} t_Sector;

typedef Vector<t_Sector> SectorList;
typedef SectorList::Iterator SectorItor;

// Функции общего назначения
//==================================
String GetNextWord(String& CommandLine);
bool CompareNoCase(String str1, String str2);
//==================================

class Cfg : public Object
{
    URHO3D_OBJECT(Cfg, Object);

public:

    Cfg(Context* context);
    Cfg(Context* context, String fileName);
    ~Cfg();

    bool Load(String fileName);
    bool Save();

    bool CommentKey(String key, String comment, String sector);
    bool CommentSector(String sector, String comment);
    String GetSectorName(int Num);
    String GetKeyName(String sector, int numInVector);
    void GetParams(String key, String sector, char separator, Vector<String> &retParams);
    bool SetValue(String key, String value, String comment, String sector);
    bool SetVar(String key, Urho3D::Variant var, String comment, String sector);

    const double GetDouble(const String& section, const String& parameter, const double defaultValue);
    const String GetString(const String& section, const String& parameter, const String& defaultValue = String::EMPTY);
    const int GetInt(const String& section, const String& parameter, const int defaultValue = 0);
    const bool GetBool(const String& section, const String& parameter, const bool defaultValue = false);
    const float GetFloat(const String& section, const String& parameter, const float defaultValue = 0.f);
    const Vector2 GetVector2(const String& section, const String& parameter, const Vector2& defaultValue = Vector2::ZERO);
    const Vector3 GetVector3(const String& section, const String& parameter, const Vector3& defaultValue = Vector3::ZERO);
    const Vector4 GetVector4(const String& section, const String& parameter, const Vector4& defaultValue = Vector4::ZERO);
    const Quaternion GetQuaternion(const String& section, const String& parameter, const Quaternion& defaultValue = Quaternion::IDENTITY);
    const Color GetColor(const String& section, const String& parameter, const Color& defaultValue = Color::WHITE);
    const IntRect GetIntRect(const String& section, const String& parameter, const IntRect& defaultValue = IntRect::ZERO);
    const IntVector2 GetIntVector2(const String& section, const String& parameter, const IntVector2& defaultValue = IntVector2::ZERO);
    const Matrix3 GetMatrix3(const String& section, const String& parameter, const Matrix3& defaultValue = Matrix3::IDENTITY);
    const Matrix3x4 GetMatrix3x4(const String& section, const String& parameter, const Matrix3x4& defaultValue = Matrix3x4::IDENTITY);
    const Matrix4 GetMatrix4(const String& section, const String& parameter, const Matrix4& defaultValue = Matrix4::IDENTITY);

    bool DeleteSector(String sector);
    bool DeleteKey(String key, String fromSector);
    bool CreateKey(String key, String value, String comment, String sector);
    bool CreateSector(String sector, String comment);
    bool CreateSector(String sector, String comment, KeyList Keys);

    int KeysCount();
    int KeysCount(String sector);
    int SectorsCount(); 

    bool Has(const String& key, const String& sector);
    String GetValue(String key, String sector);
    
    void Reset();
    void SetFileName(String fileName);

    int SetType(int cfgType){type = cfgType;}
    
    String fileName;
    StringHash fileHash;
    int type;

protected:

    t_Sector* GetSector(String sector);
    t_Key*	GetKey(String key, String sector);
    
	SectorList	Sectors_;		 // Наш список секторов
	String	FileName_;	 // Имя файла в который писать
	bool		Dirty_;		 // Треки были ли данные изменены или нет.
	bool		FileOpen_;		 // Если не существует load() false
	bool		FileSave_;		 // Если не enable save() false
};
