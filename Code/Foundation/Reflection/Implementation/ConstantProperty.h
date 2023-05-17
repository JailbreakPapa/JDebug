#pragma once

/// \file

#include <Foundation/Reflection/Implementation/AbstractProperty.h>
#include <Foundation/Reflection/Implementation/StaticRTTI.h>

/// \brief The base class for all typed member properties. Ie. once the type of a property is determined, it can be cast to the proper
/// version of this.
///
/// For example, when you have a pointer to an wdAbstractMemberProperty and it returns that the property is of type 'int', you can cast the
/// pointer to an pointer to wdTypedMemberProperty<int> which then allows you to access its values.
template <typename Type>
class wdTypedConstantProperty : public wdAbstractConstantProperty
{
public:
  /// \brief Passes the property name through to wdAbstractMemberProperty.
  wdTypedConstantProperty(const char* szPropertyName)
    : wdAbstractConstantProperty(szPropertyName)
  {
    m_Flags = wdPropertyFlags::GetParameterFlags<Type>();
  }

  /// \brief Returns the actual type of the property. You can then compare that with known types, eg. compare it to wdGetStaticRTTI<int>()
  /// to see whether this is an int property.
  virtual const wdRTTI* GetSpecificType() const override // [tested]
  {
    return wdGetStaticRTTI<typename wdTypeTraits<Type>::NonConstReferenceType>();
  }

  /// \brief Returns the value of the property. Pass the instance pointer to the surrounding class along.
  virtual Type GetValue() const = 0;
};

/// \brief [internal] An implementation of wdTypedConstantProperty that accesses the property data directly.
template <typename Type>
class wdConstantProperty : public wdTypedConstantProperty<Type>
{
public:
  /// \brief Constructor.
  wdConstantProperty(const char* szPropertyName, Type value)
    : wdTypedConstantProperty<Type>(szPropertyName)
    , m_Value(value)
  {
    WD_ASSERT_DEBUG(this->m_Flags.IsSet(wdPropertyFlags::StandardType), "Only constants that can be put in an wdVariant are currently supported!");
  }

  /// \brief Returns a pointer to the member property.
  virtual void* GetPropertyPointer() const override { return (void*)&m_Value; }

  /// \brief Returns the value of the property. Pass the instance pointer to the surrounding class along.
  virtual Type GetValue() const override // [tested]
  {
    return m_Value;
  }

  virtual wdVariant GetConstant() const override { return wdVariant(m_Value); }

private:
  Type m_Value;
};
