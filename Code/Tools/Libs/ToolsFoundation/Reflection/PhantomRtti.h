/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

class nsPhantomRTTI : public nsRTTI
{
  friend class nsPhantomRttiManager;

public:
  ~nsPhantomRTTI();

private:
  nsPhantomRTTI(nsStringView sName, const nsRTTI* pParentType, nsUInt32 uiTypeSize, nsUInt32 uiTypeVersion, nsUInt8 uiVariantType,
    nsBitflags<nsTypeFlags> flags, nsStringView sPluginName);

  void SetProperties(nsDynamicArray<nsReflectedPropertyDescriptor>& properties);
  void SetFunctions(nsDynamicArray<nsReflectedFunctionDescriptor>& functions);
  void SetAttributes(nsDynamicArray<const nsPropertyAttribute*>& attributes);
  bool IsEqualToDescriptor(const nsReflectedTypeDescriptor& desc);

  void UpdateType(nsReflectedTypeDescriptor& desc);

private:
  nsString m_sTypeNameStorage;
  nsString m_sPluginNameStorage;
  nsDynamicArray<nsAbstractProperty*> m_PropertiesStorage;
  nsDynamicArray<nsAbstractFunctionProperty*> m_FunctionsStorage;
  nsDynamicArray<const nsPropertyAttribute*> m_AttributesStorage;
};
