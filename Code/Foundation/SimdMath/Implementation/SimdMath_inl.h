#pragma once

///\todo optimize these methods if needed

// static
NS_FORCE_INLINE nsSimdVec4f nsSimdMath::Exp(const nsSimdVec4f& f)
{
#if NS_ENABLED(NS_COMPILER_MSVC) && NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
  return _mm_exp_ps(f.m_v);
#else
  return nsSimdVec4f(nsMath::Exp(f.x()), nsMath::Exp(f.y()), nsMath::Exp(f.z()), nsMath::Exp(f.w()));
#endif
}

// static
NS_FORCE_INLINE nsSimdVec4f nsSimdMath::Ln(const nsSimdVec4f& f)
{
#if NS_ENABLED(NS_COMPILER_MSVC) && NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
  return _mm_log_ps(f.m_v);
#else
  return nsSimdVec4f(nsMath::Ln(f.x()), nsMath::Ln(f.y()), nsMath::Ln(f.z()), nsMath::Ln(f.w()));
#endif
}

// static
NS_FORCE_INLINE nsSimdVec4f nsSimdMath::Log2(const nsSimdVec4f& f)
{
#if NS_ENABLED(NS_COMPILER_MSVC) && NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
  return _mm_log2_ps(f.m_v);
#else
  return nsSimdVec4f(nsMath::Log2(f.x()), nsMath::Log2(f.y()), nsMath::Log2(f.z()), nsMath::Log2(f.w()));
#endif
}

// static
NS_FORCE_INLINE nsSimdVec4i nsSimdMath::Log2i(const nsSimdVec4i& i)
{
  return nsSimdVec4i(nsMath::Log2i(i.x()), nsMath::Log2i(i.y()), nsMath::Log2i(i.z()), nsMath::Log2i(i.w()));
}

// static
NS_FORCE_INLINE nsSimdVec4f nsSimdMath::Log10(const nsSimdVec4f& f)
{
#if NS_ENABLED(NS_COMPILER_MSVC) && NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
  return _mm_log10_ps(f.m_v);
#else
  return nsSimdVec4f(nsMath::Log10(f.x()), nsMath::Log10(f.y()), nsMath::Log10(f.z()), nsMath::Log10(f.w()));
#endif
}

// static
NS_FORCE_INLINE nsSimdVec4f nsSimdMath::Pow2(const nsSimdVec4f& f)
{
#if NS_ENABLED(NS_COMPILER_MSVC) && NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
  return _mm_exp2_ps(f.m_v);
#else
  return nsSimdVec4f(nsMath::Pow2(f.x()), nsMath::Pow2(f.y()), nsMath::Pow2(f.z()), nsMath::Pow2(f.w()));
#endif
}

// static
NS_FORCE_INLINE nsSimdVec4f nsSimdMath::Sin(const nsSimdVec4f& f)
{
#if NS_ENABLED(NS_COMPILER_MSVC) && NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
  return _mm_sin_ps(f.m_v);
#else
  return nsSimdVec4f(nsMath::Sin(nsAngle::MakeFromRadian(f.x())), nsMath::Sin(nsAngle::MakeFromRadian(f.y())), nsMath::Sin(nsAngle::MakeFromRadian(f.z())),
    nsMath::Sin(nsAngle::MakeFromRadian(f.w())));
#endif
}

// static
NS_FORCE_INLINE nsSimdVec4f nsSimdMath::Cos(const nsSimdVec4f& f)
{
#if NS_ENABLED(NS_COMPILER_MSVC) && NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
  return _mm_cos_ps(f.m_v);
#else
  return nsSimdVec4f(nsMath::Cos(nsAngle::MakeFromRadian(f.x())), nsMath::Cos(nsAngle::MakeFromRadian(f.y())), nsMath::Cos(nsAngle::MakeFromRadian(f.z())),
    nsMath::Cos(nsAngle::MakeFromRadian(f.w())));
#endif
}

// static
NS_FORCE_INLINE nsSimdVec4f nsSimdMath::Tan(const nsSimdVec4f& f)
{
#if NS_ENABLED(NS_COMPILER_MSVC) && NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
  return _mm_tan_ps(f.m_v);
#else
  return nsSimdVec4f(nsMath::Tan(nsAngle::MakeFromRadian(f.x())), nsMath::Tan(nsAngle::MakeFromRadian(f.y())), nsMath::Tan(nsAngle::MakeFromRadian(f.z())),
    nsMath::Tan(nsAngle::MakeFromRadian(f.w())));
#endif
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdMath::ASin(const nsSimdVec4f& f)
{
  return nsSimdVec4f(nsMath::Pi<float>() * 0.5f) - ACos(f);
}

// 4th order polynomial approximation
// 7 * 10^-5 radians precision
// Reference : Handbook of Mathematical Functions (chapter : Elementary Transcendental Functions), M. Abramowitz and I.A. Stegun, Ed.
// static
NS_FORCE_INLINE nsSimdVec4f nsSimdMath::ACos(const nsSimdVec4f& f)
{
  nsSimdVec4f x1 = f.Abs();
  nsSimdVec4f x2 = x1.CompMul(x1);
  nsSimdVec4f x3 = x2.CompMul(x1);

  nsSimdVec4f s = x1 * -0.2121144f + nsSimdVec4f(1.5707288f);
  s += x2 * 0.0742610f;
  s += x3 * -0.0187293f;
  s = s.CompMul((nsSimdVec4f(1.0f) - x1).GetSqrt());

  return nsSimdVec4f::Select(f >= nsSimdVec4f::MakeZero(), s, nsSimdVec4f(nsMath::Pi<float>()) - s);
}

// Reference: https://seblagarde.wordpress.com/2014/12/01/inverse-trigonometric-functions-gpu-optimization-for-amd-gcn-architecture/
// static
NS_FORCE_INLINE nsSimdVec4f nsSimdMath::ATan(const nsSimdVec4f& f)
{
  nsSimdVec4f x = f.Abs();
  nsSimdVec4f t0 = nsSimdVec4f::Select(x < nsSimdVec4f(1.0f), x, x.GetReciprocal());
  nsSimdVec4f t1 = t0.CompMul(t0);
  nsSimdVec4f poly = nsSimdVec4f(0.0872929f);
  poly = nsSimdVec4f(-0.301895f) + poly.CompMul(t1);
  poly = nsSimdVec4f(1.0f) + poly.CompMul(t1);
  poly = poly.CompMul(t0);
  t0 = nsSimdVec4f::Select(x < nsSimdVec4f(1.0f), poly, nsSimdVec4f(nsMath::Pi<float>() * 0.5f) - poly);

  return nsSimdVec4f::Select(f < nsSimdVec4f::MakeZero(), -t0, t0);
}
