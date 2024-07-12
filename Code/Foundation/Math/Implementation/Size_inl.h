#pragma once

template <typename Type>
NS_ALWAYS_INLINE nsSizeTemplate<Type>::nsSizeTemplate() = default;

template <typename Type>
NS_ALWAYS_INLINE nsSizeTemplate<Type>::nsSizeTemplate(Type width, Type height)
  : width(width)
  , height(height)
{
}

template <typename Type>
NS_ALWAYS_INLINE bool nsSizeTemplate<Type>::HasNonZeroArea() const
{
  return (width > 0) && (height > 0);
}

template <typename Type>
NS_ALWAYS_INLINE bool operator==(const nsSizeTemplate<Type>& v1, const nsSizeTemplate<Type>& v2)
{
  return v1.height == v2.height && v1.width == v2.width;
}

template <typename Type>
NS_ALWAYS_INLINE bool operator!=(const nsSizeTemplate<Type>& v1, const nsSizeTemplate<Type>& v2)
{
  return v1.height != v2.height || v1.width != v2.width;
}
