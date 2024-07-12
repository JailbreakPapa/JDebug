#pragma once

#include <Foundation/Containers/Set.h>
#include <Foundation/Reflection/Reflection.h>

class nsVariant;
class nsAbstractProperty;

/// \brief Helper functions for handling reflection related operations.
class NS_FOUNDATION_DLL nsReflectionUtils
{
public:
  static const nsRTTI* GetCommonBaseType(const nsRTTI* pRtti1, const nsRTTI* pRtti2);

  /// \brief Returns whether a type can be stored directly inside a nsVariant.
  static bool IsBasicType(const nsRTTI* pRtti);

  /// \brief Returns whether the property is a non-ptr basic type or custom type.
  static bool IsValueType(const nsAbstractProperty* pProp);

  /// \brief Returns the RTTI type matching the variant's type.
  static const nsRTTI* GetTypeFromVariant(const nsVariant& value);
  static const nsRTTI* GetTypeFromVariant(nsVariantType::Enum type);

  /// \brief Sets the Nth component of the vector to the given value.
  ///
  /// vector's type needs to be in between nsVariant::Type::Vector2 and nsVariant::Type::Vector4U.
  static nsUInt32 GetComponentCount(nsVariantType::Enum type);
  static void SetComponent(nsVariant& ref_vector, nsUInt32 uiComponent, double fValue);                             // [tested]
  static double GetComponent(const nsVariant& vector, nsUInt32 uiComponent);

  static nsVariant GetMemberPropertyValue(const nsAbstractMemberProperty* pProp, const void* pObject);              // [tested] via ToolsFoundation
  static void SetMemberPropertyValue(const nsAbstractMemberProperty* pProp, void* pObject, const nsVariant& value); // [tested] via ToolsFoundation

  static nsVariant GetArrayPropertyValue(const nsAbstractArrayProperty* pProp, const void* pObject, nsUInt32 uiIndex);
  static void SetArrayPropertyValue(const nsAbstractArrayProperty* pProp, void* pObject, nsUInt32 uiIndex, const nsVariant& value);

  static void InsertSetPropertyValue(const nsAbstractSetProperty* pProp, void* pObject, const nsVariant& value);
  static void RemoveSetPropertyValue(const nsAbstractSetProperty* pProp, void* pObject, const nsVariant& value);

  static nsVariant GetMapPropertyValue(const nsAbstractMapProperty* pProp, const void* pObject, const char* szKey);
  static void SetMapPropertyValue(const nsAbstractMapProperty* pProp, void* pObject, const char* szKey, const nsVariant& value);

  static void InsertArrayPropertyValue(const nsAbstractArrayProperty* pProp, void* pObject, const nsVariant& value, nsUInt32 uiIndex);
  static void RemoveArrayPropertyValue(const nsAbstractArrayProperty* pProp, void* pObject, nsUInt32 uiIndex);

  static const nsAbstractMemberProperty* GetMemberProperty(const nsRTTI* pRtti, nsUInt32 uiPropertyIndex);
  static const nsAbstractMemberProperty* GetMemberProperty(const nsRTTI* pRtti, const char* szPropertyName); // [tested] via ToolsFoundation

  /// \brief Gathers all RTTI types that are derived from pRtti.
  ///
  /// This includes all classes that have pRtti as a base class, either direct or indirect.
  ///
  /// \sa GatherDependentTypes
  static void GatherTypesDerivedFromClass(const nsRTTI* pRtti, nsSet<const nsRTTI*>& out_types);

  /// \brief Gathers all RTTI types that pRtti depends on and adds them to inout_types.
  ///
  /// Dependencies are either member properties or base classes. The output contains the transitive closure of the dependencies.
  /// Note that inout_typesAsSet is not cleared when this function is called.
  /// out_pTypesAsStack is all the dependencies sorted by their appearance in the dependency chain.
  /// The last entry is the lowest in the chain and has no dependencies on its own.
  static void GatherDependentTypes(const nsRTTI* pRtti, nsSet<const nsRTTI*>& inout_typesAsSet, nsDynamicArray<const nsRTTI*>* out_pTypesAsStack = nullptr);

  /// \brief Sorts the input types according to their dependencies.
  ///
  /// Types that have no dependences come first in the output followed by types that have their dependencies met by
  /// the previous entries in the output.
  /// If a dependent type is not in the given types set the function will fail.
  static nsResult CreateDependencySortedTypeArray(const nsSet<const nsRTTI*>& types, nsDynamicArray<const nsRTTI*>& out_sortedTypes);

  struct EnumConversionMode
  {
    enum Enum
    {
      FullyQualifiedName,
      ValueNameOnly,
      Default = FullyQualifiedName
    };

    using StorageType = nsUInt8;
  };

  /// \brief Converts an enum or bitfield value into its string representation.
  ///
  /// The type of pEnumerationRtti will be automatically detected. The syntax of out_sOutput equals MSVC debugger output.
  static bool EnumerationToString(const nsRTTI* pEnumerationRtti, nsInt64 iValue, nsStringBuilder& out_sOutput,
    nsEnum<EnumConversionMode> conversionMode = EnumConversionMode::Default); // [tested]

  /// \brief Helper template to shorten the call for nsEnums
  template <typename T>
  static bool EnumerationToString(nsEnum<T> value, nsStringBuilder& out_sOutput, nsEnum<EnumConversionMode> conversionMode = EnumConversionMode::Default)
  {
    return EnumerationToString(nsGetStaticRTTI<T>(), value.GetValue(), out_sOutput, conversionMode);
  }

  /// \brief Helper template to shorten the call for nsBitflags
  template <typename T>
  static bool BitflagsToString(nsBitflags<T> value, nsStringBuilder& out_sOutput, nsEnum<EnumConversionMode> conversionMode = EnumConversionMode::Default)
  {
    return EnumerationToString(nsGetStaticRTTI<T>(), value.GetValue(), out_sOutput, conversionMode);
  }

  struct EnumKeyValuePair
  {
    nsString m_sKey;
    nsInt32 m_iValue = 0;
  };

  /// \brief If the given type is an enum, \a entries will be filled with all available keys (strings) and values (integers).
  static void GetEnumKeysAndValues(const nsRTTI* pEnumerationRtti, nsDynamicArray<EnumKeyValuePair>& ref_entries, nsEnum<EnumConversionMode> conversionMode = EnumConversionMode::Default);

  /// \brief Converts an enum or bitfield in its string representation to its value.
  ///
  /// The type of pEnumerationRtti will be automatically detected. The syntax of szValue must equal the MSVC debugger output.
  static bool StringToEnumeration(const nsRTTI* pEnumerationRtti, const char* szValue, nsInt64& out_iValue); // [tested]

  /// \brief Helper template to shorten the call for nsEnums
  template <typename T>
  static bool StringToEnumeration(const char* szValue, nsEnum<T>& out_value)
  {
    nsInt64 value;
    const auto retval = StringToEnumeration(nsGetStaticRTTI<T>(), szValue, value);
    out_value = static_cast<typename T::Enum>(value);
    return retval;
  }

  /// \brief Returns the default value (Enum::Default) for the given enumeration type.
  static nsInt64 DefaultEnumerationValue(const nsRTTI* pEnumerationRtti); // [tested]

  /// \brief Makes sure the given value is valid under the given enumeration type.
  ///
  /// Invalid bitflag bits are removed and an invalid enum value is replaced by the default value.
  static nsInt64 MakeEnumerationValid(const nsRTTI* pEnumerationRtti, nsInt64 iValue); // [tested]

  /// \brief Templated convenience function that calls IsEqual and automatically deduces the type.
  template <typename T>
  static bool IsEqual(const T* pObject, const T* pObject2)
  {
    return IsEqual(pObject, pObject2, nsGetStaticRTTI<T>());
  }

  /// \brief Compares pObject with pObject2 of type pType and returns whether they are equal.
  ///
  /// In case a class derived from nsReflectedClass is passed in the correct derived type
  /// will automatically be determined so it is not necessary to put the exact type into pType,
  /// any derived class type will do. However, the function will return false  pObject and pObject2
  /// actually have a different type.
  static bool IsEqual(const void* pObject, const void* pObject2, const nsRTTI* pType); // [tested]

  /// \brief Compares property pProp of pObject and pObject2 and returns whether it is equal in both.
  static bool IsEqual(const void* pObject, const void* pObject2, const nsAbstractProperty* pProp);

  /// \brief Deletes pObject using the allocator found in the owning property's type.
  static void DeleteObject(void* pObject, const nsAbstractProperty* pOwnerProperty);

  /// \brief Returns a global default initialization value for the given variant type.
  static nsVariant GetDefaultVariantFromType(nsVariant::Type::Enum type); // [tested]

  /// \brief Returns the default value for the specific type
  static nsVariant GetDefaultVariantFromType(const nsRTTI* pRtti);

  /// \brief Returns the default value for the specific type of the given property.
  static nsVariant GetDefaultValue(const nsAbstractProperty* pProperty, nsVariant index = nsVariant());


  /// \brief Sets all member properties in \a pObject of type \a pRtti to the value returned by nsToolsReflectionUtils::GetDefaultValue()
  static void SetAllMemberPropertiesToDefault(const nsRTTI* pRtti, void* pObject);

  /// \brief If pAttrib is valid and its min/max values are compatible, value will be clamped to them.
  /// Returns false if a clamp attribute exists but no clamp code was executed.
  static nsResult ClampValue(nsVariant& value, const nsClampValueAttribute* pAttrib);
};
