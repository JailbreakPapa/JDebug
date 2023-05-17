#pragma once

#include <Foundation/Containers/Set.h>
#include <Foundation/Reflection/Reflection.h>

class wdVariant;
class wdAbstractProperty;

/// \brief Helper functions for handling reflection related operations.
class WD_FOUNDATION_DLL wdReflectionUtils
{
public:
  static const wdRTTI* GetCommonBaseType(const wdRTTI* pRtti1, const wdRTTI* pRtti2);

  /// \brief Returns whether a type can be stored directly inside a wdVariant.
  static bool IsBasicType(const wdRTTI* pRtti);

  /// \brief Returns whether the property is a non-ptr basic type or custom type.
  static bool IsValueType(const wdAbstractProperty* pProp);

  /// \brief Returns the RTTI type matching the variant's type.
  static const wdRTTI* GetTypeFromVariant(const wdVariant& value);
  static const wdRTTI* GetTypeFromVariant(wdVariantType::Enum type);

  /// \brief Sets the Nth component of the vector to the given value.
  ///
  /// vector's type needs to be in between wdVariant::Type::Vector2 and wdVariant::Type::Vector4U.
  static wdUInt32 GetComponentCount(wdVariantType::Enum type);
  static void SetComponent(wdVariant& ref_vector, wdUInt32 uiComponent, double fValue); // [tested]
  static double GetComponent(const wdVariant& vector, wdUInt32 uiComponent);

  static wdVariant GetMemberPropertyValue(const wdAbstractMemberProperty* pProp, const void* pObject);        // [tested] via ToolsFoundation
  static void SetMemberPropertyValue(wdAbstractMemberProperty* pProp, void* pObject, const wdVariant& value); // [tested] via ToolsFoundation

  static wdVariant GetArrayPropertyValue(const wdAbstractArrayProperty* pProp, const void* pObject, wdUInt32 uiIndex);
  static void SetArrayPropertyValue(wdAbstractArrayProperty* pProp, void* pObject, wdUInt32 uiIndex, const wdVariant& value);

  static void InsertSetPropertyValue(wdAbstractSetProperty* pProp, void* pObject, const wdVariant& value);
  static void RemoveSetPropertyValue(wdAbstractSetProperty* pProp, void* pObject, const wdVariant& value);

  static wdVariant GetMapPropertyValue(const wdAbstractMapProperty* pProp, const void* pObject, const char* szKey);
  static void SetMapPropertyValue(wdAbstractMapProperty* pProp, void* pObject, const char* szKey, const wdVariant& value);

  static void InsertArrayPropertyValue(wdAbstractArrayProperty* pProp, void* pObject, const wdVariant& value, wdUInt32 uiIndex);
  static void RemoveArrayPropertyValue(wdAbstractArrayProperty* pProp, void* pObject, wdUInt32 uiIndex);

  static wdAbstractMemberProperty* GetMemberProperty(const wdRTTI* pRtti, wdUInt32 uiPropertyIndex);
  static wdAbstractMemberProperty* GetMemberProperty(const wdRTTI* pRtti, const char* szPropertyName); // [tested] via ToolsFoundation

  /// \brief Gathers all RTTI types that are derived from pRtti.
  ///
  /// This includes all classes that have pRtti as a base class, either direct or indirect.
  /// If bIncludeDependencies is set to true, the resulting set will also contain all dependent types.
  ///
  /// \sa GatherDependentTypes
  static void GatherTypesDerivedFromClass(const wdRTTI* pRtti, wdSet<const wdRTTI*>& out_types, bool bIncludeDependencies);

  /// \brief Gathers all RTTI types that pRtti depends on and adds them to inout_types.
  ///
  /// Dependencies are either member properties or base classes. The output contains the transitive closure of the dependencies.
  /// Note that inout_types is not cleared when this function is called.
  static void GatherDependentTypes(const wdRTTI* pRtti, wdSet<const wdRTTI*>& inout_types);

  /// \brief Sorts the input types according to their dependencies.
  ///
  /// Types that have no dependences come first in the output followed by types that have their dependencies met by
  /// the previous entries in the output.
  /// If circular dependencies are found the function returns false.
  static bool CreateDependencySortedTypeArray(const wdSet<const wdRTTI*>& types, wdDynamicArray<const wdRTTI*>& out_sortedTypes);

  struct EnumConversionMode
  {
    enum Enum
    {
      FullyQualifiedName,
      ValueNameOnly,
      Default = FullyQualifiedName
    };

    using StorageType = wdUInt8;
  };

  /// \brief Converts an enum or bitfield value into its string representation.
  ///
  /// The type of pEnumerationRtti will be automatically detected. The syntax of out_sOutput equals MSVC debugger output.
  static bool EnumerationToString(const wdRTTI* pEnumerationRtti, wdInt64 iValue, wdStringBuilder& out_sOutput,
    wdEnum<EnumConversionMode> conversionMode = EnumConversionMode::Default); // [tested]

  /// \brief Helper template to shorten the call for wdEnums
  template <typename T>
  static bool EnumerationToString(wdEnum<T> value, wdStringBuilder& out_sOutput, wdEnum<EnumConversionMode> conversionMode = EnumConversionMode::Default)
  {
    return EnumerationToString(wdGetStaticRTTI<T>(), value.GetValue(), out_sOutput, conversionMode);
  }

  /// \brief Helper template to shorten the call for wdBitflags
  template <typename T>
  static bool BitflagsToString(wdBitflags<T> value, wdStringBuilder& out_sOutput, wdEnum<EnumConversionMode> conversionMode = EnumConversionMode::Default)
  {
    return EnumerationToString(wdGetStaticRTTI<T>(), value.GetValue(), out_sOutput, conversionMode);
  }

  struct EnumKeyValuePair
  {
    wdString m_sKey;
    wdInt32 m_iValue = 0;
  };

  /// \brief If the given type is an enum, \a entries will be filled with all available keys (strings) and values (integers).
  static void GetEnumKeysAndValues(const wdRTTI* pEnumerationRtti, wdDynamicArray<EnumKeyValuePair>& ref_entries, wdEnum<EnumConversionMode> conversionMode = EnumConversionMode::Default);

  /// \brief Converts an enum or bitfield in its string representation to its value.
  ///
  /// The type of pEnumerationRtti will be automatically detected. The syntax of szValue must equal the MSVC debugger output.
  static bool StringToEnumeration(const wdRTTI* pEnumerationRtti, const char* szValue, wdInt64& out_iValue); // [tested]

  /// \brief Helper template to shorten the call for wdEnums
  template <typename T>
  static bool StringToEnumeration(const char* szValue, wdEnum<T>& out_value)
  {
    wdInt64 value;
    const auto retval = StringToEnumeration(wdGetStaticRTTI<T>(), szValue, value);
    out_value = static_cast<typename T::Enum>(value);
    return retval;
  }

  /// \brief Returns the default value (Enum::Default) for the given enumeration type.
  static wdInt64 DefaultEnumerationValue(const wdRTTI* pEnumerationRtti); // [tested]

  /// \brief Makes sure the given value is valid under the given enumeration type.
  ///
  /// Invalid bitflag bits are removed and an invalid enum value is replaced by the default value.
  static wdInt64 MakeEnumerationValid(const wdRTTI* pEnumerationRtti, wdInt64 iValue); // [tested]

  /// \brief Templated convenience function that calls IsEqual and automatically deduces the type.
  template <typename T>
  static bool IsEqual(const T* pObject, const T* pObject2)
  {
    return IsEqual(pObject, pObject2, wdGetStaticRTTI<T>());
  }

  /// \brief Compares pObject with pObject2 of type pType and returns whether they are equal.
  ///
  /// In case a class derived from wdReflectedClass is passed in the correct derived type
  /// will automatically be determined so it is not necessary to put the exact type into pType,
  /// any derived class type will do. However, the function will return false  pObject and pObject2
  /// actually have a different type.
  static bool IsEqual(const void* pObject, const void* pObject2, const wdRTTI* pType); // [tested]

  /// \brief Compares property pProp of pObject and pObject2 and returns whether it is equal in both.
  static bool IsEqual(const void* pObject, const void* pObject2, wdAbstractProperty* pProp);

  /// \brief Deletes pObject using the allocator found in the owning property's type.
  static void DeleteObject(void* pObject, wdAbstractProperty* pOwnerProperty);

  /// \brief Returns a global default initialization value for the given variant type.
  static wdVariant GetDefaultVariantFromType(wdVariant::Type::Enum type); // [tested]

  /// \brief Returns the default value for the specific type
  static wdVariant GetDefaultVariantFromType(const wdRTTI* pRtti);

  /// \brief Returns the default value for the specific type of the given property.
  static wdVariant GetDefaultValue(const wdAbstractProperty* pProperty, wdVariant index = wdVariant());


  /// \brief Sets all member properties in \a pObject of type \a pRtti to the value returned by wdToolsReflectionUtils::GetDefaultValue()
  static void SetAllMemberPropertiesToDefault(const wdRTTI* pRtti, void* pObject);

  /// \brief If pAttrib is valid and its min/max values are compatible, value will be clamped to them.
  /// Returns false if a clamp attribute exists but no clamp code was executed.
  static wdResult ClampValue(wdVariant& value, const wdClampValueAttribute* pAttrib);
};
