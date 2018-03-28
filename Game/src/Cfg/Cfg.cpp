#include "Cfg/Cfg.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/Variant.h>

#include <Urho3D/Container/Str.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Container/HashSet.h>

#include <Urho3D/IO/File.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/IO/Log.h>

using namespace Urho3D;

// Переопределение конструктора без имени файла.
Cfg::Cfg(Context* context)
    : Object(context)
{

}

// Конструктор.  Если он может загрузить файл, он будет делать это и заполнит
// список Секторов значениями из файла.
Cfg::Cfg(Context* context, String fileName)
    : Object(context)
{
	Dirty_ = false;

	t_Sector pTmp ;
	Sectors_.Push( pTmp );

	Load(fileName);
}

// Сохранение файла если какие-то значения были изменены с момента последнего сохранения.
Cfg::~Cfg()
{
	if ( Dirty_ )
		Save();

	Reset() ;
}

// Сброс переменных-членов в значения по умолчанию
void Cfg::Reset()
{
	Dirty_ = false;
	FileName_ = "";
	Sectors_.Clear();
}

// Устанавливаем значение FileName_ переменной-члена. Для использования при создании INIParser
// объекта вручную (без или с загрузкой из файла)
void Cfg::SetFileName(String fileName)
{
	if (FileName_.Length() != 0 && CompareNoCase(fileName, FileName_) != true)
	{
		Dirty_ = true;

//		Report(E_WARN, "[INIParser::SetFileName] The filename has changed from <%s> to <%s>.",
//			   FileName_.c_str(), fileName.c_str());
	}

	FileName_ = fileName;
}

// Пытается загрузить текстовый файл. В случае успеха будет заполнен
// список секторов с парами ключ/значение взятыми из файла. Обратите внимание, что комментарии
// Сохраняются, поэтому они могут быть переписаны в файл позже.
bool Cfg::Load(String fileName)
{
	// Если загружаем поверх и существующий не сохранен то сохранить его сначала.
	if ( Dirty_ ) 
	{
		// Сохранить только если есть что сохранять
		if ( KeysCount() != 0 || SectorsCount() != 0 ) 
		{
			// Сохранить только если имя файла правильное (существует)
			if ( FileName_.Length() != 0 )
				Save();
		}
	}

    File file(context_, fileName, Urho3D::FILE_READ);

    unsigned dataSize = file.GetSize();

    if (!dataSize && !file.GetName().Empty())
    {
         URHO3D_LOGERROR("Zero sized data in - " + file.GetName());
         return false;
    }

    if ( file.IsOpen() )
    {
    	String Line;
    	String comment;
    	t_Sector* pSector = GetSector("");
    
        while (!file.IsEof())
        {
             // Построчное чтение файла
             String line = file.ReadLine().Trimmed();
    
    		if ( line.Find("#") != String::NPOS )
    		{
    		    line.Erase( 0, 1 );
    		    if (comment.Length() > 0 )
    		    {
    		        comment += "\n# ";
    		    }
    			comment += line;//.Trimmed();
    		}
    		else if ( line.Find("[") != String::NPOS ) // новый сектор
    		{
    			line.Erase( 0, 1 );
    			line.Erase( line.FindLast(']'), 1 );
    
    			CreateSector(line, comment);
    			pSector = GetSector(line);
    			comment = ""; // String::EMPTY
    		}        
    		else if ( line.Length() > 0 ) // мы имеем ключь, добовляем его в key/value пары
    		{
    			String key = GetNextWord(line);
    			String value = line;
    
    			if ( key.Length() > 0 && value.Length() > 0 )
    			{
    				SetValue(key, value, comment, pSector->sectorName);
    				comment = ""; // String::EMPTY
    			}
    		}
        }
    }
    else
    {
		FileOpen_ = false;
        URHO3D_LOGERROR("[Cfg::Load] Unable to open file. Does it exist?");
		return false;    
    }
    
	file.Close();

	// Если файл только что был загружен, то данные не могут быть грязными!
	Dirty_ = false ;
	// Запись имени файла после успешной загрузки
	FileName_ = fileName;

	return true;
}

// Попытка сохранить список секторов и ключи в файл. Заметим, что если Load
// функция не вызывалась (INIParser объект был создан вручную), то вы
// Должны установить значение FileName_ переменной перед вызовом Save.
bool Cfg::Save()
{
	if ( KeysCount() == 0 && SectorsCount() == 0 )
	{
		// нет смысла в сохранении
		URHO3D_LOGERROR("[Cfg::Save] Empty data");
		return false; 
	}

	if ( FileName_.Length() == 0 )
	{
		URHO3D_LOGERROR("[Cfg::Save] No filename has been set");
		return false;
	}

	File file(context_, FileName_, Urho3D::FILE_WRITE);

	if ( file.IsOpen() )
	{
		FileSave_ = true;

		SectorItor s_pos;
		KeyItor k_pos;
		t_Sector Sector;
		t_Key Key;

		for (s_pos = Sectors_.Begin(); s_pos != Sectors_.End(); s_pos++)
		{
			Sector = (*s_pos);
			bool bWroteComment = false;

			if ( Sector.comment.Length() > 0 )
			{
				bWroteComment = true;
				file.WriteLine("");
				file.WriteLine("# " + Sector.comment); // "# " + 
			}

			if ( Sector.sectorName.Length() > 0 )
			{
			    if (!bWroteComment)
			    {
			        file.WriteLine("");
			    }
                file.WriteLine("[" + Sector.sectorName + "]");
			}

			for (k_pos = Sector.Keys.Begin(); k_pos != Sector.Keys.End(); k_pos++)
			{
				Key = (*k_pos);

				if ( Key.key.Length() > 0 && Key.value.Length() > 0 )
				{
        			if ( Key.comment.Length() > 0 )
        			{
        				file.WriteLine("# " + Key.comment); // \n
        			}
                    
                    file.WriteLine(Key.key + " = " + Key.value);
				}
			}
		}		
	}
	else
	{
		FileSave_ = false;
		URHO3D_LOGERROR("[Cfg::Save] Unable to save file.");
		return false;
	}

	Dirty_ = false;
	
	file.Flush();
	file.Close();

	return true;
}

// Комментарование заданного ключа. Возвращает false, если ключ не найден.
bool Cfg::CommentKey(String key, String comment, String sector)
{
	KeyItor k_pos;
	t_Sector* pSector;

	if ( (pSector = GetSector(sector)) == NULL )
		return false;

	for (k_pos = pSector->Keys.Begin(); k_pos != pSector->Keys.End(); k_pos++)
	{
		if ( CompareNoCase( (*k_pos).key, key ) )
		{
			(*k_pos).comment = comment;
			Dirty_ = true;
			return true;
		}
	}
	return false;
}

// Комментирование заданного сектора. Возвращает FALSE, если сектор не найден.
bool Cfg::CommentSector(String sector, String comment)
{
	SectorItor s_pos;

	for (s_pos = Sectors_.Begin(); s_pos != Sectors_.End(); s_pos++)
	{
		if ( CompareNoCase( (*s_pos).sectorName, sector ) ) 
		{
		    (*s_pos).comment = comment;
			Dirty_ = true;
			return true;
		}
	}
	return false;
}

String Cfg::GetSectorName(int Num)
{
    if (Sectors_[Num].sectorName.Length())
	{
	    return Sectors_[Num].sectorName;
    }
	else
	{
        return String::EMPTY;
	}
}

void Cfg::GetParams(String key, String sector, char separator, Vector<String> &retParams)
{
	String KeyString = GetValue(key, sector);
	if(KeyString.Length())
	{
        retParams = KeyString.Split(separator, false);
	}
}

// Получая ключ, значение и сектор, эта функция будет пытаться найти
// ключь в заданном секторе, и если найдет его, изменит его значение на новое.
// Если не найдет ключ, то будет создан новый ключ в заданном секторе и присвоено ему значение
bool Cfg::SetValue(String key, String value, String comment, String sector)
{
	t_Key* pKey = GetKey(key, sector);
	t_Sector* pSector = GetSector(sector);

	if (pSector == NULL)
	{
		if ( !CreateSector(sector,"")) // !(Flags_ & AUTOCREATE_SECTORS) ||
			return false;

		pSector = GetSector(sector);
	}

	// Разумная проверка ...
	if ( pSector == NULL )
		return false;

	// Если ключ не существует в этом секторе, и переданное значение 
	// не пустое, то добавить новый ключь.
	if ( pKey == NULL && value.Length() > 0 )//&& (Flags_ & AUTOCREATE_KEYS))
	{
		pKey = new t_Key;

		pKey->key = key;
		pKey->value = value;
		pKey->comment = comment;
		
		Dirty_ = true;
		pSector->Keys.Push(*pKey);
		delete pKey ;
		return true;
	}

	if ( pKey != NULL )
	{
		pKey->value = value;
		pKey->comment = comment;

		Dirty_ = true;
		
		return true;
	}

	return false;
}

// Преобразует полученный var в строку для SetValue
bool Cfg::SetVar(String key, Urho3D::Variant var, String comment, String sector)
{
    return SetValue(key, var.ToString(), comment, sector);
}

// Возвращает значение ключа как std::string объект. Возвращаемое значение
// std::string("") говорит о том что ключь не был найден.
String Cfg::GetValue(String key, String sector) 
{
	t_Key* pKey = GetKey(key, sector);
	
	if (pKey == NULL)
	{
	   return String::EMPTY;
	}
	else
	{
	   return pKey->value.Trimmed();
	}
}

const String Cfg::GetString(const String& section, const String& parameter, const String& defaultValue)
{
    String ret = GetValue(parameter, section);

    if (ret == String::EMPTY)
    {
        return defaultValue;
    }
    else
    {
        return ret;
    }
}

const float Cfg::GetFloat(const String& section, const String& parameter, const float defaultValue)
{
    String property = GetValue(parameter, section);

    if (property == String::EMPTY)
        return defaultValue;

    return ToFloat(property);
}

const double Cfg::GetDouble(const String& section, const String& parameter, const double defaultValue)
{
    String property = GetValue(parameter, section);

    if (property == String::EMPTY)
        return defaultValue;

    return ToDouble(property);
}

const int Cfg::GetInt(const String& section, const String& parameter, const int defaultValue)
{
    String property = GetValue(parameter, section);

    if (property == String::EMPTY)
        return defaultValue;

    return ToInt(property);
}

const bool Cfg::GetBool(const String& section, const String& parameter, const bool defaultValue)
{
    String property = GetValue(parameter, section);

    if (property == String::EMPTY)
        return defaultValue;

    return ToBool(property);
}

const Vector2 Cfg::GetVector2(const String& section, const String& parameter, const Vector2& defaultValue)
{
    String property = GetValue(parameter, section);

    if (property == String::EMPTY)
        return defaultValue;

    return ToVector2(property);
}

const Vector3 Cfg::GetVector3(const String& section, const String& parameter, const Vector3& defaultValue)
{
    String property = GetValue(parameter, section);
    
    if (property == String::EMPTY)
        return defaultValue;
    
    return ToVector3(property);
}

const Vector4 Cfg::GetVector4(const String& section, const String& parameter, const Vector4& defaultValue)
{
    String property = GetValue(parameter, section);
    
    if (property == String::EMPTY)
        return defaultValue;
    
    return ToVector4(property);
}

const Quaternion Cfg::GetQuaternion(const String& section, const String& parameter, const Quaternion& defaultValue)
{
    String property = GetValue(parameter, section);
    
    if (property == String::EMPTY)
        return defaultValue;
    
    return ToQuaternion(property);
}

const Color Cfg::GetColor(const String& section, const String& parameter, const Color& defaultValue)
{
    String property = GetValue(parameter, section);
    
    if (property == String::EMPTY)
        return defaultValue;
    
    return ToColor(property);
}

const IntRect Cfg::GetIntRect(const String& section, const String& parameter, const IntRect& defaultValue)
{
    String property = GetValue(parameter, section);
    
    if (property == String::EMPTY)
        return defaultValue;
    
    return ToIntRect(property);
}

const IntVector2 Cfg::GetIntVector2(const String& section, const String& parameter, const IntVector2& defaultValue)
{
    String property = GetValue(parameter, section);
    
    if (property == String::EMPTY)
        return defaultValue;
    
    return ToIntVector2(property);
}

const Matrix3 Cfg::GetMatrix3(const String& section, const String& parameter, const Matrix3& defaultValue)
{
    String property = GetValue(parameter, section);
    
    if (property == String::EMPTY)
        return defaultValue;
    
    return ToMatrix3(property);
}

const Matrix3x4 Cfg::GetMatrix3x4(const String& section, const String& parameter, const Matrix3x4& defaultValue)
{
    String property = GetValue(parameter, section);
    
    if (property == String::EMPTY)
        return defaultValue;
    
    return ToMatrix3x4(property);
}

const Matrix4 Cfg::GetMatrix4(const String& section, const String& parameter, const Matrix4& defaultValue)
{
    String property = GetValue(parameter, section);
    
    if (property == String::EMPTY)
        return defaultValue;
    
    return ToMatrix4(property);
}

// Удаляет указанный сектор. Возвращает false если сектор не найден
// или true после удаления.
bool Cfg::DeleteSector(String sector)
{
	SectorItor s_pos;

	for (s_pos = Sectors_.Begin(); s_pos != Sectors_.End(); s_pos++)
	{
		if ( CompareNoCase( (*s_pos).sectorName, sector ) ) 
		{
			Sectors_.Erase(s_pos);
			return true;
		}
	}

	return false;
}

// Удаляет указанный ключь в указанном секторе. Возвращает false еси ключь не найден
// или true после удаления.
bool Cfg::DeleteKey(String key, String fromSector)
{
	KeyItor k_pos;
	t_Sector* pSector;

	if ( (pSector = GetSector(fromSector)) == NULL )
		return false;

	for (k_pos = pSector->Keys.Begin(); k_pos != pSector->Keys.End(); k_pos++)
	{
		if ( CompareNoCase( (*k_pos).key, key ) )
		{
			pSector->Keys.Erase(k_pos);
			return true;
		}
	}
	return false;
}

// Получая ключь, значение и сектор, эта функция будет пытаться найти
// ключь в заданном секторе, и если найдет, изменит его значение на новое
// Если не найдет, то создаст новый ключ, присвоит значение и поместит ключь в заданный сектор.
bool Cfg::CreateKey(String key, String value, String comment, String sector)
{
	return SetValue(key, value, comment, sector);
}

// Получая имя сектора, Эта функция проверит, существует ли данный сектор
// в списке или нет, если нет, то создаст новый сектор и
// Присвоит ему комментарий переданный в comment. Функция возвращает true, если
// успешно создан, или false в противном случае.
bool Cfg::CreateSector(String sector, String comment)
{
	t_Sector* pSector = GetSector(sector);

	if ( pSector )
	{
        URHO3D_LOGERROR("[Cfg::CreateSector] Sector " + sector +" allready exists.");
		return false;
	}

	pSector = new t_Sector;

	pSector->sectorName = sector;
	pSector->comment = comment;
	Sectors_.Push(*pSector);

	delete pSector ;
	Dirty_ = true;

	return true;
}

// Получая имя сектора, Эта функция проверит, существует ли данный сектор
// в списке или нет, если нет, то создаст новый сектор и
// Присвоит ему комментарий переданный в comment. Функция возвращает true, если
// успешно создан, или false в противном случае.
// Эта версия принимает KeyList и создает сектор с ключами из этого списка.
bool Cfg::CreateSector(String sector, String comment, KeyList Keys)
{
	if ( !CreateSector(sector, comment) )
		return false;

	t_Sector* pSector = GetSector(sector);

	if ( !pSector )
		return false;

	KeyItor k_pos;

	pSector->sectorName = sector;
	for (k_pos = Keys.Begin(); k_pos != Keys.End(); k_pos++)
	{
		t_Key* pKey = new t_Key;
		pKey->comment = (*k_pos).comment;
		pKey->key = (*k_pos).key;
		pKey->value = (*k_pos).value;

		pSector->Keys.Push(*pKey);

		delete pKey ;
	}

	Sectors_.Push(*pSector);
	Dirty_ = true;

	return true;
}

// Просто возвращает количество секторов в списке.
int Cfg::SectorsCount() 
{ 
	return Sectors_.Size(); 
}

// Возвращает общее количество ключей, в указанном секторе.
int Cfg::KeysCount(String sector)
{
	int nCounter = 0;

    t_Sector* sct =	GetSector(sector);
    nCounter = sct->Keys.Size();

	return nCounter;
}

// KeysCount
// Возвращает общее количество ключей, содержащихся во всех секторах.
int Cfg::KeysCount()
{
	int nCounter = 0;
	SectorItor s_pos;

	for (s_pos = Sectors_.Begin(); s_pos != Sectors_.End(); s_pos++)
		nCounter += (*s_pos).Keys.Size();

	return nCounter;
}

String Cfg::GetKeyName(String sector, int numInVector)
{
    t_Sector* sct = GetSector(sector);
    return sct->Keys[numInVector].key;
}

// Защищенные функции-члены 
//==========================

// Получая имя сектора, находит сектор в списке и возвращает его указатель
// Если сектор не найден, возвращает NULL
t_Sector* Cfg::GetSector(String sector)
{
	SectorItor s_pos;

	for (s_pos = Sectors_.Begin(); s_pos != Sectors_.End(); s_pos++)
	{
		if ( CompareNoCase( (*s_pos).sectorName, sector )) 
			return (t_Sector*)&(*s_pos);
	}

	return NULL;
}

// Получая ключь и сектор, ищет ключ и, если находит, возвращает
// Указатель на ключ, в противном случае возвращает NULL.
t_Key*	Cfg::GetKey(String key, String sector)
{
	KeyItor k_pos;
	t_Sector* pSector;

	// Since our default Sector has a name value of std::string("") this should
	// always return a valid Sector, wether or not it has any keys in it is
	// another matter.
	if ( (pSector = GetSector(sector)) == NULL )
		return NULL;

	for (k_pos = pSector->Keys.Begin(); k_pos != pSector->Keys.End(); k_pos++)
	{
		if ( CompareNoCase( (*k_pos).key, key ))
			return (t_Key*)&(*k_pos);
	}

	return NULL;
}


bool Cfg::Has(const String& key, const String& sector)
{
    return GetValue(key, sector) != String::EMPTY;
}


// Вспомогательные функции 
//=========================

// Given a key +delimiter+ value string, pulls the key name from the string,
// deletes the delimiter and alters the original string to contain the
// remainder.  Returns the key
String GetNextWord(String& CommandLine)
{
	int nPos = CommandLine.Find("=");
	String sWord = "";

	if ( nPos > -1 )
	{
		sWord = CommandLine.Substring(0, nPos);
		CommandLine.Erase(0, nPos+1);
	}
	else
	{
		sWord = CommandLine;
		CommandLine = "";
	}

	//Trim(sWord);
	return sWord.Trimmed();
}

bool CompareNoCase(String str1, String str2)
{
    if (str1.ToLower() == str2.ToLower())
    return true;
    else
    return false;
}