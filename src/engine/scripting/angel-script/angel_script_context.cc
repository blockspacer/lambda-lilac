#include "angel_script_context.h"
#include <angelscript.h>
#include "addons/scriptarray.h"
#include "addons/scriptbuilder.h"
#include "addons/scriptdictionary.h"
#include "addons/scriptlmbstring.h"
#include "addons/scriptmath.h"
#include "addons/debugger.h"
#include "utils/angle.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <utils/console.h>
#include <utils/file_system.h>
#include "angel_script_entity.h"
#include "systems/entity_system.h"
#include "scripting/script_vector.h"
#include "scripting/script_noise.h"
#include "interfaces/iworld.h"
#include "platform/scene.h"

namespace lambda
{
  namespace scripting
  {
    static entity::EntitySystem* k_entity_system = nullptr;
    static AngelScriptComponentManager* k_component_manager = nullptr;

    void entityd1(AngelScriptEntity* entity) { entity->release(); }
    void entityc1(AngelScriptEntity* mem) { new(mem) AngelScriptEntity(); }
    void entityc2(const AngelScriptEntity& c, AngelScriptEntity* mem) { new(mem) AngelScriptEntity(c); }
    void RegisterScriptEntity(asIScriptEngine* engine)
    {
      k_component_manager = foundation::Memory::construct<AngelScriptComponentManager>();
      k_component_manager->setEntitySystem(k_entity_system);
      AngelScriptEntity::setComponentManager(k_component_manager);

      int r;

      r = engine->RegisterObjectType("Entity", sizeof(AngelScriptEntity), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(entityc1, (AngelScriptEntity*), void), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_CONSTRUCT, "void f(const Entity &in)", asFUNCTION(entityc2), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_CONSTRUCT, "void f(Entity)", asFUNCTION(entityc2), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_DESTRUCT,  "void f()", asFUNCTION(entityd1), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectMethod("Entity", "void opAssign(const Entity &in)", asMETHODPR(AngelScriptEntity, operator=, (const AngelScriptEntity&), void), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Entity", "void AddComponent(?&out)", asMETHOD(AngelScriptEntity, addComponent), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Entity", "void GetComponent(?&out)", asMETHOD(AngelScriptEntity, getComponent), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Entity", "void RemoveComponent(?&out)", asMETHOD(AngelScriptEntity, removeComponent), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Entity", "uint64 GetId() const", asMETHOD(AngelScriptEntity, getId), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Entity", "void Construct()", asMETHOD(AngelScriptEntity, construct), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Entity", "void Create()", asMETHOD(AngelScriptEntity, construct), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Entity", "void Make()", asMETHOD(AngelScriptEntity, construct), asCALL_THISCALL); assert(r >= 0);
    }
    void vec2c1(ScriptVec2* mem) { new(mem) ScriptVec2(); };
    void vec2c2(const ScriptVec2& c, ScriptVec2* mem) { new(mem) ScriptVec2(c); };
    void vec2c3(const float& v, ScriptVec2* mem) { new(mem) ScriptVec2(v); };
    void vec2c4(const float& x, const float& y, ScriptVec2* mem) { new(mem) ScriptVec2(x, y); };
    void RegisterScriptVec2(asIScriptEngine* engine)
    {
      int r;

      r = engine->RegisterObjectType("Vec2", sizeof(ScriptVec2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA); assert(r >= 0);
      // Constructors.
      r = engine->RegisterObjectBehaviour("Vec2", asBEHAVE_CONSTRUCT, "void f()",                                 asFUNCTION(vec2c1), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Vec2", asBEHAVE_CONSTRUCT, "void f(const Vec2 &in)",                   asFUNCTION(vec2c2), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Vec2", asBEHAVE_CONSTRUCT, "void f(const float &in)",                  asFUNCTION(vec2c3), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Vec2", asBEHAVE_CONSTRUCT, "void f(const float &in, const float &in)", asFUNCTION(vec2c4), asCALL_CDECL_OBJLAST); assert(r >= 0);
      // Op add Vec2.
      r = engine->RegisterObjectMethod("Vec2", "Vec2 opAdd(const Vec2 &in) const",    asMETHODPR(ScriptVec2, operator+,  (const ScriptVec2&) const, ScriptVec2), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2 opSub(const Vec2 &in) const",    asMETHODPR(ScriptVec2, operator-,  (const ScriptVec2&) const, ScriptVec2), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2 opMul(const Vec2 &in) const",    asMETHODPR(ScriptVec2, operator*,  (const ScriptVec2&) const, ScriptVec2), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2 opDiv(const Vec2 &in) const",    asMETHODPR(ScriptVec2, operator/,  (const ScriptVec2&) const, ScriptVec2), asCALL_THISCALL); assert(r >= 0);
      // Op add float.
      r = engine->RegisterObjectMethod("Vec2", "Vec2 opAdd(const float &in) const",   asMETHODPR(ScriptVec2, operator+,  (const float&) const, ScriptVec2),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2 opSub(const float &in) const",   asMETHODPR(ScriptVec2, operator-,  (const float&) const, ScriptVec2),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2 opMul(const float &in) const",   asMETHODPR(ScriptVec2, operator*,  (const float&) const, ScriptVec2),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2 opDiv(const float &in) const",   asMETHODPR(ScriptVec2, operator/,  (const float&) const, ScriptVec2),      asCALL_THISCALL); assert(r >= 0);
      // Op add assign Vec2.
      r = engine->RegisterObjectMethod("Vec2", "Vec2& opAddAssign(const Vec2 &in)",   asMETHODPR(ScriptVec2, operator+=, (const ScriptVec2&), ScriptVec2&),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2& opSubAssign(const Vec2 &in)",   asMETHODPR(ScriptVec2, operator-=, (const ScriptVec2&), ScriptVec2&),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2& opMulAssign(const Vec2 &in)",   asMETHODPR(ScriptVec2, operator*=, (const ScriptVec2&), ScriptVec2&),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2& opDivAssign(const Vec2 &in)",   asMETHODPR(ScriptVec2, operator/=, (const ScriptVec2&), ScriptVec2&),      asCALL_THISCALL); assert(r >= 0);
      // Op add assign float.
      r = engine->RegisterObjectMethod("Vec2", "Vec2& opAddAssign(const float &in)",  asMETHODPR(ScriptVec2, operator+=, (const float&), ScriptVec2&),           asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2& opSubAssign(const float &in)",  asMETHODPR(ScriptVec2, operator-=, (const float&), ScriptVec2&),           asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2& opMulAssign(const float &in)",  asMETHODPR(ScriptVec2, operator*=, (const float&), ScriptVec2&),           asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2& opDivAssign(const float &in)",  asMETHODPR(ScriptVec2, operator/=, (const float&), ScriptVec2&),           asCALL_THISCALL); assert(r >= 0);
      // Op other.
      r = engine->RegisterObjectMethod("Vec2", "bool opEquals(const Vec2 &in) const", asMETHODPR(ScriptVec2, operator==, (const ScriptVec2&) const, bool),       asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "int opCmp(const Vec2 &in) const",     asMETHODPR(ScriptVec2, opCmp,      (const ScriptVec2&) const, int),        asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "void opAssign(const Vec2 &in)",       asMETHODPR(ScriptVec2, operator=,  (const ScriptVec2&), void),             asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2 opNeg() const",                  asMETHODPR(ScriptVec2, operator-,  () const, ScriptVec2),                  asCALL_THISCALL); assert(r >= 0);
      // Functions.
      r = engine->RegisterObjectMethod("Vec2", "float LengthSqr() const",        asMETHODPR(ScriptVec2, LengthSquared, () const, float),                  asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "float LengthSquared() const",    asMETHODPR(ScriptVec2, LengthSquared, () const, float),                  asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "Vec2 Normalized() const",        asMETHODPR(ScriptVec2, Normalized,    () const, ScriptVec2),             asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "float Length() const",           asMETHODPR(ScriptVec2, Length,        () const, float),                  asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "void Normalize()",               asMETHODPR(ScriptVec2, Normalize,     (), void),                         asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "float Dot(const Vec2&in) const", asMETHODPR(ScriptVec2, Dot,           (const ScriptVec2&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "String opConv() const",          asMETHODPR(ScriptVec2, ToString,      () const, String),                 asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "String opImplConv() const",      asMETHODPR(ScriptVec2, ToString,      () const, String),                 asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec2", "String ToString() const",        asMETHODPR(ScriptVec2, ToString,      () const, String),                 asCALL_THISCALL); assert(r >= 0);
      // Properties.
      r = engine->RegisterObjectProperty("Vec2", "float x", offsetof(ScriptVec2, x)); assert(r >= 0);
      r = engine->RegisterObjectProperty("Vec2", "float y", offsetof(ScriptVec2, y)); assert(r >= 0);
      engine->RegisterTypedef("Vec2", "Vector2");
    }
    void vec3c1(ScriptVec3* mem) { new(mem) ScriptVec3(); };
    void vec3c2(const ScriptVec3& c, ScriptVec3* mem) { new(mem) ScriptVec3(c); };
    void vec3c3(const float& v, ScriptVec3* mem) { new(mem) ScriptVec3(v); };
    void vec3c4(const float& x, const float& y, const float& z, ScriptVec3* mem) { new(mem) ScriptVec3(x, y, z); };
    void RegisterScriptVec3(asIScriptEngine* engine)
    {
      int r;

      r = engine->RegisterObjectType("Vec3", sizeof(ScriptVec3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA); assert(r >= 0);
      // Constructors.
      r = engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f()",                                                  asFUNCTION(vec3c1), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f(const Vec3 &in)",                                    asFUNCTION(vec3c2), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f(const float &in)",                                   asFUNCTION(vec3c3), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f(const float &in, const float &in, const float &in)", asFUNCTION(vec3c4), asCALL_CDECL_OBJLAST); assert(r >= 0);
      // Op add Vec3.
      r = engine->RegisterObjectMethod("Vec3", "Vec3 opAdd(const Vec3 &in) const",    asMETHODPR(ScriptVec3, operator+,  (const ScriptVec3&) const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3 opSub(const Vec3 &in) const",    asMETHODPR(ScriptVec3, operator-,  (const ScriptVec3&) const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3 opMul(const Vec3 &in) const",    asMETHODPR(ScriptVec3, operator*,  (const ScriptVec3&) const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3 opDiv(const Vec3 &in) const",    asMETHODPR(ScriptVec3, operator/,  (const ScriptVec3&) const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
      // Op add float.
      r = engine->RegisterObjectMethod("Vec3", "Vec3 opAdd(const float &in) const",   asMETHODPR(ScriptVec3, operator+,  (const float&) const, ScriptVec3),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3 opSub(const float &in) const",   asMETHODPR(ScriptVec3, operator-,  (const float&) const, ScriptVec3),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3 opMul(const float &in) const",   asMETHODPR(ScriptVec3, operator*,  (const float&) const, ScriptVec3),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3 opDiv(const float &in) const",   asMETHODPR(ScriptVec3, operator/,  (const float&) const, ScriptVec3),      asCALL_THISCALL); assert(r >= 0);
      // Op add assign Vec3.
      r = engine->RegisterObjectMethod("Vec3", "Vec3& opAddAssign(const Vec3 &in)",   asMETHODPR(ScriptVec3, operator+=, (const ScriptVec3&), ScriptVec3&),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3& opSubAssign(const Vec3 &in)",   asMETHODPR(ScriptVec3, operator-=, (const ScriptVec3&), ScriptVec3&),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3& opMulAssign(const Vec3 &in)",   asMETHODPR(ScriptVec3, operator*=, (const ScriptVec3&), ScriptVec3&),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3& opDivAssign(const Vec3 &in)",   asMETHODPR(ScriptVec3, operator/=, (const ScriptVec3&), ScriptVec3&),      asCALL_THISCALL); assert(r >= 0);
      // Op add assign float.
      r = engine->RegisterObjectMethod("Vec3", "Vec3& opAddAssign(const float &in)",  asMETHODPR(ScriptVec3, operator+=, (const float&), ScriptVec3&),           asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3& opSubAssign(const float &in)",  asMETHODPR(ScriptVec3, operator-=, (const float&), ScriptVec3&),           asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3& opMulAssign(const float &in)",  asMETHODPR(ScriptVec3, operator*=, (const float&), ScriptVec3&),           asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3& opDivAssign(const float &in)",  asMETHODPR(ScriptVec3, operator/=, (const float&), ScriptVec3&),           asCALL_THISCALL); assert(r >= 0);
      // Op other.
      r = engine->RegisterObjectMethod("Vec3", "bool opEquals(const Vec3 &in) const", asMETHODPR(ScriptVec3, operator==, (const ScriptVec3&) const, bool),       asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "int opCmp(const Vec3 &in) const",     asMETHODPR(ScriptVec3, opCmp,      (const ScriptVec3&) const, int),        asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "void opAssign(const Vec3 &in)",       asMETHODPR(ScriptVec3, operator=,  (const ScriptVec3&), void),             asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3 opNeg() const",                  asMETHODPR(ScriptVec3, operator-,  () const, ScriptVec3),                  asCALL_THISCALL); assert(r >= 0);
      // Functions.
      r = engine->RegisterObjectMethod("Vec3", "float LengthSqr() const",         asMETHODPR(ScriptVec3, LengthSquared, () const, float),                       asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "float LengthSquared() const",     asMETHODPR(ScriptVec3, LengthSquared, () const, float),                       asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3 Normalized() const",         asMETHODPR(ScriptVec3, Normalized,    () const, ScriptVec3),                  asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "float Length() const",            asMETHODPR(ScriptVec3, Length,        () const, float),                       asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "void Normalize()",                asMETHODPR(ScriptVec3, Normalize,     (), void),                              asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "float Dot(const Vec3&in) const",  asMETHODPR(ScriptVec3, Dot,           (const ScriptVec3&) const, float),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "Vec3 Cross(const Vec3&in) const", asMETHODPR(ScriptVec3, Cross,         (const ScriptVec3&) const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "String opConv() const",           asMETHODPR(ScriptVec3, ToString,      () const, String),                      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "String opImplConv() const",       asMETHODPR(ScriptVec3, ToString,      () const, String),                      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec3", "String ToString() const",         asMETHODPR(ScriptVec3, ToString,      () const, String),                      asCALL_THISCALL); assert(r >= 0);
      // Properties.
      r = engine->RegisterObjectProperty("Vec3", "float x", asOFFSET(ScriptVec3, x)); assert(r >= 0);
      r = engine->RegisterObjectProperty("Vec3", "float y", asOFFSET(ScriptVec3, y)); assert(r >= 0);
      r = engine->RegisterObjectProperty("Vec3", "float z", asOFFSET(ScriptVec3, z)); assert(r >= 0);
      engine->RegisterTypedef("Vec3", "Vector3");
    }
    void vec4c1(ScriptVec4* mem) { new(mem) ScriptVec4(); };
    void vec4c2(const ScriptVec4& c, ScriptVec4* mem) { new(mem) ScriptVec4(c); };
    void vec4c3(const float& v, ScriptVec4* mem) { new(mem) ScriptVec4(v); };
    void vec4c4(const float& x, const float& y, const float& z, const float& w, ScriptVec4* mem) { new(mem) ScriptVec4(x, y, z, w); };
    void RegisterScriptVec4(asIScriptEngine* engine)
    {
      int r;

      r = engine->RegisterObjectType("Vec4", sizeof(ScriptVec4), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA); assert(r >= 0);
      // Constructors.
      r = engine->RegisterObjectBehaviour("Vec4", asBEHAVE_CONSTRUCT, "void f()",                                                                   asFUNCTION(vec4c1), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Vec4", asBEHAVE_CONSTRUCT, "void f(const Vec4 &in)",                                                     asFUNCTION(vec4c2), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Vec4", asBEHAVE_CONSTRUCT, "void f(const float &in)",                                                    asFUNCTION(vec4c3), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Vec4", asBEHAVE_CONSTRUCT, "void f(const float &in, const float &in, const float &in, const float &in)", asFUNCTION(vec4c4), asCALL_CDECL_OBJLAST); assert(r >= 0);
      // Op add Vec4.
      r = engine->RegisterObjectMethod("Vec4", "Vec4 opAdd(const Vec4 &in) const",    asMETHODPR(ScriptVec4, operator+,  (const ScriptVec4&) const, ScriptVec4), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4 opSub(const Vec4 &in) const",    asMETHODPR(ScriptVec4, operator-,  (const ScriptVec4&) const, ScriptVec4), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4 opMul(const Vec4 &in) const",    asMETHODPR(ScriptVec4, operator*,  (const ScriptVec4&) const, ScriptVec4), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4 opDiv(const Vec4 &in) const",    asMETHODPR(ScriptVec4, operator/,  (const ScriptVec4&) const, ScriptVec4), asCALL_THISCALL); assert(r >= 0);
      // Op add float.
      r = engine->RegisterObjectMethod("Vec4", "Vec4 opAdd(const float &in) const",   asMETHODPR(ScriptVec4, operator+,  (const float&) const, ScriptVec4),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4 opSub(const float &in) const",   asMETHODPR(ScriptVec4, operator-,  (const float&) const, ScriptVec4),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4 opMul(const float &in) const",   asMETHODPR(ScriptVec4, operator*,  (const float&) const, ScriptVec4),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4 opDiv(const float &in) const",   asMETHODPR(ScriptVec4, operator/,  (const float&) const, ScriptVec4),      asCALL_THISCALL); assert(r >= 0);
      // Op add assign Vec4.
      r = engine->RegisterObjectMethod("Vec4", "Vec4& opAddAssign(const Vec4 &in)",   asMETHODPR(ScriptVec4, operator+=, (const ScriptVec4&), ScriptVec4&),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4& opSubAssign(const Vec4 &in)",   asMETHODPR(ScriptVec4, operator-=, (const ScriptVec4&), ScriptVec4&),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4& opMulAssign(const Vec4 &in)",   asMETHODPR(ScriptVec4, operator*=, (const ScriptVec4&), ScriptVec4&),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4& opDivAssign(const Vec4 &in)",   asMETHODPR(ScriptVec4, operator/=, (const ScriptVec4&), ScriptVec4&),      asCALL_THISCALL); assert(r >= 0);
      // Op add assign float.
      r = engine->RegisterObjectMethod("Vec4", "Vec4& opAddAssign(const float &in)",  asMETHODPR(ScriptVec4, operator+=, (const float&), ScriptVec4&),           asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4& opSubAssign(const float &in)",  asMETHODPR(ScriptVec4, operator-=, (const float&), ScriptVec4&),           asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4& opMulAssign(const float &in)",  asMETHODPR(ScriptVec4, operator*=, (const float&), ScriptVec4&),           asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4& opDivAssign(const float &in)",  asMETHODPR(ScriptVec4, operator/=, (const float&), ScriptVec4&),           asCALL_THISCALL); assert(r >= 0);
      // Op other.
      r = engine->RegisterObjectMethod("Vec4", "bool opEquals(const Vec4 &in) const", asMETHODPR(ScriptVec4, operator==, (const ScriptVec4&) const, bool),       asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "int opCmp(const Vec4 &in) const",     asMETHODPR(ScriptVec4, opCmp,      (const ScriptVec4&) const, int),        asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "void opAssign(const Vec4 &in)",       asMETHODPR(ScriptVec4, operator=,  (const ScriptVec4&), void),             asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4 opNeg() const",                  asMETHODPR(ScriptVec4, operator-,  () const, ScriptVec4),                  asCALL_THISCALL); assert(r >= 0);
      // Functions.
      r = engine->RegisterObjectMethod("Vec4", "float LengthSqr() const",         asMETHODPR(ScriptVec4, LengthSquared, () const, float),                       asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "float LengthSquared() const",     asMETHODPR(ScriptVec4, LengthSquared, () const, float),                       asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec4 Normalized() const",         asMETHODPR(ScriptVec4, Normalized,    () const, ScriptVec4),                  asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "float Length() const",            asMETHODPR(ScriptVec4, Length,        () const, float),                       asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "void Normalize()",                asMETHODPR(ScriptVec4, Normalize,     (), void),                              asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "float Dot(const Vec4&in) const",  asMETHODPR(ScriptVec4, Dot,           (const ScriptVec4&) const, float),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "Vec3 Cross(const Vec4&in) const", asMETHODPR(ScriptVec4, Cross,         (const ScriptVec4&) const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "String opConv() const",           asMETHODPR(ScriptVec4, ToString,      () const, String),                      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "String opImplConv() const",       asMETHODPR(ScriptVec4, ToString,      () const, String),                      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Vec4", "String ToString() const",         asMETHODPR(ScriptVec4, ToString,      () const, String),                      asCALL_THISCALL); assert(r >= 0);
      // Properties.
      r = engine->RegisterObjectProperty("Vec4", "float x", asOFFSET(ScriptVec4, x)); assert(r >= 0);
      r = engine->RegisterObjectProperty("Vec4", "float y", asOFFSET(ScriptVec4, y)); assert(r >= 0);
      r = engine->RegisterObjectProperty("Vec4", "float z", asOFFSET(ScriptVec4, z)); assert(r >= 0);
      r = engine->RegisterObjectProperty("Vec4", "float w", asOFFSET(ScriptVec4, w)); assert(r >= 0);
      engine->RegisterTypedef("Vec4", "Vector4");
    }
    void quatc1(ScriptQuat* mem) { new(mem) ScriptQuat(); };
    void quatc2(const ScriptQuat& c, ScriptQuat* mem) { new(mem) ScriptQuat(c); };
    void quatc3(const float& x, const float& y, const float& z, ScriptQuat* mem) { new(mem) ScriptQuat(x, y, z); };
    void quatc4(const float& w, const float& x, const float& y, const float& z, ScriptQuat* mem) { new(mem) ScriptQuat(x, y, z, w); };
    void quatc5(const ScriptVec3& euler, ScriptQuat* mem) { new(mem) ScriptQuat(euler); };
    void RegisterScriptQuat(asIScriptEngine* engine)
    {
      int r;

      r = engine->RegisterObjectType("Quat", sizeof(ScriptQuat), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA); assert(r >= 0);
      // Constructors.
      r = engine->RegisterObjectBehaviour("Quat", asBEHAVE_CONSTRUCT, "void f()",                                                                   asFUNCTION(quatc1), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Quat", asBEHAVE_CONSTRUCT, "void f(const Quat &in)",                                                     asFUNCTION(quatc2), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Quat", asBEHAVE_CONSTRUCT, "void f(const float &in, const float &in, const float &in)",                  asFUNCTION(quatc3), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Quat", asBEHAVE_CONSTRUCT, "void f(const float &in, const float &in, const float &in, const float &in)", asFUNCTION(quatc4), asCALL_CDECL_OBJLAST); assert(r >= 0);
      r = engine->RegisterObjectBehaviour("Quat", asBEHAVE_CONSTRUCT, "void f(const Vec3 &in)",                                                     asFUNCTION(quatc5), asCALL_CDECL_OBJLAST); assert(r >= 0);
      // Op add Quat.
      r = engine->RegisterObjectMethod("Quat", "Quat opAdd(const Quat &in) const",    asMETHODPR(ScriptQuat, operator+,  (const ScriptQuat&) const, ScriptQuat), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "Quat opSub(const Quat &in) const",    asMETHODPR(ScriptQuat, operator-,  (const ScriptQuat&) const, ScriptQuat), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "Quat opMul(const Quat &in) const",    asMETHODPR(ScriptQuat, operator*,  (const ScriptQuat&) const, ScriptQuat), asCALL_THISCALL); assert(r >= 0);
      // Op add assign Quat.
      r = engine->RegisterObjectMethod("Quat", "Quat& opAddAssign(const Quat &in)",   asMETHODPR(ScriptQuat, operator+=, (const ScriptQuat&), ScriptQuat&),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "Quat& opSubAssign(const Quat &in)",   asMETHODPR(ScriptQuat, operator-=, (const ScriptQuat&), ScriptQuat&),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "Quat& opMulAssign(const Quat &in)",   asMETHODPR(ScriptQuat, operator*=, (const ScriptQuat&), ScriptQuat&),      asCALL_THISCALL); assert(r >= 0);
      // Op other.
      r = engine->RegisterObjectMethod("Quat", "bool opEquals(const Quat &in) const", asMETHODPR(ScriptQuat, operator==, (const ScriptQuat&) const, bool),       asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "int opCmp(const Quat &in) const",     asMETHODPR(ScriptQuat, opCmp,      (const ScriptQuat&) const, int),        asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "void opAssign(const Quat &in)",       asMETHODPR(ScriptQuat, operator=,  (const ScriptQuat&), void),             asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "Quat opNeg() const",                  asMETHODPR(ScriptQuat, operator-,  () const, ScriptQuat),                  asCALL_THISCALL); assert(r >= 0);
      // Functions.
      r = engine->RegisterObjectMethod("Quat", "float LengthSqr() const",         asMETHODPR(ScriptQuat, LengthSquared, () const, float),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "float LengthSquared() const",     asMETHODPR(ScriptQuat, LengthSquared, () const, float),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "Quat Normalized() const",         asMETHODPR(ScriptQuat, Normalized,    () const, ScriptQuat), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "float Length() const",            asMETHODPR(ScriptQuat, Length,        () const, float),      asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "void Normalize()",                asMETHODPR(ScriptQuat, Normalize,     (), void),             asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "Vec3 ToEuler() const",            asMETHODPR(ScriptQuat, ToEuler,       () const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "Vec3 AsEuler() const",            asMETHODPR(ScriptQuat, ToEuler,       () const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "Vec3 EulerAngles() const",        asMETHODPR(ScriptQuat, ToEuler,       () const, ScriptVec3), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "String opConv() const",           asMETHODPR(ScriptQuat, ToString,      () const, String),     asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "String opImplConv() const",       asMETHODPR(ScriptQuat, ToString,      () const, String),     asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Quat", "String ToString() const",         asMETHODPR(ScriptQuat, ToString,      () const, String),     asCALL_THISCALL); assert(r >= 0);
      // Properties.
      r = engine->RegisterObjectProperty("Quat", "float x", asOFFSET(ScriptQuat, x)); assert(r >= 0);
      r = engine->RegisterObjectProperty("Quat", "float y", asOFFSET(ScriptQuat, y)); assert(r >= 0);
      r = engine->RegisterObjectProperty("Quat", "float z", asOFFSET(ScriptQuat, z)); assert(r >= 0);
      r = engine->RegisterObjectProperty("Quat", "float w", asOFFSET(ScriptQuat, w)); assert(r >= 0);
      engine->RegisterTypedef("Quat", "Quaternion");
    }
    void RegisterScriptNoise(asIScriptEngine* engine)
    {
      int r;

      r = engine->RegisterObjectType("Noise", sizeof(ScriptNoise), asOBJ_VALUE | asOBJ_POD); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "void SetSeed(const int&in)",          asMETHOD(ScriptNoise, SetSeed),          asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "int GetSeed() const",                 asMETHOD(ScriptNoise, GetSeed),          asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "void SetFrequency(const float&in)",   asMETHOD(ScriptNoise, SetFrequency),     asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetFrequency() const",          asMETHOD(ScriptNoise, GetFrequency),     asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "void SetInterpolation(const int&in)", asMETHOD(ScriptNoise, SetInterpolation), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "int GetInterpolation() const",        asMETHOD(ScriptNoise, GetInterpolation), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "void SetInterp(const int&in)",        asMETHOD(ScriptNoise, SetInterpolation), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "int GetInterp() const",               asMETHOD(ScriptNoise, GetInterpolation), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetPerlin(const Vec2&in) const",         asMETHODPR(ScriptNoise, GetPerlin,         (const ScriptVec2&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetPerlin(const Vec3&in) const",         asMETHODPR(ScriptNoise, GetPerlin,         (const ScriptVec3&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetPerlinFractal(const Vec2&in) const",  asMETHODPR(ScriptNoise, GetPerlinFractal,  (const ScriptVec2&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetPerlinFractal(const Vec3&in) const",  asMETHODPR(ScriptNoise, GetPerlinFractal,  (const ScriptVec3&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetCellular(const Vec2&in) const",       asMETHODPR(ScriptNoise, GetCellular,       (const ScriptVec2&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetCellular(const Vec3&in) const",       asMETHODPR(ScriptNoise, GetCellular,       (const ScriptVec3&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetCubic(const Vec2&in) const",          asMETHODPR(ScriptNoise, GetCubic,          (const ScriptVec2&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetCubic(const Vec3&in) const",          asMETHODPR(ScriptNoise, GetCubic,          (const ScriptVec3&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetCubicFractal(const Vec2&in) const",   asMETHODPR(ScriptNoise, GetCubicFractal,   (const ScriptVec2&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetCubicFractal(const Vec3&in) const",   asMETHODPR(ScriptNoise, GetCubicFractal,   (const ScriptVec3&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetSimplex(const Vec2&in) const",        asMETHODPR(ScriptNoise, GetSimplex,        (const ScriptVec2&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetSimplex(const Vec3&in) const",        asMETHODPR(ScriptNoise, GetSimplex,        (const ScriptVec3&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetSimplexFractal(const Vec2&in) const", asMETHODPR(ScriptNoise, GetSimplexFractal, (const ScriptVec2&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetSimplexFractal(const Vec3&in) const", asMETHODPR(ScriptNoise, GetSimplexFractal, (const ScriptVec3&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetValue(const Vec2&in) const",          asMETHODPR(ScriptNoise, GetValue,          (const ScriptVec2&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetValue(const Vec3&in) const",          asMETHODPR(ScriptNoise, GetValue,          (const ScriptVec3&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetValueFractal(const Vec2&in) const",   asMETHODPR(ScriptNoise, GetValueFractal,   (const ScriptVec2&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetValueFractal(const Vec3&in) const",   asMETHODPR(ScriptNoise, GetValueFractal,   (const ScriptVec3&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetWhiteNoise(const Vec2&in) const",     asMETHODPR(ScriptNoise, GetWhiteNoise,     (const ScriptVec2&) const, float), asCALL_THISCALL); assert(r >= 0);
      r = engine->RegisterObjectMethod("Noise", "float GetWhiteNoise(const Vec3&in) const",     asMETHODPR(ScriptNoise, GetWhiteNoise,     (const ScriptVec3&) const, float), asCALL_THISCALL); assert(r >= 0);
    }

    static int kStringTypeId = -1;
    static int kVec2TypeId   = -1;
    static int kVec3TypeId   = -1;
    static int kVec4TypeId   = -1;
    ScriptValue::Type asTypeToScriptType(int type)
    {
      switch (type)
      {
      case 0:
      default:
        if (type == kStringTypeId)
        {
          return ScriptValue::Type::kString;
        }
        else if (type == kVec2TypeId)
        {
          return ScriptValue::Type::kVec2;
        }
        else if (type == kVec3TypeId)
        {
          return ScriptValue::Type::kVec3;
        }
        else if (type == kVec4TypeId)
        {
          return ScriptValue::Type::kVec4;
        }
        return ScriptValue::Type::kNull;
        break;
      case 1:
        return ScriptValue::Type::kBoolean;
        break;
      case 2:
        return ScriptValue::Type::kInt8;
        break;
      case 3:
        return ScriptValue::Type::kInt16;
        break;
      case 4:
        return ScriptValue::Type::kInt32;
        break;
      case 5:
        return ScriptValue::Type::kInt64;
        break;
      case 6:
        return ScriptValue::Type::kUint8;
        break;
      case 7:
        return ScriptValue::Type::kUint16;
        break;
      case 8:
        return ScriptValue::Type::kUint32;
        break;
      case 9:
        return ScriptValue::Type::kUint64;
        break;
      case 10:
        return ScriptValue::Type::kFloat;
        break;
      case 11:
        return ScriptValue::Type::kDouble;
        break;
      }
    }

    void MessageCallback(const asSMessageInfo *msg, void *param)
    {
      String str(String(msg->section) + "(" + toString(msg->row) + ", " + toString(msg->col) + " : " + msg->message + "\n");
      if (msg->type == asMSGTYPE_WARNING)
      {
        foundation::Warning(str);
      }
      else if (msg->type == asMSGTYPE_INFORMATION)
      {
        foundation::Info(str);
      }
      else
      {
        foundation::Error(str);
      }
    }

    Vector<String> split(String phrase, String delimiter)
    {
      Vector<String> list;
      String s = phrase;
      size_t pos = 0;
      String token;
      while ((pos = s.find(delimiter)) != String::npos)
      {
        token = s.substr(0, pos);
        list.push_back(token);
        s.erase(0, pos + delimiter.length());
      }
      list.push_back(s);
      return list;
    }

    void* asAlloc(size_t size)
    {
      return foundation::Memory::allocate(size);
    }
    void asFree(void* data)
    {
      foundation::Memory::deallocate(data);
    }

    AngelScriptContext::~AngelScriptContext()
    {
    }

    bool AngelScriptContext::initialize(const Map<String, void*>& functions)
    {
      asSetGlobalMemoryFunctions(asAlloc, asFree);
      debugger_ = nullptr;
      engine_ = asCreateScriptEngine();
      int ret = engine_->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL); assert(ret >= 0);

      RegisterScriptArray(engine_, true);
      RegisterLmbString(engine_);
      RegisterScriptMath(engine_);
      RegisterScriptDictionary(engine_);
      RegisterScriptEntity(engine_);
      RegisterScriptVec2(engine_);
      RegisterScriptVec3(engine_);
      RegisterScriptVec4(engine_);
      RegisterScriptQuat(engine_);
      RegisterScriptNoise(engine_);
      
      kStringTypeId = engine_->GetTypeInfoByName("String")->GetTypeId();
      kVec2TypeId   = engine_->GetTypeInfoByName("Vec2")->GetTypeId();
      kVec3TypeId   = engine_->GetTypeInfoByName("Vec3")->GetTypeId();
      kVec4TypeId   = engine_->GetTypeInfoByName("Vec4")->GetTypeId();

      
      static constexpr float pi = 3.14159265f;
      static constexpr float tau = 6.28318531f;
      static constexpr float degToRad = tau / 360.0f;
      static constexpr float radToDeg = 360.0f / tau;

      ret = engine_->RegisterGlobalProperty("const float Deg2Rad",  (void*)&degToRad); assert(ret >= 0);
      ret = engine_->RegisterGlobalProperty("const float Rad2Deg",  (void*)&radToDeg); assert(ret >= 0);
      ret = engine_->RegisterGlobalProperty("const float DegToRad", (void*)&degToRad); assert(ret >= 0);
      ret = engine_->RegisterGlobalProperty("const float RadToDeg", (void*)&radToDeg); assert(ret >= 0);
      ret = engine_->RegisterGlobalProperty("const float Tau",      (void*)&tau); assert(ret >= 0);
      ret = engine_->RegisterGlobalProperty("const float Pi",       (void*)&pi);  assert(ret >= 0);

      ret = engine_->RegisterEnum("NoiseInterpolation"); assert(ret >= 0);
      ret = engine_->RegisterEnumValue("NoiseInterpolation", "kLinear",  0); assert(ret >= 0);
      ret = engine_->RegisterEnumValue("NoiseInterpolation", "kHermite", 1); assert(ret >= 0);
      ret = engine_->RegisterEnumValue("NoiseInterpolation", "kQuintic", 2); assert(ret >= 0);

      for (const auto& function : functions)
      {
        Vector<String> prefixes = split(function.first, "::");
        if (prefixes.size() > 1)
        {
          String prefix;
          for (unsigned char i = 0u; i < prefixes.size() - 1; ++i)
          {
            prefix += prefixes.at(i);
          }

          ret = engine_->SetDefaultNamespace(split(prefix, " ").at(1u).c_str());
          assert(ret >= 0);
        }

        ret = engine_->RegisterGlobalFunction(function.first.c_str(), asFUNCTION(function.second), asCALL_CDECL);
        assert(ret >= 0);

        if (prefixes.size() > 1)
        {
          ret = engine_->SetDefaultNamespace("");
          assert(ret >= 0);
        }
      }

      return true;
    }

    String GetPath(String file_name)
    {
      eastl::replace(file_name.begin(), file_name.end(), '\\', '/');
      std::size_t found = file_name.find_last_of('/');
      return file_name.substr(0, found);
    }
    String GetFileName(String file_name)
    {
      eastl::replace(file_name.begin(), file_name.end(), '\\', '/');
      std::size_t found = file_name.find_last_of('/');
      return file_name.substr(found + 1);
    }


    int includeCallback(const char* include, const char* from, CScriptBuilder* builder, void* userParam)
    {
      String full_path = GetPath(from) + '/' + include;
      Vector<char> data = FileSystem::FileToVector(full_path);
      return builder->AddSectionFromMemory(full_path.c_str() , data.data(), (unsigned int)data.size());
    }

    void printExceptionInfo(asIScriptContext* ctx)
    {
      asIScriptEngine *engine = ctx->GetEngine();
      const asIScriptFunction *function = ctx->GetExceptionFunction();

      String str;
      str += "Description: ";
      str += ctx->GetExceptionString();
      str += "\nFunction:  ";
      str += function->GetDeclaration();
      str += "\nModule:    ";
      str += function->GetModuleName();
      str += "\nSection:   ";
      str += function->GetScriptSectionName();
      str += "\nLine:      ";
      str += toString(ctx->GetExceptionLineNumber());

      LMB_ASSERT(false, str.c_str());
    }

    bool AngelScriptContext::loadScripts(const Vector<String>& files)
    {
      AngelScriptComponent::k_terminating = false;
     
      CScriptBuilder builder;
      builder.SetIncludeCallback(includeCallback, nullptr);
      int ret = builder.StartNewModule(engine_, "CoreGame");
      if (ret < 0)
      {
        foundation::Error("Unrecoverable error while starting a new module. (Probably out of memory)\n");
        return false;
      }

      for (const String& file : files)
      {
        Vector<char> data = FileSystem::FileToVector(file);
        ret = builder.AddSectionFromMemory(file.c_str(), data.data(), (unsigned int)data.size());
        if (ret < 0)
        {
          foundation::Error("Please correct the errors in the script and try again.\n");
          return false;
        }
      }

      ret = builder.BuildModule();
      if (ret < 0)
      {
        foundation::Error("Please correct the errors in the script and try again.\n");
        return false;
      }

      module_  = engine_->GetModule("CoreGame");
      context_ = engine_->CreateContext();

      for (uint32_t i = 0u; i < module_->GetFunctionCount(); ++i)
      {
        asIScriptFunction* function = module_->GetFunctionByIndex(i);
        String full_name = function->GetNamespace();
        full_name = (full_name.size() > 0 ? (full_name + "::") : "") + function->GetName();

        functions_.insert(eastl::make_pair(full_name, AngelScriptFunction(function)));
      }
      
      asITypeInfo* game_info = module_->GetTypeInfoByName("Game");
      game_ = (asIScriptObject*)engine_->CreateScriptObject(game_info);
      game_initialize_   = game_info->GetMethodByName("Initialize"); 
      game_terminate_    = game_info->GetMethodByName("Terminate"); 
      game_update_       = game_info->GetMethodByName("Update"); 
      game_fixed_update_ = game_info->GetMethodByName("FixedUpdate"); 

      return true;
    }

    bool AngelScriptContext::terminate()
    {
      game_->Release();

      collectGarbage();

      functions_.clear();

      collectGarbage();

      AngelScriptComponent::k_terminating = true;
      int ret = context_->Release(); assert(ret >= 0);
      
      collectGarbage();

      module_->Discard();

      collectGarbage();

      ret = engine_->ShutDownAndRelease(); assert(ret >= 0); 

      //DeinitializeLmbString();

      if (debugger_ != nullptr)
      {
        foundation::Memory::destruct(debugger_);
      }

      return true;
    }
    
    void AngelScriptContext::collectGarbage()
    {
      int ret = engine_->GarbageCollect(asGC_FULL_CYCLE | asGC_DESTROY_GARBAGE); assert(ret >= 0);
    }

    ScriptValue AngelScriptContext::executeFunction(const String& declaration, const Vector<ScriptValue>& args)
    {
      if (declaration.at(0u) == 'G' && declaration.find("Game", 0u, 4u) != String::npos)
      {
        int ret;
        switch (declaration.at(6u))
        {
        case 'I':
          ret = context_->Prepare(game_initialize_); assert(ret >= 0);
          ret = context_->SetObject(game_); assert(ret >= 0);
          ret = context_->Execute(); assert(ret >= 0);
          break;
        case 'T':
          ret = context_->Prepare(game_terminate_); assert(ret >= 0);
          ret = context_->SetObject(game_); assert(ret >= 0);
          ret = context_->Execute(); assert(ret >= 0);
          break;
        case 'U':
          ret = context_->Prepare(game_update_); assert(ret >= 0);
          ret = context_->SetArgFloat(0, args.at(0u).getFloat()); assert(ret >= 0);
          ret = context_->SetObject(game_); assert(ret >= 0);
          ret = context_->Execute(); assert(ret >= 0);
          break;
        case 'F':
          ret = context_->Prepare(game_fixed_update_); assert(ret >= 0);
          ret = context_->SetArgFloat(0, args.at(0u).getFloat()); assert(ret >= 0);
          ret = context_->SetObject(game_); assert(ret >= 0);
          ret = context_->Execute(); assert(ret >= 0);
          break;
        }

        if (ret == asEXECUTION_EXCEPTION)
        {
          printExceptionInfo(context_);
        }

        return ScriptValue();
      }

      const AngelScriptFunction& function = functions_.at(declaration);
      int ret = context_->Prepare(function.function); assert(ret >= 0);

      for (unsigned int i = 0; i < args.size(); ++i)
      {
        const lambda::scripting::ScriptValue& arg = args.at(i);
        switch (arg.getType())
        {
        case lambda::scripting::ScriptValue::Type::kBoolean:
          ret = context_->SetArgByte(i, arg.getBool()); assert(ret >= 0);
          break;
        case lambda::scripting::ScriptValue::Type::kInt8:
          ret = context_->SetArgByte(i, arg.getInt8()); assert(ret >= 0);
          break;
        case lambda::scripting::ScriptValue::Type::kUint8:
          ret = context_->SetArgByte(i, arg.getUint8()); assert(ret >= 0);
          break;
        case lambda::scripting::ScriptValue::Type::kInt16:
          ret = context_->SetArgWord(i, arg.getInt16()); assert(ret >= 0);
          break;
        case lambda::scripting::ScriptValue::Type::kUint16:
          ret = context_->SetArgWord(i, arg.getUint16()); assert(ret >= 0);
          break;
        case lambda::scripting::ScriptValue::Type::kInt32:
          ret = context_->SetArgDWord(i, arg.getInt32()); assert(ret >= 0);
          break;
        case lambda::scripting::ScriptValue::Type::kUint32:
          ret = context_->SetArgDWord(i, arg.getUint32()); assert(ret >= 0);
          break;
        case lambda::scripting::ScriptValue::Type::kInt64:
          ret = context_->SetArgQWord(i, arg.getInt64()); assert(ret >= 0);
          break;
        case lambda::scripting::ScriptValue::Type::kUint64:
          ret = context_->SetArgQWord(i, arg.getUint64()); assert(ret >= 0);
          break;
        case lambda::scripting::ScriptValue::Type::kFloat:
          ret = context_->SetArgFloat(i, arg.getFloat()); assert(ret >= 0);
          break;
        case lambda::scripting::ScriptValue::Type::kDouble:
          ret = context_->SetArgDouble(i, arg.getDouble()); assert(ret >= 0);
          break;
        case lambda::scripting::ScriptValue::Type::kString:
        {
          String str = arg.getString();
          ret = context_->SetArgObject(i, &str);
          assert(ret >= 0);
          break;
        }
        case lambda::scripting::ScriptValue::Type::kVec2:
        {
          ScriptVec2 v = arg.getVec2();
          ret = context_->SetArgObject(i, &v);
          assert(ret >= 0);
          break;
        }
        case lambda::scripting::ScriptValue::Type::kVec3:
        {
          ScriptVec3 v = arg.getVec3();
          ret = context_->SetArgObject(i, &v);
          assert(ret >= 0);
          break;
        }
        case lambda::scripting::ScriptValue::Type::kVec4:
        {
          ScriptVec4 v = arg.getVec4();
          ret = context_->SetArgObject(i, &v);
          assert(ret >= 0);
          break;
        }
        default: break;
        }
      }

      ExecuteWithDebugger();

      switch (function.getReturnType())
      {
      case ScriptValue::Type::kNull:
      default:
        return scripting::ScriptValue();
        break;
      case ScriptValue::Type::kBoolean:
        return scripting::ScriptValue(context_->GetReturnByte() > 0 ? true : false);
        break;
      case ScriptValue::Type::kUint8:
      {
        int v = (int)context_->GetReturnByte();
        return scripting::ScriptValue((int8_t)(v < 127 ? v : (v - 256)));
        break;
      }
      case ScriptValue::Type::kUint16:
      {
        int v = (int)context_->GetReturnWord();
        return scripting::ScriptValue((int16_t)(v < 32767 ? v : (v - 65536)));
        break;
      }
      case ScriptValue::Type::kUint32:
        return scripting::ScriptValue((int32_t)context_->GetReturnDWord());
        break;
      case ScriptValue::Type::kUint64:
        return scripting::ScriptValue((int64_t)context_->GetReturnDWord());
        break;
      case ScriptValue::Type::kInt8:
        return scripting::ScriptValue((int8_t)context_->GetReturnByte());
        break;
      case ScriptValue::Type::kInt16:
        return scripting::ScriptValue((int16_t)context_->GetReturnWord());
        break;
      case ScriptValue::Type::kInt32:
        return scripting::ScriptValue((int32_t)context_->GetReturnDWord());
        break;
      case ScriptValue::Type::kInt64:
        return scripting::ScriptValue((int64_t)context_->GetReturnDWord());
        break;
      case ScriptValue::Type::kFloat:
        return scripting::ScriptValue(context_->GetReturnFloat());
        break;
      case ScriptValue::Type::kDouble:
        return scripting::ScriptValue(context_->GetReturnDouble());
        break;
      case ScriptValue::Type::kString:
        return scripting::ScriptValue(*((String*)context_->GetReturnObject()));
        break;
      case ScriptValue::Type::kVec2:
        return scripting::ScriptValue(*((ScriptVec2*)context_->GetReturnObject()));
        break;
      case ScriptValue::Type::kVec3:
        return scripting::ScriptValue(*((ScriptVec3*)context_->GetReturnObject()));
        break;
      case ScriptValue::Type::kVec4:
        return scripting::ScriptValue(*((ScriptVec4*)context_->GetReturnObject()));
        break;
      }
    }

    ScriptValue AngelScriptContext::executeFunction(const void * object, const void * function, const Vector<ScriptValue>& args)
    {
      LMB_ASSERT(false, "ANGELSCRIPT: ExecuteFunction(object, method) is not yet implemented");
      return ScriptValue();
    }

    void AngelScriptContext::freeHandle(void* handle)
    {
      LMB_ASSERT(false, "ANGELSCRIPT: FreeHandle is not yet implemented");
    }

    void AngelScriptContext::setBreakPoint(const String& file, const int16_t& line)
    {
      if (debugger_ == nullptr)
      {
        debugger_ = foundation::Memory::construct<CDebugger>();
      }
      debugger_->AddFileBreakPoint(file, line);
    }

    ScriptArray AngelScriptContext::scriptArray(const void* data)
    {
      CScriptArray* a = (CScriptArray*)data;
      ScriptArray sa;
      
      switch (asTypeToScriptType(a->GetElementTypeId()))
      {
      case ScriptValue::Type::kString:
        sa.vec_string.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_string.size(); ++i)
          memcpy(sa.vec_string.data() + i, a->At(i), sizeof(String));
        break;
      case ScriptValue::Type::kVec2:
        sa.vec_vec2.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_vec2.size(); ++i)
          memcpy(sa.vec_vec2.data() + i, a->At(i), sizeof(ScriptVec2));
        break;
      case ScriptValue::Type::kVec3:
        sa.vec_vec3.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_vec3.size(); ++i)
          memcpy(sa.vec_vec3.data() + i, a->At(i), sizeof(ScriptVec3));
        break;
      case ScriptValue::Type::kVec4:
        sa.vec_vec4.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_vec4.size(); ++i)
          memcpy(sa.vec_vec4.data() + i, a->At(i), sizeof(ScriptVec4));
        break;
      case ScriptValue::Type::kFloat:
        sa.vec_float.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_float.size(); ++i)
          memcpy(sa.vec_float.data() + i, a->At(i), sizeof(float));
        break;
      case ScriptValue::Type::kDouble:
        sa.vec_double.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_double.size(); ++i)
          memcpy(sa.vec_double.data() + i, a->At(i), sizeof(double));
        break;
      case ScriptValue::Type::kBoolean:
        sa.vec_bool.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_bool.size(); ++i)
          memcpy(sa.vec_bool.data() + i, a->At(i), sizeof(bool));
        break;
      case ScriptValue::Type::kInt8:
        sa.vec_int8.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_int8.size(); ++i)
          memcpy(sa.vec_int8.data() + i, a->At(i), sizeof(int8_t));
        break;
      case ScriptValue::Type::kInt16:
        sa.vec_int16.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_int16.size(); ++i)
          memcpy(sa.vec_int16.data() + i, a->At(i), sizeof(int16_t));
        break;
      case ScriptValue::Type::kInt32:
        sa.vec_int32.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_int32.size(); ++i)
          memcpy(sa.vec_int32.data() + i, a->At(i), sizeof(int32_t));
        break;
      case ScriptValue::Type::kInt64:
        sa.vec_int64.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_int64.size(); ++i)
          memcpy(sa.vec_int64.data() + i, a->At(i), sizeof(int64_t));
        break;
      case ScriptValue::Type::kUint8:
        sa.vec_uint8.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_uint8.size(); ++i)
          memcpy(sa.vec_uint8.data() + i, a->At(i), sizeof(uint8_t));
        break;
      case ScriptValue::Type::kUint16:
        sa.vec_uint16.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_uint16.size(); ++i)
          memcpy(sa.vec_uint16.data() + i, a->At(i), sizeof(uint16_t));
        break;
      case ScriptValue::Type::kUint32:
        sa.vec_uint32.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_uint32.size(); ++i)
          memcpy(sa.vec_uint32.data() + i, a->At(i), sizeof(uint32_t));
        break;
      case ScriptValue::Type::kUint64:
        sa.vec_uint64.resize(a->GetSize());
        for (uint32_t i = 0u; i < sa.vec_uint64.size(); ++i)
          memcpy(sa.vec_uint64.data() + i, a->At(i), sizeof(uint64_t));
        break;
      default:
      case ScriptValue::Type::kNull:
        break;
      }
      
      return sa;
    }

    void AngelScriptContext::setWorld(world::IWorld* world)
    {
      k_entity_system = world->getScene().getSystem<entity::EntitySystem>().get();
      if (k_component_manager != nullptr)
      {
        k_component_manager->setEntitySystem(k_entity_system);
      }
    }

    void AngelScriptContext::ExecuteWithDebugger()
    {
      if (debugger_ != nullptr)
      {
        // Tell the context to invoke the debugger's line callback
        context_->SetLineCallback(asMETHOD(CDebugger, LineCallback), debugger_, asCALL_THISCALL);
        // Allow the user to initialize the debugging before moving on
        debugger_->TakeCommands(context_);
      }

      int r = context_->Execute();
      if (r != asEXECUTION_FINISHED)
      {
        if (r == asEXECUTION_EXCEPTION)
        {
          foundation::Error("An exception '" + String(context_->GetExceptionString())  + "' occurred. Please correct the code and try again.\n");
        }
      }
    }

    AngelScriptContext::AngelScriptFunction::AngelScriptFunction(asIScriptFunction* function) :
      IScriptFunction(function->GetName(), function->GetNamespace(), {}, asTypeToScriptType(function->GetReturnTypeId())), function(function)
    {
      for (uint32_t j = 0u; j < function->GetParamCount(); ++j)
      {
        int type;
        asDWORD flags;
        const char* name_buffer;
        const char* default_arg;

        int ret = function->GetParam(j,&type,&flags,&name_buffer,&default_arg);
        assert(ret >= 0);
        parameters_.push_back(IScriptParameter(name_buffer, asTypeToScriptType(type)));
      }
    }
  }
}
