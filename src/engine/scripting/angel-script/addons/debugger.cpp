#include "debugger.h"
#include <iostream>  // cout
#include <sstream>   // std::stringstream
#include <stdlib.h>  // atoi
#include <assert.h>  // assert

using namespace lambda;

BEGIN_AS_NAMESPACE

CDebugger::CDebugger()
{
	m_action = CONTINUE;
	m_lastFunction = 0;
	m_engine = 0;
}

CDebugger::~CDebugger()
{
	SetEngine(0);
}

lambda::String CDebugger::ToString(void *value, asUINT typeId, int expandMembers, asIScriptEngine *engine)
{
	if( value == 0 )
		return "<null>";

	// If no engine pointer was provided use the default
	if( engine == 0 )
		engine = m_engine;

	std::stringstream s;
	if( typeId == asTYPEID_VOID )
		return "<void>";
	else if( typeId == asTYPEID_BOOL )
		return *(bool*)value ? "true" : "false";
	else if( typeId == asTYPEID_INT8 )
		s << (int)*(signed char*)value;
	else if( typeId == asTYPEID_INT16 )
		s << (int)*(signed short*)value;
	else if( typeId == asTYPEID_INT32 )
		s << *(signed int*)value;
	else if( typeId == asTYPEID_INT64 )
#if defined(_MSC_VER) && _MSC_VER <= 1200
		s << "{...}"; // MSVC6 doesn't like the << operator for 64bit integer
#else
		s << *(asINT64*)value;
#endif
	else if( typeId == asTYPEID_UINT8 )
		s << (unsigned int)*(unsigned char*)value;
	else if( typeId == asTYPEID_UINT16 )
		s << (unsigned int)*(unsigned short*)value;
	else if( typeId == asTYPEID_UINT32 )
		s << *(unsigned int*)value;
	else if( typeId == asTYPEID_UINT64 )
#if defined(_MSC_VER) && _MSC_VER <= 1200
		s << "{...}"; // MSVC6 doesn't like the << operator for 64bit integer
#else
		s << *(asQWORD*)value;
#endif
	else if( typeId == asTYPEID_FLOAT )
		s << *(float*)value;
	else if( typeId == asTYPEID_DOUBLE )
		s << *(double*)value;
	else if( (typeId & asTYPEID_MASK_OBJECT) == 0 )
	{
		// The type is an enum
		s << *(asUINT*)value;

		// Check if the value matches one of the defined enums
		if( engine )
		{
			asITypeInfo *t = engine->GetTypeInfoById(typeId);
			for( int n = t->GetEnumValueCount(); n-- > 0; )
			{
				int enumVal;
				const char *enumName = t->GetEnumValueByIndex(n, &enumVal);
				if( enumVal == *(int*)value )
				{
					s << ", " << enumName;
					break;
				}
			}
		}
	}
	else if( typeId & asTYPEID_SCRIPTOBJECT )
	{
		// Dereference handles, so we can see what it points to
		if( typeId & asTYPEID_OBJHANDLE )
			value = *(void**)value;

		asIScriptObject *obj = (asIScriptObject *)value;
		
		// Print the address of the object
		s << "{" << obj << "}";

		// Print the members
		if( obj && expandMembers > 0 )
		{
			asITypeInfo *type = obj->GetObjectType();
			for( asUINT n = 0; n < obj->GetPropertyCount(); n++ )
			{
				if( n == 0 )
					s << " ";
				else
					s << ", ";

				s << type->GetPropertyDeclaration(n) << " = " << ToString(obj->GetAddressOfProperty(n), obj->GetPropertyTypeId(n), expandMembers - 1, type->GetEngine()).c_str();
			}
		}
	}
	else
	{
		// Dereference handles, so we can see what it points to
		if( typeId & asTYPEID_OBJHANDLE )
			value = *(void**)value;

		// Print the address for reference types so it will be
		// possible to see when handles point to the same object
		if( engine )
		{
			asITypeInfo *type = engine->GetTypeInfoById(typeId);
			if( type->GetFlags() & asOBJ_REF )
				s << "{" << value << "}";

			if( value )
			{
				// Check if there is a registered to-lambda::String callback
        lambda::Map<const asITypeInfo*, ToStringCallback>::iterator it = m_toStringCallbacks.find(type);
				if( it == m_toStringCallbacks.end() )
				{
					// If the type is a template instance, there might be a
					// to-lambda::String callback for the generic template type
					if( type->GetFlags() & asOBJ_TEMPLATE )
					{
						asITypeInfo *tmplType = engine->GetTypeInfoByName(type->GetName());
						it = m_toStringCallbacks.find(tmplType);
					}
				}

				if( it != m_toStringCallbacks.end() )
				{
					if( type->GetFlags() & asOBJ_REF )
						s << " ";

					// Invoke the callback to get the lambda::String representation of this type
					lambda::String str = it->second(value, expandMembers, this);
					s << str.c_str();
				}
			}
		}
		else
			s << "{no engine}";
	}

	return s.str().c_str();
}

void CDebugger::RegisterToStringCallback(const asITypeInfo *ot, ToStringCallback callback)
{
	if( m_toStringCallbacks.find(ot) == m_toStringCallbacks.end() )
		m_toStringCallbacks.insert(lambda::Map<const asITypeInfo*, ToStringCallback>::value_type(ot, callback));
}

void CDebugger::LineCallback(asIScriptContext *ctx)
{
	assert( ctx );

	// This should never happen, but it doesn't hurt to validate it
	if( ctx == 0 )
		return;

	// By default we ignore callbacks when the context is not active.
	// An application might override this to for example disconnect the
	// debugger as the execution finished.
	if( ctx->GetState() != asEXECUTION_ACTIVE )
		return;

	if( m_action == CONTINUE )
	{
		if( !CheckBreakPoint(ctx) )
			return;
	}
	else if( m_action == STEP_OVER )
	{
		if( ctx->GetCallstackSize() > m_lastCommandAtStackLevel )
		{
			if( !CheckBreakPoint(ctx) )
				return;
		}
	}
	else if( m_action == STEP_OUT )
	{
		if( ctx->GetCallstackSize() >= m_lastCommandAtStackLevel )
		{
			if( !CheckBreakPoint(ctx) )
				return;
		}
	}
	else if( m_action == STEP_INTO )
	{
		CheckBreakPoint(ctx);

		// Always break, but we call the check break point anyway 
		// to tell user when break point has been reached
	}

	std::stringstream s;
	const char *file = 0;
	int lineNbr = ctx->GetLineNumber(0, 0, &file);
	s << (file ? file : "{unnamed}") << ":" << lineNbr << "; " << ctx->GetFunction()->GetDeclaration() << std::endl;
	Output(s.str().c_str());

	TakeCommands(ctx);
}

bool CDebugger::CheckBreakPoint(asIScriptContext *ctx)
{
	if( ctx == 0 )
		return false;

	// TODO: Should cache the break points in a function by checking which possible break points
	//       can be hit when entering a function. If there are no break points in the current function
	//       then there is no need to check every line.

	const char *tmp = 0;
	int lineNbr = ctx->GetLineNumber(0, 0, &tmp);

	// Consider just filename, not the full path
	lambda::String file = tmp ? tmp : "";
	size_t r = file.find_last_of("\\/");
	if( r != lambda::String::npos )
		file = file.substr(r+1);

	// Did we move into a new function?
	asIScriptFunction *func = ctx->GetFunction();
	if( m_lastFunction != func )
	{
		// Check if any breakpoints need adjusting
		for( size_t n = 0; n < m_breakPoints.size(); n++ )
		{
			// We need to check for a breakpoint at entering the function
			if( m_breakPoints[n].func )
			{
				if( m_breakPoints[n].name == func->GetName() )
				{
					std::stringstream s;
					s << "Entering function '" << m_breakPoints[n].name.c_str() << "'. Transforming it into break point" << std::endl;
					Output(s.str().c_str());

					// Transform the function breakpoint into a file breakpoint
					m_breakPoints[n].name           = file;
					m_breakPoints[n].lineNbr        = lineNbr;
					m_breakPoints[n].func           = false;
					m_breakPoints[n].needsAdjusting = false;
				}
			}
			// Check if a given breakpoint fall on a line with code or else adjust it to the next line
			else if( m_breakPoints[n].needsAdjusting &&
					 m_breakPoints[n].name == file )
			{
				int line = func->FindNextLineWithCode(m_breakPoints[n].lineNbr);
				if( line >= 0 )
				{
					m_breakPoints[n].needsAdjusting = false;
					if( line != m_breakPoints[n].lineNbr )
					{
						std::stringstream s;
						s << "Moving break point " << n << " in file '" << file.c_str() << "' to next line with code at line " << line << std::endl;
						Output(s.str().c_str());

						// Move the breakpoint to the next line
						m_breakPoints[n].lineNbr = line;
					}
				}
			}
		}
	}
	m_lastFunction = func;

	// Determine if there is a breakpoint at the current line
	for( size_t n = 0; n < m_breakPoints.size(); n++ )
	{
		// TODO: do case-less comparison for file name

		// Should we break?
		if( !m_breakPoints[n].func &&
			m_breakPoints[n].lineNbr == lineNbr &&
			m_breakPoints[n].name == file )
		{
			std::stringstream s;
			s << "Reached break point " << n << " in file '" << file.c_str() << "' at line " << lineNbr << std::endl;
			Output(s.str().c_str());
			return true;
		}
	}

	return false;
}

void CDebugger::TakeCommands(asIScriptContext *ctx)
{
	for(;;)
	{
		char buf[512];

		Output("[dbg]> ");
		std::cin.getline(buf, 512);

		if( InterpretCommand(lambda::String(buf), ctx) )
			break;
	}
}

bool CDebugger::InterpretCommand(const lambda::String &cmd, asIScriptContext *ctx)
{
	if( cmd.length() == 0 ) return true;

	switch( cmd[0] )
	{
	case 'c':
		m_action = CONTINUE;
		break;

	case 's':
		m_action = STEP_INTO;
		break;

	case 'n':
		if( ctx == 0 )
		{
			Output("No script is running\n");
			return false;
		}
		m_action = STEP_OVER;
		m_lastCommandAtStackLevel = ctx->GetCallstackSize();
		break;

	case 'o':
		if( ctx == 0 )
		{
			Output("No script is running\n");
			return false;
		}
		m_action = STEP_OUT;
		m_lastCommandAtStackLevel = ctx->GetCallstackSize();
		break;

	case 'b':
		{
			// Set break point
			size_t div = cmd.find(':'); 
			if( div != lambda::String::npos && div > 2 )
			{
				lambda::String file = cmd.substr(2, div-2);
				lambda::String line = cmd.substr(div+1);

				int nbr = atoi(line.c_str());

				AddFileBreakPoint(file, nbr);
			}
			else if( div == lambda::String::npos && (div = cmd.find_first_not_of(" \t", 1)) != lambda::String::npos )
			{
				lambda::String func = cmd.substr(div);

				AddFuncBreakPoint(func);
			}
			else
			{
				Output("Incorrect format for setting break point, expected one of:\n"
				       "b <file name>:<line number>\n"
				       "b <function name>\n");
			}
		}
		// take more commands
		return false;

	case 'r':
		{
			// Remove break point
			if( cmd.length() > 2 )
			{
				lambda::String br = cmd.substr(2);
				if( br == "all" )
				{
					m_breakPoints.clear();
					Output("All break points have been removed\n");
				}
				else
				{
					int nbr = atoi(br.c_str());
					if( nbr >= 0 && nbr < (int)m_breakPoints.size() )
						m_breakPoints.erase(m_breakPoints.begin()+nbr);
					ListBreakPoints();
				}
			}
			else
			{
				Output("Incorrect format for removing break points, expected:\n"
				       "r <all|number of break point>\n");
			}
		}
		// take more commands
		return false;

	case 'l':
		{
			// List something
			size_t p = cmd.find_first_not_of(" \t", 1);
			if( p != lambda::String::npos )
			{
				if( cmd[p] == 'b' )
				{
					ListBreakPoints();
				}
				else if( cmd[p] == 'v' )
				{
					ListLocalVariables(ctx);
				}
				else if( cmd[p] == 'g' )
				{
					ListGlobalVariables(ctx);
				}
				else if( cmd[p] == 'm' )
				{
					ListMemberProperties(ctx);
				}
				else if( cmd[p] == 's' )
				{
					ListStatistics(ctx);
				}
				else
				{
					Output("Unknown list option, expected one of:\n"
					       "b - breakpoints\n"
					       "v - local variables\n"
						   "m - member properties\n"
					       "g - global variables\n"
						   "s - statistics\n");
				}
			}
			else 
			{
				Output("Incorrect format for list, expected:\n"
				       "l <list option>\n");
			}
		}
		// take more commands
		return false;

	case 'h':
		PrintHelp();
		// take more commands
		return false;

	case 'p':
		{
			// Print a value 
			size_t p = cmd.find_first_not_of(" \t", 1);
			if( p != lambda::String::npos )
			{
				PrintValue(cmd.substr(p), ctx);
			}
			else
			{
				Output("Incorrect format for print, expected:\n"
					   "p <expression>\n");
			}
		}
		// take more commands
		return false;

	case 'w':
		// Where am I?
		PrintCallstack(ctx);
		// take more commands
		return false;

	case 'a':
		// abort the execution
		if( ctx == 0 )
		{
			Output("No script is running\n");
			return false;
		}
		ctx->Abort();
		break;

	default:
		Output("Unknown command\n");
		// take more commands
		return false;
	}

	// Continue execution
	return true;
}

void CDebugger::PrintValue(const lambda::String &expr, asIScriptContext *ctx)
{
	if( ctx == 0 )
	{
		Output("No script is running\n");
		return;
	}

	asIScriptEngine *engine = ctx->GetEngine();

	// Tokenize the input lambda::String to get the variable scope and name
	asUINT len = 0;
	lambda::String scope;
	lambda::String name;
	lambda::String str = expr;
	asETokenClass t = engine->ParseToken(str.c_str(), 0, &len);
	while( t == asTC_IDENTIFIER || (t == asTC_KEYWORD && len == 2 && str.compare("::")) )
	{
		if( t == asTC_KEYWORD )
		{
			if( scope == "" && name == "" )
				scope = "::";			// global scope
			else if( scope == "::" || scope == "" )
				scope = name;			// namespace
			else
				scope += "::" + name;	// nested namespace
			name = "";
		}
		else if( t == asTC_IDENTIFIER )
			name.assign(str.c_str(), len);

		// Skip the parsed token and get the next one
		str = str.substr(len);
		t = engine->ParseToken(str.c_str(), 0, &len);
	}

	if( name.size() )
	{
		// Find the variable
		void *ptr = 0;
		int typeId = 0;

		asIScriptFunction *func = ctx->GetFunction();
		if( !func ) return;

		// skip local variables if a scope was informed
		if( scope == "" )
		{
			// We start from the end, in case the same name is reused in different scopes
			for( asUINT n = func->GetVarCount(); n-- > 0; )
			{
				if( ctx->IsVarInScope(n) && name == ctx->GetVarName(n) )
				{
					ptr = ctx->GetAddressOfVar(n);
					typeId = ctx->GetVarTypeId(n);
					break;
				}
			}

			// Look for class members, if we're in a class method
			if( !ptr && func->GetObjectType() )
			{
				if( name == "this" )
				{
					ptr = ctx->GetThisPointer();
					typeId = ctx->GetThisTypeId();
				}
				else
				{
					asITypeInfo *type = engine->GetTypeInfoById(ctx->GetThisTypeId());
					for( asUINT n = 0; n < type->GetPropertyCount(); n++ )
					{
						const char *propName = 0;
						int offset = 0;
						bool isReference = 0;
						type->GetProperty(n, &propName, &typeId, 0, 0, &offset, &isReference);
						if( name == propName )
						{
							ptr = (void*)(((asBYTE*)ctx->GetThisPointer())+offset);
							if( isReference ) ptr = *(void**)ptr;
							break;
						}
					}
				}
			}
		}

		// Look for global variables
		if( !ptr )
		{
			if( scope == "" )
			{
				// If no explicit scope was informed then use the namespace of the current function by default
				scope = func->GetNamespace();
			}
			else if( scope == "::" )
			{
				// The global namespace will be empty
				scope = "";
			}

			asIScriptModule *mod = func->GetModule();
			if( mod )
			{
				for( asUINT n = 0; n < mod->GetGlobalVarCount(); n++ )
				{
					const char *varName = 0, *nameSpace = 0;
					mod->GetGlobalVar(n, &varName, &nameSpace, &typeId);

					// Check if both name and namespace match
					if( name == varName && scope == nameSpace )
					{
						ptr = mod->GetAddressOfGlobalVar(n);
						break;
					}
				}
			}
		}

		if( ptr )
		{
			// TODO: If there is a . after the identifier, check for members
			// TODO: If there is a [ after the identifier try to call the 'opIndex(expr) const' method 

			std::stringstream s;
			// TODO: Allow user to set if members should be expanded
			// Expand members by default to 3 recursive levels only
			s << ToString(ptr, typeId, 3, engine).c_str() << std::endl;
			Output(s.str().c_str());
		}
	}
	else
	{
		Output("Invalid expression. Expected identifier\n");
	}
}

void CDebugger::ListBreakPoints()
{
	// List all break points
	std::stringstream s;
	for( size_t b = 0; b < m_breakPoints.size(); b++ )
		if( m_breakPoints[b].func )
			s << b << " - " << m_breakPoints[b].name.c_str() << std::endl;
		else
			s << b << " - " << m_breakPoints[b].name.c_str() << ":" << m_breakPoints[b].lineNbr << std::endl;
	Output(s.str().c_str());
}

void CDebugger::ListMemberProperties(asIScriptContext *ctx)
{
	if( ctx == 0 )
	{
		Output("No script is running\n");
		return;
	}

	void *ptr = ctx->GetThisPointer();
	if( ptr )
	{
		std::stringstream s;
		// TODO: Allow user to define if members should be expanded or not
		// Expand members by default to 3 recursive levels only
		s << "this = " << ToString(ptr, ctx->GetThisTypeId(), 3, ctx->GetEngine()).c_str() << std::endl;
		Output(s.str().c_str());
	}
}

void CDebugger::ListLocalVariables(asIScriptContext *ctx)
{
	if( ctx == 0 )
	{
		Output("No script is running\n");
		return;
	}

	asIScriptFunction *func = ctx->GetFunction();
	if( !func ) return;

	std::stringstream s;
	for( asUINT n = 0; n < func->GetVarCount(); n++ )
	{
		if( ctx->IsVarInScope(n) )
		{
			// TODO: Allow user to set if members should be expanded or not
			// Expand members by default to 3 recursive levels only
			s << func->GetVarDecl(n) << " = " << ToString(ctx->GetAddressOfVar(n), ctx->GetVarTypeId(n), 3, ctx->GetEngine()).c_str() << std::endl;
		}
	}
	Output(s.str().c_str());
}

void CDebugger::ListGlobalVariables(asIScriptContext *ctx)
{
	if( ctx == 0 )
	{
		Output("No script is running\n");
		return;
	}

	// Determine the current module from the function
	asIScriptFunction *func = ctx->GetFunction();
	if( !func ) return;

	asIScriptModule *mod = func->GetModule();
	if( !mod ) return;

	std::stringstream s;
	for( asUINT n = 0; n < mod->GetGlobalVarCount(); n++ )
	{
		int typeId = 0;
		mod->GetGlobalVar(n, 0, 0, &typeId);
		// TODO: Allow user to set how many recursive expansions should be done
		// Expand members by default to 3 recursive levels only
		s << mod->GetGlobalVarDeclaration(n) << " = " << ToString(mod->GetAddressOfGlobalVar(n), typeId, 3, ctx->GetEngine()).c_str() << std::endl;
	}
	Output(s.str().c_str());
}

void CDebugger::ListStatistics(asIScriptContext *ctx)
{
	if( ctx == 0 )
	{
		Output("No script is running\n");
		return;
	}

	asIScriptEngine *engine = ctx->GetEngine();
	
	asUINT gcCurrSize, gcTotalDestr, gcTotalDet, gcNewObjects, gcTotalNewDestr;
	engine->GetGCStatistics(&gcCurrSize, &gcTotalDestr, &gcTotalDet, &gcNewObjects, &gcTotalNewDestr);

	std::stringstream s;
	s << "Garbage collector:" << std::endl;
	s << " current size:          " << gcCurrSize << std::endl;
	s << " total destroyed:       " << gcTotalDestr << std::endl;
	s << " total detected:        " << gcTotalDet << std::endl;
	s << " new objects:           " << gcNewObjects << std::endl;
	s << " new objects destroyed: " << gcTotalNewDestr << std::endl;

	Output(s.str().c_str());
}

void CDebugger::PrintCallstack(asIScriptContext *ctx)
{
	if( ctx == 0 )
	{
		Output("No script is running\n");
		return;
	}

	std::stringstream s;
	const char *file = 0;
	int lineNbr = 0;
	for( asUINT n = 0; n < ctx->GetCallstackSize(); n++ )
	{
		lineNbr = ctx->GetLineNumber(n, 0, &file);
		s << file << ":" << lineNbr << "; " << ctx->GetFunction(n)->GetDeclaration() << std::endl;
	}
	Output(s.str().c_str());
}

void CDebugger::AddFuncBreakPoint(const lambda::String &func)
{
	// Trim the function name
	size_t b = func.find_first_not_of(" \t");
	size_t e = func.find_last_not_of(" \t");
	lambda::String actual = func.substr(b, e != lambda::String::npos ? e-b+1 : lambda::String::npos);

	std::stringstream s;
	s << "Adding deferred break point for function '" << actual.c_str() << "'" << std::endl;
	Output(s.str().c_str());

	BreakPoint bp(actual, 0, true);
	m_breakPoints.push_back(bp);
}

void CDebugger::AddFileBreakPoint(const lambda::String &file, int lineNbr)
{
	// Store just file name, not entire path
	size_t r = file.find_last_of("\\/");
	lambda::String actual;
	if( r != lambda::String::npos )
		actual = file.substr(r+1);
	else
		actual = file;

	// Trim the file name
	size_t b = actual.find_first_not_of(" \t");
	size_t e = actual.find_last_not_of(" \t");
	actual = actual.substr(b, e != lambda::String::npos ? e-b+1 : lambda::String::npos);

	std::stringstream s;
	s << "Setting break point in file '" << actual.c_str() << "' at line " << lineNbr << std::endl;
	Output(s.str().c_str());

	BreakPoint bp(actual, lineNbr, false);
	m_breakPoints.push_back(bp);
}

void CDebugger::PrintHelp()
{
	Output("c - Continue\n"
	       "s - Step into\n"
	       "n - Next step\n"
	       "o - Step out\n"
	       "b - Set break point\n"
	       "l - List various things\n"
	       "r - Remove break point\n"
	       "p - Print value\n"
	       "w - Where am I?\n"
	       "a - Abort execution\n"
	       "h - Print this help text\n");
}

void CDebugger::Output(const lambda::String &str)
{
	// By default we just output to stdout
	std::cout << str.c_str();
}

void CDebugger::SetEngine(asIScriptEngine *engine)
{
	if( m_engine != engine )
	{
		if( m_engine )
			m_engine->Release();
		m_engine = engine;
		if( m_engine )
			m_engine->AddRef();
	}
}

asIScriptEngine *CDebugger::GetEngine()
{
	return m_engine;
}

END_AS_NAMESPACE
