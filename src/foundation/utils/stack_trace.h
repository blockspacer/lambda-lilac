#pragma once

// WARNING: The returned cstr needs to be manually freed using std free!
extern const char* captureCallStack(int to_skip = 0);
