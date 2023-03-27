#ifndef DEBUGGER_H
#define DEBUGGER_H

#ifndef ANGELSCRIPT_H 
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

#include <containers/containers.h>

BEGIN_AS_NAMESPACE

class CDebugger
{
public:
	CDebugger();
	virtual ~CDebugger();

	// Register callbacks to handle to-string conversions of application types
	// The expandMembersLevel is a counter for how many recursive levels the members should be expanded.
	// If the object that is being converted to a string has members of its own the callback should call
	// the debugger's ToString passing in expandMembersLevel - 1.
	typedef lambda::String (*ToStringCallback)(void *obj, int expandMembersLevel, CDebugger *dbg);
	virtual void RegisterToStringCallback(const asITypeInfo *ti, ToStringCallback callback);

	// User interaction
	virtual void TakeCommands(asIScriptContext *ctx);
	virtual void Output(const lambda::String &str);

	// Line callback invoked by context
	virtual void LineCallback(asIScriptContext *ctx);

	// Commands
	virtual void PrintHelp();
	virtual void AddFileBreakPoint(const lambda::String &file, int lineNbr);
	virtual void AddFuncBreakPoint(const lambda::String &func);
	virtual void ListBreakPoints();
	virtual void ListLocalVariables(asIScriptContext *ctx);
	virtual void ListGlobalVariables(asIScriptContext *ctx);
	virtual void ListMemberProperties(asIScriptContext *ctx);
	virtual void ListStatistics(asIScriptContext *ctx);
	virtual void PrintCallstack(asIScriptContext *ctx);
	virtual void PrintValue(const lambda::String &expr, asIScriptContext *ctx);

	// Helpers
	virtual bool InterpretCommand(const lambda::String &cmd, asIScriptContext *ctx);
	virtual bool CheckBreakPoint(asIScriptContext *ctx);
	virtual lambda::String ToString(void *value, asUINT typeId, int expandMembersLevel, asIScriptEngine *engine);

	// Optionally set the engine pointer in the debugger so it can be retrieved
	// by callbacks that need it. This will hold a reference to the engine.
	virtual void SetEngine(asIScriptEngine *engine);
	virtual asIScriptEngine *GetEngine();
	
protected:
	enum DebugAction
	{
		CONTINUE,  // continue until next break point
		STEP_INTO, // stop at next instruction
		STEP_OVER, // stop at next instruction, skipping called functions
		STEP_OUT   // run until returning from current function
	};
	DebugAction        m_action;
	asUINT             m_lastCommandAtStackLevel;
	asIScriptFunction *m_lastFunction;

	struct BreakPoint
	{
		BreakPoint(lambda::String f, int n, bool _func) : name(f), lineNbr(n), func(_func), needsAdjusting(true) {}
		lambda::String name;
		int         lineNbr;
		bool        func;
		bool        needsAdjusting;
	};
	lambda::Vector<BreakPoint> m_breakPoints;

	asIScriptEngine *m_engine;

	// Registered callbacks for converting types to strings
	lambda::Map<const asITypeInfo*, ToStringCallback> m_toStringCallbacks;
};

END_AS_NAMESPACE

#endif