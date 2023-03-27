#ifndef AS_SCRIPTFILESYSTEM_H
#define AS_SCRIPTFILESYSTEM_H

#ifndef ANGELSCRIPT_H 
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

#include <stdio.h>

#include "scriptarray.h"
#include <containers/containers.h>

BEGIN_AS_NAMESPACE

class CScriptFileSystem
{
public:
    CScriptFileSystem();

    void AddRef() const;
    void Release() const;

	// Sets the current path that should be used in other calls when using relative paths
	// It can use relative paths too, so moving up a directory is used by passing in ".."
	bool ChangeCurrentPath(const lambda::String &path);
  lambda::String GetCurrentPath() const;

	// Returns true if the path is a directory. Input can be either a full path or a relative path
	bool IsDir(const lambda::String &path) const;

	// Returns a list of the files in the current path
	CScriptArray *GetFiles() const;

	// Returns a list of the directories in the current path
	CScriptArray *GetDirs() const;

protected:
    ~CScriptFileSystem();

    mutable int refCount;
    lambda::String currentPath;
};

void RegisterScriptFileSystem(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif
