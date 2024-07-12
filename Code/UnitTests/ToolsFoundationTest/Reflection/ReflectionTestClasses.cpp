#include <ToolsFoundationTest/ToolsFoundationTestPCH.h>

#include <Foundation/Reflection/Reflection.h>
#include <ToolsFoundationTest/Reflection/ReflectionTestClasses.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsIntegerStruct, nsNoBase, 1, nsRTTIDefaultAllocator<nsIntegerStruct>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Int8", GetInt8, SetInt8),
    NS_ACCESSOR_PROPERTY("UInt8", GetUInt8, SetUInt8),
    NS_MEMBER_PROPERTY("Int16", m_iInt16),
    NS_MEMBER_PROPERTY("UInt16", m_iUInt16),
    NS_ACCESSOR_PROPERTY("Int32", GetInt32, SetInt32),
    NS_ACCESSOR_PROPERTY("UInt32", GetUInt32, SetUInt32),
    NS_MEMBER_PROPERTY("Int64", m_iInt64),
    NS_MEMBER_PROPERTY("UInt64", m_iUInt64),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;


NS_BEGIN_STATIC_REFLECTED_TYPE(nsFloatStruct, nsNoBase, 1, nsRTTIDefaultAllocator<nsFloatStruct>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Float", GetFloat, SetFloat),
    NS_ACCESSOR_PROPERTY("Double", GetDouble, SetDouble),
    NS_ACCESSOR_PROPERTY("Time", GetTime, SetTime),
    NS_ACCESSOR_PROPERTY("Angle", GetAngle, SetAngle),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;


NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsPODClass, 1, nsRTTIDefaultAllocator<nsPODClass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Integer", m_IntegerStruct),
    NS_MEMBER_PROPERTY("Float", m_FloatStruct),
    NS_ACCESSOR_PROPERTY("Bool", GetBool, SetBool),
    NS_ACCESSOR_PROPERTY("Color", GetColor, SetColor),
    NS_MEMBER_PROPERTY("ColorUB", m_Color2),
    NS_ACCESSOR_PROPERTY("CharPtr", GetCharPtr, SetCharPtr),
    NS_ACCESSOR_PROPERTY("String", GetString, SetString),
    NS_ACCESSOR_PROPERTY("StringView", GetStringView, SetStringView),
    NS_ACCESSOR_PROPERTY("Buffer", GetBuffer, SetBuffer),
    NS_ACCESSOR_PROPERTY("VarianceAngle", GetCustom, SetCustom),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;


NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMathClass, 1, nsRTTIDefaultAllocator<nsMathClass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Vec2", GetVec2, SetVec2),
    NS_ACCESSOR_PROPERTY("Vec3", GetVec3, SetVec3),
    NS_ACCESSOR_PROPERTY("Vec4", GetVec4, SetVec4),
    NS_MEMBER_PROPERTY("Vec2I", m_Vec2I),
    NS_MEMBER_PROPERTY("Vec3I", m_Vec3I),
    NS_MEMBER_PROPERTY("Vec4I", m_Vec4I),
    NS_ACCESSOR_PROPERTY("Quat", GetQuat, SetQuat),
    NS_ACCESSOR_PROPERTY("Mat3", GetMat3, SetMat3),
    NS_ACCESSOR_PROPERTY("Mat4", GetMat4, SetMat4),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;


NS_BEGIN_STATIC_REFLECTED_ENUM(nsExampleEnum, 1)
  NS_ENUM_CONSTANTS(nsExampleEnum::Value1, nsExampleEnum::Value2)
  NS_ENUM_CONSTANT(nsExampleEnum::Value3),
NS_END_STATIC_REFLECTED_ENUM;


NS_BEGIN_STATIC_REFLECTED_BITFLAGS(nsExampleBitflags, 1)
  NS_BITFLAGS_CONSTANTS(nsExampleBitflags::Value1, nsExampleBitflags::Value2)
  NS_BITFLAGS_CONSTANT(nsExampleBitflags::Value3),
NS_END_STATIC_REFLECTED_BITFLAGS;


NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsEnumerationsClass, 1, nsRTTIDefaultAllocator<nsEnumerationsClass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ENUM_ACCESSOR_PROPERTY("Enum", nsExampleEnum, GetEnum, SetEnum),
    NS_BITFLAGS_ACCESSOR_PROPERTY("Bitflags", nsExampleBitflags, GetBitflags, SetBitflags),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;


NS_BEGIN_STATIC_REFLECTED_TYPE(InnerStruct, nsNoBase, 1, nsRTTIDefaultAllocator<InnerStruct>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("IP1", m_fP1),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;


NS_BEGIN_DYNAMIC_REFLECTED_TYPE(OuterClass, 1, nsRTTIDefaultAllocator<OuterClass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Inner", m_Inner1),
    NS_MEMBER_PROPERTY("OP1", m_fP1),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;


NS_BEGIN_DYNAMIC_REFLECTED_TYPE(ExtendedOuterClass, 1, nsRTTIDefaultAllocator<ExtendedOuterClass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("MORE", m_more),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;


NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsObjectTest, 1, nsRTTIDefaultAllocator<nsObjectTest>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("MemberClass", m_MemberClass),
    NS_ARRAY_MEMBER_PROPERTY("StandardTypeArray", m_StandardTypeArray),
    NS_ARRAY_MEMBER_PROPERTY("ClassArray", m_ClassArray),
    NS_ARRAY_MEMBER_PROPERTY("ClassPtrArray", m_ClassPtrArray)->AddFlags(nsPropertyFlags::PointerOwner),
    NS_SET_ACCESSOR_PROPERTY("StandardTypeSet", GetStandardTypeSet, StandardTypeSetInsert, StandardTypeSetRemove),
    NS_SET_MEMBER_PROPERTY("SubObjectSet", m_SubObjectSet)->AddFlags(nsPropertyFlags::PointerOwner),
    NS_MAP_MEMBER_PROPERTY("StandardTypeMap", m_StandardTypeMap),
    NS_MAP_MEMBER_PROPERTY("ClassMap", m_ClassMap),
    NS_MAP_MEMBER_PROPERTY("ClassPtrMap", m_ClassPtrMap)->AddFlags(nsPropertyFlags::PointerOwner),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMirrorTest, 1, nsRTTIDefaultAllocator<nsMirrorTest>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Math", m_math),
    NS_MEMBER_PROPERTY("Object", m_object),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsArrayPtr<const nsString> nsObjectTest::GetStandardTypeSet() const
{
  return m_StandardTypeSet;
}

void nsObjectTest::StandardTypeSetInsert(const nsString& value)
{
  if (!m_StandardTypeSet.Contains(value))
    m_StandardTypeSet.PushBack(value);
}

void nsObjectTest::StandardTypeSetRemove(const nsString& value)
{
  m_StandardTypeSet.RemoveAndCopy(value);
}
