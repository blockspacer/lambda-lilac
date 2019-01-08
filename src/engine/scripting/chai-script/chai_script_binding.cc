#pragma warning(disable: 4503)
#include "chai_script_binding.h"
#include <chaiscript/chaiscript.hpp>
#include "../script_vector.h"
#include <string>
#include <utils/console.h>
#include "chai_script_entity.h"
#include "components/chai_script_camera.h"
#include "components/chai_script_transform.h"
#include "interfaces/iworld.h"
#include "platform/scene.h"
#include "systems/entity_system.h"
#include "systems/transform_system.h"
#include "systems/camera_system.h"
#include "assets/chai_script_name.h"
#include "assets/shader_program.h"
#include "assets/shader_pass.h"

namespace lambda
{
  namespace scripting
  {
    world::IWorld* k_chai_world = nullptr;
    entity::EntitySystem* k_chai_entity_system = nullptr;
    components::TransformSystem* k_chai_transform_system = nullptr;

    chaiscript::ModulePtr bindUtilities()
    {
      chaiscript::ModulePtr utilities = chaiscript::ModulePtr(new chaiscript::Module());
      chaiscript::utility::add_class<utilities::Time>(*utilities, "Time",
      {}, { // Functions.
        { chaiscript::fun(&utilities::Time::seconds), "AsSeconds" },
        { chaiscript::fun(&utilities::Time::milliseconds), "AsMilliseconds" },
      }
      );
      chaiscript::utility::add_class<utilities::Timer>(*utilities, "Timer",
      { // Constructors.
        chaiscript::constructor<utilities::Timer()>(),
        chaiscript::constructor<utilities::Timer(bool)>(),
      }, { // Functions.
        { chaiscript::fun(&utilities::Timer::reset), "Reset" },
        { chaiscript::fun(&utilities::Timer::elapsed), "Elapsed" },
      }
      );
      chaiscript::utility::add_class<utilities::Distance>(*utilities, "Distance",
      { // Constructors.
        chaiscript::constructor<utilities::Distance()>(),
        chaiscript::constructor<utilities::Distance(float)>(),
      }, { // Functions.
        { chaiscript::fun(&utilities::Distance::asMeter), "AsMeter" },
        { chaiscript::fun(&utilities::Distance::asCentimeter), "AsCentimeter" },
        { chaiscript::fun(&utilities::Distance::asMillimeter), "AsMillimeter" },
        { chaiscript::fun(&utilities::Distance::setMeter), "SetMeter" },
        { chaiscript::fun(&utilities::Distance::setCentimeter), "SetCentimeter" },
        { chaiscript::fun(&utilities::Distance::setMillimeter), "SetMillimeter" },
      }
      );
      utilities->add(chaiscript::fun(&utilities::Distance::fromMeter), "FromMeter");
      utilities->add(chaiscript::fun(&utilities::Distance::fromCentimeter), "FromCentimeter");
      utilities->add(chaiscript::fun(&utilities::Distance::fromMillimeter), "FromMillimeter");

      chaiscript::utility::add_class<utilities::Angle>(*utilities, "Angle",
      { // Constructors.
        chaiscript::constructor<utilities::Angle()>(),
        chaiscript::constructor<utilities::Angle(float)>(),
      }, { // Functions.
        { chaiscript::fun(&utilities::Angle::asDeg), "AsDeg" },
        { chaiscript::fun(&utilities::Angle::asDeg), "AsDegrees" },
        { chaiscript::fun(&utilities::Angle::asRad), "AsRad" },
        { chaiscript::fun(&utilities::Angle::asRad), "AsRadians" },
        { chaiscript::fun(&utilities::Angle::setDeg), "SetDeg" },
        { chaiscript::fun(&utilities::Angle::setDeg), "SetDegrees" },
        { chaiscript::fun(&utilities::Angle::setRad), "SetRad" },
        { chaiscript::fun(&utilities::Angle::setRad), "SetRadians" },
      }
      );
      utilities->add(chaiscript::fun(&utilities::Angle::fromDeg), "FromDeg");
      utilities->add(chaiscript::fun(&utilities::Angle::fromDeg), "FromDegrees");
      utilities->add(chaiscript::fun(&utilities::Angle::fromRad), "FromRad");
      utilities->add(chaiscript::fun(&utilities::Angle::fromRad), "FromRadians");
      utilities->add_global_const(chaiscript::const_var(utilities::Angle::pi), "Pi");
      utilities->add_global_const(chaiscript::const_var(utilities::Angle::tau), "Tau");
      utilities->add_global_const(chaiscript::const_var(utilities::Angle::degToRad), "Deg2Rad");
      utilities->add_global_const(chaiscript::const_var(utilities::Angle::degToRad), "DegToRad");
      utilities->add_global_const(chaiscript::const_var(utilities::Angle::radToDeg), "Rad2Deg");
      utilities->add_global_const(chaiscript::const_var(utilities::Angle::radToDeg), "RadToDeg");

      return utilities;
    }
    chaiscript::ModulePtr bindVec2()
    {
      chaiscript::ModulePtr vec2 = chaiscript::ModulePtr(new chaiscript::Module());
      chaiscript::utility::add_class<ScriptVec2>(*vec2, "Vec2",
      { // Constructors.
        chaiscript::constructor<ScriptVec2()>(),
        chaiscript::constructor<ScriptVec2(const ScriptVec2&)>(),
        chaiscript::constructor<ScriptVec2(const float&)>(),
        chaiscript::constructor<ScriptVec2(const float&, const float&)>(),
      }, { // Functions.
        { chaiscript::fun(static_cast<void(ScriptVec2::*)(const ScriptVec2&)>(&ScriptVec2::operator=)), "=" },
        { chaiscript::fun(&ScriptVec2::operator==), "==" },
        { chaiscript::fun(&ScriptVec2::operator!=), "!=" },
        { chaiscript::fun(static_cast<ScriptVec2&(ScriptVec2::*)(const ScriptVec2&)>(&ScriptVec2::operator+=)), "+=" },
        { chaiscript::fun(static_cast<ScriptVec2&(ScriptVec2::*)(const ScriptVec2&)>(&ScriptVec2::operator-=)), "-=" },
        { chaiscript::fun(static_cast<ScriptVec2&(ScriptVec2::*)(const ScriptVec2&)>(&ScriptVec2::operator/=)), "/=" },
        { chaiscript::fun(static_cast<ScriptVec2&(ScriptVec2::*)(const ScriptVec2&)>(&ScriptVec2::operator*=)), "*=" },
        { chaiscript::fun(static_cast<ScriptVec2&(ScriptVec2::*)(const float&)>(&ScriptVec2::operator+=)), "+=" },
        { chaiscript::fun(static_cast<ScriptVec2&(ScriptVec2::*)(const float&)>(&ScriptVec2::operator-=)), "-=" },
        { chaiscript::fun(static_cast<ScriptVec2&(ScriptVec2::*)(const float&)>(&ScriptVec2::operator/=)), "/=" },
        { chaiscript::fun(static_cast<ScriptVec2&(ScriptVec2::*)(const float&)>(&ScriptVec2::operator*=)), "*=" },
        { chaiscript::fun(static_cast<ScriptVec2(ScriptVec2::*)(const ScriptVec2&) const>(&ScriptVec2::operator+)), "+" },
        { chaiscript::fun(static_cast<ScriptVec2(ScriptVec2::*)(const ScriptVec2&) const>(&ScriptVec2::operator-)), "-" },
        { chaiscript::fun(static_cast<ScriptVec2(ScriptVec2::*)(const ScriptVec2&) const>(&ScriptVec2::operator/)), "/" },
        { chaiscript::fun(static_cast<ScriptVec2(ScriptVec2::*)(const ScriptVec2&) const>(&ScriptVec2::operator*)), "*" },
        { chaiscript::fun(static_cast<ScriptVec2(ScriptVec2::*)(const float&) const>(&ScriptVec2::operator+)), "+" },
        { chaiscript::fun(static_cast<ScriptVec2(ScriptVec2::*)(const float&) const>(&ScriptVec2::operator-)), "-" },
        { chaiscript::fun(static_cast<ScriptVec2(ScriptVec2::*)(const float&) const>(&ScriptVec2::operator/)), "/" },
        { chaiscript::fun(static_cast<ScriptVec2(ScriptVec2::*)(const float&) const>(&ScriptVec2::operator*)), "*" },
        { chaiscript::fun(static_cast<ScriptVec2(ScriptVec2::*)() const>(&ScriptVec2::operator-)), "-" },
        { chaiscript::fun(static_cast<float(ScriptVec2::*)() const>(&ScriptVec2::LengthSquared)), "LengthSquared" },
        { chaiscript::fun(static_cast<float(ScriptVec2::*)() const>(&ScriptVec2::LengthSquared)), "MagnitudeSquared" },
        { chaiscript::fun(static_cast<float(ScriptVec2::*)() const>(&ScriptVec2::Length)), "Length" },
        { chaiscript::fun(static_cast<float(ScriptVec2::*)() const>(&ScriptVec2::Length)), "Magnitude" },
        { chaiscript::fun(static_cast<void(ScriptVec2::*)()>(&ScriptVec2::Normalize)), "Normalize" },
        { chaiscript::fun(static_cast<ScriptVec2(ScriptVec2::*)() const>(&ScriptVec2::Normalized)), "Normalized" },
        { chaiscript::fun(static_cast<float(ScriptVec2::*)(const ScriptVec2&) const>(&ScriptVec2::Dot)), "Dot" },
        { chaiscript::fun(static_cast<std::string(ScriptVec2::*)() const>(&ScriptVec2::ToStringStd)), "ToString" },
        { chaiscript::fun(&ScriptVec2::x), "x" },
        { chaiscript::fun(&ScriptVec2::y), "y" },
      }
      );

      return vec2;
    }
    chaiscript::ModulePtr bindVec3()
    {
      chaiscript::ModulePtr vec3 = chaiscript::ModulePtr(new chaiscript::Module());
      chaiscript::utility::add_class<ScriptVec3>(*vec3, "Vec3",
      { // Constructors.
        chaiscript::constructor<ScriptVec3()>(),
        chaiscript::constructor<ScriptVec3(const ScriptVec3&)>(),
        chaiscript::constructor<ScriptVec3(const float&)>(),
        chaiscript::constructor<ScriptVec3(const float&, const float&, const float&)>(),
      }, { // Functions.
        { chaiscript::fun(static_cast<void(ScriptVec3::*)(const ScriptVec3&)>(&ScriptVec3::operator=)), "=" },
        { chaiscript::fun(&ScriptVec3::operator==), "==" },
        { chaiscript::fun(&ScriptVec3::operator!=), "!=" },
        { chaiscript::fun(static_cast<ScriptVec3&(ScriptVec3::*)(const ScriptVec3&)>(&ScriptVec3::operator+=)), "+=" },
        { chaiscript::fun(static_cast<ScriptVec3&(ScriptVec3::*)(const ScriptVec3&)>(&ScriptVec3::operator-=)), "-=" },
        { chaiscript::fun(static_cast<ScriptVec3&(ScriptVec3::*)(const ScriptVec3&)>(&ScriptVec3::operator/=)), "/=" },
        { chaiscript::fun(static_cast<ScriptVec3&(ScriptVec3::*)(const ScriptVec3&)>(&ScriptVec3::operator*=)), "*=" },
        { chaiscript::fun(static_cast<ScriptVec3&(ScriptVec3::*)(const float&)>(&ScriptVec3::operator+=)), "+=" },
        { chaiscript::fun(static_cast<ScriptVec3&(ScriptVec3::*)(const float&)>(&ScriptVec3::operator-=)), "-=" },
        { chaiscript::fun(static_cast<ScriptVec3&(ScriptVec3::*)(const float&)>(&ScriptVec3::operator/=)), "/=" },
        { chaiscript::fun(static_cast<ScriptVec3&(ScriptVec3::*)(const float&)>(&ScriptVec3::operator*=)), "*=" },
        { chaiscript::fun(static_cast<ScriptVec3(ScriptVec3::*)(const ScriptVec3&) const>(&ScriptVec3::operator+)), "+" },
        { chaiscript::fun(static_cast<ScriptVec3(ScriptVec3::*)(const ScriptVec3&) const>(&ScriptVec3::operator-)), "-" },
        { chaiscript::fun(static_cast<ScriptVec3(ScriptVec3::*)(const ScriptVec3&) const>(&ScriptVec3::operator/)), "/" },
        { chaiscript::fun(static_cast<ScriptVec3(ScriptVec3::*)(const ScriptVec3&) const>(&ScriptVec3::operator*)), "*" },
        { chaiscript::fun(static_cast<ScriptVec3(ScriptVec3::*)(const float&) const>(&ScriptVec3::operator+)), "+" },
        { chaiscript::fun(static_cast<ScriptVec3(ScriptVec3::*)(const float&) const>(&ScriptVec3::operator-)), "-" },
        { chaiscript::fun(static_cast<ScriptVec3(ScriptVec3::*)(const float&) const>(&ScriptVec3::operator/)), "/" },
        { chaiscript::fun(static_cast<ScriptVec3(ScriptVec3::*)(const float&) const>(&ScriptVec3::operator*)), "*" },
        { chaiscript::fun(static_cast<ScriptVec3(ScriptVec3::*)() const>(&ScriptVec3::operator-)), "-" },
        { chaiscript::fun(static_cast<float(ScriptVec3::*)() const>(&ScriptVec3::LengthSquared)), "LengthSquared" },
        { chaiscript::fun(static_cast<float(ScriptVec3::*)() const>(&ScriptVec3::LengthSquared)), "MagnitudeSquared" },
        { chaiscript::fun(static_cast<float(ScriptVec3::*)() const>(&ScriptVec3::Length)), "Length" },
        { chaiscript::fun(static_cast<float(ScriptVec3::*)() const>(&ScriptVec3::Length)), "Magnitude" },
        { chaiscript::fun(static_cast<void(ScriptVec3::*)()>(&ScriptVec3::Normalize)), "Normalize" },
        { chaiscript::fun(static_cast<ScriptVec3(ScriptVec3::*)() const>(&ScriptVec3::Normalized)), "Normalized" },
        { chaiscript::fun(static_cast<float(ScriptVec3::*)(const ScriptVec3&) const>(&ScriptVec3::Dot)), "Dot" },
        { chaiscript::fun(static_cast<std::string(ScriptVec3::*)() const>(&ScriptVec3::ToStringStd)), "ToString" },
        { chaiscript::fun(&ScriptVec3::x), "x" },
        { chaiscript::fun(&ScriptVec3::y), "y" },
        { chaiscript::fun(&ScriptVec3::z), "z" },
      }
      );
      return vec3;
    }
    chaiscript::ModulePtr bindVec4()
    {
      chaiscript::ModulePtr vec4 = chaiscript::ModulePtr(new chaiscript::Module());
      chaiscript::utility::add_class<ScriptVec4>(*vec4, "Vec4",
      { // Constructors.
        chaiscript::constructor<ScriptVec4()>(),
        chaiscript::constructor<ScriptVec4(const ScriptVec4&)>(),
        chaiscript::constructor<ScriptVec4(const float&)>(),
        chaiscript::constructor<ScriptVec4(const float&, const float&, const float&, const float&)>(),
      }, { // Functions.
        { chaiscript::fun(static_cast<void(ScriptVec4::*)(const ScriptVec4&)>(&ScriptVec4::operator=)), "=" },
        { chaiscript::fun(&ScriptVec4::operator==), "==" },
        { chaiscript::fun(&ScriptVec4::operator!=), "!=" },
        { chaiscript::fun(static_cast<ScriptVec4&(ScriptVec4::*)(const ScriptVec4&)>(&ScriptVec4::operator+=)), "+=" },
        { chaiscript::fun(static_cast<ScriptVec4&(ScriptVec4::*)(const ScriptVec4&)>(&ScriptVec4::operator-=)), "-=" },
        { chaiscript::fun(static_cast<ScriptVec4&(ScriptVec4::*)(const ScriptVec4&)>(&ScriptVec4::operator/=)), "/=" },
        { chaiscript::fun(static_cast<ScriptVec4&(ScriptVec4::*)(const ScriptVec4&)>(&ScriptVec4::operator*=)), "*=" },
        { chaiscript::fun(static_cast<ScriptVec4&(ScriptVec4::*)(const float&)>(&ScriptVec4::operator+=)), "+=" },
        { chaiscript::fun(static_cast<ScriptVec4&(ScriptVec4::*)(const float&)>(&ScriptVec4::operator-=)), "-=" },
        { chaiscript::fun(static_cast<ScriptVec4&(ScriptVec4::*)(const float&)>(&ScriptVec4::operator/=)), "/=" },
        { chaiscript::fun(static_cast<ScriptVec4&(ScriptVec4::*)(const float&)>(&ScriptVec4::operator*=)), "*=" },
        { chaiscript::fun(static_cast<ScriptVec4(ScriptVec4::*)(const ScriptVec4&) const>(&ScriptVec4::operator+)), "+" },
        { chaiscript::fun(static_cast<ScriptVec4(ScriptVec4::*)(const ScriptVec4&) const>(&ScriptVec4::operator-)), "-" },
        { chaiscript::fun(static_cast<ScriptVec4(ScriptVec4::*)(const ScriptVec4&) const>(&ScriptVec4::operator/)), "/" },
        { chaiscript::fun(static_cast<ScriptVec4(ScriptVec4::*)(const ScriptVec4&) const>(&ScriptVec4::operator*)), "*" },
        { chaiscript::fun(static_cast<ScriptVec4(ScriptVec4::*)(const float&) const>(&ScriptVec4::operator+)), "+" },
        { chaiscript::fun(static_cast<ScriptVec4(ScriptVec4::*)(const float&) const>(&ScriptVec4::operator-)), "-" },
        { chaiscript::fun(static_cast<ScriptVec4(ScriptVec4::*)(const float&) const>(&ScriptVec4::operator/)), "/" },
        { chaiscript::fun(static_cast<ScriptVec4(ScriptVec4::*)(const float&) const>(&ScriptVec4::operator*)), "*" },
        { chaiscript::fun(static_cast<ScriptVec4(ScriptVec4::*)() const>(&ScriptVec4::operator-)), "-" },
        { chaiscript::fun(static_cast<float(ScriptVec4::*)() const>(&ScriptVec4::LengthSquared)), "LengthSquared" },
        { chaiscript::fun(static_cast<float(ScriptVec4::*)() const>(&ScriptVec4::LengthSquared)), "MagnitudeSquared" },
        { chaiscript::fun(static_cast<float(ScriptVec4::*)() const>(&ScriptVec4::Length)), "Length" },
        { chaiscript::fun(static_cast<float(ScriptVec4::*)() const>(&ScriptVec4::Length)), "Magnitude" },
        { chaiscript::fun(static_cast<void(ScriptVec4::*)()>(&ScriptVec4::Normalize)), "Normalize" },
        { chaiscript::fun(static_cast<ScriptVec4(ScriptVec4::*)() const>(&ScriptVec4::Normalized)), "Normalized" },
        { chaiscript::fun(static_cast<float(ScriptVec4::*)(const ScriptVec4&) const>(&ScriptVec4::Dot)), "Dot" },
        { chaiscript::fun(static_cast<std::string(ScriptVec4::*)() const>(&ScriptVec4::ToStringStd)), "ToString" },
        { chaiscript::fun(&ScriptVec4::x), "x" },
        { chaiscript::fun(&ScriptVec4::y), "y" },
        { chaiscript::fun(&ScriptVec4::z), "z" },
        { chaiscript::fun(&ScriptVec4::w), "w" },
      }
      );
      return vec4;
    }
    chaiscript::ModulePtr bindQuat()
    {
      chaiscript::ModulePtr quat = chaiscript::ModulePtr(new chaiscript::Module());
      chaiscript::utility::add_class<ScriptQuat>(*quat, "Quat",
      { // Constructors.
        chaiscript::constructor<ScriptQuat()>(),
        chaiscript::constructor<ScriptQuat(const ScriptQuat&)>(),
        chaiscript::constructor<ScriptQuat(const ScriptVec3&)>(),
        chaiscript::constructor<ScriptQuat(const float&, const float&, const float&)>(),
        chaiscript::constructor<ScriptQuat(const float&, const float&, const float&, const float&)>(),
      }, { // Functions.
        { chaiscript::fun(static_cast<void(ScriptQuat::*)(const ScriptQuat&)>(&ScriptQuat::operator=)), "=" },
        { chaiscript::fun(&ScriptQuat::operator==), "==" },
        { chaiscript::fun(&ScriptQuat::operator!=), "!=" },
        { chaiscript::fun(static_cast<ScriptQuat&(ScriptQuat::*)(const ScriptQuat&)>(&ScriptQuat::operator+=)), "+=" },
        { chaiscript::fun(static_cast<ScriptQuat&(ScriptQuat::*)(const ScriptQuat&)>(&ScriptQuat::operator-=)), "-=" },
        { chaiscript::fun(static_cast<ScriptQuat&(ScriptQuat::*)(const ScriptQuat&)>(&ScriptQuat::operator*=)), "*=" },
        { chaiscript::fun(static_cast<ScriptQuat(ScriptQuat::*)(const ScriptQuat&) const>(&ScriptQuat::operator+)), "+" },
        { chaiscript::fun(static_cast<ScriptQuat(ScriptQuat::*)(const ScriptQuat&) const>(&ScriptQuat::operator-)), "-" },
        { chaiscript::fun(static_cast<ScriptQuat(ScriptQuat::*)(const ScriptQuat&) const>(&ScriptQuat::operator*)), "*" },
        { chaiscript::fun(static_cast<ScriptQuat(ScriptQuat::*)() const>(&ScriptQuat::operator-)), "-" },
        { chaiscript::fun(static_cast<float(ScriptQuat::*)() const>(&ScriptQuat::LengthSquared)), "LengthSquared" },
        { chaiscript::fun(static_cast<float(ScriptQuat::*)() const>(&ScriptQuat::LengthSquared)), "MagnitudeSquared" },
        { chaiscript::fun(static_cast<float(ScriptQuat::*)() const>(&ScriptQuat::Length)), "Length" },
        { chaiscript::fun(static_cast<float(ScriptQuat::*)() const>(&ScriptQuat::Length)), "Magnitude" },
        { chaiscript::fun(static_cast<void(ScriptQuat::*)()>(&ScriptQuat::Normalize)), "Normalize" },
        { chaiscript::fun(static_cast<ScriptQuat(ScriptQuat::*)() const>(&ScriptQuat::Normalized)), "Normalized" },
        { chaiscript::fun(static_cast<ScriptVec3(ScriptQuat::*)() const>(&ScriptQuat::ToEuler)), "ToEuler" },
        { chaiscript::fun(static_cast<std::string(ScriptQuat::*)() const>(&ScriptQuat::ToStringStd)), "ToString" },
        { chaiscript::fun(&ScriptQuat::x), "x" },
        { chaiscript::fun(&ScriptQuat::y), "y" },
        { chaiscript::fun(&ScriptQuat::z), "z" },
        { chaiscript::fun(&ScriptQuat::w), "w" },
      }
      );
      return quat;
    }
    chaiscript::ModulePtr bindConversion()
    {
      chaiscript::ModulePtr conversion = chaiscript::ModulePtr(new chaiscript::Module());
      conversion->add(chaiscript::fun([](const float& v)    { return std::to_string(v); }), "ToString");
      conversion->add(chaiscript::fun([](const double& v)   { return std::to_string(v); }), "ToString");
      conversion->add(chaiscript::fun([](const bool& v)     { return std::to_string(v); }), "ToString");
      conversion->add(chaiscript::fun([](const uint8_t& v)  { return std::to_string(v); }), "ToString");
      conversion->add(chaiscript::fun([](const uint16_t& v) { return std::to_string(v); }), "ToString");
      conversion->add(chaiscript::fun([](const uint32_t& v) { return std::to_string(v); }), "ToString");
      conversion->add(chaiscript::fun([](const uint64_t& v) { return std::to_string(v); }), "ToString");
      conversion->add(chaiscript::fun([](const int8_t& v)   { return std::to_string(v); }), "ToString");
      conversion->add(chaiscript::fun([](const int16_t& v)  { return std::to_string(v); }), "ToString");
      conversion->add(chaiscript::fun([](const int32_t& v)  { return std::to_string(v); }), "ToString");
      conversion->add(chaiscript::fun([](const int64_t& v)  { return std::to_string(v); }), "ToString");
      return conversion;
    }
    chaiscript::ModulePtr bindConsole()
    {
      chaiscript::ModulePtr console = chaiscript::ModulePtr(new chaiscript::Module());
      console->add(chaiscript::fun([](const std::string& str) { foundation::Error  (lmbString(str) + "\n"); }), "Error");
      console->add(chaiscript::fun([](const std::string& str) { foundation::Warning(lmbString(str) + "\n"); }), "Warning");
      console->add(chaiscript::fun([](const std::string& str) { foundation::Debug  (lmbString(str) + "\n"); }), "Debug");
      console->add(chaiscript::fun([](const std::string& str) { foundation::Info   (lmbString(str) + "\n"); }), "Info");
      console->add(chaiscript::fun([](const bool& v, const std::string& str) { LMB_ASSERT(v, lmbString(str)); }), "Assert");
      return console;
    }
    chaiscript::ModulePtr bindComponent()
    {
      chaiscript::ModulePtr component = chaiscript::ModulePtr(new chaiscript::Module());
      chaiscript::utility::add_class<ChaiComponent>(*component, "Component", {}, 
        { { chaiscript::fun(&ChaiComponent::GetGameObject), "GetGameObject" } }
      );
      return component;
    }
    chaiscript::ModulePtr bindTransform()
    {
      chaiscript::ModulePtr transform = chaiscript::ModulePtr(new chaiscript::Module());
      chaiscript::utility::add_class<ChaiTransform>(*transform, "Transform",
      {}, { // Functions.
        { chaiscript::fun(&ChaiTransform::setLocalTranslation), "SetLocalTranslation" },
        { chaiscript::fun(&ChaiTransform::setLocalTranslation), "SetLocalPosition" },
        { chaiscript::fun(static_cast<void(ChaiTransform::*)(const ScriptQuat&)>(&ChaiTransform::setLocalRotation)), "SetLocalRotation" },
        { chaiscript::fun(static_cast<void(ChaiTransform::*)(const ScriptVec3&)>(&ChaiTransform::setLocalRotation)), "SetLocalRotation" },
        { chaiscript::fun(&ChaiTransform::setLocalScale), "SetLocalScale" },
        { chaiscript::fun(&ChaiTransform::getLocalTranslation), "GetLocalTranslation" },
        { chaiscript::fun(&ChaiTransform::getLocalTranslation), "GetLocalPosition" },
        { chaiscript::fun(&ChaiTransform::getLocalRotation), "GetLocalRotation" },
        { chaiscript::fun(&ChaiTransform::getLocalScale), "GetLocalScale" },
        { chaiscript::fun(&ChaiTransform::moveLocal), "MoveLocal" },
        { chaiscript::fun(&ChaiTransform::rotateLocal), "RotateLocal" },
        { chaiscript::fun(&ChaiTransform::scaleLocal), "ScaleLocal" },
        { chaiscript::fun(&ChaiTransform::setWorldTranslation), "SetWorldTranslation" },
        { chaiscript::fun(&ChaiTransform::setWorldTranslation), "SetWorldPosition" },
        { chaiscript::fun(static_cast<void(ChaiTransform::*)(const ScriptQuat&)>(&ChaiTransform::setWorldRotation)), "SetWorldRotation" },
        { chaiscript::fun(static_cast<void(ChaiTransform::*)(const ScriptVec3&)>(&ChaiTransform::setWorldRotation)), "SetWorldRotation" },
        { chaiscript::fun(&ChaiTransform::setWorldScale), "SetWorldScale" },
        { chaiscript::fun(&ChaiTransform::getWorldTranslation), "GetWorldTranslation" },
        { chaiscript::fun(&ChaiTransform::getWorldTranslation), "GetWorldPosition" },
        { chaiscript::fun(&ChaiTransform::getWorldRotation), "GetWorldRotation" },
        { chaiscript::fun(&ChaiTransform::getWorldScale), "GetWorldScale" },
        { chaiscript::fun(&ChaiTransform::moveWorld), "MoveWorld" },
        { chaiscript::fun(&ChaiTransform::rotateWorld), "RotateWorld" },
        { chaiscript::fun(&ChaiTransform::scaleWorld), "ScaleWorld" },
        { chaiscript::fun(&ChaiTransform::transformPoint), "TransformPoint" },
        { chaiscript::fun(&ChaiTransform::transformVector), "TransformVector" },
        { chaiscript::fun(&ChaiTransform::transformDirection), "TransformDirection" },
        { chaiscript::fun(&ChaiTransform::transformLocalPoint), "TransformLocalPoint" },
        { chaiscript::fun(&ChaiTransform::transformLocalVector), "TransformLocalVector" },
        { chaiscript::fun(&ChaiTransform::transformLocalDirection), "TransformLocalDirection" },
        { chaiscript::fun(&ChaiTransform::inverseTransformPoint), "InverseTransformPoint" },
        { chaiscript::fun(&ChaiTransform::inverseTransformVector), "InverseTransformVector" },
        { chaiscript::fun(&ChaiTransform::getWorldForward), "GetWorldForward" },
        { chaiscript::fun(&ChaiTransform::getWorldUp), "GetWorldUp" },
        { chaiscript::fun(&ChaiTransform::getWorldRight), "GetWorldRight" },
        { chaiscript::fun(&ChaiTransform::lookAt), "LookAt" },
        { chaiscript::fun(&ChaiTransform::lookAtLocal), "LookAtLocal" },
      }
      );
      transform->add(chaiscript::base_class<ChaiComponent, ChaiTransform>());
      return transform;
    }
    chaiscript::ModulePtr bindCamera()
    {
      chaiscript::ModulePtr camera = chaiscript::ModulePtr(new chaiscript::Module());
      chaiscript::utility::add_class<ChaiCamera>(*camera, "Camera",
      {}, { // Functions.
        { chaiscript::fun(&ChaiCamera::setFov), "SetFov" },
        { chaiscript::fun(&ChaiCamera::getFov), "GetFov" },
        { chaiscript::fun(&ChaiCamera::setNearPlane), "SetNearPlane" },
        { chaiscript::fun(&ChaiCamera::getNearPlane), "GetNearPlane" },
        { chaiscript::fun(&ChaiCamera::setFarPlane), "SetFarPlane" },
        { chaiscript::fun(&ChaiCamera::getFarPlane), "GetFarPlane" },
        { chaiscript::fun(&ChaiCamera::setShaderPass), "SetShaderPass" },
      }
      );
      camera->add(chaiscript::base_class<ChaiComponent, ChaiCamera>());
      return camera;
    }
    chaiscript::ModulePtr bindEntity()
    {
      chaiscript::ModulePtr game_object = chaiscript::ModulePtr(new chaiscript::Module());
      chaiscript::utility::add_class<ChaiGameObject>(*game_object, "GameObject",
      { // Constructors.
        { chaiscript::constructor<ChaiGameObject(const ChaiGameObject&)>() }
      }, { // Functions.
        { chaiscript::fun(&ChaiGameObject::operator=), "=" },
        { chaiscript::fun(&ChaiGameObject::GetId), "GetId" },
        { chaiscript::fun(&ChaiGameObject::AddComponent<ChaiTransform>), "AddTransform" },
        { chaiscript::fun(&ChaiGameObject::GetComponent<ChaiTransform>), "GetTransform" },
        { chaiscript::fun(&ChaiGameObject::RemoveComponent<ChaiTransform>), "RemoveTransform" },
        { chaiscript::fun(&ChaiGameObject::AddComponent<ChaiCamera>), "AddCamera" },
        { chaiscript::fun(&ChaiGameObject::GetComponent<ChaiCamera>), "GetCamera" },
        { chaiscript::fun(&ChaiGameObject::RemoveComponent<ChaiCamera>), "RemoveCamera" },
      }
      );
      
      game_object->add(chaiscript::fun([]() { return foundation::Memory::construct<ChaiGameObject>(k_chai_entity_system->createEntity().id()); }), "CreateGameObject");
      game_object->add(chaiscript::fun([](ChaiGameObject* go) { k_chai_entity_system->destroyEntity(entity::Entity(go->GetId(), k_chai_entity_system)); go->destroy(); foundation::Memory::destruct(go); }), "DestroyGameObject");
      return game_object;
    }
    chaiscript::ModulePtr bindName()
    {
      chaiscript::ModulePtr name = chaiscript::ModulePtr(new chaiscript::Module());
      chaiscript::utility::add_class<ChaiName>(*name, "Name",
      { // Constructors.
        { chaiscript::constructor<ChaiName()>() },
        { chaiscript::constructor<ChaiName(const std::string&)>() },
        { chaiscript::constructor<ChaiName(const ChaiName&)>() },
      }, { // Functions.
        { chaiscript::fun(static_cast<void(ChaiName::*)(const ChaiName&)>(&ChaiName::operator=)), "=" },
        { chaiscript::fun(static_cast<void(ChaiName::*)(const size_t&)>(&ChaiName::operator=)), "=" },
        { chaiscript::fun(static_cast<void(ChaiName::*)(const std::string&)>(&ChaiName::operator=)), "=" },
        { chaiscript::fun(&ChaiName::getName), "GetName" },
        { chaiscript::fun(&ChaiName::getHash), "GetHash" },
        { chaiscript::fun(&ChaiName::operator==), "==" },
        { chaiscript::fun(&ChaiName::operator!=), "!=" },
      }
      );

      return name;
    }
    chaiscript::ModulePtr bindTexture()
    {
      chaiscript::ModulePtr texture = chaiscript::ModulePtr(new chaiscript::Module());
      chaiscript::utility::add_class<asset::VioletTextureHandle>(*texture, "Texture",
      { // Constructors.
        { chaiscript::constructor<asset::VioletTextureHandle()>() },
      }, { // Functions.
      }
      );

      return texture;
    }
    chaiscript::ModulePtr bindShaderProgram()
    {
      chaiscript::ModulePtr shader_program = chaiscript::ModulePtr(new chaiscript::Module());
      chaiscript::utility::add_class<asset::VioletShaderProgramHandle>(*shader_program, "ShaderProgram",
      { // Constructors.
      }, { // Functions.
      }
      );

      return shader_program;
    }
    chaiscript::ModulePtr bindShaderPass()
    {
      chaiscript::ModulePtr shader_pass = chaiscript::ModulePtr(new chaiscript::Module());
      chaiscript::utility::add_class<asset::VioletShaderPassHandle>(*shader_pass, "ShaderPass",
      { // Constructors.
        { chaiscript::constructor<asset::VioletShaderPassHandle()>() },
      }, { // Functions.
        { chaiscript::fun([](asset::VioletShaderPassHandle shader_pass, asset::VioletShaderProgramHandle program) { shader_pass->addProgram(program); }), "AddProgram" },
        { chaiscript::fun([](asset::VioletShaderPassHandle shader_pass, asset::VioletTextureHandle input) { shader_pass->addInput(input); }), "AddInput" },
        { chaiscript::fun([](asset::VioletShaderPassHandle shader_pass, asset::VioletTextureHandle output) { shader_pass->addOutput(output); }), "AddOutput" },
      }
      );
      shader_pass->add(chaiscript::fun([](const ChaiName& name) { return asset::VioletShaderPassHandle(foundation::Memory::construct<asset::VioletShaderPass>(), name); }), "CreateShaderPass");

      return shader_pass;
    }

    void bind(chaiscript::ChaiScript* context)
    {
      // Foundation.
      context->add(bindUtilities());
      context->add(bindVec2());
      context->add(bindVec3());
      context->add(bindVec4());
      context->add(bindQuat());
      context->add(bindConversion());
      context->add(bindConsole());

      // Assets.
      context->add(bindName());
      context->add(bindTexture());
      context->add(bindShaderProgram());
      context->add(bindShaderPass());

      // Components.
      context->add(bindComponent());
      context->add(bindTransform());
      context->add(bindCamera());
      context->add(bindEntity());
    }

    void setWorld(world::IWorld* world)
    {
      k_chai_world = world;
      k_chai_entity_system    = k_chai_world->getScene().getSystem<entity::EntitySystem>().get();
      k_chai_transform_system = k_chai_world->getScene().getSystem<components::TransformSystem>().get();
    }
  }
}