#pragma once

#include <Foundation/Containers/Set.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/Enum.h>
#include <Foundation/Types/Id.h>
#include <Foundation/Types/Variant.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class nsRTTI;
class nsPhantomRttiManager;
class nsReflectedTypeStorageManager;

/// \brief Event message used by the nsPhantomRttiManager.
struct NS_TOOLSFOUNDATION_DLL nsPhantomTypeChange
{
  const nsRTTI* m_pChangedType = nullptr;
};

struct NS_TOOLSFOUNDATION_DLL nsAttributeHolder
{
  nsAttributeHolder();
  nsAttributeHolder(const nsAttributeHolder& rhs);
  virtual ~nsAttributeHolder();

  nsUInt32 GetCount() const;
  const nsPropertyAttribute* GetValue(nsUInt32 uiIndex) const;
  void SetValue(nsUInt32 uiIndex, const nsPropertyAttribute* value);
  void Insert(nsUInt32 uiIndex, const nsPropertyAttribute* value);
  void Remove(nsUInt32 uiIndex);

  void operator=(const nsAttributeHolder& rhs);

  mutable nsHybridArray<const nsPropertyAttribute*, 2> m_Attributes;
  nsArrayPtr<const nsPropertyAttribute* const> m_ReferenceAttributes;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_TOOLSFOUNDATION_DLL, nsAttributeHolder);

/// \brief Stores the description of a reflected property in a serializable form, used by nsReflectedTypeDescriptor.
struct NS_TOOLSFOUNDATION_DLL nsReflectedPropertyDescriptor : public nsAttributeHolder
{
  nsReflectedPropertyDescriptor() = default;
  nsReflectedPropertyDescriptor(nsPropertyCategory::Enum category, nsStringView sName, nsStringView sType, nsBitflags<nsPropertyFlags> flags);
  nsReflectedPropertyDescriptor(nsPropertyCategory::Enum category, nsStringView sName, nsStringView sType, nsBitflags<nsPropertyFlags> flags,
    nsArrayPtr<const nsPropertyAttribute* const> attributes); // [tested]
  /// \brief Initialize to a constant.
  nsReflectedPropertyDescriptor(nsStringView sName, const nsVariant& constantValue, nsArrayPtr<const nsPropertyAttribute* const> attributes); // [tested]
  nsReflectedPropertyDescriptor(const nsReflectedPropertyDescriptor& rhs);
  ~nsReflectedPropertyDescriptor();

  void operator=(const nsReflectedPropertyDescriptor& rhs);

  nsEnum<nsPropertyCategory> m_Category;
  nsString m_sName; ///< The name of this property. E.g. what nsAbstractProperty::GetPropertyName() returns.
  nsString m_sType; ///< The name of the type of the property. E.g. nsAbstractProperty::GetSpecificType().GetTypeName()

  nsBitflags<nsPropertyFlags> m_Flags;
  nsVariant m_ConstantValue;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_TOOLSFOUNDATION_DLL, nsReflectedPropertyDescriptor);

struct NS_TOOLSFOUNDATION_DLL nsFunctionArgumentDescriptor
{
  nsFunctionArgumentDescriptor();
  nsFunctionArgumentDescriptor(nsStringView sType, nsBitflags<nsPropertyFlags> flags);
  nsString m_sType;
  nsBitflags<nsPropertyFlags> m_Flags;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_TOOLSFOUNDATION_DLL, nsFunctionArgumentDescriptor);

/// \brief Stores the description of a reflected function in a serializable form, used by nsReflectedTypeDescriptor.
struct NS_TOOLSFOUNDATION_DLL nsReflectedFunctionDescriptor : public nsAttributeHolder
{
  nsReflectedFunctionDescriptor();
  nsReflectedFunctionDescriptor(nsStringView sName, nsBitflags<nsPropertyFlags> flags, nsEnum<nsFunctionType> type, nsArrayPtr<const nsPropertyAttribute* const> attributes);

  nsReflectedFunctionDescriptor(const nsReflectedFunctionDescriptor& rhs);
  ~nsReflectedFunctionDescriptor();

  void operator=(const nsReflectedFunctionDescriptor& rhs);

  nsString m_sName;
  nsBitflags<nsPropertyFlags> m_Flags;
  nsEnum<nsFunctionType> m_Type;
  nsFunctionArgumentDescriptor m_ReturnValue;
  nsDynamicArray<nsFunctionArgumentDescriptor> m_Arguments;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_TOOLSFOUNDATION_DLL, nsReflectedFunctionDescriptor);


/// \brief Stores the description of a reflected type in a serializable form. Used by nsPhantomRttiManager to add new types.
struct NS_TOOLSFOUNDATION_DLL nsReflectedTypeDescriptor : public nsAttributeHolder
{
  ~nsReflectedTypeDescriptor();

  nsString m_sTypeName;
  nsString m_sPluginName;
  nsString m_sParentTypeName;

  nsBitflags<nsTypeFlags> m_Flags;
  nsDynamicArray<nsReflectedPropertyDescriptor> m_Properties;
  nsDynamicArray<nsReflectedFunctionDescriptor> m_Functions;
  nsUInt32 m_uiTypeVersion = 1;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_TOOLSFOUNDATION_DLL, nsReflectedTypeDescriptor);
