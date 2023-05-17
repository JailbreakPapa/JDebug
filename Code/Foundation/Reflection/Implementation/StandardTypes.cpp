#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Transform.h>
#include <Foundation/Reflection/Reflection.h>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdEnumBase, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdBitflagsBase, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdReflectedClass, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

// *********************************************
// ***** Standard POD Types for Properties *****

WD_BEGIN_STATIC_REFLECTED_TYPE(bool, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(float, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(double, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdInt8, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdUInt8, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdInt16, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdUInt16, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdInt32, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdUInt32, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdInt64, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdUInt64, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdConstCharPtr, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdTime, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_FUNCTIONS
  {
    WD_FUNCTION_PROPERTY(Nanoseconds),
    WD_FUNCTION_PROPERTY(Microseconds),
    WD_FUNCTION_PROPERTY(Milliseconds),
    WD_FUNCTION_PROPERTY(Seconds),
    WD_FUNCTION_PROPERTY(Zero),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdColor, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("r", r),
    WD_MEMBER_PROPERTY("g", g),
    WD_MEMBER_PROPERTY("b", b),
    WD_MEMBER_PROPERTY("a", a),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(float, float, float),
    WD_CONSTRUCTOR_PROPERTY(float, float, float, float),
    WD_CONSTRUCTOR_PROPERTY(wdColorLinearUB),
    WD_CONSTRUCTOR_PROPERTY(wdColorGammaUB),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdColorBaseUB, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("r", r),
    WD_MEMBER_PROPERTY("g", g),
    WD_MEMBER_PROPERTY("b", b),
    WD_MEMBER_PROPERTY("a", a),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdUInt8, wdUInt8, wdUInt8),
    WD_CONSTRUCTOR_PROPERTY(wdUInt8, wdUInt8, wdUInt8, wdUInt8),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdColorGammaUB, wdColorBaseUB, 1, wdRTTINoAllocator)
{
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdUInt8, wdUInt8, wdUInt8),
    WD_CONSTRUCTOR_PROPERTY(wdUInt8, wdUInt8, wdUInt8, wdUInt8),
    WD_CONSTRUCTOR_PROPERTY(const wdColor&),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdColorLinearUB, wdColorBaseUB, 1, wdRTTINoAllocator)
{
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdUInt8, wdUInt8, wdUInt8),
    WD_CONSTRUCTOR_PROPERTY(wdUInt8, wdUInt8, wdUInt8, wdUInt8),
    WD_CONSTRUCTOR_PROPERTY(const wdColor&),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdVec2, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("x", x),
    WD_MEMBER_PROPERTY("y", y),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(float),
    WD_CONSTRUCTOR_PROPERTY(float, float),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdVec3, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("x", x),
    WD_MEMBER_PROPERTY("y", y),
    WD_MEMBER_PROPERTY("z", z),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(float),
    WD_CONSTRUCTOR_PROPERTY(float, float, float),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdVec4, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("x", x),
    WD_MEMBER_PROPERTY("y", y),
    WD_MEMBER_PROPERTY("z", z),
    WD_MEMBER_PROPERTY("w", w),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(float),
    WD_CONSTRUCTOR_PROPERTY(float, float, float, float),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdVec2I32, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("x", x),
    WD_MEMBER_PROPERTY("y", y),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdInt32),
    WD_CONSTRUCTOR_PROPERTY(wdInt32, wdInt32),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdVec3I32, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("x", x),
    WD_MEMBER_PROPERTY("y", y),
    WD_MEMBER_PROPERTY("z", z),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdInt32),
    WD_CONSTRUCTOR_PROPERTY(wdInt32, wdInt32, wdInt32),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdVec4I32, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("x", x),
    WD_MEMBER_PROPERTY("y", y),
    WD_MEMBER_PROPERTY("z", z),
    WD_MEMBER_PROPERTY("w", w),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdInt32),
    WD_CONSTRUCTOR_PROPERTY(wdInt32, wdInt32, wdInt32, wdInt32),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdVec2U32, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("x", x),
    WD_MEMBER_PROPERTY("y", y),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdUInt32),
    WD_CONSTRUCTOR_PROPERTY(wdUInt32, wdUInt32),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdVec3U32, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("x", x),
    WD_MEMBER_PROPERTY("y", y),
    WD_MEMBER_PROPERTY("z", z),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdUInt32),
    WD_CONSTRUCTOR_PROPERTY(wdUInt32, wdUInt32, wdUInt32),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdVec4U32, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("x", x),
    WD_MEMBER_PROPERTY("y", y),
    WD_MEMBER_PROPERTY("z", z),
    WD_MEMBER_PROPERTY("w", w),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdUInt32),
    WD_CONSTRUCTOR_PROPERTY(wdUInt32, wdUInt32, wdUInt32, wdUInt32),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdQuat, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("v", v),
    WD_MEMBER_PROPERTY("w", w),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(float, float, float, float),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdMat3, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdMat4, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdTransform, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Position", m_vPosition),
    WD_MEMBER_PROPERTY("Rotation", m_qRotation),
    WD_MEMBER_PROPERTY("Scale", m_vScale),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdVec3, wdQuat),
    WD_CONSTRUCTOR_PROPERTY(wdVec3, wdQuat, wdVec3),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_ENUM(wdBasisAxis, 1)
WD_ENUM_CONSTANT(wdBasisAxis::PositiveX),
WD_ENUM_CONSTANT(wdBasisAxis::PositiveY),
WD_ENUM_CONSTANT(wdBasisAxis::PositiveZ),
WD_ENUM_CONSTANT(wdBasisAxis::NegativeX),
WD_ENUM_CONSTANT(wdBasisAxis::NegativeY),
WD_ENUM_CONSTANT(wdBasisAxis::NegativeZ),
WD_END_STATIC_REFLECTED_ENUM;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdUuid, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdVariant, wdNoBase, 3, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdString, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdUntrackedString, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdStringView, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdDataBuffer, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdAngle, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_FUNCTIONS
  {
    WD_FUNCTION_PROPERTY(Degree),
    WD_FUNCTION_PROPERTY(Radian),
  }
  WD_END_FUNCTIONS;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdFloatInterval, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Start", m_StartValue),
    WD_MEMBER_PROPERTY("End", m_EndValue),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdIntInterval, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Start", m_StartValue),
    WD_MEMBER_PROPERTY("End", m_EndValue),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;

// **********************************************************************
// ***** Various RTTI infos that can't be put next to their classes *****

WD_BEGIN_STATIC_REFLECTED_BITFLAGS(wdTypeFlags, 1)
WD_BITFLAGS_CONSTANTS(wdTypeFlags::StandardType, wdTypeFlags::IsEnum, wdTypeFlags::Bitflags, wdTypeFlags::Class, wdTypeFlags::Abstract, wdTypeFlags::Phantom, wdTypeFlags::Minimal)
WD_END_STATIC_REFLECTED_BITFLAGS;

WD_BEGIN_STATIC_REFLECTED_BITFLAGS(wdPropertyFlags, 1)
WD_BITFLAGS_CONSTANTS(wdPropertyFlags::StandardType, wdPropertyFlags::IsEnum, wdPropertyFlags::Bitflags, wdPropertyFlags::Class)
WD_BITFLAGS_CONSTANTS(wdPropertyFlags::Const, wdPropertyFlags::Reference, wdPropertyFlags::Pointer)
WD_BITFLAGS_CONSTANTS(wdPropertyFlags::PointerOwner, wdPropertyFlags::ReadOnly, wdPropertyFlags::Hidden, wdPropertyFlags::Phantom)
WD_END_STATIC_REFLECTED_BITFLAGS;

WD_BEGIN_STATIC_REFLECTED_ENUM(wdFunctionType, 1)
WD_BITFLAGS_CONSTANTS(wdFunctionType::Member, wdFunctionType::StaticMember, wdFunctionType::Constructor)
WD_END_STATIC_REFLECTED_ENUM;

WD_BEGIN_STATIC_REFLECTED_ENUM(wdVariantType, 1)
WD_BITFLAGS_CONSTANTS(wdVariantType::Invalid, wdVariantType::Bool, wdVariantType::Int8, wdVariantType::UInt8, wdVariantType::Int16, wdVariantType::UInt16)
WD_BITFLAGS_CONSTANTS(wdVariantType::Int32, wdVariantType::UInt32, wdVariantType::Int64, wdVariantType::UInt64, wdVariantType::Float, wdVariantType::Double)
WD_BITFLAGS_CONSTANTS(wdVariantType::Color, wdVariantType::Vector2, wdVariantType::Vector3, wdVariantType::Vector4)
WD_BITFLAGS_CONSTANTS(wdVariantType::Vector2I, wdVariantType::Vector3I, wdVariantType::Vector4I, wdVariantType::Vector2U, wdVariantType::Vector3U, wdVariantType::Vector4U)
WD_BITFLAGS_CONSTANTS(wdVariantType::Quaternion, wdVariantType::Matrix3, wdVariantType::Matrix4, wdVariantType::Transform)
WD_BITFLAGS_CONSTANTS(wdVariantType::String, wdVariantType::StringView, wdVariantType::DataBuffer, wdVariantType::Time, wdVariantType::Uuid, wdVariantType::Angle, wdVariantType::ColorGamma)
WD_BITFLAGS_CONSTANTS(wdVariantType::VariantArray, wdVariantType::VariantDictionary, wdVariantType::TypedPointer, wdVariantType::TypedObject)
WD_END_STATIC_REFLECTED_ENUM;

WD_BEGIN_STATIC_REFLECTED_ENUM(wdPropertyCategory, 1)
WD_BITFLAGS_CONSTANTS(wdPropertyCategory::Constant, wdPropertyCategory::Member, wdPropertyCategory::Function, wdPropertyCategory::Array, wdPropertyCategory::Set, wdPropertyCategory::Map)
WD_END_STATIC_REFLECTED_ENUM;
// clang-format on

WD_STATICLINK_FILE(Foundation, Foundation_Reflection_Implementation_StandardTypes);
