#pragma once

///\todo optimize these methods if needed

// static
WD_FORCE_INLINE wdSimdVec4f wdSimdMath::Exp(const wdSimdVec4f& f)
{
#if WD_ENABLED(WD_COMPILER_MSVC) && WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
  return _mm_exp_ps(f.m_v);
#else
  return wdSimdVec4f(wdMath::Exp(f.x()), wdMath::Exp(f.y()), wdMath::Exp(f.z()), wdMath::Exp(f.w()));
#endif
}

// static
WD_FORCE_INLINE wdSimdVec4f wdSimdMath::Ln(const wdSimdVec4f& f)
{
#if WD_ENABLED(WD_COMPILER_MSVC) && WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
  return _mm_log_ps(f.m_v);
#else
  return wdSimdVec4f(wdMath::Ln(f.x()), wdMath::Ln(f.y()), wdMath::Ln(f.z()), wdMath::Ln(f.w()));
#endif
}

// static
WD_FORCE_INLINE wdSimdVec4f wdSimdMath::Log2(const wdSimdVec4f& f)
{
#if WD_ENABLED(WD_COMPILER_MSVC) && WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
  return _mm_log2_ps(f.m_v);
#else
  return wdSimdVec4f(wdMath::Log2(f.x()), wdMath::Log2(f.y()), wdMath::Log2(f.z()), wdMath::Log2(f.w()));
#endif
}

// static
WD_FORCE_INLINE wdSimdVec4i wdSimdMath::Log2i(const wdSimdVec4i& i)
{
  return wdSimdVec4i(wdMath::Log2i(i.x()), wdMath::Log2i(i.y()), wdMath::Log2i(i.z()), wdMath::Log2i(i.w()));
}

// static
WD_FORCE_INLINE wdSimdVec4f wdSimdMath::Log10(const wdSimdVec4f& f)
{
#if WD_ENABLED(WD_COMPILER_MSVC) && WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
  return _mm_log10_ps(f.m_v);
#else
  return wdSimdVec4f(wdMath::Log10(f.x()), wdMath::Log10(f.y()), wdMath::Log10(f.z()), wdMath::Log10(f.w()));
#endif
}

// static
WD_FORCE_INLINE wdSimdVec4f wdSimdMath::Pow2(const wdSimdVec4f& f)
{
#if WD_ENABLED(WD_COMPILER_MSVC) && WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
  return _mm_exp2_ps(f.m_v);
#else
  return wdSimdVec4f(wdMath::Pow2(f.x()), wdMath::Pow2(f.y()), wdMath::Pow2(f.z()), wdMath::Pow2(f.w()));
#endif
}

// static
WD_FORCE_INLINE wdSimdVec4f wdSimdMath::Sin(const wdSimdVec4f& f)
{
#if WD_ENABLED(WD_COMPILER_MSVC) && WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
  return _mm_sin_ps(f.m_v);
#else
  return wdSimdVec4f(wdMath::Sin(wdAngle::Radian(f.x())), wdMath::Sin(wdAngle::Radian(f.y())), wdMath::Sin(wdAngle::Radian(f.z())),
    wdMath::Sin(wdAngle::Radian(f.w())));
#endif
}

// static
WD_FORCE_INLINE wdSimdVec4f wdSimdMath::Cos(const wdSimdVec4f& f)
{
#if WD_ENABLED(WD_COMPILER_MSVC) && WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
  return _mm_cos_ps(f.m_v);
#else
  return wdSimdVec4f(wdMath::Cos(wdAngle::Radian(f.x())), wdMath::Cos(wdAngle::Radian(f.y())), wdMath::Cos(wdAngle::Radian(f.z())),
    wdMath::Cos(wdAngle::Radian(f.w())));
#endif
}

// static
WD_FORCE_INLINE wdSimdVec4f wdSimdMath::Tan(const wdSimdVec4f& f)
{
#if WD_ENABLED(WD_COMPILER_MSVC) && WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
  return _mm_tan_ps(f.m_v);
#else
  return wdSimdVec4f(wdMath::Tan(wdAngle::Radian(f.x())), wdMath::Tan(wdAngle::Radian(f.y())), wdMath::Tan(wdAngle::Radian(f.z())),
    wdMath::Tan(wdAngle::Radian(f.w())));
#endif
}

// static
WD_ALWAYS_INLINE wdSimdVec4f wdSimdMath::ASin(const wdSimdVec4f& f)
{
  return wdSimdVec4f(wdMath::Pi<float>() * 0.5f) - ACos(f);
}

// 4th order polynomial approximation
// 7 * 10^-5 radians precision
// Reference : Handbook of Mathematical Functions (chapter : Elementary Transcendental Functions), M. Abramowitz and I.A. Stegun, Ed.
// static
WD_FORCE_INLINE wdSimdVec4f wdSimdMath::ACos(const wdSimdVec4f& f)
{
  wdSimdVec4f x1 = f.Abs();
  wdSimdVec4f x2 = x1.CompMul(x1);
  wdSimdVec4f x3 = x2.CompMul(x1);

  wdSimdVec4f s = x1 * -0.2121144f + wdSimdVec4f(1.5707288f);
  s += x2 * 0.0742610f;
  s += x3 * -0.0187293f;
  s = s.CompMul((wdSimdVec4f(1.0f) - x1).GetSqrt());

  return wdSimdVec4f::Select(f >= wdSimdVec4f::ZeroVector(), s, wdSimdVec4f(wdMath::Pi<float>()) - s);
}

// Reference: https://seblagarde.wordpress.com/2014/12/01/inverse-trigonometric-functions-gpu-optimization-for-amd-gcn-architecture/
// static
WD_FORCE_INLINE wdSimdVec4f wdSimdMath::ATan(const wdSimdVec4f& f)
{
  wdSimdVec4f x = f.Abs();
  wdSimdVec4f t0 = wdSimdVec4f::Select(x < wdSimdVec4f(1.0f), x, x.GetReciprocal());
  wdSimdVec4f t1 = t0.CompMul(t0);
  wdSimdVec4f poly = wdSimdVec4f(0.0872929f);
  poly = wdSimdVec4f(-0.301895f) + poly.CompMul(t1);
  poly = wdSimdVec4f(1.0f) + poly.CompMul(t1);
  poly = poly.CompMul(t0);
  t0 = wdSimdVec4f::Select(x < wdSimdVec4f(1.0f), poly, wdSimdVec4f(wdMath::Pi<float>() * 0.5f) - poly);

  return wdSimdVec4f::Select(f < wdSimdVec4f::ZeroVector(), -t0, t0);
}
