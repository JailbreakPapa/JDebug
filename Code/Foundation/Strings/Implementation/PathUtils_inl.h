#pragma once

NS_ALWAYS_INLINE bool nsPathUtils::IsPathSeparator(nsUInt32 c)
{
  return (c == '/' || c == '\\');
}
