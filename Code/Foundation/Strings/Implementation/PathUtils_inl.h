#pragma once

WD_ALWAYS_INLINE bool wdPathUtils::IsPathSeparator(wdUInt32 c)
{
  return (c == '/' || c == '\\');
}
