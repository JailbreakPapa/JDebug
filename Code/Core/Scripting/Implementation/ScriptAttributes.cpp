#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptAttributes.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsScriptExtensionAttribute, 1, nsRTTIDefaultAllocator<nsScriptExtensionAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("TypeName", m_sTypeName),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsScriptExtensionAttribute::nsScriptExtensionAttribute() = default;
nsScriptExtensionAttribute::nsScriptExtensionAttribute(nsStringView sTypeName)
  : m_sTypeName(sTypeName)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsScriptBaseClassFunctionAttribute, 1, nsRTTIDefaultAllocator<nsScriptBaseClassFunctionAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Index", m_uiIndex),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsScriptBaseClassFunctionAttribute::nsScriptBaseClassFunctionAttribute() = default;
nsScriptBaseClassFunctionAttribute::nsScriptBaseClassFunctionAttribute(nsUInt16 uiIndex)
  : m_uiIndex(uiIndex)
{
}


NS_STATICLINK_FILE(Core, Core_Scripting_Implementation_ScriptAttributes);
