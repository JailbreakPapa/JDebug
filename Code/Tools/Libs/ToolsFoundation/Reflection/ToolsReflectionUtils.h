/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Foundation/Reflection/ReflectionUtils.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

class nsIReflectedTypeAccessor;
class nsDocumentObject;
class nsAbstractObjectGraph;

/// \brief Helper functions for handling reflection related operations.
///
/// Also check out nsToolsSerializationUtils for related functionality.
class NS_TOOLSFOUNDATION_DLL nsToolsReflectionUtils
{
public:
  /// \brief Returns the default value for the entire property as it is stored on the editor side.
  static nsVariant GetStorageDefault(const nsAbstractProperty* pProperty);

  static bool GetFloatFromVariant(const nsVariant& val, double& out_fValue);
  static bool GetVariantFromFloat(double fValue, nsVariantType::Enum type, nsVariant& out_val);

  /// \brief Creates a ReflectedTypeDescriptor from an nsRTTI instance that can be serialized and registered at the nsPhantomRttiManager.
  static void GetReflectedTypeDescriptorFromRtti(const nsRTTI* pRtti, nsReflectedTypeDescriptor& out_desc); // [tested]
  static void GetMinimalReflectedTypeDescriptorFromRtti(const nsRTTI* pRtti, nsReflectedTypeDescriptor& out_desc);

  static void GatherObjectTypes(const nsDocumentObject* pObject, nsSet<const nsRTTI*>& inout_types);

  static bool DependencySortTypeDescriptorArray(nsDynamicArray<nsReflectedTypeDescriptor*>& ref_descriptors);
};
