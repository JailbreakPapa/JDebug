#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Reflection/Reflection.h>

class NS_CORE_DLL nsScriptExtensionClass_Log
{
public:
  static void Info(nsStringView sText, const nsVariantArray& params);
  static void Warning(nsStringView sText, const nsVariantArray& params);
  static void Error(nsStringView sText, const nsVariantArray& params);
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsScriptExtensionClass_Log);
