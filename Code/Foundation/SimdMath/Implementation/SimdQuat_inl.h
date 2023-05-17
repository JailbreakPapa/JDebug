#pragma once

WD_ALWAYS_INLINE wdSimdQuat::wdSimdQuat() {}

WD_ALWAYS_INLINE wdSimdQuat::wdSimdQuat(const wdSimdVec4f& v)
{
  m_v = v;
}

// static
WD_ALWAYS_INLINE wdSimdQuat wdSimdQuat::IdentityQuaternion()
{
  return wdSimdQuat(wdSimdVec4f(0.0f, 0.0f, 0.0f, 1.0f));
}

WD_ALWAYS_INLINE void wdSimdQuat::SetIdentity()
{
  m_v.Set(0.0f, 0.0f, 0.0f, 1.0f);
}

WD_ALWAYS_INLINE void wdSimdQuat::SetFromAxisAndAngle(const wdSimdVec4f& vRotationAxis, const wdSimdFloat& fAngle)
{
  ///\todo optimize
  const wdAngle halfAngle = wdAngle::Radian(fAngle) * 0.5f;
  float s = wdMath::Sin(halfAngle);
  float c = wdMath::Cos(halfAngle);

  m_v = vRotationAxis * s;
  m_v.SetW(c);
}

WD_ALWAYS_INLINE void wdSimdQuat::Normalize()
{
  m_v.Normalize<4>();
}

inline wdResult wdSimdQuat::GetRotationAxisAndAngle(wdSimdVec4f& ref_vAxis, wdSimdFloat& ref_fAngle, const wdSimdFloat& fEpsilon) const
{
  ///\todo optimize
  const wdAngle acos = wdMath::ACos(m_v.w());
  const float d = wdMath::Sin(acos);

  if (d < fEpsilon)
  {
    ref_vAxis.Set(1.0f, 0.0f, 0.0f, 0.0f);
  }
  else
  {
    ref_vAxis = m_v / d;
  }

  ref_fAngle = acos * 2;

  return WD_SUCCESS;
}

WD_ALWAYS_INLINE wdSimdMat4f wdSimdQuat::GetAsMat4() const
{
  const wdSimdVec4f xyz = m_v;
  const wdSimdVec4f x2y2z2 = xyz + xyz;
  const wdSimdVec4f xx2yy2zz2 = x2y2z2.CompMul(xyz);

  // diagonal terms
  // 1 - (yy2 + zz2)
  // 1 - (xx2 + zz2)
  // 1 - (xx2 + yy2)
  const wdSimdVec4f yy2_xx2_xx2 = xx2yy2zz2.Get<wdSwizzle::YXXX>();
  const wdSimdVec4f zz2_zz2_yy2 = xx2yy2zz2.Get<wdSwizzle::ZZYX>();
  wdSimdVec4f diagonal = wdSimdVec4f(1.0f) - (yy2_xx2_xx2 + zz2_zz2_yy2);
  diagonal.SetW(wdSimdFloat::Zero());

  // non diagonal terms
  // xy2 +- wz2
  // yz2 +- wx2
  // xz2 +- wy2
  const wdSimdVec4f x_y_x = xyz.Get<wdSwizzle::XYXX>();
  const wdSimdVec4f y2_z2_z2 = x2y2z2.Get<wdSwizzle::YZZX>();
  const wdSimdVec4f base = x_y_x.CompMul(y2_z2_z2);

  const wdSimdVec4f z2_x2_y2 = x2y2z2.Get<wdSwizzle::ZXYX>();
  const wdSimdVec4f offset = z2_x2_y2 * m_v.w();

  const wdSimdVec4f adds = base + offset;
  const wdSimdVec4f subs = base - offset;

  // final matrix layout
  // col0 = (diaX, addX, subZ, diaW)
  const wdSimdVec4f addX_u_diaX_u = adds.GetCombined<wdSwizzle::XXXX>(diagonal);
  const wdSimdVec4f subZ_u_diaW_u = subs.GetCombined<wdSwizzle::ZXWX>(diagonal);
  const wdSimdVec4f col0 = addX_u_diaX_u.GetCombined<wdSwizzle::ZXXZ>(subZ_u_diaW_u);

  // col1 = (subX, diaY, addY, diaW)
  const wdSimdVec4f subX_u_diaY_u = subs.GetCombined<wdSwizzle::XXYX>(diagonal);
  const wdSimdVec4f addY_u_diaW_u = adds.GetCombined<wdSwizzle::YXWX>(diagonal);
  const wdSimdVec4f col1 = subX_u_diaY_u.GetCombined<wdSwizzle::XZXZ>(addY_u_diaW_u);

  // col2 = (addZ, subY, diaZ, diaW)
  const wdSimdVec4f addZ_u_subY_u = adds.GetCombined<wdSwizzle::ZXYX>(subs);
  const wdSimdVec4f col2 = addZ_u_subY_u.GetCombined<wdSwizzle::XZZW>(diagonal);

  return wdSimdMat4f(col0, col1, col2, wdSimdVec4f(0, 0, 0, 1));
}

WD_ALWAYS_INLINE bool wdSimdQuat::IsValid(const wdSimdFloat& fEpsilon) const
{
  return m_v.IsNormalized<4>(fEpsilon);
}

WD_ALWAYS_INLINE bool wdSimdQuat::IsNaN() const
{
  return m_v.IsNaN<4>();
}

WD_ALWAYS_INLINE wdSimdQuat wdSimdQuat::operator-() const
{
  return m_v.FlipSign(wdSimdVec4b(true, true, true, false));
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdQuat::operator*(const wdSimdVec4f& v) const
{
  wdSimdVec4f t = m_v.CrossRH(v);
  t += t;
  return v + t * m_v.w() + m_v.CrossRH(t);
}

WD_ALWAYS_INLINE wdSimdQuat wdSimdQuat::operator*(const wdSimdQuat& q2) const
{
  wdSimdQuat q;

  q.m_v = q2.m_v * m_v.w() + m_v * q2.m_v.w() + m_v.CrossRH(q2.m_v);
  q.m_v.SetW(m_v.w() * q2.m_v.w() - m_v.Dot<3>(q2.m_v));

  return q;
}

WD_ALWAYS_INLINE bool wdSimdQuat::operator==(const wdSimdQuat& q2) const
{
  return (m_v == q2.m_v).AllSet<4>();
}

WD_ALWAYS_INLINE bool wdSimdQuat::operator!=(const wdSimdQuat& q2) const
{
  return (m_v != q2.m_v).AnySet<4>();
}
