#ifndef SCRIPTBUILDER_H
#define SCRIPTBUILDER_H

//---------------------------
// Compilation settings
//

// Set this flag to turn on/off metadata processing
//  0 = off
//  1 = on
#ifndef AS_PROCESS_METADATA
#define AS_PROCESS_METADATA 1
#endif

// TODO: Implement flags for turning on/off include directives and conditional programming



//---------------------------
// Declaration
//

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif


#if defined(_MSC_VER) && _MSC_VER <= 1200
// disable the annoying warnings on MSVC 6
#pragma warning (disable:4786)
#endif

#include <containers/containers.h>
#include <string.h> // _strcmpi

BEGIN_AS_NAMESPACE

class CScriptBuilder;

// This callback will be called for each #include directive encountered by the
// builder. The callback should call the AddSectionFromFile or AddSectionFromMemory
// to add the included section to the script. If the include cannot be resolved
// then the function should return a negative value to abort the compilation.
typedef int (*INCLUDECALLBACK_t)(const char *include, const char *from, CScriptBuilder *builder, void *userParam);

// Helper class for loading and pre-processing script files to
// support include directives and metadata declarations
class CScriptBuilder
{
public:
	CScriptBuilder();

	// Start a new module
	int StartNewModule(asIScriptEngine *engine, const char *moduleName);

	// Load a script section from a file on disk
	// Returns  1 if the file was included
	//          0 if the file had already been included before
	//         <0 on error
	int AddSectionFromFile(const char *filename);

	// Load a script section from memory
	// Returns  1 if the section was included
	//          0 if a section with the same name had already been included before
	//         <0 on error
	int AddSectionFromMemory(const char *sectionName,
							 const char *scriptCode,
							 unsigned int scriptLength = 0,
							 int lineOffset = 0);

	// Build the added script sections
	int BuildModule();

	// Returns the current module
	asIScriptModule *GetModule();

	// Register the callback for resolving include directive
	void SetIncludeCallback(INCLUDECALLBACK_t callback, void *userParam);

	// Add a pre-processor define for conditional compilation
	void DefineWord(const char *word);

	// Enumerate included script sections
	unsigned int GetSectionCount() const;
	lambda::String  GetSectionName(unsigned int idx) const;

#if AS_PROCESS_METADATA == 1
	// Get metadata declared for class types and interfaces
	const char *GetMetadataStringForType(int typeId);

	// Get metadata declared for functions
	const char *GetMetadataStringForFunc(asIScriptFunction *func);

	// Get metadata declared for global variables
	const char *GetMetadataStringForVar(int varIdx);

	// Get metadata declared for class variables
	const char *GetMetadataStringForTypeProperty(int typeId, int varIdx);

	// Get metadata declared for class functions
	const char *GetMetadataStringForTypeMethod(int typeId, asIScriptFunction *method);
#endif

protected:
	void ClearAll();
	int  Build();
	int  ProcessScriptSection(const char *script, unsigned int length, const char *sectionname, int lineOffset);
	int  LoadScriptSection(const char *filename);
	bool IncludeIfNotAlreadyIncluded(const char *filename);

	int  SkipStatement(int pos);

	int  ExcludeCode(int start);
	void OverwriteCode(int start, int len);

	asIScriptEngine           *engine;
	asIScriptModule           *module;
	lambda::String                modifiedScript;

	INCLUDECALLBACK_t  includeCallback;
	void              *callbackParam;

#if AS_PROCESS_METADATA == 1
	int  ExtractMetadataString(int pos, lambda::String &outMetadata);
	int  ExtractDeclaration(int pos, lambda::String &outDeclaration, int &outType);

	// Temporary structure for storing metadata and declaration
	struct SMetadataDecl
	{
		SMetadataDecl(lambda::String m, lambda::String d, int t, lambda::String c, lambda::String ns) : metadata(m), declaration(d), type(t), parentClass(c), nameSpace(ns) {}
		lambda::String metadata;
		lambda::String declaration;
		int         type;
		lambda::String parentClass;
		lambda::String nameSpace;
	};
	lambda::Vector<SMetadataDecl> foundDeclarations;
	lambda::String currentClass;
	lambda::String currentNamespace;

	// Storage of metadata for global declarations
	lambda::Map<int, lambda::String> typeMetadataMap;
	lambda::Map<int, lambda::String> funcMetadataMap;
	lambda::Map<int, lambda::String> varMetadataMap;

	// Storage of metadata for class member declarations
	struct SClassMetadata
	{
		SClassMetadata(const lambda::String& aName) : className(aName) {}
		lambda::String className;
		lambda::Map<int, lambda::String> funcMetadataMap;
		lambda::Map<int, lambda::String> varMetadataMap;
	};
	lambda::Map<int, SClassMetadata> classMetadataMap;

#endif

#ifdef _WIN32
	// On Windows the filenames are case insensitive so the comparisons to
	// avoid duplicate includes must also be case insensitive. True case insensitive
	// is not easy as it must be language aware, but a simple implementation such
	// as strcmpi should suffice in almost all cases.
	//
	// ref: http://www.gotw.ca/gotw/029.htm
	// ref: https://msdn.microsoft.com/en-us/library/windows/desktop/dd317761(v=vs.85).aspx
	// ref: http://site.icu-project.org/

	// TODO: Strings by default are treated as UTF8 encoded. If the application choses to
	//       use a different encoding, the comparison algorithm should be adjusted as well

	struct ci_less
	{
		bool operator()(const lambda::String &a, const lambda::String &b) const
		{
			return _strcmpi(a.c_str(), b.c_str()) < 0;
		}
	};
  lambda::Set<lambda::String, ci_less> includedScripts;
#else
  lambda::Set<lambda::String>      includedScripts;
#endif

  lambda::Set<lambda::String>      definedWords;
};

END_AS_NAMESPACE

#endif
