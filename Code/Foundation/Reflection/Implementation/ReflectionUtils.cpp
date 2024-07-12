#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Types/ScopeExit.h>
#include <Foundation/Types/VariantTypeRegistry.h>

namespace
{
  // for some reason MSVC does not accept the template keyword here
#if NS_ENABLED(NS_COMPILER_MSVC_PURE)
#  define CALL_FUNCTOR(functor, type) functor.operator()<type>(std::forward<Args>(args)...)
#else
#  define CALL_FUNCTOR(functor, type) functor.template operator()<type>(std::forward<Args>(args)...)
#endif

  template <typename Functor, class... Args>
  void DispatchTo(Functor& ref_functor, const nsAbstractProperty* pProp, Args&&... args)
  {
    const bool bIsPtr = pProp->GetFlags().IsSet(nsPropertyFlags::Pointer);
    if (bIsPtr)
    {
      CALL_FUNCTOR(ref_functor, nsTypedPointer);
      return;
    }
    else if (pProp->GetSpecificType() == nsGetStaticRTTI<const char*>())
    {
      CALL_FUNCTOR(ref_functor, const char*);
      return;
    }
    else if (pProp->GetSpecificType() == nsGetStaticRTTI<nsUntrackedString>())
    {
      CALL_FUNCTOR(ref_functor, nsUntrackedString);
      return;
    }
    else if (pProp->GetSpecificType() == nsGetStaticRTTI<nsVariant>())
    {
      CALL_FUNCTOR(ref_functor, nsVariant);
      return;
    }
    else if (pProp->GetFlags().IsSet(nsPropertyFlags::StandardType))
    {
      nsVariant::DispatchTo(ref_functor, pProp->GetSpecificType()->GetVariantType(), std::forward<Args>(args)...);
      return;
    }
    else if (pProp->GetFlags().IsSet(nsPropertyFlags::IsEnum))
    {
      CALL_FUNCTOR(ref_functor, nsEnumBase);
      return;
    }
    else if (pProp->GetFlags().IsSet(nsPropertyFlags::Bitflags))
    {
      CALL_FUNCTOR(ref_functor, nsBitflagsBase);
      return;
    }
    else if (pProp->GetSpecificType()->GetVariantType() == nsVariantType::TypedObject)
    {
      CALL_FUNCTOR(ref_functor, nsTypedObject);
      return;
    }

    NS_REPORT_FAILURE("Unknown dispatch type");
  }

#undef CALL_FUNCTOR

  struct GetTypeFromVariantTypeFunc
  {
    template <typename T>
    NS_ALWAYS_INLINE void operator()()
    {
      m_pType = nsGetStaticRTTI<T>();
    }
    const nsRTTI* m_pType;
  };

  template <>
  NS_ALWAYS_INLINE void GetTypeFromVariantTypeFunc::operator()<nsTypedPointer>()
  {
    m_pType = nullptr;
  }
  template <>
  NS_ALWAYS_INLINE void GetTypeFromVariantTypeFunc::operator()<nsTypedObject>()
  {
    m_pType = nullptr;
  }

  //////////////////////////////////////////////////////////////////////////



  template <typename T>
  struct nsPropertyValue
  {
    using Type = T;
    using StorageType = typename nsVariantTypeDeduction<T>::StorageType;
  };
  template <>
  struct nsPropertyValue<nsEnumBase>
  {
    using Type = nsInt64;
    using StorageType = nsInt64;
  };
  template <>
  struct nsPropertyValue<nsBitflagsBase>
  {
    using Type = nsInt64;
    using StorageType = nsInt64;
  };

  //////////////////////////////////////////////////////////////////////////

  template <class T>
  struct nsVariantFromProperty
  {
    nsVariantFromProperty(nsVariant& value, const nsAbstractProperty* pProp)
      : m_value(value)
    {
    }
    ~nsVariantFromProperty()
    {
      if (m_bSuccess)
        m_value = m_tempValue;
    }

    operator void*()
    {
      return &m_tempValue;
    }

    nsVariant& m_value;
    typename nsPropertyValue<T>::Type m_tempValue = {};
    bool m_bSuccess = true;
  };

  template <>
  struct nsVariantFromProperty<nsVariant>
  {
    nsVariantFromProperty(nsVariant& value, const nsAbstractProperty* pProp)
      : m_value(value)
    {
    }

    operator void*()
    {
      return &m_value;
    }

    nsVariant& m_value;
    bool m_bSuccess = true;
  };

  template <>
  struct nsVariantFromProperty<nsTypedPointer>
  {
    nsVariantFromProperty(nsVariant& value, const nsAbstractProperty* pProp)
      : m_value(value)
      , m_pProp(pProp)
    {
    }
    ~nsVariantFromProperty()
    {
      if (m_bSuccess)
        m_value = nsVariant(m_ptr, m_pProp->GetSpecificType());
    }

    operator void*()
    {
      return &m_ptr;
    }

    nsVariant& m_value;
    const nsAbstractProperty* m_pProp = nullptr;
    void* m_ptr = nullptr;
    bool m_bSuccess = true;
  };

  template <>
  struct nsVariantFromProperty<nsTypedObject>
  {
    nsVariantFromProperty(nsVariant& value, const nsAbstractProperty* pProp)
      : m_value(value)
      , m_pProp(pProp)
    {
      m_ptr = m_pProp->GetSpecificType()->GetAllocator()->Allocate<void>();
    }
    ~nsVariantFromProperty()
    {
      if (m_bSuccess)
        m_value.MoveTypedObject(m_ptr, m_pProp->GetSpecificType());
      else
        m_pProp->GetSpecificType()->GetAllocator()->Deallocate(m_ptr);
    }

    operator void*()
    {
      return m_ptr;
    }

    nsVariant& m_value;
    const nsAbstractProperty* m_pProp = nullptr;
    void* m_ptr = nullptr;
    bool m_bSuccess = true;
  };

  //////////////////////////////////////////////////////////////////////////

  template <class T>
  struct nsVariantToProperty
  {
    nsVariantToProperty(const nsVariant& value, const nsAbstractProperty* pProp)
    {
      m_tempValue = value.ConvertTo<typename nsPropertyValue<T>::StorageType>();
    }

    operator const void*()
    {
      return &m_tempValue;
    }

    typename nsPropertyValue<T>::Type m_tempValue = {};
  };

  template <>
  struct nsVariantToProperty<const char*>
  {
    nsVariantToProperty(const nsVariant& value, const nsAbstractProperty* pProp)
    {
      m_sData = value.ConvertTo<nsString>();
      m_pValue = m_sData;
    }

    operator const void*()
    {
      return &m_pValue;
    }
    nsString m_sData;
    const char* m_pValue;
  };

  template <>
  struct nsVariantToProperty<nsVariant>
  {
    nsVariantToProperty(const nsVariant& value, const nsAbstractProperty* pProp)
      : m_value(value)
    {
    }

    operator const void*()
    {
      return const_cast<nsVariant*>(&m_value);
    }

    const nsVariant& m_value;
  };

  template <>
  struct nsVariantToProperty<nsTypedPointer>
  {
    nsVariantToProperty(const nsVariant& value, const nsAbstractProperty* pProp)
    {
      m_ptr = value.Get<nsTypedPointer>();
      NS_ASSERT_DEBUG(!m_ptr.m_pType || m_ptr.m_pType->IsDerivedFrom(pProp->GetSpecificType()),
        "Pointer of type '{0}' does not derive from '{}'", m_ptr.m_pType->GetTypeName(), pProp->GetSpecificType()->GetTypeName());
    }

    operator const void*()
    {
      return &m_ptr.m_pObject;
    }

    nsTypedPointer m_ptr;
  };


  template <>
  struct nsVariantToProperty<nsTypedObject>
  {
    nsVariantToProperty(const nsVariant& value, const nsAbstractProperty* pProp)
    {
      m_pPtr = value.GetData();
    }

    operator const void*()
    {
      return m_pPtr;
    }
    const void* m_pPtr = nullptr;
  };

  //////////////////////////////////////////////////////////////////////////

  struct GetValueFunc
  {
    template <typename T>
    NS_ALWAYS_INLINE void operator()(const nsAbstractMemberProperty* pProp, const void* pObject, nsVariant& value)
    {
      nsVariantFromProperty<T> getter(value, pProp);
      pProp->GetValuePtr(pObject, getter);
    }
  };

  struct SetValueFunc
  {
    template <typename T>
    NS_FORCE_INLINE void operator()(const nsAbstractMemberProperty* pProp, void* pObject, const nsVariant& value)
    {
      nsVariantToProperty<T> setter(value, pProp);
      pProp->SetValuePtr(pObject, setter);
    }
  };

  struct GetArrayValueFunc
  {
    template <typename T>
    NS_FORCE_INLINE void operator()(const nsAbstractArrayProperty* pProp, const void* pObject, nsUInt32 uiIndex, nsVariant& value)
    {
      nsVariantFromProperty<T> getter(value, pProp);
      pProp->GetValue(pObject, uiIndex, getter);
    }
  };

  struct SetArrayValueFunc
  {
    template <typename T>
    NS_FORCE_INLINE void operator()(const nsAbstractArrayProperty* pProp, void* pObject, nsUInt32 uiIndex, const nsVariant& value)
    {
      nsVariantToProperty<T> setter(value, pProp);
      pProp->SetValue(pObject, uiIndex, setter);
    }
  };

  struct InsertArrayValueFunc
  {
    template <typename T>
    NS_FORCE_INLINE void operator()(const nsAbstractArrayProperty* pProp, void* pObject, nsUInt32 uiIndex, const nsVariant& value)
    {
      nsVariantToProperty<T> setter(value, pProp);
      pProp->Insert(pObject, uiIndex, setter);
    }
  };

  struct InsertSetValueFunc
  {
    template <typename T>
    NS_FORCE_INLINE void operator()(const nsAbstractSetProperty* pProp, void* pObject, const nsVariant& value)
    {
      nsVariantToProperty<T> setter(value, pProp);
      pProp->Insert(pObject, setter);
    }
  };

  struct RemoveSetValueFunc
  {
    template <typename T>
    NS_FORCE_INLINE void operator()(const nsAbstractSetProperty* pProp, void* pObject, const nsVariant& value)
    {
      nsVariantToProperty<T> setter(value, pProp);
      pProp->Remove(pObject, setter);
    }
  };

  struct GetMapValueFunc
  {
    template <typename T>
    NS_FORCE_INLINE void operator()(const nsAbstractMapProperty* pProp, const void* pObject, const char* szKey, nsVariant& value)
    {
      nsVariantFromProperty<T> getter(value, pProp);
      getter.m_bSuccess = pProp->GetValue(pObject, szKey, getter);
    }
  };

  struct SetMapValueFunc
  {
    template <typename T>
    NS_FORCE_INLINE void operator()(const nsAbstractMapProperty* pProp, void* pObject, const char* szKey, const nsVariant& value)
    {
      nsVariantToProperty<T> setter(value, pProp);
      pProp->Insert(pObject, szKey, setter);
    }
  };

  static bool CompareProperties(const void* pObject, const void* pObject2, const nsRTTI* pType)
  {
    if (pType->GetParentType())
    {
      if (!CompareProperties(pObject, pObject2, pType->GetParentType()))
        return false;
    }

    for (auto* pProp : pType->GetProperties())
    {
      if (!nsReflectionUtils::IsEqual(pObject, pObject2, pProp))
        return false;
    }

    return true;
  }

  template <typename T>
  struct SetComponentValueImpl
  {
    NS_FORCE_INLINE static void impl(nsVariant* pVector, nsUInt32 uiComponent, double fValue) { NS_ASSERT_DEBUG(false, "nsReflectionUtils::SetComponent was called with a non-vector variant '{0}'", pVector->GetType()); }
  };

  template <typename T>
  struct SetComponentValueImpl<nsVec2Template<T>>
  {
    NS_FORCE_INLINE static void impl(nsVariant* pVector, nsUInt32 uiComponent, double fValue)
    {
      auto vec = pVector->Get<nsVec2Template<T>>();
      switch (uiComponent)
      {
        case 0:
          vec.x = static_cast<T>(fValue);
          break;
        case 1:
          vec.y = static_cast<T>(fValue);
          break;
      }
      *pVector = vec;
    }
  };

  template <typename T>
  struct SetComponentValueImpl<nsVec3Template<T>>
  {
    NS_FORCE_INLINE static void impl(nsVariant* pVector, nsUInt32 uiComponent, double fValue)
    {
      auto vec = pVector->Get<nsVec3Template<T>>();
      switch (uiComponent)
      {
        case 0:
          vec.x = static_cast<T>(fValue);
          break;
        case 1:
          vec.y = static_cast<T>(fValue);
          break;
        case 2:
          vec.z = static_cast<T>(fValue);
          break;
      }
      *pVector = vec;
    }
  };

  template <typename T>
  struct SetComponentValueImpl<nsVec4Template<T>>
  {
    NS_FORCE_INLINE static void impl(nsVariant* pVector, nsUInt32 uiComponent, double fValue)
    {
      auto vec = pVector->Get<nsVec4Template<T>>();
      switch (uiComponent)
      {
        case 0:
          vec.x = static_cast<T>(fValue);
          break;
        case 1:
          vec.y = static_cast<T>(fValue);
          break;
        case 2:
          vec.z = static_cast<T>(fValue);
          break;
        case 3:
          vec.w = static_cast<T>(fValue);
          break;
      }
      *pVector = vec;
    }
  };

  struct SetComponentValueFunc
  {
    template <typename T>
    NS_FORCE_INLINE void operator()()
    {
      SetComponentValueImpl<T>::impl(m_pVector, m_iComponent, m_fValue);
    }
    nsVariant* m_pVector;
    nsUInt32 m_iComponent;
    double m_fValue;
  };

  template <typename T>
  struct GetComponentValueImpl
  {
    NS_FORCE_INLINE static void impl(const nsVariant* pVector, nsUInt32 uiComponent, double& ref_fValue) { NS_ASSERT_DEBUG(false, "nsReflectionUtils::SetComponent was called with a non-vector variant '{0}'", pVector->GetType()); }
  };

  template <typename T>
  struct GetComponentValueImpl<nsVec2Template<T>>
  {
    NS_FORCE_INLINE static void impl(const nsVariant* pVector, nsUInt32 uiComponent, double& ref_fValue)
    {
      const auto& vec = pVector->Get<nsVec2Template<T>>();
      switch (uiComponent)
      {
        case 0:
          ref_fValue = static_cast<double>(vec.x);
          break;
        case 1:
          ref_fValue = static_cast<double>(vec.y);
          break;
      }
    }
  };

  template <typename T>
  struct GetComponentValueImpl<nsVec3Template<T>>
  {
    NS_FORCE_INLINE static void impl(const nsVariant* pVector, nsUInt32 uiComponent, double& ref_fValue)
    {
      const auto& vec = pVector->Get<nsVec3Template<T>>();
      switch (uiComponent)
      {
        case 0:
          ref_fValue = static_cast<double>(vec.x);
          break;
        case 1:
          ref_fValue = static_cast<double>(vec.y);
          break;
        case 2:
          ref_fValue = static_cast<double>(vec.z);
          break;
      }
    }
  };

  template <typename T>
  struct GetComponentValueImpl<nsVec4Template<T>>
  {
    NS_FORCE_INLINE static void impl(const nsVariant* pVector, nsUInt32 uiComponent, double& ref_fValue)
    {
      const auto& vec = pVector->Get<nsVec4Template<T>>();
      switch (uiComponent)
      {
        case 0:
          ref_fValue = static_cast<double>(vec.x);
          break;
        case 1:
          ref_fValue = static_cast<double>(vec.y);
          break;
        case 2:
          ref_fValue = static_cast<double>(vec.z);
          break;
        case 3:
          ref_fValue = static_cast<double>(vec.w);
          break;
      }
    }
  };

  struct GetComponentValueFunc
  {
    template <typename T>
    NS_FORCE_INLINE void operator()()
    {
      GetComponentValueImpl<T>::impl(m_pVector, m_iComponent, m_fValue);
    }
    const nsVariant* m_pVector;
    nsUInt32 m_iComponent;
    double m_fValue;
  };
} // namespace

const nsRTTI* nsReflectionUtils::GetCommonBaseType(const nsRTTI* pRtti1, const nsRTTI* pRtti2)
{
  if (pRtti2 == nullptr)
    return nullptr;

  while (pRtti1 != nullptr)
  {
    const nsRTTI* pRtti2Parent = pRtti2;

    while (pRtti2Parent != nullptr)
    {
      if (pRtti1 == pRtti2Parent)
        return pRtti2Parent;

      pRtti2Parent = pRtti2Parent->GetParentType();
    }

    pRtti1 = pRtti1->GetParentType();
  }

  return nullptr;
}

bool nsReflectionUtils::IsBasicType(const nsRTTI* pRtti)
{
  NS_ASSERT_DEBUG(pRtti != nullptr, "IsBasicType: missing data!");
  nsVariant::Type::Enum type = pRtti->GetVariantType();
  return type >= nsVariant::Type::FirstStandardType && type <= nsVariant::Type::LastStandardType;
}

bool nsReflectionUtils::IsValueType(const nsAbstractProperty* pProp)
{
  return !pProp->GetFlags().IsSet(nsPropertyFlags::Pointer) && (pProp->GetFlags().IsSet(nsPropertyFlags::StandardType) || nsVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(pProp->GetSpecificType()));
}

const nsRTTI* nsReflectionUtils::GetTypeFromVariant(const nsVariant& value)
{
  return value.GetReflectedType();
}

const nsRTTI* nsReflectionUtils::GetTypeFromVariant(nsVariantType::Enum type)
{
  GetTypeFromVariantTypeFunc func;
  func.m_pType = nullptr;
  nsVariant::DispatchTo(func, type);

  return func.m_pType;
}

nsUInt32 nsReflectionUtils::GetComponentCount(nsVariantType::Enum type)
{
  switch (type)
  {
    case nsVariant::Type::Vector2:
    case nsVariant::Type::Vector2I:
    case nsVariant::Type::Vector2U:
      return 2;
    case nsVariant::Type::Vector3:
    case nsVariant::Type::Vector3I:
    case nsVariant::Type::Vector3U:
      return 3;
    case nsVariant::Type::Vector4:
    case nsVariant::Type::Vector4I:
    case nsVariant::Type::Vector4U:
      return 4;
    default:
      NS_REPORT_FAILURE("Not a vector type: '{0}'", type);
      return 0;
  }
}

void nsReflectionUtils::SetComponent(nsVariant& ref_vector, nsUInt32 uiComponent, double fValue)
{
  SetComponentValueFunc func;
  func.m_pVector = &ref_vector;
  func.m_iComponent = uiComponent;
  func.m_fValue = fValue;
  nsVariant::DispatchTo(func, ref_vector.GetType());
}

double nsReflectionUtils::GetComponent(const nsVariant& vector, nsUInt32 uiComponent)
{
  GetComponentValueFunc func;
  func.m_pVector = &vector;
  func.m_iComponent = uiComponent;
  nsVariant::DispatchTo(func, vector.GetType());
  return func.m_fValue;
}

nsVariant nsReflectionUtils::GetMemberPropertyValue(const nsAbstractMemberProperty* pProp, const void* pObject)
{
  nsVariant res;
  NS_ASSERT_DEBUG(pProp != nullptr, "GetMemberPropertyValue: missing data!");

  GetValueFunc func;
  DispatchTo(func, pProp, pProp, pObject, res);

  return res;
}

void nsReflectionUtils::SetMemberPropertyValue(const nsAbstractMemberProperty* pProp, void* pObject, const nsVariant& value)
{
  NS_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "SetMemberPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
    return;

  if (pProp->GetFlags().IsAnySet(nsPropertyFlags::Bitflags | nsPropertyFlags::IsEnum))
  {
    auto pEnumerationProp = static_cast<const nsAbstractEnumerationProperty*>(pProp);

    // Value can either be an integer or a string (human readable value)
    if (value.IsA<nsString>())
    {
      nsInt64 iValue;
      nsReflectionUtils::StringToEnumeration(pProp->GetSpecificType(), value.Get<nsString>(), iValue);
      pEnumerationProp->SetValue(pObject, iValue);
    }
    else
    {
      pEnumerationProp->SetValue(pObject, value.ConvertTo<nsInt64>());
    }
  }
  else
  {
    SetValueFunc func;
    DispatchTo(func, pProp, pProp, pObject, value);
  }
}

nsVariant nsReflectionUtils::GetArrayPropertyValue(const nsAbstractArrayProperty* pProp, const void* pObject, nsUInt32 uiIndex)
{
  nsVariant res;
  NS_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "GetArrayPropertyValue: missing data!");
  auto uiCount = pProp->GetCount(pObject);
  if (uiIndex >= uiCount)
  {
    nsLog::Error("GetArrayPropertyValue: Invalid index: {0}", uiIndex);
  }
  else
  {
    GetArrayValueFunc func;
    DispatchTo(func, pProp, pProp, pObject, uiIndex, res);
  }
  return res;
}

void nsReflectionUtils::SetArrayPropertyValue(const nsAbstractArrayProperty* pProp, void* pObject, nsUInt32 uiIndex, const nsVariant& value)
{
  NS_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "GetArrayPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
    return;

  auto uiCount = pProp->GetCount(pObject);
  if (uiIndex >= uiCount)
  {
    nsLog::Error("SetArrayPropertyValue: Invalid index: {0}", uiIndex);
  }
  else
  {
    SetArrayValueFunc func;
    DispatchTo(func, pProp, pProp, pObject, uiIndex, value);
  }
}

void nsReflectionUtils::InsertSetPropertyValue(const nsAbstractSetProperty* pProp, void* pObject, const nsVariant& value)
{
  NS_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "InsertSetPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
    return;

  InsertSetValueFunc func;
  DispatchTo(func, pProp, pProp, pObject, value);
}

void nsReflectionUtils::RemoveSetPropertyValue(const nsAbstractSetProperty* pProp, void* pObject, const nsVariant& value)
{
  NS_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "RemoveSetPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
    return;

  RemoveSetValueFunc func;
  DispatchTo(func, pProp, pProp, pObject, value);
}

nsVariant nsReflectionUtils::GetMapPropertyValue(const nsAbstractMapProperty* pProp, const void* pObject, const char* szKey)
{
  nsVariant value;
  NS_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "GetMapPropertyValue: missing data!");

  GetMapValueFunc func;
  DispatchTo(func, pProp, pProp, pObject, szKey, value);
  return value;
}

void nsReflectionUtils::SetMapPropertyValue(const nsAbstractMapProperty* pProp, void* pObject, const char* szKey, const nsVariant& value)
{
  NS_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "SetMapPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
    return;

  SetMapValueFunc func;
  DispatchTo(func, pProp, pProp, pObject, szKey, value);
}

void nsReflectionUtils::InsertArrayPropertyValue(const nsAbstractArrayProperty* pProp, void* pObject, const nsVariant& value, nsUInt32 uiIndex)
{
  NS_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "InsertArrayPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
    return;

  auto uiCount = pProp->GetCount(pObject);
  if (uiIndex > uiCount)
  {
    nsLog::Error("InsertArrayPropertyValue: Invalid index: {0}", uiIndex);
    return;
  }

  InsertArrayValueFunc func;
  DispatchTo(func, pProp, pProp, pObject, uiIndex, value);
}

void nsReflectionUtils::RemoveArrayPropertyValue(const nsAbstractArrayProperty* pProp, void* pObject, nsUInt32 uiIndex)
{
  NS_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "RemoveArrayPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
    return;

  auto uiCount = pProp->GetCount(pObject);
  if (uiIndex >= uiCount)
  {
    nsLog::Error("RemoveArrayPropertyValue: Invalid index: {0}", uiIndex);
    return;
  }

  pProp->Remove(pObject, uiIndex);
}

const nsAbstractMemberProperty* nsReflectionUtils::GetMemberProperty(const nsRTTI* pRtti, nsUInt32 uiPropertyIndex)
{
  if (pRtti == nullptr)
    return nullptr;

  nsHybridArray<const nsAbstractProperty*, 32> props;
  pRtti->GetAllProperties(props);
  if (uiPropertyIndex < props.GetCount())
  {
    const nsAbstractProperty* pProp = props[uiPropertyIndex];
    if (pProp->GetCategory() == nsPropertyCategory::Member)
      return static_cast<const nsAbstractMemberProperty*>(pProp);
  }

  return nullptr;
}

const nsAbstractMemberProperty* nsReflectionUtils::GetMemberProperty(const nsRTTI* pRtti, const char* szPropertyName)
{
  if (pRtti == nullptr)
    return nullptr;

  if (const nsAbstractProperty* pProp = pRtti->FindPropertyByName(szPropertyName))
  {
    if (pProp->GetCategory() == nsPropertyCategory::Member)
      return static_cast<const nsAbstractMemberProperty*>(pProp);
  }

  return nullptr;
}

void nsReflectionUtils::GatherTypesDerivedFromClass(const nsRTTI* pBaseRtti, nsSet<const nsRTTI*>& out_types)
{
  nsRTTI::ForEachDerivedType(pBaseRtti,
    [&](const nsRTTI* pRtti)
    {
      out_types.Insert(pRtti);
    });
}

void nsReflectionUtils::GatherDependentTypes(const nsRTTI* pRtti, nsSet<const nsRTTI*>& inout_typesAsSet, nsDynamicArray<const nsRTTI*>* out_pTypesAsStack /*= nullptr*/)
{
  auto AddType = [&](const nsRTTI* pNewRtti)
  {
    if (pNewRtti != pRtti && pNewRtti->GetTypeFlags().IsSet(nsTypeFlags::StandardType) == false && inout_typesAsSet.Contains(pNewRtti) == false)
    {
      inout_typesAsSet.Insert(pNewRtti);
      if (out_pTypesAsStack != nullptr)
      {
        out_pTypesAsStack->PushBack(pNewRtti);
      }

      GatherDependentTypes(pNewRtti, inout_typesAsSet, out_pTypesAsStack);
    }
  };

  if (const nsRTTI* pParentRtti = pRtti->GetParentType())
  {
    AddType(pParentRtti);
  }

  for (const nsAbstractProperty* prop : pRtti->GetProperties())
  {
    if (prop->GetCategory() == nsPropertyCategory::Constant)
      continue;

    if (prop->GetAttributeByType<nsTemporaryAttribute>() != nullptr)
      continue;

    AddType(prop->GetSpecificType());
  }

  for (const nsAbstractFunctionProperty* func : pRtti->GetFunctions())
  {
    nsUInt32 uiNumArgs = func->GetArgumentCount();
    for (nsUInt32 i = 0; i < uiNumArgs; ++i)
    {
      AddType(func->GetArgumentType(i));
    }
  }

  for (const nsPropertyAttribute* attr : pRtti->GetAttributes())
  {
    AddType(attr->GetDynamicRTTI());
  }
}

nsResult nsReflectionUtils::CreateDependencySortedTypeArray(const nsSet<const nsRTTI*>& types, nsDynamicArray<const nsRTTI*>& out_sortedTypes)
{
  out_sortedTypes.Clear();
  out_sortedTypes.Reserve(types.GetCount());

  nsSet<const nsRTTI*> accu;
  nsDynamicArray<const nsRTTI*> tmpStack;

  for (const nsRTTI* pType : types)
  {
    if (accu.Contains(pType))
      continue;

    GatherDependentTypes(pType, accu, &tmpStack);

    while (tmpStack.IsEmpty() == false)
    {
      const nsRTTI* pDependentType = tmpStack.PeekBack();
      NS_ASSERT_DEBUG(pDependentType != pType, "A type must not be reported as dependency of itself");
      tmpStack.PopBack();

      if (types.Contains(pDependentType) == false)
        return NS_FAILURE;

      out_sortedTypes.PushBack(pDependentType);
    }

    accu.Insert(pType);
    out_sortedTypes.PushBack(pType);
  }

  NS_ASSERT_DEV(types.GetCount() == out_sortedTypes.GetCount(), "Not all types have been sorted or the sorted list contains duplicates");
  return NS_SUCCESS;
}

bool nsReflectionUtils::EnumerationToString(const nsRTTI* pEnumerationRtti, nsInt64 iValue, nsStringBuilder& out_sOutput, nsEnum<EnumConversionMode> conversionMode)
{
  out_sOutput.Clear();
  if (pEnumerationRtti->IsDerivedFrom<nsEnumBase>())
  {
    for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
    {
      if (pProp->GetCategory() == nsPropertyCategory::Constant)
      {
        nsVariant value = static_cast<const nsAbstractConstantProperty*>(pProp)->GetConstant();
        if (value.ConvertTo<nsInt64>() == iValue)
        {
          out_sOutput = conversionMode == EnumConversionMode::FullyQualifiedName ? pProp->GetPropertyName() : nsStringUtils::FindLastSubString(pProp->GetPropertyName(), "::") + 2;
          return true;
        }
      }
    }
    return false;
  }
  else if (pEnumerationRtti->IsDerivedFrom<nsBitflagsBase>())
  {
    for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
    {
      if (pProp->GetCategory() == nsPropertyCategory::Constant)
      {
        nsVariant value = static_cast<const nsAbstractConstantProperty*>(pProp)->GetConstant();
        if ((value.ConvertTo<nsInt64>() & iValue) != 0)
        {
          out_sOutput.Append(conversionMode == EnumConversionMode::FullyQualifiedName ? pProp->GetPropertyName() : nsStringUtils::FindLastSubString(pProp->GetPropertyName(), "::") + 2, "|");
        }
      }
    }
    out_sOutput.Shrink(0, 1);
    return true;
  }
  else
  {
    NS_ASSERT_DEV(false, "The RTTI class '{0}' is not an enum or bitflags class", pEnumerationRtti->GetTypeName());
    return false;
  }
}

void nsReflectionUtils::GetEnumKeysAndValues(const nsRTTI* pEnumerationRtti, nsDynamicArray<EnumKeyValuePair>& ref_entries, nsEnum<EnumConversionMode> conversionMode)
{
  /// \test This is new.

  ref_entries.Clear();

  if (pEnumerationRtti->IsDerivedFrom<nsEnumBase>())
  {
    for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
    {
      if (pProp->GetCategory() == nsPropertyCategory::Constant)
      {
        nsVariant value = static_cast<const nsAbstractConstantProperty*>(pProp)->GetConstant();

        auto& e = ref_entries.ExpandAndGetRef();
        e.m_sKey = conversionMode == EnumConversionMode::FullyQualifiedName ? pProp->GetPropertyName() : nsStringUtils::FindLastSubString(pProp->GetPropertyName(), "::") + 2;
        e.m_iValue = value.ConvertTo<nsInt32>();
      }
    }
  }
}

bool nsReflectionUtils::StringToEnumeration(const nsRTTI* pEnumerationRtti, const char* szValue, nsInt64& out_iValue)
{
  out_iValue = 0;
  if (pEnumerationRtti->IsDerivedFrom<nsEnumBase>())
  {
    for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
    {
      if (pProp->GetCategory() == nsPropertyCategory::Constant)
      {
        // Testing fully qualified and short value name
        const char* valueNameOnly = nsStringUtils::FindLastSubString(pProp->GetPropertyName(), "::", nullptr);
        if (nsStringUtils::IsEqual(pProp->GetPropertyName(), szValue) || (valueNameOnly != nullptr && nsStringUtils::IsEqual(valueNameOnly + 2, szValue)))
        {
          nsVariant value = static_cast<const nsAbstractConstantProperty*>(pProp)->GetConstant();
          out_iValue = value.ConvertTo<nsInt64>();
          return true;
        }
      }
    }
    return false;
  }
  else if (pEnumerationRtti->IsDerivedFrom<nsBitflagsBase>())
  {
    nsStringBuilder temp = szValue;
    nsHybridArray<nsStringView, 32> values;
    temp.Split(false, values, "|");
    for (auto sValue : values)
    {
      for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
      {
        if (pProp->GetCategory() == nsPropertyCategory::Constant)
        {
          // Testing fully qualified and short value name
          const char* valueNameOnly = nsStringUtils::FindLastSubString(pProp->GetPropertyName(), "::", nullptr);
          if (sValue.IsEqual(pProp->GetPropertyName()) || (valueNameOnly != nullptr && sValue.IsEqual(valueNameOnly + 2)))
          {
            nsVariant value = static_cast<const nsAbstractConstantProperty*>(pProp)->GetConstant();
            out_iValue |= value.ConvertTo<nsInt64>();
          }
        }
      }
    }
    return true;
  }
  else
  {
    NS_REPORT_FAILURE("The RTTI class '{0}' is not an enum or bitflags class", pEnumerationRtti->GetTypeName());
    return false;
  }
}

nsInt64 nsReflectionUtils::DefaultEnumerationValue(const nsRTTI* pEnumerationRtti)
{
  if (pEnumerationRtti->IsDerivedFrom<nsEnumBase>() || pEnumerationRtti->IsDerivedFrom<nsBitflagsBase>())
  {
    auto pProp = pEnumerationRtti->GetProperties()[0];
    NS_ASSERT_DEBUG(pProp->GetCategory() == nsPropertyCategory::Constant && nsStringUtils::EndsWith(pProp->GetPropertyName(), "::Default"), "First enumeration property must be the default value constant.");
    return static_cast<const nsAbstractConstantProperty*>(pProp)->GetConstant().ConvertTo<nsInt64>();
  }
  else
  {
    NS_REPORT_FAILURE("The RTTI class '{0}' is not an enum or bitflags class", pEnumerationRtti->GetTypeName());
    return 0;
  }
}

nsInt64 nsReflectionUtils::MakeEnumerationValid(const nsRTTI* pEnumerationRtti, nsInt64 iValue)
{
  if (pEnumerationRtti->IsDerivedFrom<nsEnumBase>())
  {
    // Find current value
    for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
    {
      if (pProp->GetCategory() == nsPropertyCategory::Constant)
      {
        nsInt64 iCurrentValue = static_cast<const nsAbstractConstantProperty*>(pProp)->GetConstant().ConvertTo<nsInt64>();
        if (iCurrentValue == iValue)
          return iValue;
      }
    }

    // Current value not found, return default value
    return nsReflectionUtils::DefaultEnumerationValue(pEnumerationRtti);
  }
  else if (pEnumerationRtti->IsDerivedFrom<nsBitflagsBase>())
  {
    nsInt64 iNewValue = 0;
    // Filter valid bits
    for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
    {
      if (pProp->GetCategory() == nsPropertyCategory::Constant)
      {
        nsInt64 iCurrentValue = static_cast<const nsAbstractConstantProperty*>(pProp)->GetConstant().ConvertTo<nsInt64>();
        if ((iCurrentValue & iValue) != 0)
        {
          iNewValue |= iCurrentValue;
        }
      }
    }
    return iNewValue;
  }
  else
  {
    NS_REPORT_FAILURE("The RTTI class '{0}' is not an enum or bitflags class", pEnumerationRtti->GetTypeName());
    return 0;
  }
}

bool nsReflectionUtils::IsEqual(const void* pObject, const void* pObject2, const nsAbstractProperty* pProp)
{
  // #VAR TEST
  const nsRTTI* pPropType = pProp->GetSpecificType();

  nsVariant vTemp;
  nsVariant vTemp2;

  const bool bIsValueType = nsReflectionUtils::IsValueType(pProp);

  switch (pProp->GetCategory())
  {
    case nsPropertyCategory::Member:
    {
      auto pSpecific = static_cast<const nsAbstractMemberProperty*>(pProp);

      if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
      {
        vTemp = nsReflectionUtils::GetMemberPropertyValue(pSpecific, pObject);
        vTemp2 = nsReflectionUtils::GetMemberPropertyValue(pSpecific, pObject2);
        void* pRefrencedObject = vTemp.ConvertTo<void*>();
        void* pRefrencedObject2 = vTemp2.ConvertTo<void*>();
        if ((pRefrencedObject == nullptr) != (pRefrencedObject2 == nullptr))
          return false;
        if ((pRefrencedObject == nullptr) && (pRefrencedObject2 == nullptr))
          return true;

        if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
        {
          return IsEqual(pRefrencedObject, pRefrencedObject2, pPropType);
        }
        else
        {
          return pRefrencedObject == pRefrencedObject2;
        }
      }
      else
      {
        if (pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags) || bIsValueType)
        {
          vTemp = nsReflectionUtils::GetMemberPropertyValue(pSpecific, pObject);
          vTemp2 = nsReflectionUtils::GetMemberPropertyValue(pSpecific, pObject2);
          return vTemp == vTemp2;
        }
        else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class))
        {
          void* pSubObject = pSpecific->GetPropertyPointer(pObject);
          void* pSubObject2 = pSpecific->GetPropertyPointer(pObject2);
          // Do we have direct access to the property?
          if (pSubObject != nullptr)
          {
            return IsEqual(pSubObject, pSubObject2, pPropType);
          }
          // If the property is behind an accessor, we need to retrieve it first.
          else if (pPropType->GetAllocator()->CanAllocate())
          {
            pSubObject = pPropType->GetAllocator()->Allocate<void>();
            pSubObject2 = pPropType->GetAllocator()->Allocate<void>();
            pSpecific->GetValuePtr(pObject, pSubObject);
            pSpecific->GetValuePtr(pObject2, pSubObject2);
            bool bEqual = IsEqual(pSubObject, pSubObject2, pPropType);
            pPropType->GetAllocator()->Deallocate(pSubObject);
            pPropType->GetAllocator()->Deallocate(pSubObject2);
            return bEqual;
          }
          else
          {
            // TODO: return false if prop can't be compared?
            return true;
          }
        }
      }
    }
    break;
    case nsPropertyCategory::Array:
    {
      auto pSpecific = static_cast<const nsAbstractArrayProperty*>(pProp);

      const nsUInt32 uiCount = pSpecific->GetCount(pObject);
      const nsUInt32 uiCount2 = pSpecific->GetCount(pObject2);
      if (uiCount != uiCount2)
        return false;

      if (pSpecific->GetFlags().IsSet(nsPropertyFlags::Pointer))
      {
        for (nsUInt32 i = 0; i < uiCount; ++i)
        {
          vTemp = nsReflectionUtils::GetArrayPropertyValue(pSpecific, pObject, i);
          vTemp2 = nsReflectionUtils::GetArrayPropertyValue(pSpecific, pObject2, i);
          void* pRefrencedObject = vTemp.ConvertTo<void*>();
          void* pRefrencedObject2 = vTemp2.ConvertTo<void*>();
          if ((pRefrencedObject == nullptr) != (pRefrencedObject2 == nullptr))
            return false;
          if ((pRefrencedObject == nullptr) && (pRefrencedObject2 == nullptr))
            continue;

          if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
          {
            if (!IsEqual(pRefrencedObject, pRefrencedObject2, pPropType))
              return false;
          }
          else
          {
            if (pRefrencedObject != pRefrencedObject2)
              return false;
          }
        }
        return true;
      }
      else
      {
        if (bIsValueType)
        {
          for (nsUInt32 i = 0; i < uiCount; ++i)
          {
            vTemp = nsReflectionUtils::GetArrayPropertyValue(pSpecific, pObject, i);
            vTemp2 = nsReflectionUtils::GetArrayPropertyValue(pSpecific, pObject2, i);
            if (vTemp != vTemp2)
              return false;
          }
          return true;
        }
        else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class) && pPropType->GetAllocator()->CanAllocate())
        {
          void* pSubObject = pPropType->GetAllocator()->Allocate<void>();
          void* pSubObject2 = pPropType->GetAllocator()->Allocate<void>();

          bool bEqual = true;
          for (nsUInt32 i = 0; i < uiCount; ++i)
          {
            pSpecific->GetValue(pObject, i, pSubObject);
            pSpecific->GetValue(pObject2, i, pSubObject2);
            bEqual = IsEqual(pSubObject, pSubObject2, pPropType);
            if (!bEqual)
              break;
          }

          pPropType->GetAllocator()->Deallocate(pSubObject);
          pPropType->GetAllocator()->Deallocate(pSubObject2);
          return bEqual;
        }
      }
    }
    break;
    case nsPropertyCategory::Set:
    {
      auto pSpecific = static_cast<const nsAbstractSetProperty*>(pProp);

      nsHybridArray<nsVariant, 16> values;
      pSpecific->GetValues(pObject, values);
      nsHybridArray<nsVariant, 16> values2;
      pSpecific->GetValues(pObject2, values2);

      const nsUInt32 uiCount = values.GetCount();
      const nsUInt32 uiCount2 = values2.GetCount();
      if (uiCount != uiCount2)
        return false;

      if (bIsValueType || (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner)))
      {
        bool bEqual = true;
        for (nsUInt32 i = 0; i < uiCount; ++i)
        {
          bEqual = values2.Contains(values[i]);
          if (!bEqual)
            break;
        }
        return bEqual;
      }
      else if (pProp->GetFlags().AreAllSet(nsPropertyFlags::Pointer | nsPropertyFlags::PointerOwner))
      {
        // TODO: pointer sets are never stable unless they use an array based pseudo set as storage.
        bool bEqual = true;
        for (nsUInt32 i = 0; i < uiCount; ++i)
        {
          if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
          {
            void* pRefrencedObject = values[i].ConvertTo<void*>();
            void* pRefrencedObject2 = values2[i].ConvertTo<void*>();
            if ((pRefrencedObject == nullptr) != (pRefrencedObject2 == nullptr))
              return false;
            if ((pRefrencedObject == nullptr) && (pRefrencedObject2 == nullptr))
              continue;

            bEqual = IsEqual(pRefrencedObject, pRefrencedObject2, pPropType);
          }
          if (!bEqual)
            break;
        }

        return bEqual;
      }
    }
    break;
    case nsPropertyCategory::Map:
    {
      auto pSpecific = static_cast<const nsAbstractMapProperty*>(pProp);

      nsHybridArray<nsString, 16> keys;
      pSpecific->GetKeys(pObject, keys);
      nsHybridArray<nsString, 16> keys2;
      pSpecific->GetKeys(pObject2, keys2);

      const nsUInt32 uiCount = keys.GetCount();
      const nsUInt32 uiCount2 = keys2.GetCount();
      if (uiCount != uiCount2)
        return false;

      if (bIsValueType || (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner)))
      {
        bool bEqual = true;
        for (nsUInt32 i = 0; i < uiCount; ++i)
        {
          bEqual = keys2.Contains(keys[i]);
          if (!bEqual)
            break;
          nsVariant value1 = GetMapPropertyValue(pSpecific, pObject, keys[i]);
          nsVariant value2 = GetMapPropertyValue(pSpecific, pObject2, keys[i]);
          bEqual = value1 == value2;
          if (!bEqual)
            break;
        }
        return bEqual;
      }
      else if ((!pProp->GetFlags().IsSet(nsPropertyFlags::Pointer) || pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner)) && pProp->GetFlags().IsSet(nsPropertyFlags::Class))
      {
        bool bEqual = true;
        for (nsUInt32 i = 0; i < uiCount; ++i)
        {
          bEqual = keys2.Contains(keys[i]);
          if (!bEqual)
            break;

          if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
          {
            const void* value1 = nullptr;
            const void* value2 = nullptr;
            pSpecific->GetValue(pObject, keys[i], &value1);
            pSpecific->GetValue(pObject2, keys[i], &value2);
            if ((value1 == nullptr) != (value2 == nullptr))
              return false;
            if ((value1 == nullptr) && (value2 == nullptr))
              continue;
            bEqual = IsEqual(value1, value2, pPropType);
          }
          else
          {
            if (pPropType->GetAllocator()->CanAllocate())
            {
              void* value1 = pPropType->GetAllocator()->Allocate<void>();
              NS_SCOPE_EXIT(pPropType->GetAllocator()->Deallocate(value1););
              void* value2 = pPropType->GetAllocator()->Allocate<void>();
              NS_SCOPE_EXIT(pPropType->GetAllocator()->Deallocate(value2););

              bool bRes1 = pSpecific->GetValue(pObject, keys[i], value1);
              bool bRes2 = pSpecific->GetValue(pObject2, keys[i], value2);
              if (bRes1 != bRes2)
                return false;
              if (!bRes1 && !bRes2)
                continue;
              bEqual = IsEqual(value1, value2, pPropType);
            }
            else
            {
              nsLog::Error("The property '{0}' can not be compared as the type '{1}' cannot be allocated.", pProp->GetPropertyName(), pPropType->GetTypeName());
            }
          }
          if (!bEqual)
            break;
        }
        return bEqual;
      }
    }
    break;

    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      break;
  }
  return true;
}

bool nsReflectionUtils::IsEqual(const void* pObject, const void* pObject2, const nsRTTI* pType)
{
  NS_ASSERT_DEV(pObject && pObject2 && pType, "invalid type.");
  if (pType->IsDerivedFrom<nsReflectedClass>())
  {
    const nsReflectedClass* pRefObject = static_cast<const nsReflectedClass*>(pObject);
    const nsReflectedClass* pRefObject2 = static_cast<const nsReflectedClass*>(pObject2);
    pType = pRefObject->GetDynamicRTTI();
    if (pType != pRefObject2->GetDynamicRTTI())
      return false;
  }

  return CompareProperties(pObject, pObject2, pType);
}


void nsReflectionUtils::DeleteObject(void* pObject, const nsAbstractProperty* pOwnerProperty)
{
  if (!pObject)
    return;

  const nsRTTI* pType = pOwnerProperty->GetSpecificType();
  if (pType->IsDerivedFrom<nsReflectedClass>())
  {
    nsReflectedClass* pRefObject = static_cast<nsReflectedClass*>(pObject);
    pType = pRefObject->GetDynamicRTTI();
  }

  if (!pType->GetAllocator()->CanAllocate())
  {
    nsLog::Error("Tried to deallocate object of type '{0}', but it has no allocator.", pType->GetTypeName());
    return;
  }
  pType->GetAllocator()->Deallocate(pObject);
}

nsVariant nsReflectionUtils::GetDefaultVariantFromType(nsVariant::Type::Enum type)
{
  switch (type)
  {
    case nsVariant::Type::Invalid:
      return nsVariant();
    case nsVariant::Type::Bool:
      return nsVariant(false);
    case nsVariant::Type::Int8:
      return nsVariant((nsInt8)0);
    case nsVariant::Type::UInt8:
      return nsVariant((nsUInt8)0);
    case nsVariant::Type::Int16:
      return nsVariant((nsInt16)0);
    case nsVariant::Type::UInt16:
      return nsVariant((nsUInt16)0);
    case nsVariant::Type::Int32:
      return nsVariant((nsInt32)0);
    case nsVariant::Type::UInt32:
      return nsVariant((nsUInt32)0);
    case nsVariant::Type::Int64:
      return nsVariant((nsInt64)0);
    case nsVariant::Type::UInt64:
      return nsVariant((nsUInt64)0);
    case nsVariant::Type::Float:
      return nsVariant(0.0f);
    case nsVariant::Type::Double:
      return nsVariant(0.0);
    case nsVariant::Type::Color:
      return nsVariant(nsColor(1.0f, 1.0f, 1.0f));
    case nsVariant::Type::ColorGamma:
      return nsVariant(nsColorGammaUB(255, 255, 255));
    case nsVariant::Type::Vector2:
      return nsVariant(nsVec2(0.0f, 0.0f));
    case nsVariant::Type::Vector3:
      return nsVariant(nsVec3(0.0f, 0.0f, 0.0f));
    case nsVariant::Type::Vector4:
      return nsVariant(nsVec4(0.0f, 0.0f, 0.0f, 0.0f));
    case nsVariant::Type::Vector2I:
      return nsVariant(nsVec2I32(0, 0));
    case nsVariant::Type::Vector3I:
      return nsVariant(nsVec3I32(0, 0, 0));
    case nsVariant::Type::Vector4I:
      return nsVariant(nsVec4I32(0, 0, 0, 0));
    case nsVariant::Type::Vector2U:
      return nsVariant(nsVec2U32(0, 0));
    case nsVariant::Type::Vector3U:
      return nsVariant(nsVec3U32(0, 0, 0));
    case nsVariant::Type::Vector4U:
      return nsVariant(nsVec4U32(0, 0, 0, 0));
    case nsVariant::Type::Quaternion:
      return nsVariant(nsQuat(0.0f, 0.0f, 0.0f, 1.0f));
    case nsVariant::Type::Matrix3:
      return nsVariant(nsMat3::MakeIdentity());
    case nsVariant::Type::Matrix4:
      return nsVariant(nsMat4::MakeIdentity());
    case nsVariant::Type::Transform:
      return nsVariant(nsTransform::MakeIdentity());
    case nsVariant::Type::String:
      return nsVariant(nsString());
    case nsVariant::Type::StringView:
      return nsVariant(nsStringView(), false);
    case nsVariant::Type::DataBuffer:
      return nsVariant(nsDataBuffer());
    case nsVariant::Type::Time:
      return nsVariant(nsTime());
    case nsVariant::Type::Uuid:
      return nsVariant(nsUuid());
    case nsVariant::Type::Angle:
      return nsVariant(nsAngle());
    case nsVariant::Type::HashedString:
      return nsVariant(nsHashedString());
    case nsVariant::Type::TempHashedString:
      return nsVariant(nsTempHashedString());
    case nsVariant::Type::VariantArray:
      return nsVariantArray();
    case nsVariant::Type::VariantDictionary:
      return nsVariantDictionary();
    case nsVariant::Type::TypedPointer:
      return nsVariant(static_cast<void*>(nullptr), nullptr);

    default:
      NS_REPORT_FAILURE("Invalid case statement");
      return nsVariant();
  }
}

nsVariant nsReflectionUtils::GetDefaultValue(const nsAbstractProperty* pProperty, nsVariant index)
{
  const bool isValueType = nsReflectionUtils::IsValueType(pProperty);
  const nsVariantType::Enum type = pProperty->GetFlags().IsSet(nsPropertyFlags::Pointer) || (pProperty->GetFlags().IsSet(nsPropertyFlags::Class) && !isValueType) ? nsVariantType::Uuid : pProperty->GetSpecificType()->GetVariantType();
  const nsDefaultValueAttribute* pAttrib = pProperty->GetAttributeByType<nsDefaultValueAttribute>();

  switch (pProperty->GetCategory())
  {
    case nsPropertyCategory::Member:
    {
      if (isValueType)
      {
        if (pAttrib)
        {
          if (pProperty->GetSpecificType() == nsGetStaticRTTI<nsVariant>())
            return pAttrib->GetValue();
          if (pAttrib->GetValue().CanConvertTo(type))
            return pAttrib->GetValue().ConvertTo(type);
        }
        return GetDefaultVariantFromType(pProperty->GetSpecificType());
      }
      else if (pProperty->GetSpecificType()->GetTypeFlags().IsAnySet(nsTypeFlags::IsEnum | nsTypeFlags::Bitflags))
      {
        nsInt64 iValue = nsReflectionUtils::DefaultEnumerationValue(pProperty->GetSpecificType());
        if (pAttrib)
        {
          if (pAttrib->GetValue().CanConvertTo(nsVariantType::Int64))
            iValue = pAttrib->GetValue().ConvertTo<nsInt64>();
        }
        return nsReflectionUtils::MakeEnumerationValid(pProperty->GetSpecificType(), iValue);
      }
      else // Class
      {
        return nsUuid();
      }
    }
    break;
    case nsPropertyCategory::Array:
    case nsPropertyCategory::Set:
      if (isValueType)
      {
        if (pAttrib)
        {
          if (pAttrib->GetValue().IsA<nsVariantArray>())
          {
            if (!index.IsValid())
              return pAttrib->GetValue();

            nsUInt32 iIndex = index.ConvertTo<nsUInt32>();
            const auto& defaultArray = pAttrib->GetValue().Get<nsVariantArray>();
            if (iIndex < defaultArray.GetCount())
            {
              return defaultArray[iIndex];
            }
            return GetDefaultVariantFromType(pProperty->GetSpecificType());
          }
          if (index.IsValid() && pAttrib->GetValue().CanConvertTo(type))
            return pAttrib->GetValue().ConvertTo(type);
        }

        if (!index.IsValid())
          return nsVariantArray();

        return GetDefaultVariantFromType(pProperty->GetSpecificType());
      }
      else
      {
        if (!index.IsValid())
          return nsVariantArray();

        return nsUuid();
      }
      break;
    case nsPropertyCategory::Map:
      if (isValueType)
      {
        if (pAttrib)
        {
          if (pAttrib->GetValue().IsA<nsVariantDictionary>())
          {
            if (!index.IsValid())
            {
              return pAttrib->GetValue();
            }
            nsString sKey = index.ConvertTo<nsString>();
            const auto& defaultDict = pAttrib->GetValue().Get<nsVariantDictionary>();
            if (auto it = defaultDict.Find(sKey); it.IsValid())
              return it.Value();

            return GetDefaultVariantFromType(pProperty->GetSpecificType());
          }
          if (index.IsValid() && pAttrib->GetValue().CanConvertTo(type))
            return pAttrib->GetValue().ConvertTo(type);
        }

        if (!index.IsValid())
          return nsVariantDictionary();
        return GetDefaultVariantFromType(pProperty->GetSpecificType());
      }
      else
      {
        if (!index.IsValid())
          return nsVariantDictionary();

        return nsUuid();
      }
      break;
    default:
      break;
  }

  NS_REPORT_FAILURE("Don't reach here");
  return nsVariant();
}

nsVariant nsReflectionUtils::GetDefaultVariantFromType(const nsRTTI* pRtti)
{
  nsVariantType::Enum type = pRtti->GetVariantType();
  switch (type)
  {
    case nsVariant::Type::TypedObject:
    {
      nsVariant val;
      val.MoveTypedObject(pRtti->GetAllocator()->Allocate<void>(), pRtti);
      return val;
    }
    break;

    default:
      return GetDefaultVariantFromType(type);
  }
}

void nsReflectionUtils::SetAllMemberPropertiesToDefault(const nsRTTI* pRtti, void* pObject)
{
  nsHybridArray<const nsAbstractProperty*, 32> properties;
  pRtti->GetAllProperties(properties);

  for (auto pProp : properties)
  {
    if (pProp->GetCategory() == nsPropertyCategory::Member)
    {
      const nsVariant defValue = nsReflectionUtils::GetDefaultValue(pProp);

      nsReflectionUtils::SetMemberPropertyValue(static_cast<const nsAbstractMemberProperty*>(pProp), pObject, defValue);
    }
  }
}

namespace
{
  template <class C>
  struct nsClampCategoryType
  {
    enum
    {
      value = (((nsVariant::TypeDeduction<C>::value >= nsVariantType::Int8 && nsVariant::TypeDeduction<C>::value <= nsVariantType::Double) || (nsVariant::TypeDeduction<C>::value == nsVariantType::Time) || (nsVariant::TypeDeduction<C>::value == nsVariantType::Angle))) + ((nsVariant::TypeDeduction<C>::value >= nsVariantType::Vector2 && nsVariant::TypeDeduction<C>::value <= nsVariantType::Vector4U) * 2)
    };
  };

  template <typename T, int V = nsClampCategoryType<T>::value>
  struct ClampVariantFuncImpl
  {
    static NS_ALWAYS_INLINE nsResult Func(nsVariant& value, const nsClampValueAttribute* pAttrib)
    {
      return NS_FAILURE;
    }
  };

  template <typename T>
  struct ClampVariantFuncImpl<T, 1> // scalar types
  {
    static NS_ALWAYS_INLINE nsResult Func(nsVariant& value, const nsClampValueAttribute* pAttrib)
    {
      if (pAttrib->GetMinValue().CanConvertTo<T>())
      {
        value = nsMath::Max(value.ConvertTo<T>(), pAttrib->GetMinValue().ConvertTo<T>());
      }
      if (pAttrib->GetMaxValue().CanConvertTo<T>())
      {
        value = nsMath::Min(value.ConvertTo<T>(), pAttrib->GetMaxValue().ConvertTo<T>());
      }
      return NS_SUCCESS;
    }
  };

  template <typename T>
  struct ClampVariantFuncImpl<T, 2> // vector types
  {
    static NS_ALWAYS_INLINE nsResult Func(nsVariant& value, const nsClampValueAttribute* pAttrib)
    {
      if (pAttrib->GetMinValue().CanConvertTo<T>())
      {
        value = value.ConvertTo<T>().CompMax(pAttrib->GetMinValue().ConvertTo<T>());
      }
      if (pAttrib->GetMaxValue().CanConvertTo<T>())
      {
        value = value.ConvertTo<T>().CompMin(pAttrib->GetMaxValue().ConvertTo<T>());
      }
      return NS_SUCCESS;
    }
  };

  struct ClampVariantFunc
  {
    template <typename T>
    NS_ALWAYS_INLINE nsResult operator()(nsVariant& value, const nsClampValueAttribute* pAttrib)
    {
      return ClampVariantFuncImpl<T>::Func(value, pAttrib);
    }
  };
} // namespace

nsResult nsReflectionUtils::ClampValue(nsVariant& value, const nsClampValueAttribute* pAttrib)
{
  nsVariantType::Enum type = value.GetType();
  if (type == nsVariantType::Invalid || pAttrib == nullptr)
    return NS_SUCCESS; // If there is nothing to clamp or no clamp attribute we call it a success.

  ClampVariantFunc func;
  return nsVariant::DispatchTo(func, type, value, pAttrib);
}
