#pragma once

template <typename Type>
WD_ALWAYS_INLINE wdSizeTemplate<Type>::wdSizeTemplate()
{
}

template <typename Type>
WD_ALWAYS_INLINE wdSizeTemplate<Type>::wdSizeTemplate(Type width, Type height)
  : width(width)
  , height(height)
{
}

template <typename Type>
WD_ALWAYS_INLINE bool wdSizeTemplate<Type>::HasNonZeroArea() const
{
  return (width > 0) && (height > 0);
}

template <typename Type>
WD_ALWAYS_INLINE bool operator==(const wdSizeTemplate<Type>& v1, const wdSizeTemplate<Type>& v2)
{
  return v1.height == v2.height && v1.width == v2.width;
}

template <typename Type>
WD_ALWAYS_INLINE bool operator!=(const wdSizeTemplate<Type>& v1, const wdSizeTemplate<Type>& v2)
{
  return v1.height != v2.height || v1.width != v2.width;
}
