#include "scriptlmbString.h"
#include <assert.h> // assert()
#include <sstream>  // std::stringstream
#include <stdio.h>	// sprintf()
#include <stdlib.h> // strtod()
#ifndef __psp2__
	#include <locale.h> // setlocale()
#endif
#include <containers/containers.h>

// This macro is used to avoid warnings about unused variables.
// Usually where the variables are only used in debug mode.
#define UNUSED_VAR(x) (void)(x)
#ifdef AS_USE_STLNAMES
#undef AS_USE_STLNAMES
#endif
#define AS_USE_STLNAMES 1

using namespace lambda;

#if AS_USE_STRINGPOOL == 1

#ifdef AS_CAN_USE_CPP11
	// The String pool doesn't need to keep a specific order in the
	// pool, so the UnorderedMap is faster than the ordinary map
BEGIN_AS_NAMESPACE
	typedef UnorderedMap<const char *, String> map_t;
END_AS_NAMESPACE
#else
BEGIN_AS_NAMESPACE
	typedef Map<const char *, String> map_t;
END_AS_NAMESPACE
#endif

BEGIN_AS_NAMESPACE

// By keeping the literal Strings in a pool the application
// performance is improved as there are less String copies created.

// The String pool will be kept as user data in the engine. We'll
// need a specific type to identify the String pool user data.
// We just define a number here that we assume nobody else is using for
// object type user data. The add-ons have reserved the numbers 1000
// through 1999 for this purpose, so we should be fine.
const asPWORD STRING_POOL = 1001;

// This global static variable is placed here rather than locally within the
// StringFactory, due to memory leak detectors that don't see the deallocation
// of global variables. By placing the variable globally it will be initialized
// before the memory leak detector starts, thus it won't report the missing
// deallocation. An example of this the Marmalade leak detector initialized with
// IwGxInit() and finished with IwGxTerminate().
static const String emptyString;

static const String &StringFactory(asUINT length, const char *s)
{
	// Each engine instance has its own String pool
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx == 0 )
	{
		// The String factory can only be called from a script
		assert( ctx );
		return emptyString;
	}
	asIScriptEngine *engine = ctx->GetEngine();

	map_t *pool = reinterpret_cast< map_t* >(engine->GetUserData(STRING_POOL));
	if( !pool )
	{
		// The String pool hasn't been created yet, so we'll create it now
		asAcquireExclusiveLock();

		// Make sure the String pool wasn't created while we were waiting for the lock
		pool = reinterpret_cast< map_t* >(engine->GetUserData(STRING_POOL));
		if( !pool )
		{
			#if defined(__S3E__)
			pool = new map_t;
			#else
			pool = new /*(nothrow)*/ map_t;
			#endif
			if( pool == 0 )
			{
				ctx->SetException("Out of memory");
				asReleaseExclusiveLock();
				return emptyString;
			}
			engine->SetUserData(pool, STRING_POOL);
		}

		asReleaseExclusiveLock();
	}

	// We can't let other threads modify the pool while we query it
	asAcquireSharedLock();

	// First check if a String object hasn't been created already
	map_t::iterator it;
	it = pool->find(s);
	if( it != pool->end() )
	{
		asReleaseSharedLock();
		return it->second;
	}

	asReleaseSharedLock();

	// Acquire an exclusive lock so we can add the new String to the pool
	asAcquireExclusiveLock();

	// Make sure the String wasn't created while we were waiting for the exclusive lock
	it = pool->find(s);
	if( it == pool->end() )
	{
		// Create a new String object
		it = pool->insert(map_t::value_type(s, String(s, length))).first;
	}

	asReleaseExclusiveLock();
	return it->second;
}

static void CleanupEngineStringPool(asIScriptEngine *engine)
{
	map_t *pool = reinterpret_cast< map_t* >(engine->GetUserData(STRING_POOL));
	if( pool )
		delete pool;
}

#else
static String StringFactory(asUINT length, const char *s)
{
	return String(s, length);
}
#endif

static void ConstructString(String *thisPointer)
{
	new(thisPointer) String();
}

static void CopyConstructString(const String &other, String *thisPointer)
{
	new(thisPointer) String(other);
}

static void DestructString(String *thisPointer)
{
	thisPointer->~String();
}

static String &AddAssignStringToString(const String &str, String &dest)
{
	// We don't register the method directly because some compilers
	// and standard libraries inline the definition, resulting in the
	// linker being unable to find the declaration.
	// Example: CLang/LLVM with XCode 4.3 on OSX 10.7
	dest += str;
	return dest;
}

// bool String::isEmpty()
// bool String::empty() // if AS_USE_STLNAMES == 1
static bool StringIsEmpty(const String &str)
{
	// We don't register the method directly because some compilers
	// and standard libraries inline the definition, resulting in the
	// linker being unable to find the declaration
	// Example: CLang/LLVM with XCode 4.3 on OSX 10.7
	return str.empty();
}

static String &AssignUInt64ToString(asQWORD i, String &dest)
{
  std::ostringstream stream;
	stream << i;
	dest = stream.str().c_str();
	return dest;
}

static String &AddAssignUInt64ToString(asQWORD i, String &dest)
{
	std::ostringstream stream;
	stream << i;
	dest += stream.str().c_str();
	return dest;
}

static String AddStringUInt64(const String &str, asQWORD i)
{
	std::ostringstream stream;
	stream << i;
	return str + stream.str().c_str();
}

static String AddInt64String(asINT64 i, const String &str)
{
	std::ostringstream stream;
	stream << i;
	return lmbString(stream.str()) + str;
}

static String &AssignInt64ToString(asINT64 i, String &dest)
{
	std::ostringstream stream;
	stream << i;
	dest = stream.str().c_str();
	return dest;
}

static String &AddAssignInt64ToString(asINT64 i, String &dest)
{
	std::ostringstream stream;
	stream << i;
	dest += stream.str().c_str();
	return dest;
}

static String AddStringInt64(const String &str, asINT64 i)
{
	std::ostringstream stream;
	stream << i;
	return str + stream.str().c_str();
}

static String AddUInt64String(asQWORD i, const String &str)
{
	std::ostringstream stream;
	stream << i;
	return stream.str().c_str() + str;
}

static String &AssignDoubleToString(double f, String &dest)
{
	std::ostringstream stream;
	stream << f;
	dest = stream.str().c_str();
	return dest;
}

static String &AddAssignDoubleToString(double f, String &dest)
{
	std::ostringstream stream;
	stream << f;
	dest += stream.str().c_str();
	return dest;
}

static String &AssignFloatToString(float f, String &dest)
{
	std::ostringstream stream;
	stream << f;
	dest = stream.str().c_str();
	return dest;
}

static String &AddAssignFloatToString(float f, String &dest)
{
	std::ostringstream stream;
	stream << f;
	dest += stream.str().c_str();
	return dest;
}

static String &AssignBoolToString(bool b, String &dest)
{
	std::ostringstream stream;
	stream << (b ? "true" : "false");
	dest = stream.str().c_str();
	return dest;
}

static String &AddAssignBoolToString(bool b, String &dest)
{
	std::ostringstream stream;
	stream << (b ? "true" : "false");
	dest += stream.str().c_str();
	return dest;
}

static String AddStringDouble(const String &str, double f)
{
	std::ostringstream stream;
	stream << f;
	return str + stream.str().c_str();
}

static String AddDoubleString(double f, const String &str)
{
	std::ostringstream stream;
	stream << f;
	return stream.str().c_str() + str;
}

static String AddStringFloat(const String &str, float f)
{
	std::ostringstream stream;
	stream << f;
	return str + stream.str().c_str();
}

static String AddFloatString(float f, const String &str)
{
	std::ostringstream stream;
	stream << f;
	return stream.str().c_str() + str;
}

static String AddStringBool(const String &str, bool b)
{
	std::ostringstream stream;
	stream << (b ? "true" : "false");
	return str + stream.str().c_str();
}

static String AddBoolString(bool b, const String &str)
{
	std::ostringstream stream;
	stream << (b ? "true" : "false");
	return stream.str().c_str() + str;
}

static char *StringCharAt(unsigned int i, String &str)
{
	if( i >= str.size() )
	{
		// Set a script exception
		asIScriptContext *ctx = asGetActiveContext();
		ctx->SetException("Out of range");

		// Return a null pointer
		return 0;
	}

	return &str[i];
}

// AngelScript signature:
// int String::opCmp(const String &in) const
static int StringCmp(const String &a, const String &b)
{
	int cmp = 0;
	if( a < b ) cmp = -1;
	else if( a > b ) cmp = 1;
	return cmp;
}

// This function returns the index of the first position where the subString
// exists in the input String. If the subString doesn't exist in the input
// String -1 is returned.
//
// AngelScript signature:
// int String::findFirst(const String &in sub, uint start = 0) const
static int StringFindFirst(const String &sub, asUINT start, const String &str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (int)str.find(sub, (size_t)(start < 0 ? String::npos : start));
}

// This function returns the index of the first position where the one of the bytes in subString
// exists in the input String. If the characters in the subString doesn't exist in the input
// String -1 is returned.
//
// AngelScript signature:
// int String::findFirstOf(const String &in sub, uint start = 0) const
static int StringFindFirstOf(const String &sub, asUINT start, const String &str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (int)str.find_first_of(sub, (size_t)(start < 0 ? String::npos : start));
}

// This function returns the index of the last position where the one of the bytes in subString
// exists in the input String. If the characters in the subString doesn't exist in the input
// String -1 is returned.
//
// AngelScript signature:
// int String::findLastOf(const String &in sub, uint start = -1) const
static int StringFindLastOf(const String &sub, asUINT start, const String &str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (int)str.find_last_of(sub, (size_t)(start < 0 ? String::npos : start));
}

// This function returns the index of the first position where a byte other than those in subString
// exists in the input String. If none is found -1 is returned.
//
// AngelScript signature:
// int String::findFirstNotOf(const String &in sub, uint start = 0) const
static int StringFindFirstNotOf(const String &sub, asUINT start, const String &str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (int)str.find_first_not_of(sub, (size_t)(start < 0 ? String::npos : start));
}

// This function returns the index of the last position where a byte other than those in subString
// exists in the input String. If none is found -1 is returned.
//
// AngelScript signature:
// int String::findLastNotOf(const String &in sub, uint start = -1) const
static int StringFindLastNotOf(const String &sub, asUINT start, const String &str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (int)str.find_last_of(sub, (size_t)(start < 0 ? String::npos : start));
}

// This function returns the index of the last position where the subString
// exists in the input String. If the subString doesn't exist in the input
// String -1 is returned.
//
// AngelScript signature:
// int String::findLast(const String &in sub, int start = -1) const
static int StringFindLast(const String &sub, int start, const String &str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (int)str.rfind(sub, (size_t)(start < 0 ? String::npos : start));
}

// AngelScript signature:
// void String::insert(uint pos, const String &in other)
static void StringInsert(unsigned int pos, const String &other, String &str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	str.insert(pos, other);
}

// AngelScript signature:
// void String::erase(uint pos, int count = -1)
static void StringErase(unsigned int pos, int count, String &str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	str.erase(pos, (size_t)(count < 0 ? String::npos : count));
}


// AngelScript signature:
// uint String::length() const
static asUINT StringLength(const String &str)
{
	// We don't register the method directly because the return type changes between 32bit and 64bit platforms
	return (asUINT)str.length();
}


// AngelScript signature:
// void String::resize(uint l)
static void StringResize(asUINT l, String &str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	str.resize(l);
}

// AngelScript signature:
// String formatInt(int64 val, const String &in options, uint width)
static String formatInt(asINT64 value, const String &options, asUINT width)
{
	bool leftJustify = options.find("l") != String::npos;
	bool padWithZero = options.find("0") != String::npos;
	bool alwaysSign  = options.find("+") != String::npos;
	bool spaceOnSign = options.find(" ") != String::npos;
	bool hexSmall    = options.find("h") != String::npos;
	bool hexLarge    = options.find("H") != String::npos;

	String fmt = "%";
	if( leftJustify ) fmt += "-";
	if( alwaysSign ) fmt += "+";
	if( spaceOnSign ) fmt += " ";
	if( padWithZero ) fmt += "0";

#ifdef _WIN32
	fmt += "*I64";
#else
#ifdef _LP64
	fmt += "*l";
#else
	fmt += "*ll";
#endif
#endif

	if( hexSmall ) fmt += "x";
	else if( hexLarge ) fmt += "X";
	else fmt += "d";

	String buf;
	buf.resize(width+30);
#if _MSC_VER >= 1400 && !defined(__S3E__)
	// MSVC 8.0 / 2005 or newer
	sprintf_s(&buf[0], buf.size(), fmt.c_str(), width, value);
#else
	sprintf(&buf[0], fmt.c_str(), width, value);
#endif
	buf.resize(strlen(&buf[0]));

	return buf;
}

// AngelScript signature:
// String formatUInt(uint64 val, const String &in options, uint width)
static String formatUInt(asQWORD value, const String &options, asUINT width)
{
	bool leftJustify = options.find("l") != String::npos;
	bool padWithZero = options.find("0") != String::npos;
	bool alwaysSign  = options.find("+") != String::npos;
	bool spaceOnSign = options.find(" ") != String::npos;
	bool hexSmall    = options.find("h") != String::npos;
	bool hexLarge    = options.find("H") != String::npos;

	String fmt = "%";
	if( leftJustify ) fmt += "-";
	if( alwaysSign ) fmt += "+";
	if( spaceOnSign ) fmt += " ";
	if( padWithZero ) fmt += "0";

#ifdef _WIN32
	fmt += "*I64";
#else
#ifdef _LP64
	fmt += "*l";
#else
	fmt += "*ll";
#endif
#endif

	if( hexSmall ) fmt += "x";
	else if( hexLarge ) fmt += "X";
	else fmt += "u";

	String buf;
	buf.resize(width+30);
#if _MSC_VER >= 1400 && !defined(__S3E__)
	// MSVC 8.0 / 2005 or newer
	sprintf_s(&buf[0], buf.size(), fmt.c_str(), width, value);
#else
	sprintf(&buf[0], fmt.c_str(), width, value);
#endif
	buf.resize(strlen(&buf[0]));

	return buf;
}

// AngelScript signature:
// String formatFloat(double val, const String &in options, uint width, uint precision)
static String formatFloat(double value, const String &options, asUINT width, asUINT precision)
{
	bool leftJustify = options.find("l") != String::npos;
	bool padWithZero = options.find("0") != String::npos;
	bool alwaysSign  = options.find("+") != String::npos;
	bool spaceOnSign = options.find(" ") != String::npos;
	bool expSmall    = options.find("e") != String::npos;
	bool expLarge    = options.find("E") != String::npos;

	String fmt = "%";
	if( leftJustify ) fmt += "-";
	if( alwaysSign ) fmt += "+";
	if( spaceOnSign ) fmt += " ";
	if( padWithZero ) fmt += "0";

	fmt += "*.*";

	if( expSmall ) fmt += "e";
	else if( expLarge ) fmt += "E";
	else fmt += "f";

	String buf;
	buf.resize(width+precision+50);
#if _MSC_VER >= 1400 && !defined(__S3E__)
	// MSVC 8.0 / 2005 or newer
	sprintf_s(&buf[0], buf.size(), fmt.c_str(), width, precision, value);
#else
	sprintf(&buf[0], fmt.c_str(), width, precision, value);
#endif
	buf.resize(strlen(&buf[0]));

	return buf;
}

// AngelScript signature:
// int64 parseInt(const String &in val, uint base = 10, uint &out byteCount = 0)
static asINT64 parseInt(const String &val, asUINT base, asUINT *byteCount)
{
	// Only accept base 10 and 16
	if( base != 10 && base != 16 )
	{
		if( byteCount ) *byteCount = 0;
		return 0;
	}

	const char *end = &val[0];

	// Determine the sign
	bool sign = false;
	if( *end == '-' )
	{
		sign = true;
		end++;
	}
	else if( *end == '+' )
		end++;

	asINT64 res = 0;
	if( base == 10 )
	{
		while( *end >= '0' && *end <= '9' )
		{
			res *= 10;
			res += *end++ - '0';
		}
	}
	else if( base == 16 )
	{
		while( (*end >= '0' && *end <= '9') ||
		       (*end >= 'a' && *end <= 'f') ||
		       (*end >= 'A' && *end <= 'F') )
		{
			res *= 16;
			if( *end >= '0' && *end <= '9' )
				res += *end++ - '0';
			else if( *end >= 'a' && *end <= 'f' )
				res += *end++ - 'a' + 10;
			else if( *end >= 'A' && *end <= 'F' )
				res += *end++ - 'A' + 10;
		}
	}

	if( byteCount )
		*byteCount = asUINT(size_t(end - val.c_str()));

	if( sign )
		res = -res;

	return res;
}

// AngelScript signature:
// uint64 parseUInt(const String &in val, uint base = 10, uint &out byteCount = 0)
static asQWORD parseUInt(const String &val, asUINT base, asUINT *byteCount)
{
	// Only accept base 10 and 16
	if (base != 10 && base != 16)
	{
		if (byteCount) *byteCount = 0;
		return 0;
	}

	const char *end = &val[0];

	asQWORD res = 0;
	if (base == 10)
	{
		while (*end >= '0' && *end <= '9')
		{
			res *= 10;
			res += *end++ - '0';
		}
	}
	else if (base == 16)
	{
		while ((*end >= '0' && *end <= '9') ||
			(*end >= 'a' && *end <= 'f') ||
			(*end >= 'A' && *end <= 'F'))
		{
			res *= 16;
			if (*end >= '0' && *end <= '9')
				res += *end++ - '0';
			else if (*end >= 'a' && *end <= 'f')
				res += *end++ - 'a' + 10;
			else if (*end >= 'A' && *end <= 'F')
				res += *end++ - 'A' + 10;
		}
	}

	if (byteCount)
		*byteCount = asUINT(size_t(end - val.c_str()));

	return res;
}

// AngelScript signature:
// double parseFloat(const String &in val, uint &out byteCount = 0)
double parseFloat(const String &val, asUINT *byteCount)
{
	char *end;

	// WinCE doesn't have setlocale. Some quick testing on my current platform
	// still manages to parse the numbers such as "3.14" even if the decimal for the
	// locale is ",".
#if !defined(_WIN32_WCE) && !defined(ANDROID) && !defined(__psp2__)
	// Set the locale to C so that we are guaranteed to parse the float value correctly
	char *orig = setlocale(LC_NUMERIC, 0);
	setlocale(LC_NUMERIC, "C");
#endif

	double res = strtod(val.c_str(), &end);

#if !defined(_WIN32_WCE) && !defined(ANDROID) && !defined(__psp2__)
	// Restore the locale
	setlocale(LC_NUMERIC, orig);
#endif

	if( byteCount )
		*byteCount = asUINT(size_t(end - val.c_str()));

	return res;
}

// This function returns a String containing the subString of the input String
// determined by the starting index and count of characters.
//
// AngelScript signature:
// String String::substr(uint start = 0, int count = -1) const
static String StringSubString(asUINT start, int count, const String &str)
{
	// Check for out-of-bounds
	String ret;
	if( start < str.length() && count != 0 )
		ret = str.substr(start, (size_t)(count < 0 ? String::npos : count));

	return ret;
}

// String equality comparison.
// Returns true iff lhs is equal to rhs.
//
// For some reason gcc 4.7 has difficulties resolving the
// asFUNCTIONPR(operator==, (const String &, const String &)
// makro, so this wrapper was introduced as work around.
static bool StringEquals(const String& lhs, const String& rhs)
{
	return lhs == rhs;
}

String StringAdd(const String& lhs, const String& rhs)
{
  return lhs + rhs;
}

void RegisterLmbString_Native(asIScriptEngine *engine)
{
	int r = 0;
	UNUSED_VAR(r);

	// Register the String type
#if AS_CAN_USE_CPP11
	// With C++11 it is possible to use asGetTypeTraits to automatically determine the correct flags to use
	r = engine->RegisterObjectType("String", sizeof(String), asOBJ_VALUE | asGetTypeTraits<String>()); assert( r >= 0 );
#else
	r = engine->RegisterObjectType("String", sizeof(String), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert( r >= 0 );
#endif

#if AS_USE_STRINGPOOL == 1
	// Register the String factory
	r = engine->RegisterStringFactory("const String &", asFUNCTION(StringFactory), asCALL_CDECL); assert( r >= 0 );

	// Register the cleanup callback for the String pool
	engine->SetEngineUserDataCleanupCallback(CleanupEngineStringPool, STRING_POOL);
#else
	// Register the String factory
	r = engine->RegisterStringFactory("String", asFUNCTION(StringFactory), asCALL_CDECL); assert( r >= 0 );
#endif

	// Register the object operator overloads
	r = engine->RegisterObjectBehaviour("String", asBEHAVE_CONSTRUCT,  "void f()",                    asFUNCTION(ConstructString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("String", asBEHAVE_CONSTRUCT,  "void f(const String &in)",    asFUNCTION(CopyConstructString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("String", asBEHAVE_DESTRUCT,   "void f()",                    asFUNCTION(DestructString),  asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String &opAssign(const String &in)", asMETHODPR(String, operator =, (const String&), String&), asCALL_THISCALL); assert( r >= 0 );
	// Need to use a wrapper on Mac OS X 10.7/XCode 4.3 and CLang/LLVM, otherwise the linker fails
	r = engine->RegisterObjectMethod("String", "String &opAddAssign(const String &in)", asFUNCTION(AddAssignStringToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
//	r = engine->RegisterObjectMethod("String", "String &opAddAssign(const String &in)", asMETHODPR(String, operator+=, (const String&), String&), asCALL_THISCALL); assert( r >= 0 );

	// Need to use a wrapper for operator== otherwise gcc 4.7 fails to compile
	r = engine->RegisterObjectMethod("String", "bool opEquals(const String &in) const", asFUNCTIONPR(StringEquals, (const String &, const String &), bool), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "int opCmp(const String &in) const", asFUNCTION(StringCmp), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd(const String &in) const", asFUNCTION(StringAdd), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

	// The String length can be accessed through methods or through virtual property
	r = engine->RegisterObjectMethod("String", "uint Length() const", asFUNCTION(StringLength), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "void Resize(uint)", asFUNCTION(StringResize), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "uint get_length() const", asFUNCTION(StringLength), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "void set_length(uint)", asFUNCTION(StringResize), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	// Need to use a wrapper on Mac OS X 10.7/XCode 4.3 and CLang/LLVM, otherwise the linker fails
//	r = engine->RegisterObjectMethod("String", "bool isEmpty() const", asMETHOD(String, empty), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "bool IsEmpty() const", asFUNCTION(StringIsEmpty), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Register the index operator, both as a mutator and as an inspector
	// Note that we don't register the operator[] directly, as it doesn't do bounds checking
	r = engine->RegisterObjectMethod("String", "uint8 &opIndex(uint)", asFUNCTION(StringCharAt), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "const uint8 &opIndex(uint) const", asFUNCTION(StringCharAt), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Automatic conversion from values
	r = engine->RegisterObjectMethod("String", "String &opAssign(double)", asFUNCTION(AssignDoubleToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String &opAddAssign(double)", asFUNCTION(AddAssignDoubleToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd(double) const", asFUNCTION(AddStringDouble), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd_r(double) const", asFUNCTION(AddDoubleString), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("String", "String &opAssign(float)", asFUNCTION(AssignFloatToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String &opAddAssign(float)", asFUNCTION(AddAssignFloatToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd(float) const", asFUNCTION(AddStringFloat), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd_r(float) const", asFUNCTION(AddFloatString), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("String", "String &opAssign(int64)", asFUNCTION(AssignInt64ToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String &opAddAssign(int64)", asFUNCTION(AddAssignInt64ToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd(int64) const", asFUNCTION(AddStringInt64), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd_r(int64) const", asFUNCTION(AddInt64String), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("String", "String &opAssign(uint64)", asFUNCTION(AssignUInt64ToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String &opAddAssign(uint64)", asFUNCTION(AddAssignUInt64ToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd(uint64) const", asFUNCTION(AddStringUInt64), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd_r(uint64) const", asFUNCTION(AddUInt64String), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("String", "String &opAssign(bool)", asFUNCTION(AssignBoolToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String &opAddAssign(bool)", asFUNCTION(AddAssignBoolToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd(bool) const", asFUNCTION(AddStringBool), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd_r(bool) const", asFUNCTION(AddBoolString), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Utilities
	r = engine->RegisterObjectMethod("String", "String Substr(uint start = 0, int count = -1) const", asFUNCTION(StringSubString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "int FindFirst(const String &in, uint start = 0) const", asFUNCTION(StringFindFirst), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "int FindFirstOf(const String &in, uint start = 0) const", asFUNCTION(StringFindFirstOf), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("String", "int FindFirstNotOf(const String &in, uint start = 0) const", asFUNCTION(StringFindFirstNotOf), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("String", "int FindLast(const String &in, int start = -1) const", asFUNCTION(StringFindLast), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "int FindLastOf(const String &in, int start = -1) const", asFUNCTION(StringFindLastOf), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("String", "int FindLastNotOf(const String &in, int start = -1) const", asFUNCTION(StringFindLastNotOf), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("String", "void Insert(uint pos, const String &in other)", asFUNCTION(StringInsert), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("String", "void Erase(uint pos, int count = -1)", asFUNCTION(StringErase), asCALL_CDECL_OBJLAST); assert(r >= 0);


	r = engine->RegisterGlobalFunction("String FormatInt(int64 val, const String &in options = \"\", uint width = 0)", asFUNCTION(formatInt), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("String FormatUInt(uint64 val, const String &in options = \"\", uint width = 0)", asFUNCTION(formatUInt), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("String FormatFloat(double val, const String &in options = \"\", uint width = 0, uint precision = 0)", asFUNCTION(formatFloat), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int64 ParseInt(const String &in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseInt), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint64 ParseUInt(const String &in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseUInt), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("double ParseFloat(const String &in, uint &out byteCount = 0)", asFUNCTION(parseFloat), asCALL_CDECL); assert(r >= 0);

#if AS_USE_STLNAMES == 1
	// Same as length
	r = engine->RegisterObjectMethod("String", "uint Size() const", asFUNCTION(StringLength), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	// Same as isEmpty
	r = engine->RegisterObjectMethod("String", "bool Empty() const", asFUNCTION(StringIsEmpty), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	// Same as findFirst
	r = engine->RegisterObjectMethod("String", "int Find(const String &in, uint start = 0) const", asFUNCTION(StringFindFirst), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	// Same as findLast
	r = engine->RegisterObjectMethod("String", "int RFind(const String &in, int start = -1) const", asFUNCTION(StringFindLast), asCALL_CDECL_OBJLAST); assert( r >= 0 );
#endif

	// TODO: Implement the following
	// findAndReplace - replaces a text found in the String
	// replaceRange - replaces a range of bytes in the String
	// multiply/times/opMul/opMul_r - takes the String and multiplies it n times, e.g. "-".multiply(5) returns "-----"
}

#if AS_USE_STRINGPOOL == 1
static void StringFactoryGeneric(asIScriptGeneric *gen)
{
	asUINT length = gen->GetArgDWord(0);
	const char *s = (const char*)gen->GetArgAddress(1);

	// Return a reference to a String
	gen->SetReturnAddress(const_cast<String*>(&StringFactory(length, s)));
}
#else
static void StringFactoryGeneric(asIScriptGeneric *gen)
{
	asUINT length = gen->GetArgDWord(0);
	const char *s = (const char*)gen->GetArgAddress(1);

	// Return a String value
	new (gen->GetAddressOfReturnLocation()) String(StringFactory(length, s));
}
#endif

static void ConstructStringGeneric(asIScriptGeneric * gen)
{
	new (gen->GetObject()) String();
}

static void CopyConstructStringGeneric(asIScriptGeneric * gen)
{
	String * a = static_cast<String *>(gen->GetArgObject(0));
	new (gen->GetObject()) String(*a);
}

static void DestructStringGeneric(asIScriptGeneric * gen)
{
	String * ptr = static_cast<String *>(gen->GetObject());
	ptr->~String();
}

static void AssignStringGeneric(asIScriptGeneric *gen)
{
	String * a = static_cast<String *>(gen->GetArgObject(0));
	String * self = static_cast<String *>(gen->GetObject());
	*self = *a;
	gen->SetReturnAddress(self);
}

static void AddAssignStringGeneric(asIScriptGeneric *gen)
{
	String * a = static_cast<String *>(gen->GetArgObject(0));
	String * self = static_cast<String *>(gen->GetObject());
	*self += *a;
	gen->SetReturnAddress(self);
}

static void StringEqualsGeneric(asIScriptGeneric * gen)
{
	String * a = static_cast<String *>(gen->GetObject());
	String * b = static_cast<String *>(gen->GetArgAddress(0));
	*(bool*)gen->GetAddressOfReturnLocation() = (*a == *b);
}

static void StringCmpGeneric(asIScriptGeneric * gen)
{
	String * a = static_cast<String *>(gen->GetObject());
	String * b = static_cast<String *>(gen->GetArgAddress(0));

	int cmp = 0;
	if( *a < *b ) cmp = -1;
	else if( *a > *b ) cmp = 1;

	*(int*)gen->GetAddressOfReturnLocation() = cmp;
}

static void StringAddGeneric(asIScriptGeneric * gen)
{
	String * a = static_cast<String *>(gen->GetObject());
	String * b = static_cast<String *>(gen->GetArgAddress(0));
	String ret_val = *a + *b;
	gen->SetReturnObject(&ret_val);
}

static void StringLengthGeneric(asIScriptGeneric * gen)
{
	String * self = static_cast<String *>(gen->GetObject());
	*static_cast<asUINT *>(gen->GetAddressOfReturnLocation()) = (asUINT)self->length();
}

static void StringIsEmptyGeneric(asIScriptGeneric * gen)
{
	String * self = reinterpret_cast<String *>(gen->GetObject());
	*reinterpret_cast<bool *>(gen->GetAddressOfReturnLocation()) = StringIsEmpty(*self);
}

static void StringResizeGeneric(asIScriptGeneric * gen)
{
	String * self = static_cast<String *>(gen->GetObject());
	self->resize(*static_cast<asUINT *>(gen->GetAddressOfArg(0)));
}

static void StringInsert_Generic(asIScriptGeneric *gen)
{
	String * self = static_cast<String *>(gen->GetObject());
	asUINT pos = gen->GetArgDWord(0);
	String *other = reinterpret_cast<String*>(gen->GetArgAddress(1));
	StringInsert(pos, *other, *self);
}

static void StringErase_Generic(asIScriptGeneric *gen)
{
	String * self = static_cast<String *>(gen->GetObject());
	asUINT pos = gen->GetArgDWord(0);
	int count = int(gen->GetArgDWord(1));
	StringErase(pos, count, *self);
}

static void StringFindFirst_Generic(asIScriptGeneric * gen)
{
	String *find = reinterpret_cast<String*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	String *self = reinterpret_cast<String *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindFirst(*find, start, *self);
}

static void StringFindLast_Generic(asIScriptGeneric * gen)
{
	String *find = reinterpret_cast<String*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	String *self = reinterpret_cast<String *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindLast(*find, start, *self);
}

static void StringFindFirstOf_Generic(asIScriptGeneric * gen)
{
	String *find = reinterpret_cast<String*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	String *self = reinterpret_cast<String *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindFirstOf(*find, start, *self);
}

static void StringFindLastOf_Generic(asIScriptGeneric * gen)
{
	String *find = reinterpret_cast<String*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	String *self = reinterpret_cast<String *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindLastOf(*find, start, *self);
}

static void StringFindFirstNotOf_Generic(asIScriptGeneric * gen)
{
	String *find = reinterpret_cast<String*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	String *self = reinterpret_cast<String *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindFirstNotOf(*find, start, *self);
}

static void StringFindLastNotOf_Generic(asIScriptGeneric * gen)
{
	String *find = reinterpret_cast<String*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	String *self = reinterpret_cast<String *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindLastNotOf(*find, start, *self);
}

static void formatInt_Generic(asIScriptGeneric * gen)
{
	asINT64 val = gen->GetArgQWord(0);
	String *options = reinterpret_cast<String*>(gen->GetArgAddress(1));
	asUINT width = gen->GetArgDWord(2);
	new(gen->GetAddressOfReturnLocation()) String(formatInt(val, *options, width));
}

static void formatUInt_Generic(asIScriptGeneric * gen)
{
	asQWORD val = gen->GetArgQWord(0);
	String *options = reinterpret_cast<String*>(gen->GetArgAddress(1));
	asUINT width = gen->GetArgDWord(2);
	new(gen->GetAddressOfReturnLocation()) String(formatUInt(val, *options, width));
}

static void formatFloat_Generic(asIScriptGeneric *gen)
{
	double val = gen->GetArgDouble(0);
	String *options = reinterpret_cast<String*>(gen->GetArgAddress(1));
	asUINT width = gen->GetArgDWord(2);
	asUINT precision = gen->GetArgDWord(3);
	new(gen->GetAddressOfReturnLocation()) String(formatFloat(val, *options, width, precision));
}

static void parseInt_Generic(asIScriptGeneric *gen)
{
	String *str = reinterpret_cast<String*>(gen->GetArgAddress(0));
	asUINT base = gen->GetArgDWord(1);
	asUINT *byteCount = reinterpret_cast<asUINT*>(gen->GetArgAddress(2));
	gen->SetReturnQWord(parseInt(*str,base,byteCount));
}

static void parseUInt_Generic(asIScriptGeneric *gen)
{
	String *str = reinterpret_cast<String*>(gen->GetArgAddress(0));
	asUINT base = gen->GetArgDWord(1);
	asUINT *byteCount = reinterpret_cast<asUINT*>(gen->GetArgAddress(2));
	gen->SetReturnQWord(parseUInt(*str, base, byteCount));
}

static void parseFloat_Generic(asIScriptGeneric *gen)
{
	String *str = reinterpret_cast<String*>(gen->GetArgAddress(0));
	asUINT *byteCount = reinterpret_cast<asUINT*>(gen->GetArgAddress(1));
	gen->SetReturnDouble(parseFloat(*str,byteCount));
}

static void StringCharAtGeneric(asIScriptGeneric * gen)
{
	unsigned int index = gen->GetArgDWord(0);
	String * self = static_cast<String *>(gen->GetObject());

	if (index >= self->size())
	{
		// Set a script exception
		asIScriptContext *ctx = asGetActiveContext();
		ctx->SetException("Out of range");

		gen->SetReturnAddress(0);
	}
	else
	{
		gen->SetReturnAddress(&(self->operator [](index)));
	}
}

static void AssignInt2StringGeneric(asIScriptGeneric *gen)
{
	asINT64 *a = static_cast<asINT64*>(gen->GetAddressOfArg(0));
	String *self = static_cast<String*>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a;
	*self = sstr.str().c_str();
	gen->SetReturnAddress(self);
}

static void AssignUInt2StringGeneric(asIScriptGeneric *gen)
{
	asQWORD *a = static_cast<asQWORD*>(gen->GetAddressOfArg(0));
	String *self = static_cast<String*>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a;
	*self = sstr.str().c_str();
	gen->SetReturnAddress(self);
}

static void AssignDouble2StringGeneric(asIScriptGeneric *gen)
{
	double *a = static_cast<double*>(gen->GetAddressOfArg(0));
	String *self = static_cast<String*>(gen->GetObject());
	std::stringstream  sstr;
	sstr << *a;
	*self = sstr.str().c_str();
	gen->SetReturnAddress(self);
}

static void AssignFloat2StringGeneric(asIScriptGeneric *gen)
{
	float *a = static_cast<float*>(gen->GetAddressOfArg(0));
	String *self = static_cast<String*>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a;
	*self = sstr.str().c_str();
	gen->SetReturnAddress(self);
}

static void AssignBool2StringGeneric(asIScriptGeneric *gen)
{
	bool *a = static_cast<bool*>(gen->GetAddressOfArg(0));
	String *self = static_cast<String*>(gen->GetObject());
	std::stringstream sstr;
	sstr << (*a ? "true" : "false");
	*self = sstr.str().c_str();
	gen->SetReturnAddress(self);
}

static void AddAssignDouble2StringGeneric(asIScriptGeneric * gen)
{
	double * a = static_cast<double *>(gen->GetAddressOfArg(0));
	String * self = static_cast<String *>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a;
	*self += sstr.str().c_str();
	gen->SetReturnAddress(self);
}

static void AddAssignFloat2StringGeneric(asIScriptGeneric * gen)
{
	float * a = static_cast<float *>(gen->GetAddressOfArg(0));
	String * self = static_cast<String *>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a;
	*self += sstr.str().c_str();
	gen->SetReturnAddress(self);
}

static void AddAssignInt2StringGeneric(asIScriptGeneric * gen)
{
	asINT64 * a = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
	String * self = static_cast<String *>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a;
	*self += sstr.str().c_str();
	gen->SetReturnAddress(self);
}

static void AddAssignUInt2StringGeneric(asIScriptGeneric * gen)
{
	asQWORD * a = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
	String * self = static_cast<String *>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a;
	*self += sstr.str().c_str();
	gen->SetReturnAddress(self);
}

static void AddAssignBool2StringGeneric(asIScriptGeneric * gen)
{
	bool * a = static_cast<bool *>(gen->GetAddressOfArg(0));
	String * self = static_cast<String *>(gen->GetObject());
	std::stringstream sstr;
	sstr << (*a ? "true" : "false");
	*self += sstr.str().c_str();
	gen->SetReturnAddress(self);
}

static void AddString2DoubleGeneric(asIScriptGeneric * gen)
{
	String * a = static_cast<String *>(gen->GetObject());
	double * b = static_cast<double *>(gen->GetAddressOfArg(0));
	std::stringstream sstr;
  sstr << a->c_str() << *b;
	String ret_val = sstr.str().c_str();
	gen->SetReturnObject(&ret_val);
}

static void AddString2FloatGeneric(asIScriptGeneric * gen)
{
	String * a = static_cast<String *>(gen->GetObject());
	float * b = static_cast<float *>(gen->GetAddressOfArg(0));
	std::stringstream sstr;
	sstr << a->c_str() << *b;
	String ret_val = sstr.str().c_str();
	gen->SetReturnObject(&ret_val);
}

static void AddString2IntGeneric(asIScriptGeneric * gen)
{
	String * a = static_cast<String *>(gen->GetObject());
	asINT64 * b = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
	std::stringstream sstr;
  sstr << a->c_str() << *b;
	String ret_val = sstr.str().c_str();
	gen->SetReturnObject(&ret_val);
}

static void AddString2UIntGeneric(asIScriptGeneric * gen)
{
	String * a = static_cast<String *>(gen->GetObject());
	asQWORD * b = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
	std::stringstream sstr;
  sstr << a->c_str() << *b;
	String ret_val = sstr.str().c_str();
	gen->SetReturnObject(&ret_val);
}

static void AddString2BoolGeneric(asIScriptGeneric * gen)
{
	String * a = static_cast<String *>(gen->GetObject());
	bool * b = static_cast<bool *>(gen->GetAddressOfArg(0));
	std::stringstream sstr;
	sstr << a->c_str() << (*b ? "true" : "false");
	String ret_val = sstr.str().c_str();
	gen->SetReturnObject(&ret_val);
}

static void AddDouble2StringGeneric(asIScriptGeneric * gen)
{
	double* a = static_cast<double *>(gen->GetAddressOfArg(0));
	String * b = static_cast<String *>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a << b->c_str();
	String ret_val = sstr.str().c_str();
	gen->SetReturnObject(&ret_val);
}

static void AddFloat2StringGeneric(asIScriptGeneric * gen)
{
	float* a = static_cast<float *>(gen->GetAddressOfArg(0));
	String * b = static_cast<String *>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a << b->c_str();
	String ret_val = sstr.str().c_str();
	gen->SetReturnObject(&ret_val);
}

static void AddInt2StringGeneric(asIScriptGeneric * gen)
{
	asINT64* a = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
	String * b = static_cast<String *>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a << b->c_str();
	String ret_val = sstr.str().c_str();
	gen->SetReturnObject(&ret_val);
}

static void AddUInt2StringGeneric(asIScriptGeneric * gen)
{
	asQWORD* a = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
	String * b = static_cast<String *>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a << b->c_str();
	String ret_val = sstr.str().c_str();
	gen->SetReturnObject(&ret_val);
}

static void AddBool2StringGeneric(asIScriptGeneric * gen)
{
	bool* a = static_cast<bool *>(gen->GetAddressOfArg(0));
	String * b = static_cast<String *>(gen->GetObject());
	std::stringstream sstr;
	sstr << (*a ? "true" : "false") << b->c_str();
	String ret_val = sstr.str().c_str();
	gen->SetReturnObject(&ret_val);
}

static void StringSubString_Generic(asIScriptGeneric *gen)
{
	// Get the arguments
	String *str   = (String*)gen->GetObject();
	asUINT  start = *(int*)gen->GetAddressOfArg(0);
	int     count = *(int*)gen->GetAddressOfArg(1);

	// Return the subString
	new(gen->GetAddressOfReturnLocation()) String(StringSubString(start, count, *str));
}

void RegisterLmbString_Generic(asIScriptEngine *engine)
{
	int r = 0;
	UNUSED_VAR(r);

	// Register the String type
	r = engine->RegisterObjectType("String", sizeof(String), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert( r >= 0 );

#if AS_USE_STRINGPOOL == 1
	// Register the String factory
	r = engine->RegisterStringFactory("const String &", asFUNCTION(StringFactoryGeneric), asCALL_GENERIC); assert( r >= 0 );

	// Register the cleanup callback for the String pool
	engine->SetEngineUserDataCleanupCallback(CleanupEngineStringPool, STRING_POOL);
#else
	// Register the String factory
	r = engine->RegisterStringFactory("String", asFUNCTION(StringFactoryGeneric), asCALL_GENERIC); assert( r >= 0 );
#endif

	// Register the object operator overloads
	r = engine->RegisterObjectBehaviour("String", asBEHAVE_CONSTRUCT,  "void f()",                    asFUNCTION(ConstructStringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("String", asBEHAVE_CONSTRUCT,  "void f(const String &in)",    asFUNCTION(CopyConstructStringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("String", asBEHAVE_DESTRUCT,   "void f()",                    asFUNCTION(DestructStringGeneric),  asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String &opAssign(const String &in)", asFUNCTION(AssignStringGeneric),    asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String &opAddAssign(const String &in)", asFUNCTION(AddAssignStringGeneric), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectMethod("String", "bool opEquals(const String &in) const", asFUNCTION(StringEqualsGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "int opCmp(const String &in) const", asFUNCTION(StringCmpGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd(const String &in) const", asFUNCTION(StringAddGeneric), asCALL_GENERIC); assert( r >= 0 );

	// Register the object methods
	r = engine->RegisterObjectMethod("String", "uint Length() const", asFUNCTION(StringLengthGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "void Resize(uint)",   asFUNCTION(StringResizeGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "uint get_length() const", asFUNCTION(StringLengthGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "void set_length(uint)", asFUNCTION(StringResizeGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "bool IsEmpty() const", asFUNCTION(StringIsEmptyGeneric), asCALL_GENERIC); assert( r >= 0 );

	// Register the index operator, both as a mutator and as an inspector
	r = engine->RegisterObjectMethod("String", "uint8 &opIndex(uint)", asFUNCTION(StringCharAtGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "const uint8 &opIndex(uint) const", asFUNCTION(StringCharAtGeneric), asCALL_GENERIC); assert( r >= 0 );

	// Automatic conversion from values
	r = engine->RegisterObjectMethod("String", "String &opAssign(double)", asFUNCTION(AssignDouble2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String &opAddAssign(double)", asFUNCTION(AddAssignDouble2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd(double) const", asFUNCTION(AddString2DoubleGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd_r(double) const", asFUNCTION(AddDouble2StringGeneric), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectMethod("String", "String &opAssign(float)", asFUNCTION(AssignFloat2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String &opAddAssign(float)", asFUNCTION(AddAssignFloat2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd(float) const", asFUNCTION(AddString2FloatGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd_r(float) const", asFUNCTION(AddFloat2StringGeneric), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectMethod("String", "String &opAssign(int64)", asFUNCTION(AssignInt2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String &opAddAssign(int64)", asFUNCTION(AddAssignInt2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd(int64) const", asFUNCTION(AddString2IntGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd_r(int64) const", asFUNCTION(AddInt2StringGeneric), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectMethod("String", "String &opAssign(uint64)", asFUNCTION(AssignUInt2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String &opAddAssign(uint64)", asFUNCTION(AddAssignUInt2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd(uint64) const", asFUNCTION(AddString2UIntGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd_r(uint64) const", asFUNCTION(AddUInt2StringGeneric), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectMethod("String", "String &opAssign(bool)", asFUNCTION(AssignBool2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String &opAddAssign(bool)", asFUNCTION(AddAssignBool2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd(bool) const", asFUNCTION(AddString2BoolGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("String", "String opAdd_r(bool) const", asFUNCTION(AddBool2StringGeneric), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectMethod("String", "String Substr(uint start = 0, int count = -1) const", asFUNCTION(StringSubString_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("String", "int FindFirst(const String &in, uint start = 0) const", asFUNCTION(StringFindFirst_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("String", "int FindFirstOf(const String &in, uint start = 0) const", asFUNCTION(StringFindFirstOf_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("String", "int FindFirstNotOf(const String &in, uint start = 0) const", asFUNCTION(StringFindFirstNotOf_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("String", "int FindLast(const String &in, int start = -1) const", asFUNCTION(StringFindLast_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("String", "int FindLastOf(const String &in, int start = -1) const", asFUNCTION(StringFindLastOf_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("String", "int FindLastNotOf(const String &in, int start = -1) const", asFUNCTION(StringFindLastNotOf_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("String", "void Insert(uint pos, const String &in other)", asFUNCTION(StringInsert_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("String", "void Erase(uint pos, int count = -1)", asFUNCTION(StringErase_Generic), asCALL_GENERIC); assert(r >= 0);


	r = engine->RegisterGlobalFunction("String FormatInt(int64 val, const String &in options = \"\", uint width = 0)", asFUNCTION(formatInt_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterGlobalFunction("String FormatUInt(uint64 val, const String &in options = \"\", uint width = 0)", asFUNCTION(formatUInt_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterGlobalFunction("String FormatFloat(double val, const String &in options = \"\", uint width = 0, uint precision = 0)", asFUNCTION(formatFloat_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int64 ParseInt(const String &in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseInt_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint64 ParseUInt(const String &in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseUInt_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterGlobalFunction("double ParseFloat(const String &in, uint &out byteCount = 0)", asFUNCTION(parseFloat_Generic), asCALL_GENERIC); assert(r >= 0);
}

void RegisterLmbString(asIScriptEngine * engine)
{
	if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY"))
		RegisterLmbString_Generic(engine);
	else
		RegisterLmbString_Native(engine);
}

END_AS_NAMESPACE




