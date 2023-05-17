#pragma once

#include <Foundation/Reflection/ReflectionUtils.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

class wdIReflectedTypeAccessor;
class wdDocumentObject;
class wdAbstractObjectGraph;

/// \brief Helper functions for handling reflection related operations.
///
/// Also check out wdToolsSerializationUtils for related functionality.
class WD_TOOLSFOUNDATION_DLL wdToolsReflectionUtils
{
public:
  /// \brief Returns the default value for the entire property as it is stored on the editor side.
  static wdVariant GetStorageDefault(const wdAbstractProperty* pProperty);

  static bool GetFloatFromVariant(const wdVariant& val, double& out_fValue);
  static bool GetVariantFromFloat(double fValue, wdVariantType::Enum type, wdVariant& out_val);

  /// \brief Creates a ReflectedTypeDescriptor from an wdRTTI instance that can be serialized and registered at the wdPhantomRttiManager.
  static void GetReflectedTypeDescriptorFromRtti(const wdRTTI* pRtti, wdReflectedTypeDescriptor& out_desc); // [tested]
  static void GetMinimalReflectedTypeDescriptorFromRtti(const wdRTTI* pRtti, wdReflectedTypeDescriptor& out_desc);

  static void GatherObjectTypes(const wdDocumentObject* pObject, wdSet<const wdRTTI*>& inout_types);

  static bool DependencySortTypeDescriptorArray(wdDynamicArray<wdReflectedTypeDescriptor*>& ref_descriptors);
};
