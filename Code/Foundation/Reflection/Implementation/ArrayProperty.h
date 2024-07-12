#pragma once

/// \file

#include <Foundation/Reflection/Implementation/AbstractProperty.h>

class nsRTTI;

/// \brief Do not cast into this class or any of its derived classes, use nsTypedArrayProperty instead.
template <typename Type>
class nsTypedArrayProperty : public nsAbstractArrayProperty
{
public:
  nsTypedArrayProperty(const char* szPropertyName)
    : nsAbstractArrayProperty(szPropertyName)
  {
    m_Flags = nsPropertyFlags::GetParameterFlags<Type>();
    NS_CHECK_AT_COMPILETIME_MSG(!std::is_pointer<Type>::value ||
                                  nsVariantTypeDeduction<typename nsTypeTraits<Type>::NonConstReferencePointerType>::value ==
                                    nsVariantType::Invalid,
      "Pointer to standard types are not supported.");
  }

  virtual const nsRTTI* GetSpecificType() const override { return nsGetStaticRTTI<typename nsTypeTraits<Type>::NonConstReferencePointerType>(); }
};

/// \brief Specialization of nsTypedArrayProperty to retain the pointer in const char*.
template <>
class nsTypedArrayProperty<const char*> : public nsAbstractArrayProperty
{
public:
  nsTypedArrayProperty(const char* szPropertyName)
    : nsAbstractArrayProperty(szPropertyName)
  {
    m_Flags = nsPropertyFlags::GetParameterFlags<const char*>();
  }

  virtual const nsRTTI* GetSpecificType() const override { return nsGetStaticRTTI<const char*>(); }
};


template <typename Class, typename Type>
class nsAccessorArrayProperty : public nsTypedArrayProperty<Type>
{
public:
  using RealType = typename nsTypeTraits<Type>::NonConstReferenceType;
  using GetCountFunc = nsUInt32 (Class::*)() const;
  using GetValueFunc = Type (Class::*)(nsUInt32 uiIndex) const;
  using SetValueFunc = void (Class::*)(nsUInt32 uiIndex, Type value);
  using InsertFunc = void (Class::*)(nsUInt32 uiIndex, Type value);
  using RemoveFunc = void (Class::*)(nsUInt32 uiIndex);


  nsAccessorArrayProperty(
    const char* szPropertyName, GetCountFunc getCount, GetValueFunc getter, SetValueFunc setter, InsertFunc insert, RemoveFunc remove)
    : nsTypedArrayProperty<Type>(szPropertyName)
  {
    NS_ASSERT_DEBUG(getCount != nullptr, "The get count function of an array property cannot be nullptr.");
    NS_ASSERT_DEBUG(getter != nullptr, "The get value function of an array property cannot be nullptr.");

    m_GetCount = getCount;
    m_Getter = getter;
    m_Setter = setter;
    m_Insert = insert;
    m_Remove = remove;

    if (m_Setter == nullptr)
      nsAbstractArrayProperty::m_Flags.Add(nsPropertyFlags::ReadOnly);
  }


  virtual nsUInt32 GetCount(const void* pInstance) const override { return (static_cast<const Class*>(pInstance)->*m_GetCount)(); }

  virtual void GetValue(const void* pInstance, nsUInt32 uiIndex, void* pObject) const override
  {
    NS_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "GetValue: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    *static_cast<RealType*>(pObject) = (static_cast<const Class*>(pInstance)->*m_Getter)(uiIndex);
  }

  virtual void SetValue(void* pInstance, nsUInt32 uiIndex, const void* pObject) const override
  {
    NS_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "SetValue: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    NS_ASSERT_DEBUG(m_Setter != nullptr, "The property '{0}' has no setter function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Setter)(uiIndex, *static_cast<const RealType*>(pObject));
  }

  virtual void Insert(void* pInstance, nsUInt32 uiIndex, const void* pObject) const override
  {
    NS_ASSERT_DEBUG(uiIndex <= GetCount(pInstance), "Insert: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    NS_ASSERT_DEBUG(m_Insert != nullptr, "The property '{0}' has no insert function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Insert)(uiIndex, *static_cast<const RealType*>(pObject));
  }

  virtual void Remove(void* pInstance, nsUInt32 uiIndex) const override
  {
    NS_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "Remove: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    NS_ASSERT_DEBUG(m_Remove != nullptr, "The property '{0}' has no setter function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Remove)(uiIndex);
  }

  virtual void Clear(void* pInstance) const override { SetCount(pInstance, 0); }

  virtual void SetCount(void* pInstance, nsUInt32 uiCount) const override
  {
    NS_ASSERT_DEBUG(m_Insert != nullptr && m_Remove != nullptr, "The property '{0}' has no remove and insert function, thus it is fixed-size.",
      nsAbstractProperty::GetPropertyName());
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
struct nsArrayPropertyAccessor
{
  using ContainerType = typename nsTypeTraits<Container>::NonConstReferenceType;
  using Type = typename nsTypeTraits<typename nsContainerSubTypeResolver<ContainerType>::Type>::NonConstReferenceType;

  static const ContainerType& GetConstContainer(const Class* pInstance) { return (*pInstance).*Member; }

  static ContainerType& GetContainer(Class* pInstance) { return (*pInstance).*Member; }
};


template <typename Class, typename Container, typename Type>
class nsMemberArrayProperty : public nsTypedArrayProperty<typename nsTypeTraits<Type>::NonConstReferenceType>
{
public:
  using RealType = typename nsTypeTraits<Type>::NonConstReferenceType;
  using GetConstContainerFunc = const Container& (*)(const Class* pInstance);
  using GetContainerFunc = Container& (*)(Class* pInstance);

  nsMemberArrayProperty(const char* szPropertyName, GetConstContainerFunc constGetter, GetContainerFunc getter)
    : nsTypedArrayProperty<RealType>(szPropertyName)
  {
    NS_ASSERT_DEBUG(constGetter != nullptr, "The const get count function of an array property cannot be nullptr.");

    m_ConstGetter = constGetter;
    m_Getter = getter;

    if (m_Getter == nullptr)
      nsAbstractArrayProperty::m_Flags.Add(nsPropertyFlags::ReadOnly);
  }

  virtual nsUInt32 GetCount(const void* pInstance) const override { return m_ConstGetter(static_cast<const Class*>(pInstance)).GetCount(); }

  virtual void GetValue(const void* pInstance, nsUInt32 uiIndex, void* pObject) const override
  {
    NS_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "GetValue: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    *static_cast<RealType*>(pObject) = m_ConstGetter(static_cast<const Class*>(pInstance))[uiIndex];
  }

  virtual void SetValue(void* pInstance, nsUInt32 uiIndex, const void* pObject) const override
  {
    NS_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "SetValue: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    NS_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const array accessor function, thus it is read-only.",
      nsAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance))[uiIndex] = *static_cast<const RealType*>(pObject);
  }

  virtual void Insert(void* pInstance, nsUInt32 uiIndex, const void* pObject) const override
  {
    NS_ASSERT_DEBUG(uiIndex <= GetCount(pInstance), "Insert: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    NS_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const array accessor function, thus it is read-only.",
      nsAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).InsertAt(uiIndex, *static_cast<const RealType*>(pObject));
  }

  virtual void Remove(void* pInstance, nsUInt32 uiIndex) const override
  {
    NS_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "Remove: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    NS_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const array accessor function, thus it is read-only.",
      nsAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).RemoveAtAndCopy(uiIndex);
  }

  virtual void Clear(void* pInstance) const override
  {
    NS_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const array accessor function, thus it is read-only.",
      nsAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).Clear();
  }

  virtual void SetCount(void* pInstance, nsUInt32 uiCount) const override
  {
    NS_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const array accessor function, thus it is read-only.",
      nsAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).SetCount(uiCount);
  }

  virtual void* GetValuePointer(void* pInstance, nsUInt32 uiIndex) const override
  {
    NS_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "GetValue: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    return &(m_Getter(static_cast<Class*>(pInstance))[uiIndex]);
  }

private:
  GetConstContainerFunc m_ConstGetter;
  GetContainerFunc m_Getter;
};

/// \brief Read only version of nsMemberArrayProperty that does not call any functions that modify the array. This is needed to reflect nsArrayPtr members.
template <typename Class, typename Container, typename Type>
class nsMemberArrayReadOnlyProperty : public nsTypedArrayProperty<typename nsTypeTraits<Type>::NonConstReferenceType>
{
public:
  using RealType = typename nsTypeTraits<Type>::NonConstReferenceType;
  using GetConstContainerFunc = const Container& (*)(const Class* pInstance);

  nsMemberArrayReadOnlyProperty(const char* szPropertyName, GetConstContainerFunc constGetter)
    : nsTypedArrayProperty<RealType>(szPropertyName)
  {
    NS_ASSERT_DEBUG(constGetter != nullptr, "The const get count function of an array property cannot be nullptr.");

    m_ConstGetter = constGetter;
    nsAbstractArrayProperty::m_Flags.Add(nsPropertyFlags::ReadOnly);
  }

  virtual nsUInt32 GetCount(const void* pInstance) const override { return m_ConstGetter(static_cast<const Class*>(pInstance)).GetCount(); }

  virtual void GetValue(const void* pInstance, nsUInt32 uiIndex, void* pObject) const override
  {
    NS_ASSERT_DEBUG(uiIndex < GetCount(pInstance), "GetValue: uiIndex ('{0}') is out of range ('{1}')", uiIndex, GetCount(pInstance));
    *static_cast<RealType*>(pObject) = m_ConstGetter(static_cast<const Class*>(pInstance))[uiIndex];
  }

  virtual void SetValue(void* pInstance, nsUInt32 uiIndex, const void* pObject) const override
  {
    NS_REPORT_FAILURE("The property '{0}' is read-only.", nsAbstractProperty::GetPropertyName());
  }

  virtual void Insert(void* pInstance, nsUInt32 uiIndex, const void* pObject) const override
  {
    NS_REPORT_FAILURE("The property '{0}' is read-only.", nsAbstractProperty::GetPropertyName());
  }

  virtual void Remove(void* pInstance, nsUInt32 uiIndex) const override
  {
    NS_REPORT_FAILURE("The property '{0}' is read-only.", nsAbstractProperty::GetPropertyName());
  }

  virtual void Clear(void* pInstance) const override
  {
    NS_REPORT_FAILURE("The property '{0}' is read-only.", nsAbstractProperty::GetPropertyName());
  }

  virtual void SetCount(void* pInstance, nsUInt32 uiCount) const override
  {
    NS_REPORT_FAILURE("The property '{0}' is read-only.", nsAbstractProperty::GetPropertyName());
  }

private:
  GetConstContainerFunc m_ConstGetter;
};
