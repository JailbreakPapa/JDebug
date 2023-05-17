#pragma once

template <typename Type>
WD_ALWAYS_INLINE wdRectTemplate<Type>::wdRectTemplate()
{
}

template <typename Type>
WD_ALWAYS_INLINE wdRectTemplate<Type>::wdRectTemplate(Type x, Type y, Type width, Type height)
  : x(x)
  , y(y)
  , width(width)
  , height(height)
{
}

template <typename Type>
WD_ALWAYS_INLINE wdRectTemplate<Type>::wdRectTemplate(Type width, Type height)
  : x(0)
  , y(0)
  , width(width)
  , height(height)
{
}

template <typename Type>
WD_ALWAYS_INLINE bool wdRectTemplate<Type>::operator==(const wdRectTemplate<Type>& rhs) const
{
  return x == rhs.x && y == rhs.y && width == rhs.width && height == rhs.height;
}

template <typename Type>
WD_ALWAYS_INLINE bool wdRectTemplate<Type>::operator!=(const wdRectTemplate<Type>& rhs) const
{
  return !(*this == rhs);
}

template <typename Type>
WD_ALWAYS_INLINE bool wdRectTemplate<Type>::HasNonZeroArea() const
{
  return (width > 0) && (height > 0);
}

template <typename Type>
WD_ALWAYS_INLINE bool wdRectTemplate<Type>::Contains(const wdVec2Template<Type>& vPoint) const
{
  if (vPoint.x >= x && vPoint.x <= Right())
  {
    if (vPoint.y >= y && vPoint.y <= Bottom())
      return true;
  }

  return false;
}

template <typename Type>
WD_ALWAYS_INLINE bool wdRectTemplate<Type>::Overlaps(const wdRectTemplate<Type>& other) const
{
  if (x < other.Right() && Right() > other.x && y < other.Bottom() && Bottom() > other.y)
    return true;

  return false;
}

template <typename Type>
void wdRectTemplate<Type>::ExpandToInclude(const wdRectTemplate<Type>& other)
{
  Type thisRight = Right();
  Type thisBottom = Bottom();

  if (other.x < x)
    x = other.x;

  if (other.y < y)
    y = other.y;

  if (other.Right() > thisRight)
    width = other.Right() - x;
  else
    width = thisRight - x;

  if (other.Bottom() > thisBottom)
    height = other.Bottom() - y;
  else
    height = thisBottom - y;
}

template <typename Type>
WD_ALWAYS_INLINE void wdRectTemplate<Type>::Clip(const wdRectTemplate<Type>& clipRect)
{
  Type newLeft = wdMath::Max<Type>(x, clipRect.x);
  Type newTop = wdMath::Max<Type>(y, clipRect.y);

  Type newRight = wdMath::Min<Type>(Right(), clipRect.Right());
  Type newBottom = wdMath::Min<Type>(Bottom(), clipRect.Bottom());

  x = newLeft;
  y = newTop;
  width = newRight - newLeft;
  height = newBottom - newTop;
}

template <typename Type>
WD_ALWAYS_INLINE void wdRectTemplate<Type>::SetInvalid()
{
  /// \test This is new

  const Type fLargeValue = wdMath::MaxValue<Type>() / 2;
  x = fLargeValue;
  y = fLargeValue;
  width = -fLargeValue;
  height = -fLargeValue;
}

template <typename Type>
WD_ALWAYS_INLINE bool wdRectTemplate<Type>::IsValid() const
{
  /// \test This is new

  return width >= 0 && height >= 0;
}

template <typename Type>
WD_ALWAYS_INLINE const wdVec2Template<Type> wdRectTemplate<Type>::GetClampedPoint(const wdVec2Template<Type>& vPoint) const
{
  /// \test This is new

  return wdVec2Template<Type>(wdMath::Clamp(vPoint.x, Left(), Right()), wdMath::Clamp(vPoint.y, Top(), Bottom()));
}

template <typename Type>
void wdRectTemplate<Type>::SetIntersection(const wdRectTemplate<Type>& r0, const wdRectTemplate<Type>& r1)
{
  /// \test This is new

  Type x1 = wdMath::Max(r0.GetX1(), r1.GetX1());
  Type y1 = wdMath::Max(r0.GetY1(), r1.GetY1());
  Type x2 = wdMath::Min(r0.GetX2(), r1.GetX2());
  Type y2 = wdMath::Min(r0.GetY2(), r1.GetY2());

  x = x1;
  y = y1;
  width = x2 - x1;
  height = y2 - y1;
}

template <typename Type>
void wdRectTemplate<Type>::SetUnion(const wdRectTemplate<Type>& r0, const wdRectTemplate<Type>& r1)
{
  /// \test This is new

  Type x1 = wdMath::Min(r0.GetX1(), r1.GetX1());
  Type y1 = wdMath::Min(r0.GetY1(), r1.GetY1());
  Type x2 = wdMath::Max(r0.GetX2(), r1.GetX2());
  Type y2 = wdMath::Max(r0.GetY2(), r1.GetY2());

  x = x1;
  y = y1;
  width = x2 - x1;
  height = y2 - y1;
}

template <typename Type>
void wdRectTemplate<Type>::Translate(Type tX, Type tY)
{
  /// \test This is new

  x += tX;
  y += tY;
}

template <typename Type>
void wdRectTemplate<Type>::Scale(Type sX, Type sY)
{
  /// \test This is new

  x *= sX;
  y *= sY;
  width *= sX;
  height *= sY;
}
