#include "scripting/angel-script/addons/scriptlmbstring.h"
#include "scripting/angel-script/addons/scriptarray.h"
#include <containers/containers.h>
#include <assert.h>
#include <stdio.h>

using namespace lambda;

BEGIN_AS_NAMESPACE

// This function takes an input String and splits it into parts by looking
// for a specified delimiter. Example:
//
// String str = "A|B||D";
// Array<String>@ array = str.split("|");
//
// The resulting array has the following elements:
//
// {"A", "B", "", "D"}
//
// AngelScript signature:
// Array<String>@ String::split(const String &in delim) const
static CScriptArray *StringSplit(const String &delim, const String &str)
{
	// Obtain a pointer to the engine
	asIScriptContext *ctx = asGetActiveContext();
	asIScriptEngine *engine = ctx->GetEngine();

	// TODO: This should only be done once
	// TODO: This assumes that CScriptArray was already registered
	asITypeInfo *arrayType = engine->GetTypeInfoByDecl("Array<String>");

	// Create the array object
	CScriptArray *array = CScriptArray::Create(arrayType);

	// Find the existence of the delimiter in the input String
	int pos = 0, prev = 0, count = 0;
	while( (pos = (int)str.find(delim, prev)) != (int)String::npos )
	{
		// Add the part to the array
		array->Resize(array->GetSize()+1);
		((String*)array->At(count))->assign(&str[prev], pos-prev);

		// Find the next part
		count++;
		prev = pos + (int)delim.length();
	}

	// Add the remaining part
	array->Resize(array->GetSize()+1);
	((String*)array->At(count))->assign(&str[prev]);

	return array;
}

static void StringSplit_Generic(asIScriptGeneric *gen)
{
	// Get the arguments
	String *str   = (String*)gen->GetObject();
	String *delim = *(String**)gen->GetAddressOfArg(0);

	// Return the array by handle
	*(CScriptArray**)gen->GetAddressOfReturnLocation() = StringSplit(*delim, *str);
}



// This function takes as input an array of String handles as well as a
// delimiter and concatenates the array elements into one delimited String.
// Example:
//
// Array<String> array = {"A", "B", "", "D"};
// String str = join(array, "|");
//
// The resulting String is:
//
// "A|B||D"
//
// AngelScript signature:
// String join(const array<String> &in array, const String &in delim)
static String StringJoin(const CScriptArray &array, const String &delim)
{
	// Create the new String
	String str = "";
	if( array.GetSize() )
	{
		int n;
		for( n = 0; n < (int)array.GetSize() - 1; n++ )
		{
			str += *(String*)array.At(n);
			str += delim;
		}

		// Add the last part
		str += *(String*)array.At(n);
	}

	return str;
}

static void StringJoin_Generic(asIScriptGeneric *gen)
{
	// Get the arguments
	CScriptArray  *array = *(CScriptArray**)gen->GetAddressOfArg(0);
	String *delim = *(String**)gen->GetAddressOfArg(1);

	// Return the String
	new(gen->GetAddressOfReturnLocation()) String(StringJoin(*array, *delim));
}

// This is where the utility functions are registered.
// The String type must have been registered first.
void RegisterLmbStringUtils(asIScriptEngine *engine)
{
	int r;

	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		r = engine->RegisterObjectMethod("String", "Array<String>@ Split(const String &in) const", asFUNCTION(StringSplit_Generic), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("String Join(const Array<String> &in, const String &in)", asFUNCTION(StringJoin_Generic), asCALL_GENERIC); assert(r >= 0);
	}
	else
	{
		r = engine->RegisterObjectMethod("String", "Array<String>@ Split(const String &in) const", asFUNCTION(StringSplit), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterGlobalFunction("String Join(const Array<String> &in, const String &in)", asFUNCTION(StringJoin), asCALL_CDECL); assert(r >= 0);
	}
}

END_AS_NAMESPACE
