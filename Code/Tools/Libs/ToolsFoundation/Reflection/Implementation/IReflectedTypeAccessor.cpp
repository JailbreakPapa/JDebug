/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Reflection/IReflectedTypeAccessor.h>

bool nsIReflectedTypeAccessor::GetValues(nsStringView sProperty, nsDynamicArray<nsVariant>& out_values) const
{
  nsHybridArray<nsVariant, 16> keys;
  if (!GetKeys(sProperty, keys))
    return false;

  out_values.Clear();
  out_values.Reserve(keys.GetCount());
  for (nsVariant key : keys)
  {
    out_values.PushBack(GetValue(sProperty, key));
  }
  return true;
}
