#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Reflection/Reflection.h>

/// \brief Add this attribute to a class to add script functions to the szTypeName class.
/// This might be necessary if the specified class is not reflected or to separate script functions from the specified class.
class NS_CORE_DLL nsScriptExtensionAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsScriptExtensionAttribute, nsPropertyAttribute);

public:
  nsScriptExtensionAttribute();
  nsScriptExtensionAttribute(nsStringView sTypeName);

  nsStringView GetTypeName() const { return m_sTypeName; }

private:
  nsUntrackedString m_sTypeName;
};

//////////////////////////////////////////////////////////////////////////

/// \brief Add this attribute to a script function to mark it as a base class function.
/// These are functions that can be entry points to visual scripts or over-writable functions in script languages like e.g. typescript.
class NS_CORE_DLL nsScriptBaseClassFunctionAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsScriptBaseClassFunctionAttribute, nsPropertyAttribute);

public:
  nsScriptBaseClassFunctionAttribute();
  nsScriptBaseClassFunctionAttribute(nsUInt16 uiIndex);

  nsUInt16 GetIndex() const { return m_uiIndex; }

private:
  nsUInt16 m_uiIndex;
};
