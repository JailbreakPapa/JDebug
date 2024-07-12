#pragma once

/// \file

#include <Foundation/Reflection/Implementation/AbstractProperty.h>
#include <Foundation/Reflection/Implementation/StaticRTTI.h>

/// \brief The base class for all typed member properties. Ie. once the type of a property is determined, it can be cast to the proper
/// version of this.
///
/// For example, when you have a pointer to an nsAbstractMemberProperty and it returns that the property is of type 'int', you can cast the
/// pointer to an pointer to nsTypedMemberProperty<int> which then allows you to access its values.
template <typename Type>
class nsTypedConstantProperty : public nsAbstractConstantProperty
{
public:
  /// \brief Passes the property name through to nsAbstractMemberProperty.
  nsTypedConstantProperty(const char* szPropertyName)
    : nsAbstractConstantProperty(szPropertyName)
  {
    m_Flags = nsPropertyFlags::GetParameterFlags<Type>();
  }

  /// \brief Returns the actual type of the property. You can then compare that with known types, eg. compare it to nsGetStaticRTTI<int>()
  /// to see whether this is an int property.
  virtual const nsRTTI* GetSpecificType() const override // [tested]
  {
    return nsGetStaticRTTI<typename nsTypeTraits<Type>::NonConstReferenceType>();
  }

  /// \brief Returns the value of the property. Pass the instance pointer to the surrounding class along.
  virtual Type GetValue() const = 0;
};

/// \brief [internal] An implementation of nsTypedConstantProperty that accesses the property data directly.
template <typename Type>
class nsConstantProperty : public nsTypedConstantProperty<Type>
{
public:
  /// \brief Constructor.
  nsConstantProperty(const char* szPropertyName, Type value)
    : nsTypedConstantProperty<Type>(szPropertyName)
    , m_Value(value)
  {
    NS_ASSERT_DEBUG(this->m_Flags.IsSet(nsPropertyFlags::StandardType), "Only constants that can be put in an nsVariant are currently supported!");
  }

  /// \brief Returns a pointer to the member property.
  virtual void* GetPropertyPointer() const override { return (void*)&m_Value; }

  /// \brief Returns the value of the property. Pass the instance pointer to the surrounding class along.
  virtual Type GetValue() const override // [tested]
  {
    return m_Value;
  }

  virtual nsVariant GetConstant() const override { return nsVariant(m_Value); }

private:
  Type m_Value;
};
