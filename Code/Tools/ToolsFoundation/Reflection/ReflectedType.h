#pragma once

#include <Foundation/Containers/Set.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/Enum.h>
#include <Foundation/Types/Id.h>
#include <Foundation/Types/Variant.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class wdRTTI;
class wdPhantomRttiManager;
class wdReflectedTypeStorageManager;

/// \brief Event message used by the wdPhantomRttiManager.
struct WD_TOOLSFOUNDATION_DLL wdPhantomTypeChange
{
  const wdRTTI* m_pChangedType;
};

struct WD_TOOLSFOUNDATION_DLL wdAttributeHolder
{
  wdAttributeHolder();
  wdAttributeHolder(const wdAttributeHolder& rhs);
  virtual ~wdAttributeHolder();

  wdUInt32 GetCount() const;
  wdPropertyAttribute* GetValue(wdUInt32 uiIndex) const;
  void SetValue(wdUInt32 uiIndex, wdPropertyAttribute* value);
  void Insert(wdUInt32 uiIndex, wdPropertyAttribute* value);
  void Remove(wdUInt32 uiIndex);

  void operator=(const wdAttributeHolder& rhs);

  mutable wdHybridArray<wdPropertyAttribute*, 2> m_Attributes;
  wdArrayPtr<wdPropertyAttribute* const> m_ReferenceAttributes;
};
WD_DECLARE_REFLECTABLE_TYPE(WD_TOOLSFOUNDATION_DLL, wdAttributeHolder);

/// \brief Stores the description of a reflected property in a serializable form, used by wdReflectedTypeDescriptor.
struct WD_TOOLSFOUNDATION_DLL wdReflectedPropertyDescriptor : public wdAttributeHolder
{
  wdReflectedPropertyDescriptor() {}
  wdReflectedPropertyDescriptor(wdPropertyCategory::Enum category, const char* szName, const char* szType, wdBitflags<wdPropertyFlags> flags);
  wdReflectedPropertyDescriptor(wdPropertyCategory::Enum category, const char* szName, const char* szType, wdBitflags<wdPropertyFlags> flags,
    const wdArrayPtr<wdPropertyAttribute* const> attributes); // [tested]
  /// \brief Initialize to a constant.
  wdReflectedPropertyDescriptor(
    const char* szName, const wdVariant& constantValue, const wdArrayPtr<wdPropertyAttribute* const> attributes); // [tested]
  wdReflectedPropertyDescriptor(const wdReflectedPropertyDescriptor& rhs);
  ~wdReflectedPropertyDescriptor();

  void operator=(const wdReflectedPropertyDescriptor& rhs);

  wdEnum<wdPropertyCategory> m_Category;
  wdString m_sName; ///< The name of this property. E.g. what wdAbstractProperty::GetPropertyName() returns.
  wdString m_sType; ///< The name of the type of the property. E.g. wdAbstractProperty::GetSpecificType().GetTypeName()

  wdBitflags<wdPropertyFlags> m_Flags;
  wdVariant m_ConstantValue;
};
WD_DECLARE_REFLECTABLE_TYPE(WD_TOOLSFOUNDATION_DLL, wdReflectedPropertyDescriptor);

struct WD_TOOLSFOUNDATION_DLL wdFunctionArgumentDescriptor
{
  wdFunctionArgumentDescriptor();
  wdFunctionArgumentDescriptor(const char* szType, wdBitflags<wdPropertyFlags> flags);
  wdString m_sType;
  wdBitflags<wdPropertyFlags> m_Flags;
};
WD_DECLARE_REFLECTABLE_TYPE(WD_TOOLSFOUNDATION_DLL, wdFunctionArgumentDescriptor);

/// \brief Stores the description of a reflected function in a serializable form, used by wdReflectedTypeDescriptor.
struct WD_TOOLSFOUNDATION_DLL wdReflectedFunctionDescriptor : public wdAttributeHolder
{
  wdReflectedFunctionDescriptor();
  wdReflectedFunctionDescriptor(
    const char* szName, wdBitflags<wdPropertyFlags> flags, wdEnum<wdFunctionType> type, const wdArrayPtr<wdPropertyAttribute* const> attributes);

  wdReflectedFunctionDescriptor(const wdReflectedFunctionDescriptor& rhs);
  ~wdReflectedFunctionDescriptor();

  void operator=(const wdReflectedFunctionDescriptor& rhs);

  wdString m_sName;
  wdBitflags<wdPropertyFlags> m_Flags;
  wdEnum<wdFunctionType> m_Type;
  wdFunctionArgumentDescriptor m_ReturnValue;
  wdDynamicArray<wdFunctionArgumentDescriptor> m_Arguments;
};
WD_DECLARE_REFLECTABLE_TYPE(WD_TOOLSFOUNDATION_DLL, wdReflectedFunctionDescriptor);


/// \brief Stores the description of a reflected type in a serializable form. Used by wdPhantomRttiManager to add new types.
struct WD_TOOLSFOUNDATION_DLL wdReflectedTypeDescriptor : public wdAttributeHolder
{
  ~wdReflectedTypeDescriptor();

  wdString m_sTypeName;
  wdString m_sPluginName;
  wdString m_sParentTypeName;

  wdBitflags<wdTypeFlags> m_Flags;
  wdDynamicArray<wdReflectedPropertyDescriptor> m_Properties;
  wdDynamicArray<wdReflectedFunctionDescriptor> m_Functions;
  wdUInt32 m_uiTypeVersion = 1;
};
WD_DECLARE_REFLECTABLE_TYPE(WD_TOOLSFOUNDATION_DLL, wdReflectedTypeDescriptor);
