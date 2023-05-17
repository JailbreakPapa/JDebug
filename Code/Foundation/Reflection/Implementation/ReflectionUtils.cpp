#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Types/ScopeExit.h>
#include <Foundation/Types/VariantTypeRegistry.h>

namespace
{
  // for some reason MSVC does not accept the template keyword here
#if WD_ENABLED(WD_COMPILER_MSVC_PURE)
#  define CALL_FUNCTOR(functor, type) functor.operator()<type>(std::forward<Args>(args)...)
#else
#  define CALL_FUNCTOR(functor, type) functor.template operator()<type>(std::forward<Args>(args)...)
#endif

  template <typename Functor, class... Args>
  void DispatchTo(Functor& ref_functor, const wdAbstractProperty* pProp, Args&&... args)
  {
    const bool bIsPtr = pProp->GetFlags().IsSet(wdPropertyFlags::Pointer);
    if (bIsPtr)
    {
      CALL_FUNCTOR(ref_functor, wdTypedPointer);
      return;
    }
    else if (pProp->GetSpecificType() == wdGetStaticRTTI<const char*>())
    {
      CALL_FUNCTOR(ref_functor, const char*);
      return;
    }
    else if (pProp->GetSpecificType() == wdGetStaticRTTI<wdUntrackedString>())
    {
      CALL_FUNCTOR(ref_functor, wdUntrackedString);
      return;
    }
    else if (pProp->GetSpecificType() == wdGetStaticRTTI<wdVariant>())
    {
      CALL_FUNCTOR(ref_functor, wdVariant);
      return;
    }
    else if (pProp->GetFlags().IsSet(wdPropertyFlags::StandardType))
    {
      wdVariant::DispatchTo(ref_functor, pProp->GetSpecificType()->GetVariantType(), std::forward<Args>(args)...);
      return;
    }
    else if (pProp->GetFlags().IsSet(wdPropertyFlags::IsEnum))
    {
      CALL_FUNCTOR(ref_functor, wdEnumBase);
      return;
    }
    else if (pProp->GetFlags().IsSet(wdPropertyFlags::Bitflags))
    {
      CALL_FUNCTOR(ref_functor, wdBitflagsBase);
      return;
    }
    else if (pProp->GetSpecificType()->GetVariantType() == wdVariantType::TypedObject)
    {
      CALL_FUNCTOR(ref_functor, wdTypedObject);
      return;
    }

    WD_REPORT_FAILURE("Unknown dispatch type");
  }

#undef CALL_FUNCTOR

  wdVariantType::Enum GetDispatchType(const wdAbstractProperty* pProp)
  {
    if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
    {
      return wdVariantType::TypedPointer;
    }
    else if (pProp->GetFlags().IsSet(wdPropertyFlags::StandardType))
    {
      return pProp->GetSpecificType()->GetVariantType();
    }
    else
    {
      return wdVariantType::TypedObject;
    }
  }

  struct GetTypeFromVariantTypeFunc
  {
    template <typename T>
    WD_ALWAYS_INLINE void operator()()
    {
      m_pType = wdGetStaticRTTI<T>();
    }
    const wdRTTI* m_pType;
  };

  template <>
  WD_ALWAYS_INLINE void GetTypeFromVariantTypeFunc::operator()<wdVariantArray>()
  {
    m_pType = nullptr;
  }
  template <>
  WD_ALWAYS_INLINE void GetTypeFromVariantTypeFunc::operator()<wdVariantDictionary>()
  {
    m_pType = nullptr;
  }
  template <>
  WD_ALWAYS_INLINE void GetTypeFromVariantTypeFunc::operator()<wdTypedPointer>()
  {
    m_pType = nullptr;
  }
  template <>
  WD_ALWAYS_INLINE void GetTypeFromVariantTypeFunc::operator()<wdTypedObject>()
  {
    m_pType = nullptr;
  }

  //////////////////////////////////////////////////////////////////////////



  template <typename T>
  struct wdPropertyValue
  {
    using Type = T;
    using StorageType = typename wdVariantTypeDeduction<T>::StorageType;
  };
  template <>
  struct wdPropertyValue<wdEnumBase>
  {
    using Type = wdInt64;
    using StorageType = wdInt64;
  };
  template <>
  struct wdPropertyValue<wdBitflagsBase>
  {
    using Type = wdInt64;
    using StorageType = wdInt64;
  };

  //////////////////////////////////////////////////////////////////////////

  template <class T>
  struct wdVariantFromProperty
  {
    wdVariantFromProperty(wdVariant& value, const wdAbstractProperty* pProp)
      : m_value(value)
    {
    }
    ~wdVariantFromProperty()
    {
      if (m_bSuccess)
        m_value = m_tempValue;
    }

    operator void*()
    {
      return &m_tempValue;
    }

    wdVariant& m_value;
    typename wdPropertyValue<T>::Type m_tempValue = {};
    bool m_bSuccess = true;
  };

  template <>
  struct wdVariantFromProperty<wdVariant>
  {
    wdVariantFromProperty(wdVariant& value, const wdAbstractProperty* pProp)
      : m_value(value)
    {
    }

    operator void*()
    {
      return &m_value;
    }

    wdVariant& m_value;
    bool m_bSuccess = true;
  };

  template <>
  struct wdVariantFromProperty<wdTypedPointer>
  {
    wdVariantFromProperty(wdVariant& value, const wdAbstractProperty* pProp)
      : m_value(value)
      , m_pProp(pProp)
    {
    }
    ~wdVariantFromProperty()
    {
      if (m_bSuccess)
        m_value = wdVariant(m_ptr, m_pProp->GetSpecificType());
    }

    operator void*()
    {
      return &m_ptr;
    }

    wdVariant& m_value;
    const wdAbstractProperty* m_pProp = nullptr;
    void* m_ptr = nullptr;
    bool m_bSuccess = true;
  };

  template <>
  struct wdVariantFromProperty<wdTypedObject>
  {
    wdVariantFromProperty(wdVariant& value, const wdAbstractProperty* pProp)
      : m_value(value)
      , m_pProp(pProp)
    {
      m_ptr = m_pProp->GetSpecificType()->GetAllocator()->Allocate<void>();
    }
    ~wdVariantFromProperty()
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

    wdVariant& m_value;
    const wdAbstractProperty* m_pProp = nullptr;
    void* m_ptr = nullptr;
    bool m_bSuccess = true;
  };

  //////////////////////////////////////////////////////////////////////////

  template <class T>
  struct wdVariantToProperty
  {
    wdVariantToProperty(const wdVariant& value, const wdAbstractProperty* pProp)
    {
      m_tempValue = value.ConvertTo<typename wdPropertyValue<T>::StorageType>();
    }

    operator const void*()
    {
      return &m_tempValue;
    }

    typename wdPropertyValue<T>::Type m_tempValue = {};
  };

  template <>
  struct wdVariantToProperty<const char*>
  {
    wdVariantToProperty(const wdVariant& value, const wdAbstractProperty* pProp)
    {
      m_sData = value.ConvertTo<wdString>();
      m_pValue = m_sData;
    }

    operator const void*()
    {
      return &m_pValue;
    }
    wdString m_sData;
    const char* m_pValue;
  };

  template <>
  struct wdVariantToProperty<wdVariant>
  {
    wdVariantToProperty(const wdVariant& value, const wdAbstractProperty* pProp)
      : m_value(value)
    {
    }

    operator const void*()
    {
      return const_cast<wdVariant*>(&m_value);
    }

    const wdVariant& m_value;
  };

  template <>
  struct wdVariantToProperty<wdTypedPointer>
  {
    wdVariantToProperty(const wdVariant& value, const wdAbstractProperty* pProp)
    {
      m_ptr = value.Get<wdTypedPointer>();
      WD_ASSERT_DEBUG(!m_ptr.m_pType || m_ptr.m_pType->IsDerivedFrom(pProp->GetSpecificType()),
        "Pointer of type '{0}' does not derive from '{}'", m_ptr.m_pType->GetTypeName(), pProp->GetSpecificType()->GetTypeName());
    }

    operator const void*()
    {
      return &m_ptr.m_pObject;
    }

    wdTypedPointer m_ptr;
  };


  template <>
  struct wdVariantToProperty<wdTypedObject>
  {
    wdVariantToProperty(const wdVariant& value, const wdAbstractProperty* pProp)
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
    WD_ALWAYS_INLINE void operator()(const wdAbstractMemberProperty* pProp, const void* pObject, wdVariant& value)
    {
      wdVariantFromProperty<T> getter(value, pProp);
      pProp->GetValuePtr(pObject, getter);
    }
  };

  struct SetValueFunc
  {
    template <typename T>
    WD_FORCE_INLINE void operator()(wdAbstractMemberProperty* pProp, void* pObject, const wdVariant& value)
    {
      wdVariantToProperty<T> setter(value, pProp);
      pProp->SetValuePtr(pObject, setter);
    }
  };

  struct GetArrayValueFunc
  {
    template <typename T>
    WD_FORCE_INLINE void operator()(const wdAbstractArrayProperty* pProp, const void* pObject, wdUInt32 uiIndex, wdVariant& value)
    {
      wdVariantFromProperty<T> getter(value, pProp);
      pProp->GetValue(pObject, uiIndex, getter);
    }
  };

  struct SetArrayValueFunc
  {
    template <typename T>
    WD_FORCE_INLINE void operator()(wdAbstractArrayProperty* pProp, void* pObject, wdUInt32 uiIndex, const wdVariant& value)
    {
      wdVariantToProperty<T> setter(value, pProp);
      pProp->SetValue(pObject, uiIndex, setter);
    }
  };

  struct InsertArrayValueFunc
  {
    template <typename T>
    WD_FORCE_INLINE void operator()(wdAbstractArrayProperty* pProp, void* pObject, wdUInt32 uiIndex, const wdVariant& value)
    {
      wdVariantToProperty<T> setter(value, pProp);
      pProp->Insert(pObject, uiIndex, setter);
    }
  };

  struct InsertSetValueFunc
  {
    template <typename T>
    WD_FORCE_INLINE void operator()(wdAbstractSetProperty* pProp, void* pObject, const wdVariant& value)
    {
      wdVariantToProperty<T> setter(value, pProp);
      pProp->Insert(pObject, setter);
    }
  };

  struct RemoveSetValueFunc
  {
    template <typename T>
    WD_FORCE_INLINE void operator()(wdAbstractSetProperty* pProp, void* pObject, const wdVariant& value)
    {
      wdVariantToProperty<T> setter(value, pProp);
      pProp->Remove(pObject, setter);
    }
  };

  struct GetMapValueFunc
  {
    template <typename T>
    WD_FORCE_INLINE void operator()(const wdAbstractMapProperty* pProp, const void* pObject, const char* szKey, wdVariant& value)
    {
      wdVariantFromProperty<T> getter(value, pProp);
      getter.m_bSuccess = pProp->GetValue(pObject, szKey, getter);
    }
  };

  struct SetMapValueFunc
  {
    template <typename T>
    WD_FORCE_INLINE void operator()(wdAbstractMapProperty* pProp, void* pObject, const char* szKey, const wdVariant& value)
    {
      wdVariantToProperty<T> setter(value, pProp);
      pProp->Insert(pObject, szKey, setter);
    }
  };

  static bool CompareProperties(const void* pObject, const void* pObject2, const wdRTTI* pType)
  {
    if (pType->GetParentType())
    {
      if (!CompareProperties(pObject, pObject2, pType->GetParentType()))
        return false;
    }

    for (auto* pProp : pType->GetProperties())
    {
      if (!wdReflectionUtils::IsEqual(pObject, pObject2, pProp))
        return false;
    }

    return true;
  }

  template <typename T>
  struct SetComponentValueImpl
  {
    WD_FORCE_INLINE static void impl(wdVariant* pVector, wdUInt32 uiComponent, double fValue) { WD_ASSERT_DEBUG(false, "wdReflectionUtils::SetComponent was called with a non-vector variant '{0}'", pVector->GetType()); }
  };

  template <typename T>
  struct SetComponentValueImpl<wdVec2Template<T>>
  {
    WD_FORCE_INLINE static void impl(wdVariant* pVector, wdUInt32 uiComponent, double fValue)
    {
      auto vec = pVector->Get<wdVec2Template<T>>();
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
  struct SetComponentValueImpl<wdVec3Template<T>>
  {
    WD_FORCE_INLINE static void impl(wdVariant* pVector, wdUInt32 uiComponent, double fValue)
    {
      auto vec = pVector->Get<wdVec3Template<T>>();
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
  struct SetComponentValueImpl<wdVec4Template<T>>
  {
    WD_FORCE_INLINE static void impl(wdVariant* pVector, wdUInt32 uiComponent, double fValue)
    {
      auto vec = pVector->Get<wdVec4Template<T>>();
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
    WD_FORCE_INLINE void operator()()
    {
      SetComponentValueImpl<T>::impl(m_pVector, m_iComponent, m_fValue);
    }
    wdVariant* m_pVector;
    wdUInt32 m_iComponent;
    double m_fValue;
  };

  template <typename T>
  struct GetComponentValueImpl
  {
    WD_FORCE_INLINE static void impl(const wdVariant* pVector, wdUInt32 uiComponent, double& ref_fValue) { WD_ASSERT_DEBUG(false, "wdReflectionUtils::SetComponent was called with a non-vector variant '{0}'", pVector->GetType()); }
  };

  template <typename T>
  struct GetComponentValueImpl<wdVec2Template<T>>
  {
    WD_FORCE_INLINE static void impl(const wdVariant* pVector, wdUInt32 uiComponent, double& ref_fValue)
    {
      const auto& vec = pVector->Get<wdVec2Template<T>>();
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
  struct GetComponentValueImpl<wdVec3Template<T>>
  {
    WD_FORCE_INLINE static void impl(const wdVariant* pVector, wdUInt32 uiComponent, double& ref_fValue)
    {
      const auto& vec = pVector->Get<wdVec3Template<T>>();
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
  struct GetComponentValueImpl<wdVec4Template<T>>
  {
    WD_FORCE_INLINE static void impl(const wdVariant* pVector, wdUInt32 uiComponent, double& ref_fValue)
    {
      const auto& vec = pVector->Get<wdVec4Template<T>>();
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
    WD_FORCE_INLINE void operator()()
    {
      GetComponentValueImpl<T>::impl(m_pVector, m_iComponent, m_fValue);
    }
    const wdVariant* m_pVector;
    wdUInt32 m_iComponent;
    double m_fValue;
  };
} // namespace

const wdRTTI* wdReflectionUtils::GetCommonBaseType(const wdRTTI* pRtti1, const wdRTTI* pRtti2)
{
  if (pRtti2 == nullptr)
    return nullptr;

  while (pRtti1 != nullptr)
  {
    const wdRTTI* pRtti2Parent = pRtti2;

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

bool wdReflectionUtils::IsBasicType(const wdRTTI* pRtti)
{
  WD_ASSERT_DEBUG(pRtti != nullptr, "IsBasicType: missing data!");
  wdVariant::Type::Enum type = pRtti->GetVariantType();
  return type >= wdVariant::Type::FirstStandardType && type <= wdVariant::Type::LastStandardType;
}

bool wdReflectionUtils::IsValueType(const wdAbstractProperty* pProp)
{
  return !pProp->GetFlags().IsSet(wdPropertyFlags::Pointer) && (pProp->GetFlags().IsSet(wdPropertyFlags::StandardType) || wdVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(pProp->GetSpecificType()));
}

const wdRTTI* wdReflectionUtils::GetTypeFromVariant(const wdVariant& value)
{
  return value.GetReflectedType();
}

const wdRTTI* wdReflectionUtils::GetTypeFromVariant(wdVariantType::Enum type)
{
  GetTypeFromVariantTypeFunc func;
  func.m_pType = nullptr;
  wdVariant::DispatchTo(func, type);

  return func.m_pType;
}

wdUInt32 wdReflectionUtils::GetComponentCount(wdVariantType::Enum type)
{
  switch (type)
  {
    case wdVariant::Type::Vector2:
    case wdVariant::Type::Vector2I:
    case wdVariant::Type::Vector2U:
      return 2;
    case wdVariant::Type::Vector3:
    case wdVariant::Type::Vector3I:
    case wdVariant::Type::Vector3U:
      return 3;
    case wdVariant::Type::Vector4:
    case wdVariant::Type::Vector4I:
    case wdVariant::Type::Vector4U:
      return 4;
    default:
      WD_REPORT_FAILURE("Not a vector type: '{0}'", type);
      return 0;
  }
}

void wdReflectionUtils::SetComponent(wdVariant& ref_vector, wdUInt32 uiComponent, double fValue)
{
  SetComponentValueFunc func;
  func.m_pVector = &ref_vector;
  func.m_iComponent = uiComponent;
  func.m_fValue = fValue;
  wdVariant::DispatchTo(func, ref_vector.GetType());
}

double wdReflectionUtils::GetComponent(const wdVariant& vector, wdUInt32 uiComponent)
{
  GetComponentValueFunc func;
  func.m_pVector = &vector;
  func.m_iComponent = uiComponent;
  wdVariant::DispatchTo(func, vector.GetType());
  return func.m_fValue;
}

wdVariant wdReflectionUtils::GetMemberPropertyValue(const wdAbstractMemberProperty* pProp, const void* pObject)
{
  wdVariant res;
  WD_ASSERT_DEBUG(pProp != nullptr, "GetMemberPropertyValue: missing data!");

  GetValueFunc func;
  DispatchTo(func, pProp, pProp, pObject, res);

  return res;
}

void wdReflectionUtils::SetMemberPropertyValue(wdAbstractMemberProperty* pProp, void* pObject, const wdVariant& value)
{
  WD_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "SetMemberPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(wdPropertyFlags::ReadOnly))
    return;

  if (pProp->GetFlags().IsAnySet(wdPropertyFlags::Bitflags | wdPropertyFlags::IsEnum))
  {
    wdAbstractEnumerationProperty* pEnumerationProp = static_cast<wdAbstractEnumerationProperty*>(pProp);

    // Value can either be an integer or a string (human readable value)
    if (value.IsA<wdString>())
    {
      wdInt64 iValue;
      wdReflectionUtils::StringToEnumeration(pProp->GetSpecificType(), value.Get<wdString>(), iValue);
      pEnumerationProp->SetValue(pObject, iValue);
    }
    else
    {
      pEnumerationProp->SetValue(pObject, value.ConvertTo<wdInt64>());
    }
  }
  else
  {
    SetValueFunc func;
    DispatchTo(func, pProp, pProp, pObject, value);
  }
}

wdVariant wdReflectionUtils::GetArrayPropertyValue(const wdAbstractArrayProperty* pProp, const void* pObject, wdUInt32 uiIndex)
{
  wdVariant res;
  WD_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "GetArrayPropertyValue: missing data!");
  auto uiCount = pProp->GetCount(pObject);
  if (uiIndex >= uiCount)
  {
    wdLog::Error("GetArrayPropertyValue: Invalid index: {0}", uiIndex);
  }
  else
  {
    GetArrayValueFunc func;
    DispatchTo(func, pProp, pProp, pObject, uiIndex, res);
  }
  return res;
}

void wdReflectionUtils::SetArrayPropertyValue(wdAbstractArrayProperty* pProp, void* pObject, wdUInt32 uiIndex, const wdVariant& value)
{
  WD_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "GetArrayPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(wdPropertyFlags::ReadOnly))
    return;

  auto uiCount = pProp->GetCount(pObject);
  if (uiIndex >= uiCount)
  {
    wdLog::Error("SetArrayPropertyValue: Invalid index: {0}", uiIndex);
  }
  else
  {
    SetArrayValueFunc func;
    DispatchTo(func, pProp, pProp, pObject, uiIndex, value);
  }
}

void wdReflectionUtils::InsertSetPropertyValue(wdAbstractSetProperty* pProp, void* pObject, const wdVariant& value)
{
  WD_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "InsertSetPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(wdPropertyFlags::ReadOnly))
    return;

  InsertSetValueFunc func;
  DispatchTo(func, pProp, pProp, pObject, value);
}

void wdReflectionUtils::RemoveSetPropertyValue(wdAbstractSetProperty* pProp, void* pObject, const wdVariant& value)
{
  WD_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "RemoveSetPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(wdPropertyFlags::ReadOnly))
    return;

  RemoveSetValueFunc func;
  DispatchTo(func, pProp, pProp, pObject, value);
}

wdVariant wdReflectionUtils::GetMapPropertyValue(const wdAbstractMapProperty* pProp, const void* pObject, const char* szKey)
{
  wdVariant value;
  WD_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "GetMapPropertyValue: missing data!");

  GetMapValueFunc func;
  DispatchTo(func, pProp, pProp, pObject, szKey, value);
  return value;
}

void wdReflectionUtils::SetMapPropertyValue(wdAbstractMapProperty* pProp, void* pObject, const char* szKey, const wdVariant& value)
{
  WD_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "SetMapPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(wdPropertyFlags::ReadOnly))
    return;

  SetMapValueFunc func;
  DispatchTo(func, pProp, pProp, pObject, szKey, value);
}

void wdReflectionUtils::InsertArrayPropertyValue(wdAbstractArrayProperty* pProp, void* pObject, const wdVariant& value, wdUInt32 uiIndex)
{
  WD_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "InsertArrayPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(wdPropertyFlags::ReadOnly))
    return;

  auto uiCount = pProp->GetCount(pObject);
  if (uiIndex > uiCount)
  {
    wdLog::Error("InsertArrayPropertyValue: Invalid index: {0}", uiIndex);
    return;
  }

  InsertArrayValueFunc func;
  DispatchTo(func, pProp, pProp, pObject, uiIndex, value);
}

void wdReflectionUtils::RemoveArrayPropertyValue(wdAbstractArrayProperty* pProp, void* pObject, wdUInt32 uiIndex)
{
  WD_ASSERT_DEBUG(pProp != nullptr && pObject != nullptr, "RemoveArrayPropertyValue: missing data!");
  if (pProp->GetFlags().IsSet(wdPropertyFlags::ReadOnly))
    return;

  auto uiCount = pProp->GetCount(pObject);
  if (uiIndex >= uiCount)
  {
    wdLog::Error("RemoveArrayPropertyValue: Invalid index: {0}", uiIndex);
    return;
  }

  pProp->Remove(pObject, uiIndex);
}

wdAbstractMemberProperty* wdReflectionUtils::GetMemberProperty(const wdRTTI* pRtti, wdUInt32 uiPropertyIndex)
{
  if (pRtti == nullptr)
    return nullptr;

  wdHybridArray<wdAbstractProperty*, 32> props;
  pRtti->GetAllProperties(props);
  if (uiPropertyIndex < props.GetCount())
  {
    wdAbstractProperty* pProp = props[uiPropertyIndex];
    if (pProp->GetCategory() == wdPropertyCategory::Member)
      return static_cast<wdAbstractMemberProperty*>(pProp);
  }

  return nullptr;
}

wdAbstractMemberProperty* wdReflectionUtils::GetMemberProperty(const wdRTTI* pRtti, const char* szPropertyName)
{
  if (pRtti == nullptr)
    return nullptr;

  if (wdAbstractProperty* pProp = pRtti->FindPropertyByName(szPropertyName))
  {
    if (pProp->GetCategory() == wdPropertyCategory::Member)
      return static_cast<wdAbstractMemberProperty*>(pProp);
  }

  return nullptr;
}

void wdReflectionUtils::GatherTypesDerivedFromClass(const wdRTTI* pRtti, wdSet<const wdRTTI*>& out_types, bool bIncludeDependencies)
{
  wdRTTI* pFirst = wdRTTI::GetFirstInstance();
  while (pFirst != nullptr)
  {
    if (pFirst->IsDerivedFrom(pRtti))
    {
      out_types.Insert(pFirst);
      if (bIncludeDependencies)
      {
        GatherDependentTypes(pFirst, out_types);
      }
    }
    pFirst = pFirst->GetNextInstance();
  }
}

void wdReflectionUtils::GatherDependentTypes(const wdRTTI* pRtti, wdSet<const wdRTTI*>& inout_types)
{
  const wdRTTI* pParentRtti = pRtti->GetParentType();
  if (pParentRtti != nullptr)
  {
    inout_types.Insert(pParentRtti);
    GatherDependentTypes(pParentRtti, inout_types);
  }

  const wdArrayPtr<wdAbstractProperty*>& rttiProps = pRtti->GetProperties();
  const wdUInt32 uiCount = rttiProps.GetCount();

  for (wdUInt32 i = 0; i < uiCount; ++i)
  {
    wdAbstractProperty* prop = rttiProps[i];
    if (prop->GetFlags().IsSet(wdPropertyFlags::StandardType))
      continue;
    if (prop->GetAttributeByType<wdTemporaryAttribute>() != nullptr)
      continue;
    switch (prop->GetCategory())
    {
      case wdPropertyCategory::Member:
      case wdPropertyCategory::Array:
      case wdPropertyCategory::Set:
      case wdPropertyCategory::Map:
      {
        const wdRTTI* pPropRtti = prop->GetSpecificType();

        if (inout_types.Contains(pPropRtti))
          continue;

        inout_types.Insert(pPropRtti);
        GatherDependentTypes(pPropRtti, inout_types);
      }
      break;
      case wdPropertyCategory::Function:
      case wdPropertyCategory::Constant:
      default:
        break;
    }
  }
}

bool wdReflectionUtils::CreateDependencySortedTypeArray(const wdSet<const wdRTTI*>& types, wdDynamicArray<const wdRTTI*>& out_sortedTypes)
{
  out_sortedTypes.Clear();
  out_sortedTypes.Reserve(types.GetCount());

  wdMap<const wdRTTI*, wdSet<const wdRTTI*>> dependencies;

  wdSet<const wdRTTI*> accu;

  for (const wdRTTI* pType : types)
  {
    auto it = dependencies.Insert(pType, wdSet<const wdRTTI*>());
    GatherDependentTypes(pType, it.Value());
  }


  while (!dependencies.IsEmpty())
  {
    bool bDeadEnd = true;
    for (auto it = dependencies.GetIterator(); it.IsValid(); ++it)
    {
      // Are the types dependencies met?
      if (accu.ContainsSet(it.Value()))
      {
        out_sortedTypes.PushBack(it.Key());
        accu.Insert(it.Key());
        dependencies.Remove(it);
        bDeadEnd = false;
        break;
      }
    }

    if (bDeadEnd)
    {
      return false;
    }
  }

  return true;
}

bool wdReflectionUtils::EnumerationToString(const wdRTTI* pEnumerationRtti, wdInt64 iValue, wdStringBuilder& out_sOutput, wdEnum<EnumConversionMode> conversionMode)
{
  out_sOutput.Clear();
  if (pEnumerationRtti->IsDerivedFrom<wdEnumBase>())
  {
    for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
    {
      if (pProp->GetCategory() == wdPropertyCategory::Constant)
      {
        wdVariant value = static_cast<const wdAbstractConstantProperty*>(pProp)->GetConstant();
        if (value.ConvertTo<wdInt64>() == iValue)
        {
          out_sOutput = conversionMode == EnumConversionMode::FullyQualifiedName ? pProp->GetPropertyName() : wdStringUtils::FindLastSubString(pProp->GetPropertyName(), "::") + 2;
          return true;
        }
      }
    }
    return false;
  }
  else if (pEnumerationRtti->IsDerivedFrom<wdBitflagsBase>())
  {
    for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
    {
      if (pProp->GetCategory() == wdPropertyCategory::Constant)
      {
        wdVariant value = static_cast<const wdAbstractConstantProperty*>(pProp)->GetConstant();
        if ((value.ConvertTo<wdInt64>() & iValue) != 0)
        {
          out_sOutput.Append(conversionMode == EnumConversionMode::FullyQualifiedName ? pProp->GetPropertyName() : wdStringUtils::FindLastSubString(pProp->GetPropertyName(), "::") + 2, "|");
        }
      }
    }
    out_sOutput.Shrink(0, 1);
    return true;
  }
  else
  {
    WD_ASSERT_DEV(false, "The RTTI class '{0}' is not an enum or bitflags class", pEnumerationRtti->GetTypeName());
    return false;
  }
}

void wdReflectionUtils::GetEnumKeysAndValues(const wdRTTI* pEnumerationRtti, wdDynamicArray<EnumKeyValuePair>& ref_entries, wdEnum<EnumConversionMode> conversionMode)
{
  /// \test This is new.

  ref_entries.Clear();

  if (pEnumerationRtti->IsDerivedFrom<wdEnumBase>())
  {
    for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
    {
      if (pProp->GetCategory() == wdPropertyCategory::Constant)
      {
        wdVariant value = static_cast<const wdAbstractConstantProperty*>(pProp)->GetConstant();

        auto& e = ref_entries.ExpandAndGetRef();
        e.m_sKey = conversionMode == EnumConversionMode::FullyQualifiedName ? pProp->GetPropertyName() : wdStringUtils::FindLastSubString(pProp->GetPropertyName(), "::") + 2;
        e.m_iValue = value.ConvertTo<wdInt32>();
      }
    }
  }
}

bool wdReflectionUtils::StringToEnumeration(const wdRTTI* pEnumerationRtti, const char* szValue, wdInt64& out_iValue)
{
  out_iValue = 0;
  if (pEnumerationRtti->IsDerivedFrom<wdEnumBase>())
  {
    for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
    {
      if (pProp->GetCategory() == wdPropertyCategory::Constant)
      {
        // Testing fully qualified and short value name
        const char* valueNameOnly = wdStringUtils::FindLastSubString(pProp->GetPropertyName(), "::", nullptr);
        if (wdStringUtils::IsEqual(pProp->GetPropertyName(), szValue) || (valueNameOnly != nullptr && wdStringUtils::IsEqual(valueNameOnly + 2, szValue)))
        {
          wdVariant value = static_cast<const wdAbstractConstantProperty*>(pProp)->GetConstant();
          out_iValue = value.ConvertTo<wdInt64>();
          return true;
        }
      }
    }
    return false;
  }
  else if (pEnumerationRtti->IsDerivedFrom<wdBitflagsBase>())
  {
    wdStringBuilder temp = szValue;
    wdHybridArray<wdStringView, 32> values;
    temp.Split(false, values, "|");
    for (auto sValue : values)
    {
      for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
      {
        if (pProp->GetCategory() == wdPropertyCategory::Constant)
        {
          // Testing fully qualified and short value name
          const char* valueNameOnly = wdStringUtils::FindLastSubString(pProp->GetPropertyName(), "::", nullptr);
          if (sValue.IsEqual(pProp->GetPropertyName()) || (valueNameOnly != nullptr && sValue.IsEqual(valueNameOnly + 2)))
          {
            wdVariant value = static_cast<const wdAbstractConstantProperty*>(pProp)->GetConstant();
            out_iValue |= value.ConvertTo<wdInt64>();
          }
        }
      }
    }
    return true;
  }
  else
  {
    WD_ASSERT_DEV(false, "The RTTI class '{0}' is not an enum or bitflags class", pEnumerationRtti->GetTypeName());
    return false;
  }
}

wdInt64 wdReflectionUtils::DefaultEnumerationValue(const wdRTTI* pEnumerationRtti)
{
  if (pEnumerationRtti->IsDerivedFrom<wdEnumBase>() || pEnumerationRtti->IsDerivedFrom<wdBitflagsBase>())
  {
    auto pProp = pEnumerationRtti->GetProperties()[0];
    WD_ASSERT_DEBUG(pProp->GetCategory() == wdPropertyCategory::Constant && wdStringUtils::EndsWith(pProp->GetPropertyName(), "::Default"), "First enumeration property must be the default value constant.");
    return static_cast<const wdAbstractConstantProperty*>(pProp)->GetConstant().ConvertTo<wdInt64>();
  }
  else
  {
    WD_ASSERT_DEV(false, "The RTTI class '{0}' is not an enum or bitflags class", pEnumerationRtti->GetTypeName());
    return 0;
  }
}

wdInt64 wdReflectionUtils::MakeEnumerationValid(const wdRTTI* pEnumerationRtti, wdInt64 iValue)
{
  if (pEnumerationRtti->IsDerivedFrom<wdEnumBase>())
  {
    // Find current value
    for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
    {
      if (pProp->GetCategory() == wdPropertyCategory::Constant)
      {
        wdInt64 iCurrentValue = static_cast<const wdAbstractConstantProperty*>(pProp)->GetConstant().ConvertTo<wdInt64>();
        if (iCurrentValue == iValue)
          return iValue;
      }
    }

    // Current value not found, return default value
    return wdReflectionUtils::DefaultEnumerationValue(pEnumerationRtti);
  }
  else if (pEnumerationRtti->IsDerivedFrom<wdBitflagsBase>())
  {
    wdInt64 iNewValue = 0;
    // Filter valid bits
    for (auto pProp : pEnumerationRtti->GetProperties().GetSubArray(1))
    {
      if (pProp->GetCategory() == wdPropertyCategory::Constant)
      {
        wdInt64 iCurrentValue = static_cast<const wdAbstractConstantProperty*>(pProp)->GetConstant().ConvertTo<wdInt64>();
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
    WD_ASSERT_DEV(false, "The RTTI class '{0}' is not an enum or bitflags class", pEnumerationRtti->GetTypeName());
    return 0;
  }
}

bool wdReflectionUtils::IsEqual(const void* pObject, const void* pObject2, wdAbstractProperty* pProp)
{
  //#VAR TEST
  const wdRTTI* pPropType = pProp->GetSpecificType();

  wdVariant vTemp;
  wdVariant vTemp2;

  const bool bIsValueType = wdReflectionUtils::IsValueType(pProp);

  switch (pProp->GetCategory())
  {
    case wdPropertyCategory::Member:
    {
      wdAbstractMemberProperty* pSpecific = static_cast<wdAbstractMemberProperty*>(pProp);

      if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
      {
        vTemp = wdReflectionUtils::GetMemberPropertyValue(pSpecific, pObject);
        vTemp2 = wdReflectionUtils::GetMemberPropertyValue(pSpecific, pObject2);
        void* pRefrencedObject = vTemp.ConvertTo<void*>();
        void* pRefrencedObject2 = vTemp2.ConvertTo<void*>();
        if ((pRefrencedObject == nullptr) != (pRefrencedObject2 == nullptr))
          return false;
        if ((pRefrencedObject == nullptr) && (pRefrencedObject2 == nullptr))
          return true;

        if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
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
        if (pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags) || bIsValueType)
        {
          vTemp = wdReflectionUtils::GetMemberPropertyValue(pSpecific, pObject);
          vTemp2 = wdReflectionUtils::GetMemberPropertyValue(pSpecific, pObject2);
          return vTemp == vTemp2;
        }
        else if (pProp->GetFlags().IsSet(wdPropertyFlags::Class))
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
    case wdPropertyCategory::Array:
    {
      wdAbstractArrayProperty* pSpecific = static_cast<wdAbstractArrayProperty*>(pProp);

      const wdUInt32 uiCount = pSpecific->GetCount(pObject);
      const wdUInt32 uiCount2 = pSpecific->GetCount(pObject2);
      if (uiCount != uiCount2)
        return false;

      if (pSpecific->GetFlags().IsSet(wdPropertyFlags::Pointer))
      {
        for (wdUInt32 i = 0; i < uiCount; ++i)
        {
          vTemp = wdReflectionUtils::GetArrayPropertyValue(pSpecific, pObject, i);
          vTemp2 = wdReflectionUtils::GetArrayPropertyValue(pSpecific, pObject2, i);
          void* pRefrencedObject = vTemp.ConvertTo<void*>();
          void* pRefrencedObject2 = vTemp2.ConvertTo<void*>();
          if ((pRefrencedObject == nullptr) != (pRefrencedObject2 == nullptr))
            return false;
          if ((pRefrencedObject == nullptr) && (pRefrencedObject2 == nullptr))
            continue;

          if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
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
          for (wdUInt32 i = 0; i < uiCount; ++i)
          {
            vTemp = wdReflectionUtils::GetArrayPropertyValue(pSpecific, pObject, i);
            vTemp2 = wdReflectionUtils::GetArrayPropertyValue(pSpecific, pObject2, i);
            if (vTemp != vTemp2)
              return false;
          }
          return true;
        }
        else if (pProp->GetFlags().IsSet(wdPropertyFlags::Class) && pPropType->GetAllocator()->CanAllocate())
        {
          void* pSubObject = pPropType->GetAllocator()->Allocate<void>();
          void* pSubObject2 = pPropType->GetAllocator()->Allocate<void>();

          bool bEqual = true;
          for (wdUInt32 i = 0; i < uiCount; ++i)
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
    case wdPropertyCategory::Set:
    {
      wdAbstractSetProperty* pSpecific = static_cast<wdAbstractSetProperty*>(pProp);

      wdHybridArray<wdVariant, 16> values;
      pSpecific->GetValues(pObject, values);
      wdHybridArray<wdVariant, 16> values2;
      pSpecific->GetValues(pObject2, values2);

      const wdUInt32 uiCount = values.GetCount();
      const wdUInt32 uiCount2 = values2.GetCount();
      if (uiCount != uiCount2)
        return false;

      if (bIsValueType || (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner)))
      {
        bool bEqual = true;
        for (wdUInt32 i = 0; i < uiCount; ++i)
        {
          bEqual = values2.Contains(values[i]);
          if (!bEqual)
            break;
        }
        return bEqual;
      }
      else if (pProp->GetFlags().AreAllSet(wdPropertyFlags::Pointer | wdPropertyFlags::PointerOwner))
      {
        // TODO: pointer sets are never stable unless they use an array based pseudo set as storage.
        bool bEqual = true;
        for (wdUInt32 i = 0; i < uiCount; ++i)
        {
          if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
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
    case wdPropertyCategory::Map:
    {
      wdAbstractMapProperty* pSpecific = static_cast<wdAbstractMapProperty*>(pProp);

      wdHybridArray<wdString, 16> keys;
      pSpecific->GetKeys(pObject, keys);
      wdHybridArray<wdString, 16> keys2;
      pSpecific->GetKeys(pObject2, keys2);

      const wdUInt32 uiCount = keys.GetCount();
      const wdUInt32 uiCount2 = keys2.GetCount();
      if (uiCount != uiCount2)
        return false;

      if (bIsValueType || (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner)))
      {
        bool bEqual = true;
        for (wdUInt32 i = 0; i < uiCount; ++i)
        {
          bEqual = keys2.Contains(keys[i]);
          if (!bEqual)
            break;
          wdVariant value1 = GetMapPropertyValue(pSpecific, pObject, keys[i]);
          wdVariant value2 = GetMapPropertyValue(pSpecific, pObject2, keys[i]);
          bEqual = value1 == value2;
          if (!bEqual)
            break;
        }
        return bEqual;
      }
      else if ((!pProp->GetFlags().IsSet(wdPropertyFlags::Pointer) || pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner)) && pProp->GetFlags().IsSet(wdPropertyFlags::Class))
      {
        bool bEqual = true;
        for (wdUInt32 i = 0; i < uiCount; ++i)
        {
          bEqual = keys2.Contains(keys[i]);
          if (!bEqual)
            break;

          if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
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
              WD_SCOPE_EXIT(pPropType->GetAllocator()->Deallocate(value1););
              void* value2 = pPropType->GetAllocator()->Allocate<void>();
              WD_SCOPE_EXIT(pPropType->GetAllocator()->Deallocate(value2););

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
              wdLog::Error("The property '{0}' can not be compared as the type '{1}' cannot be allocated.", pProp->GetPropertyName(), pPropType->GetTypeName());
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
      WD_ASSERT_NOT_IMPLEMENTED;
      break;
  }
  return true;
}

bool wdReflectionUtils::IsEqual(const void* pObject, const void* pObject2, const wdRTTI* pType)
{
  WD_ASSERT_DEV(pObject && pObject2 && pType, "invalid type.");
  if (pType->IsDerivedFrom<wdReflectedClass>())
  {
    const wdReflectedClass* pRefObject = static_cast<const wdReflectedClass*>(pObject);
    const wdReflectedClass* pRefObject2 = static_cast<const wdReflectedClass*>(pObject2);
    pType = pRefObject->GetDynamicRTTI();
    if (pType != pRefObject2->GetDynamicRTTI())
      return false;
  }

  return CompareProperties(pObject, pObject2, pType);
}


void wdReflectionUtils::DeleteObject(void* pObject, wdAbstractProperty* pOwnerProperty)
{
  if (!pObject)
    return;

  const wdRTTI* pType = pOwnerProperty->GetSpecificType();
  if (pType->IsDerivedFrom<wdReflectedClass>())
  {
    wdReflectedClass* pRefObject = static_cast<wdReflectedClass*>(pObject);
    pType = pRefObject->GetDynamicRTTI();
  }

  if (!pType->GetAllocator()->CanAllocate())
  {
    wdLog::Error("Tried to deallocate object of type '{0}', but it has no allocator.", pType->GetTypeName());
    return;
  }
  pType->GetAllocator()->Deallocate(pObject);
}

wdVariant wdReflectionUtils::GetDefaultVariantFromType(wdVariant::Type::Enum type)
{
  switch (type)
  {
    case wdVariant::Type::Invalid:
      return wdVariant();
    case wdVariant::Type::Bool:
      return wdVariant(false);
    case wdVariant::Type::Int8:
      return wdVariant((wdInt8)0);
    case wdVariant::Type::UInt8:
      return wdVariant((wdUInt8)0);
    case wdVariant::Type::Int16:
      return wdVariant((wdInt16)0);
    case wdVariant::Type::UInt16:
      return wdVariant((wdUInt16)0);
    case wdVariant::Type::Int32:
      return wdVariant((wdInt32)0);
    case wdVariant::Type::UInt32:
      return wdVariant((wdUInt32)0);
    case wdVariant::Type::Int64:
      return wdVariant((wdInt64)0);
    case wdVariant::Type::UInt64:
      return wdVariant((wdUInt64)0);
    case wdVariant::Type::Float:
      return wdVariant(0.0f);
    case wdVariant::Type::Double:
      return wdVariant(0.0);
    case wdVariant::Type::Color:
      return wdVariant(wdColor(1.0f, 1.0f, 1.0f));
    case wdVariant::Type::ColorGamma:
      return wdVariant(wdColorGammaUB(255, 255, 255));
    case wdVariant::Type::Vector2:
      return wdVariant(wdVec2(0.0f, 0.0f));
    case wdVariant::Type::Vector3:
      return wdVariant(wdVec3(0.0f, 0.0f, 0.0f));
    case wdVariant::Type::Vector4:
      return wdVariant(wdVec4(0.0f, 0.0f, 0.0f, 0.0f));
    case wdVariant::Type::Vector2I:
      return wdVariant(wdVec2I32(0, 0));
    case wdVariant::Type::Vector3I:
      return wdVariant(wdVec3I32(0, 0, 0));
    case wdVariant::Type::Vector4I:
      return wdVariant(wdVec4I32(0, 0, 0, 0));
    case wdVariant::Type::Vector2U:
      return wdVariant(wdVec2U32(0, 0));
    case wdVariant::Type::Vector3U:
      return wdVariant(wdVec3U32(0, 0, 0));
    case wdVariant::Type::Vector4U:
      return wdVariant(wdVec4U32(0, 0, 0, 0));
    case wdVariant::Type::Quaternion:
      return wdVariant(wdQuat(0.0f, 0.0f, 0.0f, 1.0f));
    case wdVariant::Type::Matrix3:
      return wdVariant(wdMat3::IdentityMatrix());
    case wdVariant::Type::Matrix4:
      return wdVariant(wdMat4::IdentityMatrix());
    case wdVariant::Type::Transform:
      return wdVariant(wdTransform::IdentityTransform());
    case wdVariant::Type::String:
      return wdVariant(wdString());
    case wdVariant::Type::StringView:
      return wdVariant(wdStringView());
    case wdVariant::Type::DataBuffer:
      return wdVariant(wdDataBuffer());
    case wdVariant::Type::Time:
      return wdVariant(wdTime());
    case wdVariant::Type::Uuid:
      return wdVariant(wdUuid());
    case wdVariant::Type::Angle:
      return wdVariant(wdAngle());
    case wdVariant::Type::VariantArray:
      return wdVariantArray();
    case wdVariant::Type::VariantDictionary:
      return wdVariantDictionary();
    case wdVariant::Type::TypedPointer:
      return wdVariant(static_cast<void*>(nullptr), nullptr);

    default:
      WD_REPORT_FAILURE("Invalid case statement");
      return wdVariant();
  }
  return wdVariant();
}

wdVariant wdReflectionUtils::GetDefaultValue(const wdAbstractProperty* pProperty, wdVariant index)
{
  const bool isValueType = wdReflectionUtils::IsValueType(pProperty);
  const wdVariantType::Enum type = pProperty->GetFlags().IsSet(wdPropertyFlags::Pointer) || (pProperty->GetFlags().IsSet(wdPropertyFlags::Class) && !isValueType) ? wdVariantType::Uuid : pProperty->GetSpecificType()->GetVariantType();
  const wdDefaultValueAttribute* pAttrib = pProperty->GetAttributeByType<wdDefaultValueAttribute>();

  switch (pProperty->GetCategory())
  {
    case wdPropertyCategory::Member:
    {
      if (isValueType)
      {
        if (pAttrib)
        {
          if (pProperty->GetSpecificType() == wdGetStaticRTTI<wdVariant>())
            return pAttrib->GetValue();
          if (pAttrib->GetValue().CanConvertTo(type))
            return pAttrib->GetValue().ConvertTo(type);
        }
        return GetDefaultVariantFromType(pProperty->GetSpecificType());
      }
      else if (pProperty->GetSpecificType()->GetTypeFlags().IsAnySet(wdTypeFlags::IsEnum | wdTypeFlags::Bitflags))
      {
        wdInt64 iValue = wdReflectionUtils::DefaultEnumerationValue(pProperty->GetSpecificType());
        if (pAttrib)
        {
          if (pAttrib->GetValue().CanConvertTo(wdVariantType::Int64))
            iValue = pAttrib->GetValue().ConvertTo<wdInt64>();
        }
        return wdReflectionUtils::MakeEnumerationValid(pProperty->GetSpecificType(), iValue);
      }
      else // Class
      {
        return wdUuid();
      }
    }
    break;
    case wdPropertyCategory::Array:
    case wdPropertyCategory::Set:
      if (isValueType)
      {
        if (pAttrib)
        {
          if (pAttrib->GetValue().IsA<wdVariantArray>())
          {
            if (!index.IsValid())
              return pAttrib->GetValue();

            wdUInt32 iIndex = index.ConvertTo<wdUInt32>();
            const auto& defaultArray = pAttrib->GetValue().Get<wdVariantArray>();
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
          return wdVariantArray();

        return GetDefaultVariantFromType(pProperty->GetSpecificType());
      }
      else
      {
        if (!index.IsValid())
          return wdVariantArray();

        return wdUuid();
      }
      break;
    case wdPropertyCategory::Map:
      if (isValueType)
      {
        if (pAttrib)
        {
          if (pAttrib->GetValue().IsA<wdVariantDictionary>())
          {
            if (!index.IsValid())
            {
              return pAttrib->GetValue();
            }
            wdString sKey = index.ConvertTo<wdString>();
            const auto& defaultDict = pAttrib->GetValue().Get<wdVariantDictionary>();
            if (auto it = defaultDict.Find(sKey); it.IsValid())
              return it.Value();

            return GetDefaultVariantFromType(pProperty->GetSpecificType());
          }
          if (index.IsValid() && pAttrib->GetValue().CanConvertTo(type))
            return pAttrib->GetValue().ConvertTo(type);
        }

        if (!index.IsValid())
          return wdVariantDictionary();
        return GetDefaultVariantFromType(pProperty->GetSpecificType());
      }
      else
      {
        if (!index.IsValid())
          return wdVariantDictionary();

        return wdUuid();
      }
      break;
    default:
      break;
  }

  WD_REPORT_FAILURE("Don't reach here");
  return wdVariant();
}

wdVariant wdReflectionUtils::GetDefaultVariantFromType(const wdRTTI* pRtti)
{
  wdVariantType::Enum type = pRtti->GetVariantType();
  switch (type)
  {
    case wdVariant::Type::TypedObject:
    {
      wdVariant val;
      val.MoveTypedObject(pRtti->GetAllocator()->Allocate<void>(), pRtti);
      return val;
    }
    break;

    default:
      return GetDefaultVariantFromType(type);
  }
  return wdVariant();
}

void wdReflectionUtils::SetAllMemberPropertiesToDefault(const wdRTTI* pRtti, void* pObject)
{
  wdHybridArray<wdAbstractProperty*, 32> properties;
  pRtti->GetAllProperties(properties);

  for (wdAbstractProperty* pProp : properties)
  {
    if (pProp->GetCategory() == wdPropertyCategory::Member)
    {
      const wdVariant defValue = wdReflectionUtils::GetDefaultValue(pProp);

      wdReflectionUtils::SetMemberPropertyValue(static_cast<wdAbstractMemberProperty*>(pProp), pObject, defValue);
    }
  }
}

namespace
{
  template <class C>
  struct wdClampCategoryType
  {
    enum
    {
      value = (((wdVariant::TypeDeduction<C>::value >= wdVariantType::Int8 && wdVariant::TypeDeduction<C>::value <= wdVariantType::Double) || (wdVariant::TypeDeduction<C>::value == wdVariantType::Time) || (wdVariant::TypeDeduction<C>::value == wdVariantType::Angle))) + ((wdVariant::TypeDeduction<C>::value >= wdVariantType::Vector2 && wdVariant::TypeDeduction<C>::value <= wdVariantType::Vector4U) * 2)
    };
  };

  template <typename T, int V = wdClampCategoryType<T>::value>
  struct ClampVariantFuncImpl
  {
    static WD_ALWAYS_INLINE wdResult Func(wdVariant& value, const wdClampValueAttribute* pAttrib)
    {
      return WD_FAILURE;
    }
  };

  template <typename T>
  struct ClampVariantFuncImpl<T, 1> // scalar types
  {
    static WD_ALWAYS_INLINE wdResult Func(wdVariant& value, const wdClampValueAttribute* pAttrib)
    {
      if (pAttrib->GetMinValue().CanConvertTo<T>())
      {
        value = wdMath::Max(value.ConvertTo<T>(), pAttrib->GetMinValue().ConvertTo<T>());
      }
      if (pAttrib->GetMaxValue().CanConvertTo<T>())
      {
        value = wdMath::Min(value.ConvertTo<T>(), pAttrib->GetMaxValue().ConvertTo<T>());
      }
      return WD_SUCCESS;
    }
  };

  template <typename T>
  struct ClampVariantFuncImpl<T, 2> // vector types
  {
    static WD_ALWAYS_INLINE wdResult Func(wdVariant& value, const wdClampValueAttribute* pAttrib)
    {
      if (pAttrib->GetMinValue().CanConvertTo<T>())
      {
        value = value.ConvertTo<T>().CompMax(pAttrib->GetMinValue().ConvertTo<T>());
      }
      if (pAttrib->GetMaxValue().CanConvertTo<T>())
      {
        value = value.ConvertTo<T>().CompMin(pAttrib->GetMaxValue().ConvertTo<T>());
      }
      return WD_SUCCESS;
    }
  };

  struct ClampVariantFunc
  {
    template <typename T>
    WD_ALWAYS_INLINE wdResult operator()(wdVariant& value, const wdClampValueAttribute* pAttrib)
    {
      return ClampVariantFuncImpl<T>::Func(value, pAttrib);
    }
  };
} // namespace

wdResult wdReflectionUtils::ClampValue(wdVariant& value, const wdClampValueAttribute* pAttrib)
{
  wdVariantType::Enum type = value.GetType();
  if (type == wdVariantType::Invalid || pAttrib == nullptr)
    return WD_SUCCESS; // If there is nothing to clamp or no clamp attribute we call it a success.

  ClampVariantFunc func;
  return wdVariant::DispatchTo(func, type, value, pAttrib);
}

WD_STATICLINK_FILE(Foundation, Foundation_Reflection_Implementation_ReflectionUtils);
