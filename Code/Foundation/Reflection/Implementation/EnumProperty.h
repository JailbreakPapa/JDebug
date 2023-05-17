#pragma once

/// \file

#include <Foundation/Reflection/Implementation/MemberProperty.h>
#include <Foundation/Reflection/Implementation/StaticRTTI.h>

/// \brief The base class for enum and bitflags member properties.
///
/// Cast any property whose type derives from wdEnumBase or wdBitflagsBase class to access its value.
class wdAbstractEnumerationProperty : public wdAbstractMemberProperty
{
public:
  /// \brief Passes the property name through to wdAbstractMemberProperty.
  wdAbstractEnumerationProperty(const char* szPropertyName)
    : wdAbstractMemberProperty(szPropertyName)
  {
  }

  /// \brief Returns the value of the property. Pass the instance pointer to the surrounding class along.
  virtual wdInt64 GetValue(const void* pInstance) const = 0;

  /// \brief Modifies the value of the property. Pass the instance pointer to the surrounding class along.
  ///
  /// \note Make sure the property is not read-only before calling this, otherwise an assert will fire.
  virtual void SetValue(void* pInstance, wdInt64 value) = 0;

  virtual void GetValuePtr(const void* pInstance, void* pObject) const override
  {
    *static_cast<wdInt64*>(pObject) = GetValue(pInstance);
  }

  virtual void SetValuePtr(void* pInstance, const void* pObject) override
  {
    SetValue(pInstance, *static_cast<const wdInt64*>(pObject));
  }
};


/// \brief [internal] Base class for enum / bitflags properties that already defines the type.
template <typename EnumType>
class wdTypedEnumProperty : public wdAbstractEnumerationProperty
{
public:
  /// \brief Passes the property name through to wdAbstractEnumerationProperty.
  wdTypedEnumProperty(const char* szPropertyName)
    : wdAbstractEnumerationProperty(szPropertyName)
  {
  }

  /// \brief Returns the actual type of the property. You can then test whether it derives from wdEnumBase or
  ///  wdBitflagsBase to determine whether we are dealing with an enum or bitflags property.
  virtual const wdRTTI* GetSpecificType() const override // [tested]
  {
    return wdGetStaticRTTI<typename wdTypeTraits<EnumType>::NonConstReferenceType>();
  }
};


/// \brief [internal] An implementation of wdTypedEnumProperty that uses custom getter / setter functions to access an enum property.
template <typename Class, typename EnumType, typename Type>
class wdEnumAccessorProperty : public wdTypedEnumProperty<EnumType>
{
public:
  using RealType = typename wdTypeTraits<Type>::NonConstReferenceType;
  using GetterFunc = Type (Class::*)() const;
  using SetterFunc = void (Class::*)(Type value);

  /// \brief Constructor.
  wdEnumAccessorProperty(const char* szPropertyName, GetterFunc getter, SetterFunc setter)
    : wdTypedEnumProperty<EnumType>(szPropertyName)
  {
    WD_ASSERT_DEBUG(getter != nullptr, "The getter of a property cannot be nullptr.");
    wdAbstractMemberProperty::m_Flags.Add(wdPropertyFlags::IsEnum);

    m_Getter = getter;
    m_Setter = setter;

    if (m_Setter == nullptr)
      wdAbstractMemberProperty::m_Flags.Add(wdPropertyFlags::ReadOnly);
  }

  virtual void* GetPropertyPointer(const void* pInstance) const override
  {
    // No access to sub-properties, if we have accessors for this property
    return nullptr;
  }

  virtual wdInt64 GetValue(const void* pInstance) const override // [tested]
  {
    wdEnum<EnumType> enumTemp = (static_cast<const Class*>(pInstance)->*m_Getter)();
    return enumTemp.GetValue();
  }

  virtual void SetValue(void* pInstance, wdInt64 value) override // [tested]
  {
    WD_ASSERT_DEV(m_Setter != nullptr, "The property '{0}' has no setter function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    if (m_Setter)
      (static_cast<Class*>(pInstance)->*m_Setter)((typename EnumType::Enum)value);
  }

private:
  GetterFunc m_Getter;
  SetterFunc m_Setter;
};


/// \brief [internal] An implementation of wdTypedEnumProperty that accesses the enum property data directly.
template <typename Class, typename EnumType, typename Type>
class wdEnumMemberProperty : public wdTypedEnumProperty<EnumType>
{
public:
  using GetterFunc = Type (*)(const Class* pInstance);
  using SetterFunc = void (*)(Class* pInstance, Type value);
  using PointerFunc = void* (*)(const Class* pInstance);

  /// \brief Constructor.
  wdEnumMemberProperty(const char* szPropertyName, GetterFunc getter, SetterFunc setter, PointerFunc pointer)
    : wdTypedEnumProperty<EnumType>(szPropertyName)
  {
    WD_ASSERT_DEBUG(getter != nullptr, "The getter of a property cannot be nullptr.");
    wdAbstractMemberProperty::m_Flags.Add(wdPropertyFlags::IsEnum);

    m_Getter = getter;
    m_Setter = setter;
    m_Pointer = pointer;

    if (m_Setter == nullptr)
      wdAbstractMemberProperty::m_Flags.Add(wdPropertyFlags::ReadOnly);
  }

  virtual void* GetPropertyPointer(const void* pInstance) const override { return m_Pointer(static_cast<const Class*>(pInstance)); }

  virtual wdInt64 GetValue(const void* pInstance) const override // [tested]
  {
    wdEnum<EnumType> enumTemp = m_Getter(static_cast<const Class*>(pInstance));
    return enumTemp.GetValue();
  }

  virtual void SetValue(void* pInstance, wdInt64 value) override // [tested]
  {
    WD_ASSERT_DEV(m_Setter != nullptr, "The property '{0}' has no setter function, thus it is read-only.", wdAbstractProperty::GetPropertyName());

    if (m_Setter)
      m_Setter(static_cast<Class*>(pInstance), (typename EnumType::Enum)value);
  }

private:
  GetterFunc m_Getter;
  SetterFunc m_Setter;
  PointerFunc m_Pointer;
};
