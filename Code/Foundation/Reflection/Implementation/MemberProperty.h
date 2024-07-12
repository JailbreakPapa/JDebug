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
/// For example, when you have a pointer to an nsAbstractMemberProperty and it returns that the property is of type 'int', you can cast the
/// pointer to an pointer to nsTypedMemberProperty<int> which then allows you to access its values.
template <typename Type>
class nsTypedMemberProperty : public nsAbstractMemberProperty
{
public:
  /// \brief Passes the property name through to nsAbstractMemberProperty.
  nsTypedMemberProperty(const char* szPropertyName)
    : nsAbstractMemberProperty(szPropertyName)
  {
    m_Flags = nsPropertyFlags::GetParameterFlags<Type>();
    NS_CHECK_AT_COMPILETIME_MSG(
      !std::is_pointer<Type>::value ||
        nsVariant::TypeDeduction<typename nsTypeTraits<Type>::NonConstReferencePointerType>::value == nsVariantType::Invalid,
      "Pointer to standard types are not supported.");
  }

  /// \brief Returns the actual type of the property. You can then compare that with known types, eg. compare it to nsGetStaticRTTI<int>()
  /// to see whether this is an int property.
  virtual const nsRTTI* GetSpecificType() const override // [tested]
  {
    return nsGetStaticRTTI<typename nsTypeTraits<Type>::NonConstReferencePointerType>();
  }

  /// \brief Returns the value of the property. Pass the instance pointer to the surrounding class along.
  virtual Type GetValue(const void* pInstance) const = 0; // [tested]

  /// \brief Modifies the value of the property. Pass the instance pointer to the surrounding class along.
  ///
  /// \note Make sure the property is not read-only before calling this, otherwise an assert will fire.
  virtual void SetValue(void* pInstance, Type value) const = 0; // [tested]

  virtual void GetValuePtr(const void* pInstance, void* pObject) const override { *static_cast<Type*>(pObject) = GetValue(pInstance); };
  virtual void SetValuePtr(void* pInstance, const void* pObject) const override { SetValue(pInstance, *static_cast<const Type*>(pObject)); };
};

/// \brief Specialization of nsTypedMemberProperty for const char*.
///
/// This works because nsTypedMemberProperty< typename nsTypeTraits<Type>::NonConstReferenceType > in nsAccessorProperty
/// does not actually remove the constness of the type but of the pointer, so const char* is not affected.
template <>
class nsTypedMemberProperty<const char*> : public nsAbstractMemberProperty
{
public:
  nsTypedMemberProperty(const char* szPropertyName)
    : nsAbstractMemberProperty(szPropertyName)
  {
    // We treat const char* as a basic type and not a pointer.
    m_Flags = nsPropertyFlags::GetParameterFlags<const char*>();
  }

  virtual const nsRTTI* GetSpecificType() const override // [tested]
  {
    return nsGetStaticRTTI<const char*>();
  }

  virtual const char* GetValue(const void* pInstance) const = 0;
  virtual void SetValue(void* pInstance, const char* value) const = 0;
  virtual void GetValuePtr(const void* pInstance, void* pObject) const override { *static_cast<const char**>(pObject) = GetValue(pInstance); };
  virtual void SetValuePtr(void* pInstance, const void* pObject) const override { SetValue(pInstance, *static_cast<const char* const*>(pObject)); };
};


// *******************************************************************
// ***** Class for properties that use custom accessor functions *****

/// \brief [internal] An implementation of nsTypedMemberProperty that uses custom getter / setter functions to access a property.
template <typename Class, typename Type>
class nsAccessorProperty : public nsTypedMemberProperty<typename nsTypeTraits<Type>::NonConstReferenceType>
{
public:
  using RealType = typename nsTypeTraits<Type>::NonConstReferenceType;
  using GetterFunc = Type (Class::*)() const;
  using SetterFunc = void (Class::*)(Type value);

  /// \brief Constructor.
  nsAccessorProperty(const char* szPropertyName, GetterFunc getter, SetterFunc setter)
    : nsTypedMemberProperty<RealType>(szPropertyName)
  {
    NS_ASSERT_DEBUG(getter != nullptr, "The getter of a property cannot be nullptr.");

    m_Getter = getter;
    m_Setter = setter;

    if (m_Setter == nullptr)
      nsAbstractMemberProperty::m_Flags.Add(nsPropertyFlags::ReadOnly);
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
  virtual void SetValue(void* pInstance, RealType value) const override // [tested]
  {
    NS_ASSERT_DEV(m_Setter != nullptr, "The property '{0}' has no setter function, thus it is read-only.", nsAbstractProperty::GetPropertyName());

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
struct nsPropertyAccessor
{
  static Type GetValue(const Class* pInstance) { return (*pInstance).*Member; }

  static void SetValue(Class* pInstance, Type value) { (*pInstance).*Member = value; }

  static void* GetPropertyPointer(const Class* pInstance) { return (void*)&((*pInstance).*Member); }
};


/// \brief [internal] An implementation of nsTypedMemberProperty that accesses the property data directly.
template <typename Class, typename Type>
class nsMemberProperty : public nsTypedMemberProperty<Type>
{
public:
  using GetterFunc = Type (*)(const Class* pInstance);
  using SetterFunc = void (*)(Class* pInstance, Type value);
  using PointerFunc = void* (*)(const Class* pInstance);

  /// \brief Constructor.
  nsMemberProperty(const char* szPropertyName, GetterFunc getter, SetterFunc setter, PointerFunc pointer)
    : nsTypedMemberProperty<Type>(szPropertyName)
  {
    NS_ASSERT_DEBUG(getter != nullptr, "The getter of a property cannot be nullptr.");

    m_Getter = getter;
    m_Setter = setter;
    m_Pointer = pointer;

    if (m_Setter == nullptr)
      nsAbstractMemberProperty::m_Flags.Add(nsPropertyFlags::ReadOnly);
  }

  /// \brief Returns a pointer to the member property.
  virtual void* GetPropertyPointer(const void* pInstance) const override { return m_Pointer(static_cast<const Class*>(pInstance)); }

  /// \brief Returns the value of the property. Pass the instance pointer to the surrounding class along.
  virtual Type GetValue(const void* pInstance) const override { return m_Getter(static_cast<const Class*>(pInstance)); }

  /// \brief Modifies the value of the property. Pass the instance pointer to the surrounding class along.
  ///
  /// \note Make sure the property is not read-only before calling this, otherwise an assert will fire.
  virtual void SetValue(void* pInstance, Type value) const override
  {
    NS_ASSERT_DEV(m_Setter != nullptr, "The property '{0}' has no setter function, thus it is read-only.", nsAbstractProperty::GetPropertyName());

    if (m_Setter)
      m_Setter(static_cast<Class*>(pInstance), value);
  }

private:
  GetterFunc m_Getter;
  SetterFunc m_Setter;
  PointerFunc m_Pointer;
};
