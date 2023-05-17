
inline wdColorLinear16f::wdColorLinear16f() {}

inline wdColorLinear16f::wdColorLinear16f(wdFloat16 r, wdFloat16 g, wdFloat16 b, wdFloat16 a)
  : r(r)
  , g(g)
  , b(b)
  , a(a)
{
}

inline wdColorLinear16f::wdColorLinear16f(const wdColor& color)
  : r(color.r)
  , g(color.g)
  , b(color.b)
  , a(color.a)
{
}

inline wdColor wdColorLinear16f::ToLinearFloat() const
{
  return wdColor(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), static_cast<float>(a));
}
