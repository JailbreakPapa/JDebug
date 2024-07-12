#pragma once

/// \file

#include <Foundation/Reflection/Implementation/MemberProperty.h>
#include <Foundation/Reflection/Implementation/StaticRTTI.h>

/// \brief The base class for enum and bitflags member properties.
///
/// Cast any property whose type derives from nsEnumBase or nsBitflagsBase class to access its value.
class nsAbstractEnumerationProperty : public nsAbstractMemberProperty
{
public:
  /// \brief Passes the property name through to nsAbstractMemberProperty.
  nsAbstractEnumerationProperty(const char* szPropertyName)
    : nsAbstractMemberProperty(szPropertyName)
  {
  }

  /// \brief Returns the value of the property. Pass the instance pointer to the surrounding class along.
  virtual nsInt64 GetValue(const void* pInstance) const = 0;

  /// \brief Modifies the value of the property. Pass the instance pointer to the surrounding class along.
  ///
  /// \note Make sure the property is not read-only before calling this, otherwise an assert will fire.
  virtual void SetValue(void* pInstance, nsInt64 value) const = 0;

  virtual void GetValuePtr(const void* pInstance, void* pObject) const override
  {
    *static_cast<nsInt64*>(pObject) = GetValue(pInstance);
  }

  virtual void SetValuePtr(void* pInstance, const void* pObject) const override
  {
    SetValue(pInstance, *static_cast<const nsInt64*>(pObject));
  }
};


/// \brief [internal] Base class for enum / bitflags properties that already defines the type.
template <typename EnumType>
class nsTypedEnumProperty : public nsAbstractEnumerationProperty
{
public:
  /// \brief Passes the property name through to nsAbstractEnumerationProperty.
  nsTypedEnumProperty(const char* szPropertyName)
    : nsAbstractEnumerationProperty(szPropertyName)
  {
  }

  /// \brief Returns the actual type of the property. You can then test whether it derives from nsEnumBase or
  ///  nsBitflagsBase to determine whether we are dealing with an enum or bitflags property.
  virtual const nsRTTI* GetSpecificType() const override // [tested]
  {
    return nsGetStaticRTTI<typename nsTypeTraits<EnumType>::NonConstReferenceType>();
  }
};


/// \brief [internal] An implementation of nsTypedEnumProperty that uses custom getter / setter functions to access an enum property.
template <typename Class, typename EnumType, typename Type>
class nsEnumAccessorProperty : public nsTypedEnumProperty<EnumType>
{
public:
  using RealType = typename nsTypeTraits<Type>::NonConstReferenceType;
  using GetterFunc = Type (Class::*)() const;
  using SetterFunc = void (Class::*)(Type value);

  /// \brief Constructor.
  nsEnumAccessorProperty(const char* szPropertyName, GetterFunc getter, SetterFunc setter)
    : nsTypedEnumProperty<EnumType>(szPropertyName)
  {
    NS_ASSERT_DEBUG(getter != nullptr, "The getter of a property cannot be nullptr.");
    nsAbstractMemberProperty::m_Flags.Add(nsPropertyFlags::IsEnum);

    m_Getter = getter;
    m_Setter = setter;

    if (m_Setter == nullptr)
      nsAbstractMemberProperty::m_Flags.Add(nsPropertyFlags::ReadOnly);
  }

  virtual void* GetPropertyPointer(const void* pInstance) const override
  {
    // No access to sub-properties, if we have accessors for this property
    return nullptr;
  }

  virtual nsInt64 GetValue(const void* pInstance) const override // [tested]
  {
    nsEnum<EnumType> enumTemp = (static_cast<const Class*>(pInstance)->*m_Getter)();
    return enumTemp.GetValue();
  }

  virtual void SetValue(void* pInstance, nsInt64 value) const override // [tested]
  {
    NS_ASSERT_DEV(m_Setter != nullptr, "The property '{0}' has no setter function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    if (m_Setter)
      (static_cast<Class*>(pInstance)->*m_Setter)((typename EnumType::Enum)value);
  }

private:
  GetterFunc m_Getter;
  SetterFunc m_Setter;
};


/// \brief [internal] An implementation of nsTypedEnumProperty that accesses the enum property data directly.
template <typename Class, typename EnumType, typename Type>
class nsEnumMemberProperty : public nsTypedEnumProperty<EnumType>
{
public:
  using GetterFunc = Type (*)(const Class* pInstance);
  using SetterFunc = void (*)(Class* pInstance, Type value);
  using PointerFunc = void* (*)(const Class* pInstance);

  /// \brief Constructor.
  nsEnumMemberProperty(const char* szPropertyName, GetterFunc getter, SetterFunc setter, PointerFunc pointer)
    : nsTypedEnumProperty<EnumType>(szPropertyName)
  {
    NS_ASSERT_DEBUG(getter != nullptr, "The getter of a property cannot be nullptr.");
    nsAbstractMemberProperty::m_Flags.Add(nsPropertyFlags::IsEnum);

    m_Getter = getter;
    m_Setter = setter;
    m_Pointer = pointer;

    if (m_Setter == nullptr)
      nsAbstractMemberProperty::m_Flags.Add(nsPropertyFlags::ReadOnly);
  }

  virtual void* GetPropertyPointer(const void* pInstance) const override { return m_Pointer(static_cast<const Class*>(pInstance)); }

  virtual nsInt64 GetValue(const void* pInstance) const override // [tested]
  {
    nsEnum<EnumType> enumTemp = m_Getter(static_cast<const Class*>(pInstance));
    return enumTemp.GetValue();
  }

  virtual void SetValue(void* pInstance, nsInt64 value) const override // [tested]
  {
    NS_ASSERT_DEV(m_Setter != nullptr, "The property '{0}' has no setter function, thus it is read-only.", nsAbstractProperty::GetPropertyName());

    if (m_Setter)
      m_Setter(static_cast<Class*>(pInstance), (typename EnumType::Enum)value);
  }

private:
  GetterFunc m_Getter;
  SetterFunc m_Setter;
  PointerFunc m_Pointer;
};
