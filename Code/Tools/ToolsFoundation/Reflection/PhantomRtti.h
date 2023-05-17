#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

class wdPhantomRTTI : public wdRTTI
{
  friend class wdPhantomRttiManager;

public:
  ~wdPhantomRTTI();

private:
  wdPhantomRTTI(const char* szName, const wdRTTI* pParentType, wdUInt32 uiTypeSize, wdUInt32 uiTypeVersion, wdUInt32 uiVariantType,
    wdBitflags<wdTypeFlags> flags, const char* szPluginName);

  void SetProperties(wdDynamicArray<wdReflectedPropertyDescriptor>& properties);
  void SetFunctions(wdDynamicArray<wdReflectedFunctionDescriptor>& functions);
  void SetAttributes(wdHybridArray<wdPropertyAttribute*, 2>& attributes);
  bool IsEqualToDescriptor(const wdReflectedTypeDescriptor& desc);

  void UpdateType(wdReflectedTypeDescriptor& desc);

private:
  wdString m_sTypeNameStorage;
  wdString m_sPluginNameStorage;
  wdDynamicArray<wdAbstractProperty*> m_PropertiesStorage;
  wdDynamicArray<wdAbstractFunctionProperty*> m_FunctionsStorage;
  wdDynamicArray<wdPropertyAttribute*> m_AttributesStorage;
};
