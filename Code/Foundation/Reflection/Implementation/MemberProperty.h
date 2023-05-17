#pragma once

/// \file

#include <Foundation/Reflection/Implementation/AbstractProperty.h>
#include <Foundation/Reflection/Implementation/StaticRTTI.h>
#include <Foundation/Types/Variant.h>

// ***********************************************
// ***** Base class for accessing properties *****


/// \brief The base class for all typed member properties. I.e. once the type of a property is determined, it can be cast to the proper
/// version of this.
///
/// For example, when you have a pointer to an wdAbstractMemberProperty and it returns that the property is of type 'int', you can cast the
/// pointer to an pointer to wdTypedMemberProperty<int> which then allows you to access its values.
template <typename Type>
class wdTypedMemberProperty : public wdAbstractMemberProperty
{
public:
  /// \brief Passes the property name through to wdAbstractMemberProperty.
  wdTypedMemberProperty(const char* szPropertyName)
    : wdAbstractMemberProperty(szPropertyName)
  {
    m_Flags = wdPropertyFlags::GetParameterFlags<Type>();
    WD_CHECK_AT_COMPILETIME_MSG(
      !std::is_pointer<Type>::value ||
        wdVariant::TypeDeduction<typename wdTypeTraits<Type>::NonConstReferencePointerType>::value == wdVariantType::Invalid,
      "Pointer to standard types are not supported.");
  }

  /// \brief Returns the actual type of the property. You can then compare that with known types, eg. compare it to wdGetStaticRTTI<int>()
  /// to see whether this is an int property.
  virtual const wdRTTI* GetSpecificType() const override // [tested]
  {
    return wdGetStaticRTTI<typename wdTypeTraits<Type>::NonConstReferencePointerType>();
  }

  /// \brief Returns the value of the property. Pass the instance pointer to the surrounding class along.
  virtual Type GetValue(const void* pInstance) const = 0; // [tested]

  /// \brief Modifies the value of the property. Pass the instance pointer to the surrounding class along.
  ///
  /// \note Make sure the property is not read-only before calling this, otherwise an assert will fire.
  virtual void SetValue(void* pInstance, Type value) = 0; // [tested]

  virtual void GetValuePtr(const void* pInstance, void* pObject) const override { *static_cast<Type*>(pObject) = GetValue(pInstance); };
  virtual void SetValuePtr(void* pInstance, const void* pObject) override { SetValue(pInstance, *static_cast<const Type*>(pObject)); };
};

/// \brief Specialization of wdTypedMemberProperty for const char*.
///
/// This works because wdTypedMemberProperty< typename wdTypeTraits<Type>::NonConstReferenceType > in wdAccessorProperty
/// does not actually remove the constness of the type but of the pointer, so const char* is not affected.
template <>
class wdTypedMemberProperty<const char*> : public wdAbstractMemberProperty
{
public:
  wdTypedMemberProperty(const char* szPropertyName)
    : wdAbstractMemberProperty(szPropertyName)
  {
    // We treat const char* as a basic type and not a pointer.
    m_Flags = wdPropertyFlags::GetParameterFlags<const char*>();
  }

  virtual const wdRTTI* GetSpecificType() const override // [tested]
  {
    return wdGetStaticRTTI<const char*>();
  }

  virtual const char* GetValue(const void* pInstance) const = 0;
  virtual void SetValue(void* pInstance, const char* value) = 0;
  virtual void GetValuePtr(const void* pInstance, void* pObject) const override { *static_cast<const char**>(pObject) = GetValue(pInstance); };
  virtual void SetValuePtr(void* pInstance, const void* pObject) override { SetValue(pInstance, *static_cast<const char* const*>(pObject)); };
};


// *******************************************************************
// ***** Class for properties that use custom accessor functions *****

/// \brief [internal] An implementation of wdTypedMemberProperty that uses custom getter / setter functions to access a property.
template <typename Class, typename Type>
class wdAccessorProperty : public wdTypedMemberProperty<typename wdTypeTraits<Type>::NonConstReferenceType>
{
public:
  using RealType = typename wdTypeTraits<Type>::NonConstReferenceType;
  using GetterFunc = Type (Class::*)() const;
  using SetterFunc = void (Class::*)(Type value);

  /// \brief Constructor.
  wdAccessorProperty(const char* szPropertyName, GetterFunc getter, SetterFunc setter)
    : wdTypedMemberProperty<RealType>(szPropertyName)
  {
    WD_ASSERT_DEBUG(getter != nullptr, "The getter of a property cannot be nullptr.");

    m_Getter = getter;
    m_Setter = setter;

    if (m_Setter == nullptr)
      wdAbstractMemberProperty::m_Flags.Add(wdPropertyFlags::ReadOnly);
  }

  /// \brief Always returns nullptr; once a property is modified through accessors, there is no point in giving more direct access to
  /// others.
  virtual void* GetPropertyPointer(const void* pInstance) const override
  {
    // No access to sub-properties, if we have accessors for this property
    return nullptr;
  }

  /// \brief Returns the value of the property. Pass the instance pointer to the surrounding class along.
  virtual RealType GetValue(const void* pInstance) const override // [tested]
  {
    return (static_cast<const Class*>(pInstance)->*m_Getter)();
  }

  /// \brief Modifies the value of the property. Pass the instance pointer to the surrounding class along.
  ///
  /// \note Make sure the property is not read-only before calling this, otherwise an assert will fire.
  virtual void SetValue(void* pInstance, RealType value) override // [tested]
  {
    WD_ASSERT_DEV(m_Setter != nullptr, "The property '{0}' has no setter function, thus it is read-only.", wdAbstractProperty::GetPropertyName());

    if (m_Setter)
      (static_cast<Class*>(pInstance)->*m_Setter)(value);
  }

private:
  GetterFunc m_Getter;
  SetterFunc m_Setter;
};


// *************************************************************
// ***** Classes for properties that are accessed directly *****

/// \brief [internal] Helper class to generate accessor functions for (private) members of another class
template <typename Class, typename Type, Type Class::*Member>
struct wdPropertyAccessor
{
  static Type GetValue(const Class* pInstance) { return (*pInstance).*Member; }

  static void SetValue(Class* pInstance, Type value) { (*pInstance).*Member = value; }

  static void* GetPropertyPointer(const Class* pInstance) { return (void*)&((*pInstance).*Member); }
};


/// \brief [internal] An implementation of wdTypedMemberProperty that accesses the property data directly.
template <typename Class, typename Type>
class wdMemberProperty : public wdTypedMemberProperty<Type>
{
public:
  using GetterFunc = Type (*)(const Class* pInstance);
  using SetterFunc = void (*)(Class* pInstance, Type value);
  using PointerFunc = void* (*)(const Class* pInstance);

  /// \brief Constructor.
  wdMemberProperty(const char* szPropertyName, GetterFunc getter, SetterFunc setter, PointerFunc pointer)
    : wdTypedMemberProperty<Type>(szPropertyName)
  {
    WD_ASSERT_DEBUG(getter != nullptr, "The getter of a property cannot be nullptr.");

    m_Getter = getter;
    m_Setter = setter;
    m_Pointer = pointer;

    if (m_Setter == nullptr)
      wdAbstractMemberProperty::m_Flags.Add(wdPropertyFlags::ReadOnly);
  }

  /// \brief Returns a pointer to the member property.
  virtual void* GetPropertyPointer(const void* pInstance) const override { return m_Pointer(static_cast<const Class*>(pInstance)); }

  /// \brief Returns the value of the property. Pass the instance pointer to the surrounding class along.
  virtual Type GetValue(const void* pInstance) const override { return m_Getter(static_cast<const Class*>(pInstance)); }

  /// \brief Modifies the value of the property. Pass the instance pointer to the surrounding class along.
  ///
  /// \note Make sure the property is not read-only before calling this, otherwise an assert will fire.
  virtual void SetValue(void* pInstance, Type value) override
  {
    WD_ASSERT_DEV(m_Setter != nullptr, "The property '{0}' has no setter function, thus it is read-only.", wdAbstractProperty::GetPropertyName());

    if (m_Setter)
      m_Setter(static_cast<Class*>(pInstance), value);
  }

private:
  GetterFunc m_Getter;
  SetterFunc m_Setter;
  PointerFunc m_Pointer;
};
