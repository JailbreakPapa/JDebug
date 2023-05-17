#pragma once

/// \file

#include <Foundation/Reflection/Implementation/AbstractProperty.h>

class wdRTTI;

/// \brief Do not cast into this class or any of its derived classes, use wdTypedArrayProperty instead.
template <typename Type>
class wdTypedArrayProperty : public wdAbstractArrayProperty
{
public:
  wdTypedArrayProperty(const char* szPropertyName)
    : wdAbstractArrayProperty(szPropertyName)
  {
    m_Flags = wdPropertyFlags::GetParameterFlags<Type>();
    WD_CHECK_AT_COMPILETIME_MSG(!std::is_pointer<Type>::value ||
                                  wdVariantTypeDeduction<typename wdTypeTraits<Type>::NonConstReferencePointerType>::value ==
                                    wdVariantType::Invalid,
      "Pointer to standard types are not supported.");
  }

  virtual const wdRTTI* GetSpecificType() const override { return wdGetStaticRTTI<typename wdTypeTraits<Type>::NonConstReferencePointerType>(); }
};

/// \brief Specialization of wdTypedArrayProperty to retain the pointer in const char*.
template <>
class wdTypedArrayProperty<const char*> : public wdAbstractArrayProperty
{
public:
  wdTypedArrayProperty(const char* szPropertyName)
    : wdAbstractArrayProperty(szPropertyName)
  {
    m_Flags = wdPropertyFlags::GetParameterFlags<const char*>();
  }

  virtual const wdRTTI* GetSpecificType() const override { return wdGetStaticRTTI<const char*>(); }
};


template <typename Class, typename Type>
class wdAccessorArrayProperty : public wdTypedArrayProperty<Type>
{
public:
  using RealType = typename wdTypeTraits<Type>::NonConstReferenceType;
  using GetCountFunc = wdUInt32 (Class::*)() const;
  using GetValueFunc = Type (Class::*)(wdUInt32 uiIndex) const;
  using SetValueFunc = void (Class::*)(wdUInt32 uiIndex, Type value);
  using InsertFunc = void (Class::*)(wdUInt32 uiIndex, Type value);
  using RemoveFunc = void (Class::*)(wdUInt32 uiIndex);


  wdAccessorArrayProperty(
    const char* szPropertyName, GetCountFunc getCount, GetValueFunc getter, SetValueFunc setter, InsertFunc insert, RemoveFunc remove)
    : wdTypedArrayProperty<Type>(szPropertyName)
  {
    WD_ASSERT_DEBUG(getCount != nullptr, "The get count function of an array property cannot be nullptr.");
    WD_ASSERT_DEBUG(getter != nullptr, "The get value function of an array property cannot be nullptr.");

    m_GetCount = getCount;
    m_Getter = getter;
    m_Setter = setter;
    m_Insert = insert;
    m_Remove = remove;

    if (m_Setter == nullptr)
      wdAbstractArrayProperty::m_Flags.Add(wdPropertyFlags::ReadOnly);
  }


  virtual wdUInt32 GetCount(const void* pInstance) const override { return (static_cast<const Class*>(pInstance)->*m_GetCount)(); }

  virtual void GetValue(const void* pInstance, wdUInt32 uiIndex, void* pObject) const override
  {
    WD_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "GetValue: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    *static_cast<RealType*>(pObject) = (static_cast<const Class*>(pInstance)->*m_Getter)(uiIndex);
  }

  virtual void SetValue(void* pInstance, wdUInt32 uiIndex, const void* pObject) override
  {
    WD_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "SetValue: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    WD_ASSERT_DEBUG(m_Setter != nullptr, "The property '{0}' has no setter function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Setter)(uiIndex, *static_cast<const RealType*>(pObject));
  }

  virtual void Insert(void* pInstance, wdUInt32 uiIndex, const void* pObject) override
  {
    WD_ASSERT_DEBUG(uiIndex <= GetCount(pInstance), "Insert: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    WD_ASSERT_DEBUG(m_Insert != nullptr, "The property '{0}' has no insert function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Insert)(uiIndex, *static_cast<const RealType*>(pObject));
  }

  virtual void Remove(void* pInstance, wdUInt32 uiIndex) override
  {
    WD_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "Remove: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    WD_ASSERT_DEBUG(m_Remove != nullptr, "The property '{0}' has no setter function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Remove)(uiIndex);
  }

  virtual void Clear(void* pInstance) override { SetCount(pInstance, 0); }

  virtual void SetCount(void* pInstance, wdUInt32 uiCount) override
  {
    WD_ASSERT_DEBUG(m_Insert != nullptr && m_Remove != nullptr, "The property '{0}' has no remove and insert function, thus it is fixed-size.",
      wdAbstractProperty::GetPropertyName());
    while (uiCount < GetCount(pInstance))
    {
      Remove(pInstance, GetCount(pInstance) - 1);
    }
    while (uiCount > GetCount(pInstance))
    {
      RealType elem = RealType();
      Insert(pInstance, GetCount(pInstance), &elem);
    }
  }

private:
  GetCountFunc m_GetCount;
  GetValueFunc m_Getter;
  SetValueFunc m_Setter;
  InsertFunc m_Insert;
  RemoveFunc m_Remove;
};



template <typename Class, typename Container, Container Class::*Member>
struct wdArrayPropertyAccessor
{
  using ContainerType = typename wdTypeTraits<Container>::NonConstReferenceType;
  using Type = typename wdTypeTraits<typename wdContainerSubTypeResolver<ContainerType>::Type>::NonConstReferenceType;

  static const ContainerType& GetConstContainer(const Class* pInstance) { return (*pInstance).*Member; }

  static ContainerType& GetContainer(Class* pInstance) { return (*pInstance).*Member; }
};


template <typename Class, typename Container, typename Type>
class wdMemberArrayProperty : public wdTypedArrayProperty<typename wdTypeTraits<Type>::NonConstReferenceType>
{
public:
  using RealType = typename wdTypeTraits<Type>::NonConstReferenceType;
  using GetConstContainerFunc = const Container& (*)(const Class* pInstance);
  using GetContainerFunc = Container& (*)(Class* pInstance);

  wdMemberArrayProperty(const char* szPropertyName, GetConstContainerFunc constGetter, GetContainerFunc getter)
    : wdTypedArrayProperty<RealType>(szPropertyName)
  {
    WD_ASSERT_DEBUG(constGetter != nullptr, "The const get count function of an array property cannot be nullptr.");

    m_ConstGetter = constGetter;
    m_Getter = getter;

    if (m_Getter == nullptr)
      wdAbstractArrayProperty::m_Flags.Add(wdPropertyFlags::ReadOnly);
  }

  virtual wdUInt32 GetCount(const void* pInstance) const override { return m_ConstGetter(static_cast<const Class*>(pInstance)).GetCount(); }

  virtual void GetValue(const void* pInstance, wdUInt32 uiIndex, void* pObject) const override
  {
    WD_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "GetValue: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    *static_cast<RealType*>(pObject) = m_ConstGetter(static_cast<const Class*>(pInstance))[uiIndex];
  }

  virtual void SetValue(void* pInstance, wdUInt32 uiIndex, const void* pObject) override
  {
    WD_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "SetValue: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    WD_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const array accessor function, thus it is read-only.",
      wdAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance))[uiIndex] = *static_cast<const RealType*>(pObject);
  }

  virtual void Insert(void* pInstance, wdUInt32 uiIndex, const void* pObject) override
  {
    WD_ASSERT_DEBUG(uiIndex <= GetCount(pInstance), "Insert: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    WD_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const array accessor function, thus it is read-only.",
      wdAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).Insert(*static_cast<const RealType*>(pObject), uiIndex);
  }

  virtual void Remove(void* pInstance, wdUInt32 uiIndex) override
  {
    WD_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "Remove: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    WD_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const array accessor function, thus it is read-only.",
      wdAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).RemoveAtAndCopy(uiIndex);
  }

  virtual void Clear(void* pInstance) override
  {
    WD_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const array accessor function, thus it is read-only.",
      wdAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).Clear();
  }

  virtual void SetCount(void* pInstance, wdUInt32 uiCount) override
  {
    WD_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const array accessor function, thus it is read-only.",
      wdAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).SetCount(uiCount);
  }

private:
  GetConstContainerFunc m_ConstGetter;
  GetContainerFunc m_Getter;
};

/// \brief Read only version of wdMemberArrayProperty that does not call any functions that modify the array. This is needed to reflect wdArrayPtr members.
template <typename Class, typename Container, typename Type>
class wdMemberArrayReadOnlyProperty : public wdTypedArrayProperty<typename wdTypeTraits<Type>::NonConstReferenceType>
{
public:
  using RealType = typename wdTypeTraits<Type>::NonConstReferenceType;
  using GetConstContainerFunc = const Container& (*)(const Class* pInstance);

  wdMemberArrayReadOnlyProperty(const char* szPropertyName, GetConstContainerFunc constGetter)
    : wdTypedArrayProperty<RealType>(szPropertyName)
  {
    WD_ASSERT_DEBUG(constGetter != nullptr, "The const get count function of an array property cannot be nullptr.");

    m_ConstGetter = constGetter;
    wdAbstractArrayProperty::m_Flags.Add(wdPropertyFlags::ReadOnly);
  }

  virtual wdUInt32 GetCount(const void* pInstance) const override { return m_ConstGetter(static_cast<const Class*>(pInstance)).GetCount(); }

  virtual void GetValue(const void* pInstance, wdUInt32 uiIndex, void* pObject) const override
  {
    WD_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "GetValue: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    *static_cast<RealType*>(pObject) = m_ConstGetter(static_cast<const Class*>(pInstance))[uiIndex];
  }

  virtual void SetValue(void* pInstance, wdUInt32 uiIndex, const void* pObject) override
  {
    WD_REPORT_FAILURE("The property '{0}' is read-only.", wdAbstractProperty::GetPropertyName());
  }

  virtual void Insert(void* pInstance, wdUInt32 uiIndex, const void* pObject) override
  {
    WD_REPORT_FAILURE("The property '{0}' is read-only.", wdAbstractProperty::GetPropertyName());
  }

  virtual void Remove(void* pInstance, wdUInt32 uiIndex) override
  {
    WD_REPORT_FAILURE("The property '{0}' is read-only.", wdAbstractProperty::GetPropertyName());
  }

  virtual void Clear(void* pInstance) override
  {
    WD_REPORT_FAILURE("The property '{0}' is read-only.", wdAbstractProperty::GetPropertyName());
  }

  virtual void SetCount(void* pInstance, wdUInt32 uiCount) override
  {
    WD_REPORT_FAILURE("The property '{0}' is read-only.", wdAbstractProperty::GetPropertyName());
  }

private:
  GetConstContainerFunc m_ConstGetter;
};
