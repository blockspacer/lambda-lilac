#include "stack_trace.h"
#include <StackWalker/StackWalker.h>
#include <stdio.h>


inline const char* sprintfHelper(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	unsigned int size = vsnprintf(nullptr, 0, format, args) + 1;
	va_end(args);

	va_start(args, format);
	char* msg = (char*)malloc(size);
	vsnprintf(msg, size, format, args);
	va_end(args);
	msg[size - 1] = '\0';
	return msg;
}


class MyStackWalker : public StackWalker
{
public:
	MyStackWalker() 
		: StackWalker()
		, num_hits(0)
		, hit_main(false)
		, callstack(nullptr) 
		, num_to_skip(2)
	{
	}
	~MyStackWalker()
	{
	}
	void reset(int to_skip)
	{
		callstack = nullptr;
		hit_main = false;
		num_hits = false;
		num_to_skip = 2 + to_skip;
	}
	const char* callstack; // TODO (Hilze): This is a huge memory leak.

protected:
	virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry& entry) override
	{
		if (num_hits++ >= 2 && !hit_main)
		{
			if (callstack)
				callstack = sprintfHelper("%s%s (%i): %s\n", callstack, entry.lineFileName, (int)entry.lineNumber, entry.name);
			else
				callstack = sprintfHelper("%s (%i): %s\n", entry.lineFileName, (int)entry.lineNumber, entry.name);
			hit_main = strlen(entry.name) == strlen("main") && strstr(entry.name, "main");
		}
		StackWalker::OnCallstackEntry(eType, entry);
	}

private:
	int num_to_skip;
	int num_hits;
	bool hit_main;
};

extern const char* captureCallStack(int to_skip)
{
	static bool kIsInCallstack = false;

	if (kIsInCallstack)
		return "";

	kIsInCallstack = true;
	static MyStackWalker sw;
	sw.reset(to_skip);
	sw.ShowCallstack();

	kIsInCallstack = false;
	return sw.callstack;
}