#include <Foundation/Logging/Log.h>

template <typename T>
T nsObjectAccessorBase::Get(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index /*= nsVariant()*/)
{
  nsVariant value;
  nsStatus res = GetValue(pObject, pProp, value, index);
  if (res.m_Result.Failed())
    nsLog::Error("GetValue failed: {0}", res.m_sMessage);
  return value.ConvertTo<T>();
}

template <typename T>
T nsObjectAccessorBase::Get(const nsDocumentObject* pObject, nsStringView sProp, nsVariant index /*= nsVariant()*/)
{
  nsVariant value;
  nsStatus res = GetValue(pObject, sProp, value, index);
  if (res.m_Result.Failed())
    nsLog::Error("GetValue failed: {0}", res.m_sMessage);
  return value.ConvertTo<T>();
}

inline nsInt32 nsObjectAccessorBase::GetCount(const nsDocumentObject* pObject, const nsAbstractProperty* pProp)
{
  nsInt32 iCount = 0;
  nsStatus res = GetCount(pObject, pProp, iCount);
  if (res.m_Result.Failed())
    nsLog::Error("GetCount failed: {0}", res.m_sMessage);
  return iCount;
}

inline nsInt32 nsObjectAccessorBase::GetCount(const nsDocumentObject* pObject, nsStringView sProp)
{
  nsInt32 iCount = 0;
  nsStatus res = GetCount(pObject, sProp, iCount);
  if (res.m_Result.Failed())
    nsLog::Error("GetCount failed: {0}", res.m_sMessage);
  return iCount;
}
