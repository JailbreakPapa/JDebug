#pragma once

/// \file

#include <Foundation/Reflection/Implementation/AbstractProperty.h>

/// \brief Do not cast into this class or any of its derived classes, use wdAbstractSetProperty instead.
template <typename Type>
class wdTypedSetProperty : public wdAbstractSetProperty
{
public:
  wdTypedSetProperty(const char* szPropertyName)
    : wdAbstractSetProperty(szPropertyName)
  {
    m_Flags = wdPropertyFlags::GetParameterFlags<Type>();
  }

  virtual const wdRTTI* GetSpecificType() const override { return wdGetStaticRTTI<typename wdTypeTraits<Type>::NonConstReferencePointerType>(); }
};

/// \brief Specialization of wdTypedArrayProperty to retain the pointer in const char*.
template <>
class wdTypedSetProperty<const char*> : public wdAbstractSetProperty
{
public:
  wdTypedSetProperty(const char* szPropertyName)
    : wdAbstractSetProperty(szPropertyName)
  {
    m_Flags = wdPropertyFlags::GetParameterFlags<const char*>();
  }

  virtual const wdRTTI* GetSpecificType() const override { return wdGetStaticRTTI<const char*>(); }
};


template <typename Class, typename Type, typename Container>
class wdAccessorSetProperty : public wdTypedSetProperty<Type>
{
public:
  using ContainerType = typename wdTypeTraits<Container>::NonConstReferenceType;
  using RealType = typename wdTypeTraits<Type>::NonConstReferenceType;

  using InsertFunc = void (Class::*)(Type value);
  using RemoveFunc = void (Class::*)(Type value);
  using GetValuesFunc = Container (Class::*)() const;

  wdAccessorSetProperty(const char* szPropertyName, GetValuesFunc getValues, InsertFunc insert, RemoveFunc remove)
    : wdTypedSetProperty<Type>(szPropertyName)
  {
    WD_ASSERT_DEBUG(getValues != nullptr, "The get values function of an set property cannot be nullptr.");

    m_GetValues = getValues;
    m_Insert = insert;
    m_Remove = remove;

    if (m_Insert == nullptr || m_Remove == nullptr)
      wdAbstractSetProperty::m_Flags.Add(wdPropertyFlags::ReadOnly);
  }


  virtual bool IsEmpty(const void* pInstance) const override { return (static_cast<const Class*>(pInstance)->*m_GetValues)().IsEmpty(); }

  virtual void Clear(void* pInstance) override
  {
    WD_ASSERT_DEBUG(m_Insert != nullptr && m_Remove != nullptr, "The property '{0}' has no remove and insert function, thus it is read-only",
      wdAbstractProperty::GetPropertyName());

    // We must not cache the container c here as the Remove can make it invalid
    // e.g. wdArrayPtr by value.
    while (!IsEmpty(pInstance))
    {
      // this should be decltype(auto) c = ...; but MSVC 16 is too dumb for that (MSVC 15 works fine)
      decltype((static_cast<const Class*>(pInstance)->*m_GetValues)()) c = (static_cast<const Class*>(pInstance)->*m_GetValues)();
      auto it = cbegin(c);
      RealType value = *it;
      Remove(pInstance, &value);
    }
  }

  virtual void Insert(void* pInstance, const void* pObject) override
  {
    WD_ASSERT_DEBUG(m_Insert != nullptr, "The property '{0}' has no insert function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Insert)(*static_cast<const RealType*>(pObject));
  }

  virtual void Remove(void* pInstance, const void* pObject) override
  {
    WD_ASSERT_DEBUG(m_Remove != nullptr, "The property '{0}' has no setter function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Remove)(*static_cast<const RealType*>(pObject));
  }

  virtual bool Contains(const void* pInstance, const void* pObject) const override
  {
    for (const auto& value : (static_cast<const Class*>(pInstance)->*m_GetValues)())
    {
      if (value == *static_cast<const RealType*>(pObject))
        return true;
    }
    return false;
  }

  virtual void GetValues(const void* pInstance, wdDynamicArray<wdVariant>& out_keys) const override
  {
    out_keys.Clear();
    for (const auto& value : (static_cast<const Class*>(pInstance)->*m_GetValues)())
    {
      out_keys.PushBack(wdVariant(value));
    }
  }

private:
  GetValuesFunc m_GetValues;
  InsertFunc m_Insert;
  RemoveFunc m_Remove;
};



template <typename Class, typename Container, Container Class::*Member>
struct wdSetPropertyAccessor
{
  using ContainerType = typename wdTypeTraits<Container>::NonConstReferenceType;
  using Type = typename wdTypeTraits<typename wdContainerSubTypeResolver<ContainerType>::Type>::NonConstReferenceType;

  static const ContainerType& GetConstContainer(const Class* pInstance) { return (*pInstance).*Member; }

  static ContainerType& GetContainer(Class* pInstance) { return (*pInstance).*Member; }
};


template <typename Class, typename Container, typename Type>
class wdMemberSetProperty : public wdTypedSetProperty<typename wdTypeTraits<Type>::NonConstReferenceType>
{
public:
  using RealType = typename wdTypeTraits<Type>::NonConstReferenceType;
  using GetConstContainerFunc = const Container& (*)(const Class* pInstance);
  using GetContainerFunc = Container& (*)(Class* pInstance);

  wdMemberSetProperty(const char* szPropertyName, GetConstContainerFunc constGetter, GetContainerFunc getter)
    : wdTypedSetProperty<RealType>(szPropertyName)
  {
    WD_ASSERT_DEBUG(constGetter != nullptr, "The const get count function of an set property cannot be nullptr.");

    m_ConstGetter = constGetter;
    m_Getter = getter;

    if (m_Getter == nullptr)
      wdAbstractSetProperty::m_Flags.Add(wdPropertyFlags::ReadOnly);
  }

  virtual bool IsEmpty(const void* pInstance) const override { return m_ConstGetter(static_cast<const Class*>(pInstance)).IsEmpty(); }

  virtual void Clear(void* pInstance) override
  {
    WD_ASSERT_DEBUG(
      m_Getter != nullptr, "The property '{0}' has no non-const set accessor function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).Clear();
  }

  virtual void Insert(void* pInstance, const void* pObject) override
  {
    WD_ASSERT_DEBUG(
      m_Getter != nullptr, "The property '{0}' has no non-const set accessor function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).Insert(*static_cast<const RealType*>(pObject));
  }

  virtual void Remove(void* pInstance, const void* pObject) override
  {
    WD_ASSERT_DEBUG(
      m_Getter != nullptr, "The property '{0}' has no non-const set accessor function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).Remove(*static_cast<const RealType*>(pObject));
  }

  virtual bool Contains(const void* pInstance, const void* pObject) const override
  {
    return m_ConstGetter(static_cast<const Class*>(pInstance)).Contains(*static_cast<const RealType*>(pObject));
  }

  virtual void GetValues(const void* pInstance, wdDynamicArray<wdVariant>& out_keys) const override
  {
    out_keys.Clear();
    for (const auto& value : m_ConstGetter(static_cast<const Class*>(pInstance)))
    {
      out_keys.PushBack(wdVariant(value));
    }
  }

private:
  GetConstContainerFunc m_ConstGetter;
  GetContainerFunc m_Getter;
};
