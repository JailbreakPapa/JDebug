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
  /// \brief Returns the type under which the property is stored on the editor side.
  static nsVariantType::Enum GetStorageType(const nsAbstractProperty* pProperty);

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
