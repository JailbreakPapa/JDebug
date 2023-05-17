
#include <Foundation/Types/Implementation/DelegateHelper_inl.h>

template <typename Function>
WD_ALWAYS_INLINE wdDelegate<Function> wdMakeDelegate(Function* pFunction)
{
  return wdDelegate<Function>(pFunction);
}

template <typename Method, typename Class>
WD_ALWAYS_INLINE typename wdMakeDelegateHelper<Method>::DelegateType wdMakeDelegate(Method method, Class* pClass)
{
  return typename wdMakeDelegateHelper<Method>::DelegateType(method, pClass);
}
