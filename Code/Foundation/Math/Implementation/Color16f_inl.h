
inline nsColorLinear16f::nsColorLinear16f() = default;

inline nsColorLinear16f::nsColorLinear16f(nsFloat16 r, nsFloat16 g, nsFloat16 b, nsFloat16 a)
  : r(r)
  , g(g)
  , b(b)
  , a(a)
{
}

inline nsColorLinear16f::nsColorLinear16f(const nsColor& color)
  : r(color.r)
  , g(color.g)
  , b(color.b)
  , a(color.a)
{
}

inline nsColor nsColorLinear16f::ToLinearFloat() const
{
  return nsColor(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), static_cast<float>(a));
}
