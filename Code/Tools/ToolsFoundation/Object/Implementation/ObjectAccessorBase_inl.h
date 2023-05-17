#include <Foundation/Logging/Log.h>

template <typename T>
T wdObjectAccessorBase::Get(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index /*= wdVariant()*/)
{
  wdVariant value;
  wdStatus res = GetValue(pObject, pProp, value, index);
  if (res.m_Result.Failed())
    wdLog::Error("GetValue failed: {0}", res.m_sMessage);
  return value.ConvertTo<T>();
}

template <typename T>
T wdObjectAccessorBase::Get(const wdDocumentObject* pObject, const char* szProp, wdVariant index /*= wdVariant()*/)
{
  wdVariant value;
  wdStatus res = GetValue(pObject, szProp, value, index);
  if (res.m_Result.Failed())
    wdLog::Error("GetValue failed: {0}", res.m_sMessage);
  return value.ConvertTo<T>();
}

inline wdInt32 wdObjectAccessorBase::GetCount(const wdDocumentObject* pObject, const wdAbstractProperty* pProp)
{
  wdInt32 iCount = 0;
  wdStatus res = GetCount(pObject, pProp, iCount);
  if (res.m_Result.Failed())
    wdLog::Error("GetCount failed: {0}", res.m_sMessage);
  return iCount;
}

inline wdInt32 wdObjectAccessorBase::GetCount(const wdDocumentObject* pObject, const char* szProp)
{
  wdInt32 iCount = 0;
  wdStatus res = GetCount(pObject, szProp, iCount);
  if (res.m_Result.Failed())
    wdLog::Error("GetCount failed: {0}", res.m_sMessage);
  return iCount;
}
