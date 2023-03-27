#include "scripting/angel-script/addons/scriptfile.h"
#include <new>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32_WCE
#include <windows.h> // For GetModuleFileName
#ifdef GetObject
#undef GetObject
#endif
#endif

using namespace lambda;

BEGIN_AS_NAMESPACE

CScriptFile *ScriptFile_Factory()
{
	return new CScriptFile();
}

void ScriptFile_Factory_Generic(asIScriptGeneric *gen)
{
	*(CScriptFile**)gen->GetAddressOfReturnLocation()	= ScriptFile_Factory();
}

void ScriptFile_AddRef_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	File->AddRef();
}

void ScriptFile_Release_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	File->Release();
}

void ScriptFile_Open_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	String *f = (String*)gen->GetArgAddress(0);
	String *m = (String*)gen->GetArgAddress(1);
	int r = File->Open(*f, *m);
	gen->SetReturnDWord(r);
}

void ScriptFile_Close_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	int r = File->Close();
	gen->SetReturnDWord(r);
}

void ScriptFile_GetSize_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	int r = File->GetSize();
	gen->SetReturnDWord(r);
}

void ScriptFile_ReadString_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	int len = gen->GetArgDWord(0);
	String str = File->ReadString(len);
	gen->SetReturnObject(&str);
}

void ScriptFile_ReadLine_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	String str = File->ReadLine();
	gen->SetReturnObject(&str);
}

void ScriptFile_ReadInt_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	asUINT bytes = *(asUINT*)gen->GetAddressOfArg(0);
	*(asINT64*)gen->GetAddressOfReturnLocation() = File->ReadInt(bytes);
}

void ScriptFile_ReadUInt_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	asUINT bytes = *(asUINT*)gen->GetAddressOfArg(0);
	*(asQWORD*)gen->GetAddressOfReturnLocation() = File->ReadUInt(bytes);
}

void ScriptFile_ReadFloat_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	*(float*)gen->GetAddressOfReturnLocation() = File->ReadFloat();
}

void ScriptFile_ReadDouble_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	*(double*)gen->GetAddressOfReturnLocation() = File->ReadDouble();
}

void ScriptFile_WriteString_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	String *str = (String*)gen->GetArgAddress(0);
	gen->SetReturnDWord(File->WriteString(*str));
}

void ScriptFile_WriteInt_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	asINT64 val = *(asINT64*)gen->GetAddressOfArg(0);
	asUINT bytes = *(asUINT*)gen->GetAddressOfArg(1);
	*(int*)gen->GetAddressOfReturnLocation() = File->WriteInt(val, bytes);
}

void ScriptFile_WriteUInt_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	asQWORD val = *(asQWORD*)gen->GetAddressOfArg(0);
	asUINT bytes = *(asUINT*)gen->GetAddressOfArg(1);
	*(int*)gen->GetAddressOfReturnLocation() = File->WriteUInt(val, bytes);
}

void ScriptFile_WriteFloat_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	float val = *(float*)gen->GetAddressOfArg(0);
	*(int*)gen->GetAddressOfReturnLocation() = File->WriteFloat(val);
}

void ScriptFile_WriteDouble_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	double val = *(double*)gen->GetAddressOfArg(0);
	*(int*)gen->GetAddressOfReturnLocation() = File->WriteDouble(val);
}

void ScriptFile_IsEOF_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	bool r = File->IsEOF();
	gen->SetReturnByte(r);
}

void ScriptFile_GetPos_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	gen->SetReturnDWord(File->GetPos());
}

void ScriptFile_SetPos_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	int pos = (int)gen->GetArgDWord(0);
	gen->SetReturnDWord(File->SetPos(pos));
}

void ScriptFile_MovePos_Generic(asIScriptGeneric *gen)
{
	CScriptFile *File = (CScriptFile*)gen->GetObject();
	int delta = (int)gen->GetArgDWord(0);
	gen->SetReturnDWord(File->MovePos(delta));
}

void RegisterScriptFile_Native(asIScriptEngine *engine)
{
    int r;

    r = engine->RegisterObjectType("File", 0, asOBJ_REF); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("File", asBEHAVE_FACTORY, "File @f()", asFUNCTION(ScriptFile_Factory), asCALL_CDECL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("File", asBEHAVE_ADDREF, "void f()", asMETHOD(CScriptFile,AddRef), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("File", asBEHAVE_RELEASE, "void f()", asMETHOD(CScriptFile,Release), asCALL_THISCALL); assert( r >= 0 );

    r = engine->RegisterObjectMethod("File", "int Open(const String &in, const String &in)", asMETHOD(CScriptFile,Open), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod("File", "int Close()", asMETHOD(CScriptFile,Close), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int GetSize() const", asMETHOD(CScriptFile,GetSize), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "bool IsEndOfFile() const", asMETHOD(CScriptFile,IsEOF), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "String ReadString(uint)", asMETHOD(CScriptFile,ReadString), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "String ReadLine()", asMETHOD(CScriptFile,ReadLine), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int64 ReadInt(uint)", asMETHOD(CScriptFile,ReadInt), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "uint64 ReadUInt(uint)", asMETHOD(CScriptFile,ReadUInt), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "float ReadFloat()", asMETHOD(CScriptFile,ReadFloat), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "double ReadDouble()", asMETHOD(CScriptFile,ReadDouble), asCALL_THISCALL); assert( r >= 0 );
#if AS_WRITE_OPS == 1
	r = engine->RegisterObjectMethod("File", "int WriteString(const String &in)", asMETHOD(CScriptFile,WriteString), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int WriteInt(int64, uint)", asMETHOD(CScriptFile,WriteInt), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int WriteUInt(uint64, uint)", asMETHOD(CScriptFile,WriteUInt), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int WriteFloat(float)", asMETHOD(CScriptFile,WriteFloat), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int WriteDouble(double)", asMETHOD(CScriptFile,WriteDouble), asCALL_THISCALL); assert( r >= 0 );
#endif
	r = engine->RegisterObjectMethod("File", "int GetPos() const", asMETHOD(CScriptFile,GetPos), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int SetPos(int)", asMETHOD(CScriptFile,SetPos), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int MovePos(int)", asMETHOD(CScriptFile,MovePos), asCALL_THISCALL); assert( r >= 0 );

	r = engine->RegisterObjectProperty("File", "bool MostSignificantByteFirst", asOFFSET(CScriptFile, mostSignificantByteFirst)); assert( r >= 0 );
}

void RegisterScriptFile_Generic(asIScriptEngine *engine)
{
	int r;

	r = engine->RegisterObjectType("File", 0, asOBJ_REF); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("File", asBEHAVE_FACTORY, "File @f()", asFUNCTION(ScriptFile_Factory_Generic), asCALL_GENERIC); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("File", asBEHAVE_ADDREF, "void f()", asFUNCTION(ScriptFile_AddRef_Generic), asCALL_GENERIC); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("File", asBEHAVE_RELEASE, "void f()", asFUNCTION(ScriptFile_Release_Generic), asCALL_GENERIC); assert( r >= 0 );

    r = engine->RegisterObjectMethod("File", "int Open(const String &in, const String &in)", asFUNCTION(ScriptFile_Open_Generic), asCALL_GENERIC); assert( r >= 0 );
    r = engine->RegisterObjectMethod("File", "int Close()", asFUNCTION(ScriptFile_Close_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int GetSize() const", asFUNCTION(ScriptFile_GetSize_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "bool IsEndOfFile() const", asFUNCTION(ScriptFile_IsEOF_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "String ReadString(uint)", asFUNCTION(ScriptFile_ReadString_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "String ReadLine()", asFUNCTION(ScriptFile_ReadLine_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int64 ReadInt(uint)", asFUNCTION(ScriptFile_ReadInt_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "uint64 ReadUInt(uint)", asFUNCTION(ScriptFile_ReadUInt_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "float ReadFloat()", asFUNCTION(ScriptFile_ReadFloat_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "double ReadDouble()", asFUNCTION(ScriptFile_ReadDouble_Generic), asCALL_GENERIC); assert( r >= 0 );
#if AS_WRITE_OPS == 1
	r = engine->RegisterObjectMethod("File", "int WriteString(const String &in)", asFUNCTION(ScriptFile_WriteString_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int WriteInt(int64, uint)", asFUNCTION(ScriptFile_WriteInt_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int WriteUInt(uint64, uint)", asFUNCTION(ScriptFile_WriteUInt_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int WriteFloat(float)", asFUNCTION(ScriptFile_WriteFloat_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int WriteDouble(double)", asFUNCTION(ScriptFile_WriteDouble_Generic), asCALL_GENERIC); assert( r >= 0 );
#endif
	r = engine->RegisterObjectMethod("File", "int GetPos() const", asFUNCTION(ScriptFile_GetPos_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int SetPos(int)", asFUNCTION(ScriptFile_SetPos_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("File", "int MovePos(int)", asFUNCTION(ScriptFile_MovePos_Generic), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectProperty("File", "bool MostSignificantByteFirst", asOFFSET(CScriptFile, mostSignificantByteFirst)); assert( r >= 0 );
}

void RegisterScriptFile(asIScriptEngine *engine)
{
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
		RegisterScriptFile_Generic(engine);
	else
		RegisterScriptFile_Native(engine);
}

CScriptFile::CScriptFile()
{
	refCount = 1;
	file = 0;
	mostSignificantByteFirst = false;
}

CScriptFile::~CScriptFile()
{
	Close();
}

void CScriptFile::AddRef() const
{
	asAtomicInc(refCount);
}

void CScriptFile::Release() const
{
	if( asAtomicDec(refCount) == 0 )
		delete this;
}

int CScriptFile::Open(const String &Filename, const String &mode)
{
	// Close the previously opened File handle
	if( file )
		Close();

	String myFilename = Filename;

	// Validate the mode
	String m;
#if AS_WRITE_OPS == 1
	if( mode != "r" && mode != "w" && mode != "a" )
#else
	if( mode != "r" )
#endif
		return -1;
	else
		m = mode;

#ifdef _WIN32_WCE
	// no relative pathing on CE
	char buf[MAX_PATH];
	static TCHAR apppath[MAX_PATH] = TEXT("");
	if (!apppath[0])
	{
		GetModuleFileName(NULL, apppath, MAX_PATH);

		int appLen = _tcslen(apppath);
		while (appLen > 1)
		{
			if (apppath[appLen-1] == TEXT('\\'))
				break;
			appLen--;
		}

		// Terminate the String after the trailing backslash
		apppath[appLen] = TEXT('\0');
	}
#ifdef _UNICODE
	wcstombs(buf, apppath, wcslen(apppath)+1);
#else
	memcpy(buf, apppath, strlen(apppath));
#endif
	myFilename = buf + myFilename;
#endif


	// By default windows translates "\r\n" to "\n", but we want to read the File as-is.
	m += "b";

	// Open the File
#if _MSC_VER >= 1400 && !defined(__S3E__) 
	// MSVC 8.0 / 2005 introduced new functions 
	// Marmalade doesn't use these, even though it uses the MSVC compiler
	fopen_s(&file, myFilename.c_str(), m.c_str());
#else
	file = fopen(myFilename.c_str(), m.c_str());
#endif
	if(file == 0 )
		return -1;

	return 0;
}

int CScriptFile::Close()
{
	if(file == 0 )
		return -1;

	fclose(file);
  file = 0;

	return 0;
}

int CScriptFile::GetSize() const
{
	if(file == 0 )
		return -1;

	int pos = ftell(file);
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, pos, SEEK_SET);

	return size;
}

int CScriptFile::GetPos() const
{
	if(file == 0 )
		return -1;

	return ftell(file);
}
 
int CScriptFile::SetPos(int pos)
{
	if(file == 0 )
		return -1;

	int r = fseek(file, pos, SEEK_SET);

	// Return -1 on error
	return r ? -1 : 0;
}

int CScriptFile::MovePos(int delta)
{
	if(file == 0 )
		return -1;

	int r = fseek(file, delta, SEEK_CUR);

	// Return -1 on error
	return r ? -1 : 0;
}

String CScriptFile::ReadString(unsigned int length)
{
	if(file == 0 )
		return "";

	// Read the String
	String str;
	str.resize(length);
	int size = (int)fread(&str[0], 1, length, file);
	str.resize(size);

	return str;
}

String CScriptFile::ReadLine()
{
	if(file == 0 )
		return "";

	// Read until the first new-line character
	String str;
	char buf[256];

	do
	{
		// Get the current position so we can determine how many characters were read
		int start = ftell(file);

		// Set the last byte to something different that 0, so that we can check if the buffer was filled up
		buf[255] = 1;

		// Read the line (or first 255 characters, which ever comes first)
		char *r = fgets(buf, 256, file);
		if( r == 0 ) break;
		
		// Get the position after the read
		int end = ftell(file);

		// Add the read characters to the output buffer
		str.append(buf, end-start);
	}
	while( !feof(file) && buf[255] == 0 && buf[254] != '\n' );

	return str;
}

asINT64 CScriptFile::ReadInt(asUINT bytes)
{
	if(file == 0 )
		return 0;

	if( bytes > 8 ) bytes = 8;
	if( bytes == 0 ) return 0;

	unsigned char buf[8];
	size_t r = fread(buf, bytes, 1, file);
	if( r == 0 ) return 0;

	asINT64 val = 0;
	if( mostSignificantByteFirst )
	{
		unsigned int n = 0;
		for( ; n < bytes; n++ )
			val |= asQWORD(buf[n]) << ((bytes-n-1)*8);

		// Check the most significant byte to determine if the rest 
		// of the qword must be filled to give a negative value
		if( buf[0] & 0x80 )
			for( ; n < 8; n++ )
				val |= asQWORD(0xFF) << (n*8);
	}
	else
	{
		unsigned int n = 0;
		for( ; n < bytes; n++ )
			val |= asQWORD(buf[n]) << (n*8);

		// Check the most significant byte to determine if the rest 
		// of the qword must be filled to give a negative value
		if( buf[bytes-1] & 0x80 )
			for( ; n < 8; n++ )
				val |= asQWORD(0xFF) << (n*8);
	}

	return val;
}

asQWORD CScriptFile::ReadUInt(asUINT bytes)
{
	if(file == 0 )
		return 0;

	if( bytes > 8 ) bytes = 8;
	if( bytes == 0 ) return 0;

	unsigned char buf[8];
	size_t r = fread(buf, bytes, 1, file);
	if( r == 0 ) return 0;

	asQWORD val = 0;
	if( mostSignificantByteFirst )
	{
		unsigned int n = 0;
		for( ; n < bytes; n++ )
			val |= asQWORD(buf[n]) << ((bytes-n-1)*8);
	}
	else
	{
		unsigned int n = 0;
		for( ; n < bytes; n++ )
			val |= asQWORD(buf[n]) << (n*8);
	}

	return val;
}

float CScriptFile::ReadFloat()
{
	if(file == 0 )
		return 0;

	unsigned char buf[4];
	size_t r = fread(buf, 4, 1, file);
	if( r == 0 ) return 0;

	asUINT val = 0;
	if( mostSignificantByteFirst )
	{
		unsigned int n = 0;
		for( ; n < 4; n++ )
			val |= asUINT(buf[n]) << ((3-n)*8);
	}
	else
	{
		unsigned int n = 0;
		for( ; n < 4; n++ )
			val |= asUINT(buf[n]) << (n*8);
	}

	return *reinterpret_cast<float*>(&val);
}

double CScriptFile::ReadDouble()
{
	if(file == 0 )
		return 0;

	unsigned char buf[8];
	size_t r = fread(buf, 8, 1, file);
	if( r == 0 ) return 0;

	asQWORD val = 0;
	if( mostSignificantByteFirst )
	{
		unsigned int n = 0;
		for( ; n < 8; n++ )
			val |= asQWORD(buf[n]) << ((7-n)*8);
	}
	else
	{
		unsigned int n = 0;
		for( ; n < 8; n++ )
			val |= asQWORD(buf[n]) << (n*8);
	}

	return *reinterpret_cast<double*>(&val);
}

bool CScriptFile::IsEOF() const
{
	if(file == 0 )
		return true;

	return feof(file) ? true : false;
}

#if AS_WRITE_OPS == 1
int CScriptFile::WriteString(const String &str)
{
	if(file == 0 )
		return -1;

	// Write the entire String
	size_t r = fwrite(&str[0], 1, str.length(), file);

	return int(r);
}

int CScriptFile::WriteInt(asINT64 val, asUINT bytes)
{
	if(file == 0 )
		return 0;

	unsigned char buf[8];
	if( mostSignificantByteFirst )
	{
		for( unsigned int n = 0; n < bytes; n++ )
			buf[n] = (val >> ((bytes-n-1)*8)) & 0xFF;
	}
	else
	{
		for( unsigned int n = 0; n < bytes; n++ )
			buf[n] = (val >> (n*8)) & 0xFF;
	}

	size_t r = fwrite(&buf, bytes, 1, file);
	return int(r);
}

int CScriptFile::WriteUInt(asQWORD val, asUINT bytes)
{
	if(file == 0 )
		return 0;

	unsigned char buf[8];
	if( mostSignificantByteFirst )
	{
		for( unsigned int n = 0; n < bytes; n++ )
			buf[n] = (val >> ((bytes-n-1)*8)) & 0xFF;
	}
	else
	{
		for( unsigned int n = 0; n < bytes; n++ )
			buf[n] = (val >> (n*8)) & 0xFF;
	}

	size_t r = fwrite(&buf, bytes, 1, file);
	return int(r);
}

int CScriptFile::WriteFloat(float f)
{
	if(file == 0 )
		return 0;

	unsigned char buf[4];
	asUINT val = *reinterpret_cast<asUINT*>(&f);
	if( mostSignificantByteFirst )
	{
		for( unsigned int n = 0; n < 4; n++ )
			buf[n] = (val >> ((3-n)*4)) & 0xFF;
	}
	else
	{
		for( unsigned int n = 0; n < 4; n++ )
			buf[n] = (val >> (n*8)) & 0xFF;
	}

	size_t r = fwrite(&buf, 4, 1, file);
	return int(r);
}

int CScriptFile::WriteDouble(double d)
{
	if(file == 0 )
		return 0;

	unsigned char buf[8];
	asQWORD val = *reinterpret_cast<asQWORD*>(&d);
	if( mostSignificantByteFirst )
	{
		for( unsigned int n = 0; n < 8; n++ )
			buf[n] = (val >> ((7-n)*8)) & 0xFF;
	}
	else
	{
		for( unsigned int n = 0; n < 8; n++ )
			buf[n] = (val >> (n*8)) & 0xFF;
	}

	size_t r = fwrite(&buf, 8, 1, file);
	return int(r);
}
#endif


END_AS_NAMESPACE
