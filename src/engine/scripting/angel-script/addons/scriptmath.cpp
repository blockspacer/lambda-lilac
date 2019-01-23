#include <assert.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include "scriptmath.h"

#ifdef __BORLANDC__
#include <cmath>

// The C++Builder RTL doesn't pull the *f functions into the global namespace per default.
using namespace std;

#if __BORLANDC__ < 0x580
// C++Builder 6 and earlier don't come with any *f variants of the math functions at all.
inline float cosf (float arg) { return std::cos (arg); }
inline float sinf (float arg) { return std::sin (arg); }
inline float tanf (float arg) { return std::tan (arg); }
inline float atan2f (float y, float x) { return std::atan2 (y, x); }
inline float logf (float arg) { return std::log (arg); }
inline float powf (float x, float y) { return std::pow (x, y); }
inline float sqrtf (float arg) { return std::sqrt (arg); }
#endif

// C++Builder doesn't define most of the non-standard float-specific math functions with
// "*f" suffix; instead it provides overloads for the standard math functions which take
// "float" arguments.
inline float acosf (float arg) { return std::acos (arg); }
inline float asinf (float arg) { return std::asin (arg); }
inline float atanf (float arg) { return std::atan (arg); }
inline float coshf (float arg) { return std::cosh (arg); }
inline float sinhf (float arg) { return std::sinh (arg); }
inline float tanhf (float arg) { return std::tanh (arg); }
inline float log10f (float arg) { return std::log10 (arg); }
inline float ceilf (float arg) { return std::ceil (arg); }
inline float fabsf (float arg) { return std::fabs (arg); }
inline float floorf (float arg) { return std::floor (arg); }

// C++Builder doesn't define a non-standard "modff" function but rather an overload of "modf"
// for float arguments. However, BCC's float overload of fmod() is broken (QC #74816; fixed
// in C++Builder 2010).
inline float modff (float x, float *y)
{
	double d;
	float f = (float) modf((double) x, &d);
	*y = (float) d;
	return f;
}
#endif

BEGIN_AS_NAMESPACE

// Determine whether the float version should be registered, or the double version
#ifndef AS_USE_FLOAT
#if !defined(_WIN32_WCE) // WinCE doesn't have the float versions of the math functions
#define AS_USE_FLOAT 1
#endif
#endif

// The modf function doesn't seem very intuitive, so I'm writing this 
// function that simply returns the fractional part of the float value
#if AS_USE_FLOAT
float fractionf(float v)
{
	float intPart;
	return modff(v, &intPart);
}
#else
double fraction(double v)
{
	double intPart;
	return modf(v, &intPart);
}
#endif

// As AngelScript doesn't allow bitwise manipulation of float types we'll provide a couple of
// functions for converting float values to IEEE 754 formatted values etc. This also allow us to 
// provide a platform agnostic representation to the script so the scripts don't have to worry
// about whether the CPU uses IEEE 754 floats or some other representation
float fpFromIEEE(asUINT raw)
{
	// TODO: Identify CPU family to provide proper conversion
	//        if the CPU doesn't natively use IEEE style floats
	return *reinterpret_cast<float*>(&raw);
}
asUINT fpToIEEE(float fp)
{
	return *reinterpret_cast<asUINT*>(&fp);
}
double fpFromIEEE(asQWORD raw)
{
	return *reinterpret_cast<double*>(&raw);
}
asQWORD fpToIEEE(double fp)
{
	return *reinterpret_cast<asQWORD*>(&fp);
}

// closeTo() is used to determine if the binary representation of two numbers are 
// relatively close to each other. Numerical errors due to rounding errors build
// up over many operations, so it is almost impossible to get exact numbers and
// this is where closeTo() comes in.
//
// It shouldn't be used to determine if two numbers are mathematically close to 
// each other.
//
// ref: http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
// ref: http://www.gamedev.net/topic/653449-scriptmath-and-closeto/
bool closeTo(float a, float b, float epsilon)
{
	// Equal numbers and infinity will return immediately
	if( a == b ) return true;

	// When very close to 0, we can use the absolute comparison
	float diff = fabsf(a - b);
	if( (a == 0 || b == 0) && (diff < epsilon) )
		return true;
	
	// Otherwise we need to use relative comparison to account for precision
	return diff / (fabs(a) + fabs(b)) < epsilon;
}

bool closeTo(double a, double b, double epsilon)
{
	if( a == b ) return true;

	double diff = fabs(a - b);
	if( (a == 0 || b == 0) && (diff < epsilon) )
		return true;
	
	return diff / (fabs(a) + fabs(b)) < epsilon;
}

// Float.
inline float toFloatI8(char arg) { return float(arg); }
inline float toFloatI16(short arg) { return float(arg); }
inline float toFloatI32(int arg) { return float(arg); }
inline float toFloatI64(long long arg) { return float(arg); }
inline float toFloatU8(unsigned char arg) { return float(arg); }
inline float toFloatU16(unsigned short arg) { return float(arg); }
inline float toFloatU32(unsigned int arg) { return float(arg); }
inline float toFloatU64(unsigned long long arg) { return float(arg); }
inline float toFloatD(double arg) { return float(arg); }
inline float toFloatB(bool arg) { return arg == true ? 1.0f : 0.0f; }
// Double.
inline double toDoubleI8(char arg) { return double(arg); }
inline double toDoubleI16(short arg) { return double(arg); }
inline double toDoubleI32(int arg) { return double(arg); }
inline double toDoubleI64(long long arg) { return double(arg); }
inline double toDoubleU8(unsigned char arg) { return double(arg); }
inline double toDoubleU16(unsigned short arg) { return double(arg); }
inline double toDoubleU32(unsigned int arg) { return double(arg); }
inline double toDoubleU64(unsigned long long arg) { return double(arg); }
inline double toDoubleF(float arg) { return double(arg); }
inline double toDoubleB(bool arg) { return arg == true ? 1.0 : 0.0; }
// Char.
inline char toInt8I16(short arg) { return char(arg); }
inline char toInt8I32(int arg) { return char(arg); }
inline char toInt8I64(long long arg) { return char(arg); }
inline char toInt8U8(unsigned char arg) { return char(arg); }
inline char toInt8U16(unsigned short arg) { return char(arg); }
inline char toInt8U32(unsigned int arg) { return char(arg); }
inline char toInt8U64(unsigned long long arg) { return char(arg); }
inline char toInt8F(float arg) { return char(arg); }
inline char toInt8D(double arg) { return char(arg); }
inline char toInt8B(bool arg) { return arg == true ? 1 : 0; }
// Short.
inline short toInt16I8(char arg) { return short(arg); }
inline short toInt16I32(int arg) { return short(arg); }
inline short toInt16I64(long long arg) { return short(arg); }
inline short toInt16U8(unsigned char arg) { return short(arg); }
inline short toInt16U16(unsigned short arg) { return short(arg); }
inline short toInt16U32(unsigned int arg) { return short(arg); }
inline short toInt16U64(unsigned long long arg) { return short(arg); }
inline short toInt16F(float arg) { return short(arg); }
inline short toInt16D(double arg) { return short(arg); }
inline short toInt16B(bool arg) { return arg == true ? 1 : 0; }
// Int.
inline int toInt32I8(char arg) { return int(arg); }
inline int toInt32I16(short arg) { return int(arg); }
inline int toInt32I64(long long arg) { return int(arg); }
inline int toInt32U8(unsigned char arg) { return int(arg); }
inline int toInt32U16(unsigned short arg) { return int(arg); }
inline int toInt32U32(unsigned int arg) { return int(arg); }
inline int toInt32U64(unsigned long long arg) { return int(arg); }
inline int toInt32F(float arg) { return int(arg); }
inline int toInt32D(double arg) { return int(arg); }
inline int toInt32B(bool arg) { return arg == true ? 1 : 0; }
// Long long.
inline long long toInt64I8(char arg) { return (long long)(arg); }
inline long long toInt64I16(short arg) { return (long long)(arg); }
inline long long toInt64I32(int arg) { return (long long)(arg); }
inline long long toInt64U8(unsigned char arg) { return (long long)(arg); }
inline long long toInt64U16(unsigned short arg) { return (long long)(arg); }
inline long long toInt64U32(unsigned int arg) { return (long long)(arg); }
inline long long toInt64U64(unsigned long long arg) { return (long long)(arg); }
inline long long toInt64F(float arg) { return (long long)(arg); }
inline long long toInt64D(double arg) { return (long long)(arg); }
inline long long toInt64B(bool arg) { return arg == true ? 1 : 0; }
// Unsigned char.
inline unsigned char toUint8I8(char arg) { return (unsigned char)(arg); }
inline unsigned char toUint8I16(short arg) { return (unsigned char)(arg); }
inline unsigned char toUint8I32(int arg) { return (unsigned char)(arg); }
inline unsigned char toUint8I64(long long arg) { return (unsigned char)(arg); }
inline unsigned char toUint8U16(unsigned short arg) { return (unsigned char)(arg); }
inline unsigned char toUint8U32(unsigned int arg) { return (unsigned char)(arg); }
inline unsigned char toUint8U64(unsigned long long arg) { return (unsigned char)(arg); }
inline unsigned char toUint8F(float arg) { return (unsigned char)(arg); }
inline unsigned char toUint8D(double arg) { return (unsigned char)(arg); }
inline unsigned char toUint8B(bool arg) { return arg == true ? 1 : 0; }
// Unsigned short.
inline unsigned short toUint16I8(char arg) { return (unsigned short)(arg); }
inline unsigned short toUint16I16(short arg) { return (unsigned short)(arg); }
inline unsigned short toUint16I32(int arg) { return (unsigned short)(arg); }
inline unsigned short toUint16I64(long long arg) { return (unsigned short)(arg); }
inline unsigned short toUint16U8(unsigned char arg) { return (unsigned short)(arg); }
inline unsigned short toUint16U32(unsigned int arg) { return (unsigned short)(arg); }
inline unsigned short toUint16U64(unsigned long long arg) { return (unsigned short)(arg); }
inline unsigned short toUint16F(float arg) { return (unsigned short)(arg); }
inline unsigned short toUint16D(double arg) { return (unsigned short)(arg); }
inline unsigned short toUint16B(bool arg) { return arg == true ? 1 : 0; }
// Unsigned int.
inline unsigned int toUint32I8(char arg) { return (unsigned int)(arg); }
inline unsigned int toUint32I16(short arg) { return (unsigned int)(arg); }
inline unsigned int toUint32I32(int arg) { return (unsigned int)(arg); }
inline unsigned int toUint32I64(long long arg) { return (unsigned int)(arg); }
inline unsigned int toUint32U8(unsigned char arg) { return (unsigned int)(arg); }
inline unsigned int toUint32U16(unsigned short arg) { return (unsigned int)(arg); }
inline unsigned int toUint32U64(unsigned long long arg) { return (unsigned int)(arg); }
inline unsigned int toUint32F(float arg) { return (unsigned int)(arg); }
inline unsigned int toUint32D(double arg) { return (unsigned int)(arg); }
inline unsigned int toUint32B(bool arg) { return arg == true ? 1 : 0; }
// Unsigned long long.
inline unsigned long long toUint64I8(char arg) { return (unsigned long long)(arg); }
inline unsigned long long toUint64I16(short arg) { return (unsigned long long)(arg); }
inline unsigned long long toUint64I32(int arg) { return (unsigned long long)(arg); }
inline unsigned long long toUint64I64(long long arg) { return (unsigned long long)(arg); }
inline unsigned long long toUint64U8(unsigned char arg) { return (unsigned long long)(arg); }
inline unsigned long long toUint64U16(unsigned short arg) { return (unsigned long long)(arg); }
inline unsigned long long toUint64U32(unsigned int arg) { return (unsigned long long)(arg); }
inline unsigned long long toUint64F(float arg) { return (unsigned long long)(arg); }
inline unsigned long long toUint64D(double arg) { return (unsigned long long)(arg); }
inline unsigned long long toUint64B(bool arg) { return arg == true ? 1 : 0; }


void RegisterCasting(asIScriptEngine* engine)
{
  int r;
  
  // Float.
  /*r = engine->RegisterGlobalFunction("float ToFloat(int8)",   asFUNCTIONPR(toFloatI8,  (char),               float), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("float ToFloat(int16)",  asFUNCTIONPR(toFloatI16, (short),              float), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("float ToFloat(int32)",  asFUNCTIONPR(toFloatI32, (int),                float), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("float ToFloat(int64)",  asFUNCTIONPR(toFloatI64, (long long),          float), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("float ToFloat(uint8)",  asFUNCTIONPR(toFloatU8,  (unsigned char),      float), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("float ToFloat(uint16)", asFUNCTIONPR(toFloatU16, (unsigned short),     float), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("float ToFloat(uint32)", asFUNCTIONPR(toFloatU32, (unsigned int),       float), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("float ToFloat(uint64)", asFUNCTIONPR(toFloatU64, (unsigned long long), float), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("float ToFloat(double)", asFUNCTIONPR(toFloatD,   (double),             float), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("float ToFloat(bool)",   asFUNCTIONPR(toFloatB,   (bool),               float), asCALL_CDECL); assert(r >= 0);
  // Double.
  r = engine->RegisterGlobalFunction("double ToDouble(int8)",   asFUNCTIONPR(toDoubleI8,  (char),               double), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double ToDouble(int16)",  asFUNCTIONPR(toDoubleI16, (short),              double), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double ToDouble(int32)",  asFUNCTIONPR(toDoubleI32, (int),                double), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double ToDouble(int64)",  asFUNCTIONPR(toDoubleI64, (long long),          double), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double ToDouble(uint8)",  asFUNCTIONPR(toDoubleU8,  (unsigned char),      double), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double ToDouble(uint16)", asFUNCTIONPR(toDoubleU16, (unsigned short),     double), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double ToDouble(uint32)", asFUNCTIONPR(toDoubleU32, (unsigned int),       double), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double ToDouble(uint64)", asFUNCTIONPR(toDoubleU64, (unsigned long long), double), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double ToDouble(double)", asFUNCTIONPR(toDoubleF,   (float),              double), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double ToDouble(bool)",   asFUNCTIONPR(toDoubleB,   (bool),               double), asCALL_CDECL); assert(r >= 0);
  // Char.
  r = engine->RegisterGlobalFunction("int8 ToInt8(int16)",  asFUNCTIONPR(toInt8I16, (short),              char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int8 ToInt8(int32)",  asFUNCTIONPR(toInt8I32, (int),                char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int8 ToInt8(int64)",  asFUNCTIONPR(toInt8I64, (long long),          char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int8 ToInt8(uint8)",  asFUNCTIONPR(toInt8U8,  (unsigned char),      char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int8 ToInt8(uint16)", asFUNCTIONPR(toInt8U16, (unsigned short),     char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int8 ToInt8(uint32)", asFUNCTIONPR(toInt8U32, (unsigned int),       char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int8 ToInt8(uint64)", asFUNCTIONPR(toInt8U64, (unsigned long long), char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int8 ToInt8(float)",  asFUNCTIONPR(toInt8F,   (float),              char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int8 ToInt8(double)", asFUNCTIONPR(toInt8D,   (double),             char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int8 ToInt8(bool)",   asFUNCTIONPR(toInt8B,   (bool),               char), asCALL_CDECL); assert(r >= 0);
  // Short.
  r = engine->RegisterGlobalFunction("int16 ToInt16(int8)",   asFUNCTIONPR(toInt16I8,  (char),               short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int16 ToInt16(int32)",  asFUNCTIONPR(toInt16I32, (int),                short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int16 ToInt16(int64)",  asFUNCTIONPR(toInt16I64, (long long),          short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int16 ToInt16(uint8)",  asFUNCTIONPR(toInt16U8,  (unsigned char),      short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int16 ToInt16(uint16)", asFUNCTIONPR(toInt16U16, (unsigned short),     short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int16 ToInt16(uint32)", asFUNCTIONPR(toInt16U32, (unsigned int),       short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int16 ToInt16(uint64)", asFUNCTIONPR(toInt16U64, (unsigned long long), short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int16 ToInt16(float)",  asFUNCTIONPR(toInt16F,   (float),              short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int16 ToInt16(double)", asFUNCTIONPR(toInt16D,   (double),             short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int16 ToInt16(bool)",   asFUNCTIONPR(toInt16B,   (bool),               short), asCALL_CDECL); assert(r >= 0);
  // Int.
  r = engine->RegisterGlobalFunction("int32 ToInt32(int8)",   asFUNCTIONPR(toInt32I8,  (char),               int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int32 ToInt32(int16)",  asFUNCTIONPR(toInt32I16, (short),              int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int32 ToInt32(int64)",  asFUNCTIONPR(toInt32I64, (long long),          int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int32 ToInt32(uint8)",  asFUNCTIONPR(toInt32U8,  (unsigned char),      int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int32 ToInt32(uint16)", asFUNCTIONPR(toInt32U16, (unsigned short),     int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int32 ToInt32(uint32)", asFUNCTIONPR(toInt32U32, (unsigned int),       int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int32 ToInt32(uint64)", asFUNCTIONPR(toInt32U64, (unsigned long long), int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int32 ToInt32(float)",  asFUNCTIONPR(toInt32F,   (float),              int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int32 ToInt32(double)", asFUNCTIONPR(toInt32D,   (double),             int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int32 ToInt32(bool)",   asFUNCTIONPR(toInt32B,   (bool),               int), asCALL_CDECL); assert(r >= 0);
  // Long long.
  r = engine->RegisterGlobalFunction("int64 ToInt64(int8)",   asFUNCTIONPR(toInt64I8,  (char),               long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int64 ToInt64(int16)",  asFUNCTIONPR(toInt64I16, (short),              long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int64 ToInt64(int32)",  asFUNCTIONPR(toInt64I32, (int),                long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int64 ToInt64(uint8)",  asFUNCTIONPR(toInt64U8,  (unsigned char),      long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int64 ToInt64(uint16)", asFUNCTIONPR(toInt64U16, (unsigned short),     long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int64 ToInt64(uint32)", asFUNCTIONPR(toInt64U32, (unsigned int),       long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int64 ToInt64(uint64)", asFUNCTIONPR(toInt64U64, (unsigned long long), long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int64 ToInt64(float)",  asFUNCTIONPR(toInt64F,   (float),              long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int64 ToInt64(double)", asFUNCTIONPR(toInt64D,   (double),             long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("int64 ToInt64(bool)",   asFUNCTIONPR(toInt64B,   (bool),               long long), asCALL_CDECL); assert(r >= 0);
  // Unsigned char.
  r = engine->RegisterGlobalFunction("uint8 ToUint8(int8)",   asFUNCTIONPR(toUint8I8,  (char),               unsigned char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint8 ToUint8(int16)",  asFUNCTIONPR(toUint8I16, (short),              unsigned char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint8 ToUint8(int32)",  asFUNCTIONPR(toUint8I32, (int),                unsigned char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint8 ToUint8(int64)",  asFUNCTIONPR(toUint8I64, (long long),          unsigned char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint8 ToUint8(uint16)", asFUNCTIONPR(toUint8U16, (unsigned short),     unsigned char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint8 ToUint8(uint32)", asFUNCTIONPR(toUint8U32, (unsigned int),       unsigned char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint8 ToUint8(uint64)", asFUNCTIONPR(toUint8U64, (unsigned long long), unsigned char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint8 ToUint8(float)",  asFUNCTIONPR(toUint8F,   (float),              unsigned char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint8 ToUint8(double)", asFUNCTIONPR(toUint8D,   (double),             unsigned char), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint8 ToUint8(bool)",   asFUNCTIONPR(toUint8B,   (bool),               unsigned char), asCALL_CDECL); assert(r >= 0);
  // Unsigned short.                  
  r = engine->RegisterGlobalFunction("uint16 ToUint16(int8)",   asFUNCTIONPR(toUint16I8,  (char),               unsigned short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint16 ToUint16(int16)",  asFUNCTIONPR(toUint16I16, (short),              unsigned short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint16 ToUint16(int32)",  asFUNCTIONPR(toUint16I32, (int),                unsigned short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint16 ToUint16(int64)",  asFUNCTIONPR(toUint16I64, (long long),          unsigned short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint16 ToUint16(uint8)",  asFUNCTIONPR(toUint16U8,  (unsigned char),      unsigned short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint16 ToUint16(uint32)", asFUNCTIONPR(toUint16U32, (unsigned int),       unsigned short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint16 ToUint16(uint64)", asFUNCTIONPR(toUint16U64, (unsigned long long), unsigned short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint16 ToUint16(float)",  asFUNCTIONPR(toUint16F,   (float),              unsigned short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint16 ToUint16(double)", asFUNCTIONPR(toUint16D,   (double),             unsigned short), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint16 ToUint16(bool)",   asFUNCTIONPR(toUint16B,   (bool),               unsigned short), asCALL_CDECL); assert(r >= 0);
  // Unsigned int.                                                                                              
  r = engine->RegisterGlobalFunction("uint32 ToUint32(int8)",   asFUNCTIONPR(toUint32I8,  (char),               unsigned int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint32 ToUint32(int16)",  asFUNCTIONPR(toUint32I16, (short),              unsigned int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint32 ToUint32(int32)",  asFUNCTIONPR(toUint32I32, (int),                unsigned int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint32 ToUint32(int64)",  asFUNCTIONPR(toUint32I64, (long long),          unsigned int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint32 ToUint32(uint8)",  asFUNCTIONPR(toUint32U8,  (unsigned char),      unsigned int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint32 ToUint32(uint16)", asFUNCTIONPR(toUint32U16, (unsigned short),     unsigned int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint32 ToUint32(uint64)", asFUNCTIONPR(toUint32U64, (unsigned long long), unsigned int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint32 ToUint32(float)",  asFUNCTIONPR(toUint32F,   (float),              unsigned int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint32 ToUint32(double)", asFUNCTIONPR(toUint32D,   (double),             unsigned int), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint32 ToUint32(bool)",   asFUNCTIONPR(toUint32B,   (bool),               unsigned int), asCALL_CDECL); assert(r >= 0);
  // Unsigned long long.                                                                                        
  r = engine->RegisterGlobalFunction("uint64 ToUint64(int8)",   asFUNCTIONPR(toUint64I8,  (char),               unsigned long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint64 ToUint64(int16)",  asFUNCTIONPR(toUint64I16, (short),              unsigned long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint64 ToUint64(int32)",  asFUNCTIONPR(toUint64I32, (int),                unsigned long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint64 ToUint64(int64)",  asFUNCTIONPR(toUint64I64, (long long),          unsigned long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint64 ToUint64(uint8)",  asFUNCTIONPR(toUint64U8,  (unsigned char),      unsigned long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint64 ToUint64(uint16)", asFUNCTIONPR(toUint64U16, (unsigned short),     unsigned long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint64 ToUint64(uint32)", asFUNCTIONPR(toUint64U32, (unsigned int),       unsigned long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint64 ToUint64(float)",  asFUNCTIONPR(toUint64F,   (float),              unsigned long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint64 ToUint64(double)", asFUNCTIONPR(toUint64D,   (double),             unsigned long long), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("uint64 ToUint64(bool)",   asFUNCTIONPR(toUint64B,   (bool),               unsigned long long), asCALL_CDECL); assert(r >= 0);*/
}

void RegisterScriptMath_Native(asIScriptEngine *engine)
{
  RegisterCasting(engine);

	int r;

	// Conversion between floating point and IEEE bits representations
	r = engine->RegisterGlobalFunction("float fpFromIEEE(uint)", asFUNCTIONPR(fpFromIEEE, (asUINT), float), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("uint fpToIEEE(float)", asFUNCTIONPR(fpToIEEE, (float), asUINT), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double fpFromIEEE(uint64)", asFUNCTIONPR(fpFromIEEE, (asQWORD), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("uint64 fpToIEEE(double)", asFUNCTIONPR(fpToIEEE, (double), asQWORD), asCALL_CDECL); assert( r >= 0 );

	// Close to comparison with epsilon 
	r = engine->RegisterGlobalFunction("bool CloseTo(float, float, float = 0.00001f)", asFUNCTIONPR(closeTo, (float, float, float), bool), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("bool CloseTo(double, double, double = 0.0000000001)", asFUNCTIONPR(closeTo, (double, double, double), bool), asCALL_CDECL); assert( r >= 0 );

#if AS_USE_FLOAT
	// Trigonometric functions
	r = engine->RegisterGlobalFunction("float Cos(float)", asFUNCTIONPR(cosf, (float), float), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Sin(float)", asFUNCTIONPR(sinf, (float), float), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Tan(float)", asFUNCTIONPR(tanf, (float), float), asCALL_CDECL); assert( r >= 0 );

	r = engine->RegisterGlobalFunction("float ACos(float)", asFUNCTIONPR(acosf, (float), float), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float ASin(float)", asFUNCTIONPR(asinf, (float), float), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float ATan(float)", asFUNCTIONPR(atanf, (float), float), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float ATan2(float,float)", asFUNCTIONPR(atan2f, (float, float), float), asCALL_CDECL); assert( r >= 0 );

	// Hyberbolic functions
	r = engine->RegisterGlobalFunction("float Cosh(float)", asFUNCTIONPR(coshf, (float), float), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Sinh(float)", asFUNCTIONPR(sinhf, (float), float), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Tanh(float)", asFUNCTIONPR(tanhf, (float), float), asCALL_CDECL); assert( r >= 0 );

	// Exponential and logarithmic functions
	r = engine->RegisterGlobalFunction("float Log(float)", asFUNCTIONPR(logf, (float), float), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Log10(float)", asFUNCTIONPR(log10f, (float), float), asCALL_CDECL); assert( r >= 0 );

	// Power functions
	r = engine->RegisterGlobalFunction("float Pow(float, float)", asFUNCTIONPR(powf, (float, float), float), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Sqrt(float)", asFUNCTIONPR(sqrtf, (float), float), asCALL_CDECL); assert( r >= 0 );

  // Nearest integer, absolute value, and remainder functions
  r = engine->RegisterGlobalFunction("float Min(float, float)", asFUNCTIONPR(fminf, (float, float), float), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("float Max(float, float)", asFUNCTIONPR(fmaxf, (float, float), float), asCALL_CDECL); assert(r >= 0);

	// Nearest integer, absolute value, and remainder functions
	r = engine->RegisterGlobalFunction("float Ceil(float)", asFUNCTIONPR(ceilf, (float), float), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Abs(float)", asFUNCTIONPR(fabsf, (float), float), asCALL_CDECL); assert( r >= 0 );
  r = engine->RegisterGlobalFunction("float Floor(float)", asFUNCTIONPR(floorf, (float), float), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("float Trunc(float)", asFUNCTIONPR(truncf, (float), float), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Fraction(float)", asFUNCTIONPR(fractionf, (float), float), asCALL_CDECL); assert( r >= 0 );

	// Don't register modf because AngelScript already supports the % operator
#else
	// double versions of the same
	r = engine->RegisterGlobalFunction("double Cos(double)", asFUNCTIONPR(cos, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Sin(double)", asFUNCTIONPR(sin, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Tan(double)", asFUNCTIONPR(tan, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double ACos(double)", asFUNCTIONPR(acos, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double ASin(double)", asFUNCTIONPR(asin, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Atan(double)", asFUNCTIONPR(atan, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Atan2(double,double)", asFUNCTIONPR(atan2, (double, double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Cosh(double)", asFUNCTIONPR(cosh, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Sinh(double)", asFUNCTIONPR(sinh, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Tanh(double)", asFUNCTIONPR(tanh, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Log(double)", asFUNCTIONPR(log, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Log10(double)", asFUNCTIONPR(log10, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Pow(double, double)", asFUNCTIONPR(pow, (double, double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Sqrt(double)", asFUNCTIONPR(sqrt, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Ceil(double)", asFUNCTIONPR(ceil, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Abs(double)", asFUNCTIONPR(fabs, (double), double), asCALL_CDECL); assert( r >= 0 );

  r = engine->RegisterGlobalFunction("double Min(double, double)", asFUNCTIONPR(fmin, (double, double), double), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double Max(double, double)", asFUNCTIONPR(fmax, (double, double), double), asCALL_CDECL); assert(r >= 0);

  r = engine->RegisterGlobalFunction("double Floor(double)", asFUNCTIONPR(floor, (double), double), asCALL_CDECL); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double Trunc(double)", asFUNCTIONPR(trunc, (double), double), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Fraction(double)", asFUNCTIONPR(fraction, (double), double), asCALL_CDECL); assert( r >= 0 );
#endif
}

#if AS_USE_FLOAT
// This macro creates simple generic wrappers for functions of type 'float func(float)'
#define GENERICff(x) \
void x##_generic(asIScriptGeneric *gen) \
{ \
	float f = *(float*)gen->GetAddressOfArg(0); \
	*(float*)gen->GetAddressOfReturnLocation() = x(f); \
}

GENERICff(cosf)
GENERICff(sinf)
GENERICff(tanf)
GENERICff(acosf)
GENERICff(asinf)
GENERICff(atanf)
GENERICff(coshf)
GENERICff(sinhf)
GENERICff(tanhf)
GENERICff(logf)
GENERICff(log10f)
GENERICff(sqrtf)
GENERICff(ceilf)
GENERICff(fabsf)
GENERICff(floorf)
GENERICff(truncf)
GENERICff(fractionf)

void powf_generic(asIScriptGeneric *gen)
{
	float f1 = *(float*)gen->GetAddressOfArg(0);
	float f2 = *(float*)gen->GetAddressOfArg(1);
	*(float*)gen->GetAddressOfReturnLocation() = powf(f1, f2);
}
void atan2f_generic(asIScriptGeneric *gen)
{
	float f1 = *(float*)gen->GetAddressOfArg(0);
	float f2 = *(float*)gen->GetAddressOfArg(1);
	*(float*)gen->GetAddressOfReturnLocation() = atan2f(f1, f2);
}
void fminf_generic(asIScriptGeneric *gen)
{
  float f1 = *(float*)gen->GetAddressOfArg(0);
  float f2 = *(float*)gen->GetAddressOfArg(1);
  *(float*)gen->GetAddressOfReturnLocation() = fminf(f1, f2);
}
void fmaxf_generic(asIScriptGeneric *gen)
{
  float f1 = *(float*)gen->GetAddressOfArg(0);
  float f2 = *(float*)gen->GetAddressOfArg(1);
  *(float*)gen->GetAddressOfReturnLocation() = fmaxf(f1, f2);
}

#else
// This macro creates simple generic wrappers for functions of type 'double func(double)'
#define GENERICdd(x) \
void x##_generic(asIScriptGeneric *gen) \
{ \
	double f = *(double*)gen->GetAddressOfArg(0); \
	*(double*)gen->GetAddressOfReturnLocation() = x(f); \
}

GENERICdd(cos)
GENERICdd(sin)
GENERICdd(tan)
GENERICdd(acos)
GENERICdd(asin)
GENERICdd(atan)
GENERICdd(cosh)
GENERICdd(sinh)
GENERICdd(tanh)
GENERICdd(log)
GENERICdd(log10)
GENERICdd(sqrt)
GENERICdd(ceil)
GENERICdd(fabs)
GENERICdd(floor)
GENERICff(trunc)
GENERICdd(fraction)

void pow_generic(asIScriptGeneric *gen)
{
	double f1 = *(double*)gen->GetAddressOfArg(0);
	double f2 = *(double*)gen->GetAddressOfArg(1);
	*(double*)gen->GetAddressOfReturnLocation() = pow(f1, f2);
}
void atan2_generic(asIScriptGeneric *gen)
{
	double f1 = *(double*)gen->GetAddressOfArg(0);
	double f2 = *(double*)gen->GetAddressOfArg(1);
	*(double*)gen->GetAddressOfReturnLocation() = atan2(f1, f2);
}
void minf_generic(asIScriptGeneric *gen)
{
  double f1 = *(double*)gen->GetAddressOfArg(0);
  double f2 = *(double*)gen->GetAddressOfArg(1);
  *(double*)gen->GetAddressOfReturnLocation() = minf(f1, f2);
}
void maxf_generic(asIScriptGeneric *gen)
{
  double f1 = *(double*)gen->GetAddressOfArg(0);
  double f2 = *(double*)gen->GetAddressOfArg(1);
  *(double*)gen->GetAddressOfReturnLocation() = maxf(f1, f2);
}
#endif
void RegisterScriptMath_Generic(asIScriptEngine *engine)
{
  RegisterCasting(engine);

	int r;

#if AS_USE_FLOAT
	// Trigonometric functions
	r = engine->RegisterGlobalFunction("float Cos(float)", asFUNCTION(cosf_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Sin(float)", asFUNCTION(sinf_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Tan(float)", asFUNCTION(tanf_generic), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterGlobalFunction("float ACos(float)", asFUNCTION(acosf_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float ASin(float)", asFUNCTION(asinf_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Atan(float)", asFUNCTION(atanf_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Atan2(float,float)", asFUNCTION(atan2f_generic), asCALL_GENERIC); assert( r >= 0 );

	// Hyberbolic functions
	r = engine->RegisterGlobalFunction("float Cosh(float)", asFUNCTION(coshf_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Sinh(float)", asFUNCTION(sinhf_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Tanh(float)", asFUNCTION(tanhf_generic), asCALL_GENERIC); assert( r >= 0 );

	// Exponential and logarithmic functions
	r = engine->RegisterGlobalFunction("float Log(float)", asFUNCTION(logf_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Log10(float)", asFUNCTION(log10f_generic), asCALL_GENERIC); assert( r >= 0 );

	// Power functions
	r = engine->RegisterGlobalFunction("float Pow(float, float)", asFUNCTION(powf_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Sqrt(float)", asFUNCTION(sqrtf_generic), asCALL_GENERIC); assert( r >= 0 );

	// Nearest integer, absolute value, and remainder functions
	r = engine->RegisterGlobalFunction("float Ceil(float)", asFUNCTION(ceilf_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Abs(float)", asFUNCTION(fabsf_generic), asCALL_GENERIC); assert( r >= 0 );
  r = engine->RegisterGlobalFunction("float Min(float, float)", asFUNCTION(fminf_generic), asCALL_GENERIC); assert(r >= 0);
  r = engine->RegisterGlobalFunction("float Max(float, float)", asFUNCTION(fmaxf_generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Floor(float)", asFUNCTION(floorf_generic), asCALL_GENERIC); assert( r >= 0 );
  r = engine->RegisterGlobalFunction("float Trunc(float)", asFUNCTION(truncf_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Fraction(float)", asFUNCTION(fractionf_generic), asCALL_GENERIC); assert( r >= 0 );

	// Don't register modf because AngelScript already supports the % operator
#else
	// double versions of the same
	r = engine->RegisterGlobalFunction("double Cos(double)", asFUNCTION(cos_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Sin(double)", asFUNCTION(sin_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Tan(double)", asFUNCTION(tan_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double ACos(double)", asFUNCTION(acos_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double ASin(double)", asFUNCTION(asin_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Atan(double)", asFUNCTION(atan_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Atan2(double,double)", asFUNCTION(atan2_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Cosh(double)", asFUNCTION(cosh_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Sinh(double)", asFUNCTION(sinh_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Tanh(double)", asFUNCTION(tanh_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Log(double)", asFUNCTION(log_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Log10(double)", asFUNCTION(log10_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Pow(double, double)", asFUNCTION(pow_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Sqrt(double)", asFUNCTION(sqrt_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Ceil(double)", asFUNCTION(ceil_generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("double Abs(double)", asFUNCTION(fabs_generic), asCALL_GENERIC); assert( r >= 0 );
  r = engine->RegisterGlobalFunction("double Min(double, double)", asFUNCTION(minf_generic), asCALL_GENERIC); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double Max(double, double)", asFUNCTION(maxf_generic), asCALL_GENERIC); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double Floor(double)", asFUNCTION(floor_generic), asCALL_GENERIC); assert(r >= 0);
  r = engine->RegisterGlobalFunction("double Trunc(double)", asFUNCTION(trunc_generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterGlobalFunction("double Fraction(double)", asFUNCTION(fraction_generic), asCALL_GENERIC); assert( r >= 0 );
#endif
}

void RegisterScriptMath(asIScriptEngine *engine)
{
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
		RegisterScriptMath_Generic(engine);
	else
		RegisterScriptMath_Native(engine);
}

END_AS_NAMESPACE


