#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Reflection/IReflectedTypeAccessor.h>

bool wdIReflectedTypeAccessor::GetValues(const char* szProperty, wdDynamicArray<wdVariant>& out_values) const
{
  wdHybridArray<wdVariant, 16> keys;
  if (!GetKeys(szProperty, keys))
    return false;

  out_values.Clear();
  out_values.Reserve(keys.GetCount());
  for (wdVariant key : keys)
  {
    out_values.PushBack(GetValue(szProperty, key));
  }
  return true;
}
