#include <assert.h>
#include <string.h> // strstr
#include <new> // new()
#include <math.h>
#include "scriptmathComplex.h"

#ifdef __BORLANDC__
// C++Builder doesn't define a non-standard "sqrtf" function but rather an overload of "sqrt"
// for float arguments.
inline float sqrtf (float x) { return sqrt (x); }
#endif

BEGIN_AS_NAMESPACE

Complex::Complex()
{
	r = 0;
	i = 0;
}

Complex::Complex(const Complex &other)
{
	r = other.r;
	i = other.i;
}

Complex::Complex(float _r, float _i)
{
	r = _r;
	i = _i;
}

bool Complex::operator==(const Complex &o) const
{
	return (r == o.r) && (i == o.i);
}

bool Complex::operator!=(const Complex &o) const
{
	return !(*this == o);
}

Complex &Complex::operator=(const Complex &other)
{
	r = other.r;
	i = other.i;
	return *this;
}

Complex &Complex::operator+=(const Complex &other)
{
	r += other.r;
	i += other.i;
	return *this;
}

Complex &Complex::operator-=(const Complex &other)
{
	r -= other.r;
	i -= other.i;
	return *this;
}

Complex &Complex::operator*=(const Complex &other)
{
	*this = *this * other;
	return *this;
}

Complex &Complex::operator/=(const Complex &other)
{
	*this = *this / other;
	return *this;
}

float Complex::squaredLength() const
{
	return r*r + i*i;
}

float Complex::length() const
{
	return sqrtf(squaredLength());
}

Complex Complex::operator+(const Complex &other) const
{
	return Complex(r + other.r, i + other.i);
}

Complex Complex::operator-(const Complex &other) const
{
	return Complex(r - other.r, i + other.i);
}

Complex Complex::operator*(const Complex &other) const
{
	return Complex(r*other.r - i*other.i, r*other.i + i*other.r);
}

Complex Complex::operator/(const Complex &other) const
{
	float squaredLen = other.squaredLength();
	if( squaredLen == 0 ) return Complex(0,0);

	return Complex((r*other.r + i*other.i)/squaredLen, (i*other.r - r*other.i)/squaredLen);
}

//-----------------------
// Swizzle operators
//-----------------------

Complex Complex::get_ri() const
{
	return *this;
}
Complex Complex::get_ir() const
{
	return Complex(r,i);
}
void Complex::set_ri(const Complex &o)
{
	*this = o;
}
void Complex::set_ir(const Complex &o)
{
	r = o.i;
	i = o.r;
}

//-----------------------
// AngelScript functions
//-----------------------

static void ComplexDefaultConstructor(Complex *self)
{
	new(self) Complex();
}

static void ComplexCopyConstructor(const Complex &other, Complex *self)
{
	new(self) Complex(other);
}

static void ComplexConvConstructor(float r, Complex *self)
{
	new(self) Complex(r);
}

static void ComplexInitConstructor(float r, float i, Complex *self)
{
	new(self) Complex(r,i);
}

static void ComplexListConstructor(float *list, Complex *self)
{
	new(self) Complex(list[0], list[1]);
}

//--------------------------------
// Registration
//-------------------------------------

static void RegisterScriptMathComplex_Native(asIScriptEngine *engine)
{
	int r;

	// Register the type
#if AS_CAN_USE_CPP11
	// With C++11 it is possible to use asGetTypeTraits to determine the correct flags to represent the C++ class, except for the asOBJ_APP_CLASS_ALLFLOATS
	r = engine->RegisterObjectType("Complex", sizeof(Complex), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Complex>() | asOBJ_APP_CLASS_ALLFLOATS); assert( r >= 0 );
#else
	r = engine->RegisterObjectType("Complex", sizeof(Complex), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS); assert( r >= 0 );
#endif

	// Register the object properties
	r = engine->RegisterObjectProperty("Complex", "float r", asOFFSET(Complex, r)); assert( r >= 0 );
	r = engine->RegisterObjectProperty("Complex", "float i", asOFFSET(Complex, i)); assert( r >= 0 );

	// Register the constructors
	r = engine->RegisterObjectBehaviour("Complex", asBEHAVE_CONSTRUCT,      "void f()",                             asFUNCTION(ComplexDefaultConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Complex", asBEHAVE_CONSTRUCT,      "void f(const Complex &in)",            asFUNCTION(ComplexCopyConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Complex", asBEHAVE_CONSTRUCT,      "void f(float)",                        asFUNCTION(ComplexConvConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Complex", asBEHAVE_CONSTRUCT,      "void f(float, float)",                 asFUNCTION(ComplexInitConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("Complex", asBEHAVE_LIST_CONSTRUCT, "void f(const int &in) {float, float}", asFUNCTION(ComplexListConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Register the operator overloads
	r = engine->RegisterObjectMethod("Complex", "Complex &opAddAssign(const Complex &in)", asMETHODPR(Complex, operator+=, (const Complex &), Complex&), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Complex", "Complex &opSubAssign(const Complex &in)", asMETHODPR(Complex, operator-=, (const Complex &), Complex&), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Complex", "Complex &opMulAssign(const Complex &in)", asMETHODPR(Complex, operator*=, (const Complex &), Complex&), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Complex", "Complex &opDivAssign(const Complex &in)", asMETHODPR(Complex, operator/=, (const Complex &), Complex&), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Complex", "bool opEquals(const Complex &in) const", asMETHODPR(Complex, operator==, (const Complex &) const, bool), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Complex", "Complex opAdd(const Complex &in) const", asMETHODPR(Complex, operator+, (const Complex &) const, Complex), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Complex", "Complex opSub(const Complex &in) const", asMETHODPR(Complex, operator-, (const Complex &) const, Complex), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Complex", "Complex opMul(const Complex &in) const", asMETHODPR(Complex, operator*, (const Complex &) const, Complex), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Complex", "Complex opDiv(const Complex &in) const", asMETHODPR(Complex, operator/, (const Complex &) const, Complex), asCALL_THISCALL); assert( r >= 0 );

	// Register the object methods
	r = engine->RegisterObjectMethod("Complex", "float Abs() const", asMETHOD(Complex,length), asCALL_THISCALL); assert( r >= 0 );

	// Register the swizzle operators
	r = engine->RegisterObjectMethod("Complex", "Complex get_ri() const", asMETHOD(Complex, get_ri), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Complex", "Complex get_ir() const", asMETHOD(Complex, get_ir), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Complex", "void set_ri(const Complex &in)", asMETHOD(Complex, set_ri), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("Complex", "void set_ir(const Complex &in)", asMETHOD(Complex, set_ir), asCALL_THISCALL); assert( r >= 0 );
}

void RegisterScriptMathComplex(asIScriptEngine *engine)
{
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		assert( false );
		// TODO: implement support for generic calling convention
		// RegisterScriptMathComplex_Generic(engine);
	}
	else
		RegisterScriptMathComplex_Native(engine);
}

END_AS_NAMESPACE


