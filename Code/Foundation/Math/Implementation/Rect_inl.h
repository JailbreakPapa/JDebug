#pragma once

template <typename Type>
NS_ALWAYS_INLINE nsRectTemplate<Type>::nsRectTemplate() = default;

template <typename Type>
NS_ALWAYS_INLINE nsRectTemplate<Type>::nsRectTemplate(Type x, Type y, Type width, Type height)
  : x(x)
  , y(y)
  , width(width)
  , height(height)
{
}

template <typename Type>
NS_ALWAYS_INLINE nsRectTemplate<Type>::nsRectTemplate(Type width, Type height)
  : x(0)
  , y(0)
  , width(width)
  , height(height)
{
}

template <typename Type>
NS_ALWAYS_INLINE nsRectTemplate<Type>::nsRectTemplate(const nsVec2Template<Type>& vTopLeftPosition, const nsVec2Template<Type>& vSize)
{
  x = vTopLeftPosition.x;
  y = vTopLeftPosition.y;
  width = vSize.x;
  height = vSize.y;
}

template <typename Type>
nsRectTemplate<Type> nsRectTemplate<Type>::MakeInvalid()
{
  /// \test This is new

  nsRectTemplate<Type> res;

  const Type fLargeValue = nsMath::MaxValue<Type>() / 2;
  res.x = fLargeValue;
  res.y = fLargeValue;
  res.width = -fLargeValue;
  res.height = -fLargeValue;

  return res;
}

template <typename Type>
nsRectTemplate<Type> nsRectTemplate<Type>::MakeIntersection(const nsRectTemplate<Type>& r0, const nsRectTemplate<Type>& r1)
{
  /// \test This is new

  nsRectTemplate<Type> res;

  Type x1 = nsMath::Max(r0.GetX1(), r1.GetX1());
  Type y1 = nsMath::Max(r0.GetY1(), r1.GetY1());
  Type x2 = nsMath::Min(r0.GetX2(), r1.GetX2());
  Type y2 = nsMath::Min(r0.GetY2(), r1.GetY2());

  res.x = x1;
  res.y = y1;
  res.width = x2 - x1;
  res.height = y2 - y1;

  return res;
}

template <typename Type>
nsRectTemplate<Type> nsRectTemplate<Type>::MakeUnion(const nsRectTemplate<Type>& r0, const nsRectTemplate<Type>& r1)
{
  /// \test This is new

  nsRectTemplate<Type> res;

  Type x1 = nsMath::Min(r0.GetX1(), r1.GetX1());
  Type y1 = nsMath::Min(r0.GetY1(), r1.GetY1());
  Type x2 = nsMath::Max(r0.GetX2(), r1.GetX2());
  Type y2 = nsMath::Max(r0.GetY2(), r1.GetY2());

  res.x = x1;
  res.y = y1;
  res.width = x2 - x1;
  res.height = y2 - y1;

  return res;
}

template <typename Type>
NS_ALWAYS_INLINE bool nsRectTemplate<Type>::operator==(const nsRectTemplate<Type>& rhs) const
{
  return x == rhs.x && y == rhs.y && width == rhs.width && height == rhs.height;
}

template <typename Type>
NS_ALWAYS_INLINE bool nsRectTemplate<Type>::operator!=(const nsRectTemplate<Type>& rhs) const
{
  return !(*this == rhs);
}

template <typename Type>
NS_ALWAYS_INLINE bool nsRectTemplate<Type>::HasNonZeroArea() const
{
  return (width > 0) && (height > 0);
}

template <typename Type>
NS_ALWAYS_INLINE bool nsRectTemplate<Type>::Contains(const nsVec2Template<Type>& vPoint) const
{
  if (vPoint.x >= x && vPoint.x <= Right())
  {
    if (vPoint.y >= y && vPoint.y <= Bottom())
      return true;
  }

  return false;
}

template <typename Type>
NS_ALWAYS_INLINE bool nsRectTemplate<Type>::Contains(const nsRectTemplate<Type>& r) const
{
  return r.x >= x && r.y >= y && r.Right() <= Right() && r.Bottom() <= Bottom();
}

template <typename Type>
NS_ALWAYS_INLINE bool nsRectTemplate<Type>::Overlaps(const nsRectTemplate<Type>& other) const
{
  if (x < other.Right() && Right() > other.x && y < other.Bottom() && Bottom() > other.y)
    return true;

  return false;
}

template <typename Type>
void nsRectTemplate<Type>::ExpandToInclude(const nsRectTemplate<Type>& other)
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
void nsRectTemplate<Type>::ExpandToInclude(const nsVec2Template<Type>& other)
{
  Type thisRight = Right();
  Type thisBottom = Bottom();

  if (other.x < x)
    x = other.x;

  if (other.y < y)
    y = other.y;

  if (other.x > thisRight)
    width = other.x - x;
  else
    width = thisRight - x;

  if (other.y > thisBottom)
    height = other.y - y;
  else
    height = thisBottom - y;
}

template <typename Type>
void nsRectTemplate<Type>::Grow(Type xy)
{
  x -= xy;
  y -= xy;
  width += xy * 2;
  height += xy * 2;
}

template <typename Type>
NS_ALWAYS_INLINE void nsRectTemplate<Type>::Clip(const nsRectTemplate<Type>& clipRect)
{
  Type newLeft = nsMath::Max<Type>(x, clipRect.x);
  Type newTop = nsMath::Max<Type>(y, clipRect.y);

  Type newRight = nsMath::Min<Type>(Right(), clipRect.Right());
  Type newBottom = nsMath::Min<Type>(Bottom(), clipRect.Bottom());

  x = newLeft;
  y = newTop;
  width = newRight - newLeft;
  height = newBottom - newTop;
}

template <typename Type>
NS_ALWAYS_INLINE bool nsRectTemplate<Type>::IsValid() const
{
  /// \test This is new

  return width >= 0 && height >= 0;
}

template <typename Type>
NS_ALWAYS_INLINE const nsVec2Template<Type> nsRectTemplate<Type>::GetClampedPoint(const nsVec2Template<Type>& vPoint) const
{
  /// \test This is new

  return nsVec2Template<Type>(nsMath::Clamp(vPoint.x, Left(), Right()), nsMath::Clamp(vPoint.y, Top(), Bottom()));
}

template <typename Type>
void nsRectTemplate<Type>::SetCenter(Type tX, Type tY)
{
  /// \test This is new

  x = tX - width / 2;
  y = tY - height / 2;
}

template <typename Type>
void nsRectTemplate<Type>::Translate(Type tX, Type tY)
{
  /// \test This is new

  x += tX;
  y += tY;
}

template <typename Type>
void nsRectTemplate<Type>::Scale(Type sX, Type sY)
{
  /// \test This is new

  x *= sX;
  y *= sY;
  width *= sX;
  height *= sY;
}
