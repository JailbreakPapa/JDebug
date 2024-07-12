
#include <Foundation/Types/Implementation/DelegateHelper_inl.h>

template <typename Function>
NS_ALWAYS_INLINE nsDelegate<Function> nsMakeDelegate(Function* pFunction)
{
  return nsDelegate<Function>(pFunction);
}

template <typename Method, typename Class>
NS_ALWAYS_INLINE typename nsMakeDelegateHelper<Method>::DelegateType nsMakeDelegate(Method method, Class* pClass)
{
  return typename nsMakeDelegateHelper<Method>::DelegateType(method, pClass);
}
