#pragma once

/// \file

#include <Foundation/Reflection/Implementation/AbstractProperty.h>

class nsRTTI;

template <typename Type>
class nsTypedMapProperty : public nsAbstractMapProperty
{
public:
  nsTypedMapProperty(const char* szPropertyName)
    : nsAbstractMapProperty(szPropertyName)
  {
    m_Flags = nsPropertyFlags::GetParameterFlags<Type>();
    NS_CHECK_AT_COMPILETIME_MSG(
      !std::is_pointer<Type>::value ||
        nsVariant::TypeDeduction<typename nsTypeTraits<Type>::NonConstReferencePointerType>::value == nsVariantType::Invalid,
      "Pointer to standard types are not supported.");
  }

  virtual const nsRTTI* GetSpecificType() const override { return nsGetStaticRTTI<typename nsTypeTraits<Type>::NonConstReferencePointerType>(); }
};


template <typename Class, typename Type, typename Container>
class nsAccessorMapProperty : public nsTypedMapProperty<Type>
{
public:
  using ContainerType = typename nsTypeTraits<Container>::NonConstReferenceType;
  using RealType = typename nsTypeTraits<Type>::NonConstReferenceType;

  using InsertFunc = void (Class::*)(const char* szKey, Type value);
  using RemoveFunc = void (Class::*)(const char* szKey);
  using GetValueFunc = bool (Class::*)(const char* szKey, RealType& value) const;
  using GetKeyRangeFunc = Container (Class::*)() const;

  nsAccessorMapProperty(const char* szPropertyName, GetKeyRangeFunc getKeys, GetValueFunc getValue, InsertFunc insert, RemoveFunc remove)
    : nsTypedMapProperty<Type>(szPropertyName)
  {
    NS_ASSERT_DEBUG(getKeys != nullptr, "The getKeys function of a map property cannot be nullptr.");
    NS_ASSERT_DEBUG(getValue != nullptr, "The GetValueFunc function of a map property cannot be nullptr.");

    m_GetKeyRange = getKeys;
    m_GetValue = getValue;
    m_Insert = insert;
    m_Remove = remove;

    if (m_Insert == nullptr || remove == nullptr)
      nsAbstractMapProperty::m_Flags.Add(nsPropertyFlags::ReadOnly);
  }

  virtual bool IsEmpty(const void* pInstance) const override
  {
    // this should be decltype(auto) c = ...; but MSVC 16 is too dumb for that (MSVC 15 works fine)
    decltype((static_cast<const Class*>(pInstance)->*m_GetKeyRange)()) c = (static_cast<const Class*>(pInstance)->*m_GetKeyRange)();

    return begin(c) == end(c);
  }

  virtual void Clear(void* pInstance) const override
  {
    while (true)
    {
      // this should be decltype(auto) c = ...; but MSVC 16 is too dumb for that (MSVC 15 works fine)
      decltype((static_cast<const Class*>(pInstance)->*m_GetKeyRange)()) c = (static_cast<const Class*>(pInstance)->*m_GetKeyRange)();

      auto it = begin(c);
      if (it != end(c))
        Remove(pInstance, *it);
      else
        return;
    }
  }

  virtual void Insert(void* pInstance, const char* szKey, const void* pObject) const override
  {
    NS_ASSERT_DEBUG(m_Insert != nullptr, "The property '{0}' has no insert function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Insert)(szKey, *static_cast<const RealType*>(pObject));
  }

  virtual void Remove(void* pInstance, const char* szKey) const override
  {
    NS_ASSERT_DEBUG(m_Remove != nullptr, "The property '{0}' has no remove function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Remove)(szKey);
  }

  virtual bool Contains(const void* pInstance, const char* szKey) const override
  {
    RealType value;
    return (static_cast<const Class*>(pInstance)->*m_GetValue)(szKey, value);
  }

  virtual bool GetValue(const void* pInstance, const char* szKey, void* pObject) const override
  {
    return (static_cast<const Class*>(pInstance)->*m_GetValue)(szKey, *static_cast<RealType*>(pObject));
  }

  virtual void GetKeys(const void* pInstance, nsHybridArray<nsString, 16>& out_keys) const override
  {
    out_keys.Clear();
    decltype(auto) c = (static_cast<const Class*>(pInstance)->*m_GetKeyRange)();
    for (const auto& key : c)
    {
      out_keys.PushBack(key);
    }
  }

private:
  GetKeyRangeFunc m_GetKeyRange;
  GetValueFunc m_GetValue;
  InsertFunc m_Insert;
  RemoveFunc m_Remove;
};


template <typename Class, typename Type, typename Container>
class nsWriteAccessorMapProperty : public nsTypedMapProperty<Type>
{
public:
  using ContainerType = typename nsTypeTraits<Container>::NonConstReferenceType;
  using ContainerSubType = typename nsContainerSubTypeResolver<ContainerType>::Type;
  using RealType = typename nsTypeTraits<Type>::NonConstReferenceType;

  using InsertFunc = void (Class::*)(const char* szKey, Type value);
  using RemoveFunc = void (Class::*)(const char* szKey);
  using GetContainerFunc = Container (Class::*)() const;

  nsWriteAccessorMapProperty(const char* szPropertyName, GetContainerFunc getContainer, InsertFunc insert, RemoveFunc remove)
    : nsTypedMapProperty<Type>(szPropertyName)
  {
    NS_ASSERT_DEBUG(getContainer != nullptr, "The get count function of a map property cannot be nullptr.");

    m_GetContainer = getContainer;
    m_Insert = insert;
    m_Remove = remove;

    if (m_Insert == nullptr)
      nsAbstractMapProperty::m_Flags.Add(nsPropertyFlags::ReadOnly);
  }

  virtual bool IsEmpty(const void* pInstance) const override { return (static_cast<const Class*>(pInstance)->*m_GetContainer)().IsEmpty(); }

  virtual void Clear(void* pInstance) const override
  {
    decltype(auto) c = (static_cast<const Class*>(pInstance)->*m_GetContainer)();
    while (!IsEmpty(pInstance))
    {
      auto it = c.GetIterator();
      Remove(pInstance, it.Key());
    }
  }

  virtual void Insert(void* pInstance, const char* szKey, const void* pObject) const override
  {
    NS_ASSERT_DEBUG(m_Insert != nullptr, "The property '{0}' has no insert function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Insert)(szKey, *static_cast<const RealType*>(pObject));
  }

  virtual void Remove(void* pInstance, const char* szKey) const override
  {
    NS_ASSERT_DEBUG(m_Remove != nullptr, "The property '{0}' has no remove function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Remove)(szKey);
  }

  virtual bool Contains(const void* pInstance, const char* szKey) const override
  {
    return (static_cast<const Class*>(pInstance)->*m_GetContainer)().Contains(szKey);
  }

  virtual bool GetValue(const void* pInstance, const char* szKey, void* pObject) const override
  {
    decltype(auto) c = (static_cast<const Class*>(pInstance)->*m_GetContainer)();
    const RealType* value = c.GetValue(szKey);
    if (value)
    {
      *static_cast<RealType*>(pObject) = *value;
    }
    return value != nullptr;
  }

  virtual void GetKeys(const void* pInstance, nsHybridArray<nsString, 16>& out_keys) const override
  {
    decltype(auto) c = (static_cast<const Class*>(pInstance)->*m_GetContainer)();
    out_keys.Clear();
    for (auto it = c.GetIterator(); it.IsValid(); ++it)
    {
      out_keys.PushBack(it.Key());
    }
  }

private:
  GetContainerFunc m_GetContainer;
  InsertFunc m_Insert;
  RemoveFunc m_Remove;
};



template <typename Class, typename Container, Container Class::*Member>
struct nsMapPropertyAccessor
{
  using ContainerType = typename nsTypeTraits<Container>::NonConstReferenceType;
  using Type = typename nsTypeTraits<typename nsContainerSubTypeResolver<ContainerType>::Type>::NonConstReferenceType;

  static const ContainerType& GetConstContainer(const Class* pInstance) { return (*pInstance).*Member; }

  static ContainerType& GetContainer(Class* pInstance) { return (*pInstance).*Member; }
};


template <typename Class, typename Container, typename Type>
class nsMemberMapProperty : public nsTypedMapProperty<typename nsTypeTraits<Type>::NonConstReferenceType>
{
public:
  using RealType = typename nsTypeTraits<Type>::NonConstReferenceType;
  using GetConstContainerFunc = const Container& (*)(const Class* pInstance);
  using GetContainerFunc = Container& (*)(Class* pInstance);

  nsMemberMapProperty(const char* szPropertyName, GetConstContainerFunc constGetter, GetContainerFunc getter)
    : nsTypedMapProperty<RealType>(szPropertyName)
  {
    NS_ASSERT_DEBUG(constGetter != nullptr, "The const get count function of an array property cannot be nullptr.");

    m_ConstGetter = constGetter;
    m_Getter = getter;

    if (m_Getter == nullptr)
      nsAbstractMapProperty::m_Flags.Add(nsPropertyFlags::ReadOnly);
  }

  virtual bool IsEmpty(const void* pInstance) const override { return m_ConstGetter(static_cast<const Class*>(pInstance)).IsEmpty(); }

  virtual void Clear(void* pInstance) const override
  {
    NS_ASSERT_DEBUG(
      m_Getter != nullptr, "The property '{0}' has no non-const set accessor function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).Clear();
  }

  virtual void Insert(void* pInstance, const char* szKey, const void* pObject) const override
  {
    NS_ASSERT_DEBUG(
      m_Getter != nullptr, "The property '{0}' has no non-const set accessor function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).Insert(szKey, *static_cast<const RealType*>(pObject));
  }

  virtual void Remove(void* pInstance, const char* szKey) const override
  {
    NS_ASSERT_DEBUG(
      m_Getter != nullptr, "The property '{0}' has no non-const set accessor function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).Remove(szKey);
  }

  virtual bool Contains(const void* pInstance, const char* szKey) const override
  {
    return m_ConstGetter(static_cast<const Class*>(pInstance)).Contains(szKey);
  }

  virtual bool GetValue(const void* pInstance, const char* szKey, void* pObject) const override
  {
    const RealType* value = m_ConstGetter(static_cast<const Class*>(pInstance)).GetValue(szKey);
    if (value)
    {
      *static_cast<RealType*>(pObject) = *value;
    }
    return value != nullptr;
  }

  virtual void GetKeys(const void* pInstance, nsHybridArray<nsString, 16>& out_keys) const override
  {
    decltype(auto) c = m_ConstGetter(static_cast<const Class*>(pInstance));
    out_keys.Clear();
    for (auto it = c.GetIterator(); it.IsValid(); ++it)
    {
      out_keys.PushBack(it.Key());
    }
  }

private:
  GetConstContainerFunc m_ConstGetter;
  GetContainerFunc m_Getter;
};
