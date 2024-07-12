#pragma once

NS_ALWAYS_INLINE nsSimdQuat::nsSimdQuat() = default;

NS_ALWAYS_INLINE nsSimdQuat::nsSimdQuat(const nsSimdVec4f& v)
  : m_v(v)
{
}

NS_ALWAYS_INLINE const nsSimdQuat nsSimdQuat::MakeIdentity()
{
  return nsSimdQuat(nsSimdVec4f(0.0f, 0.0f, 0.0f, 1.0f));
}

NS_ALWAYS_INLINE nsSimdQuat nsSimdQuat::MakeFromElements(nsSimdFloat x, nsSimdFloat y, nsSimdFloat z, nsSimdFloat w)
{
  return nsSimdQuat(nsSimdVec4f(x, y, z, w));
}

inline nsSimdQuat nsSimdQuat::MakeFromAxisAndAngle(const nsSimdVec4f& vRotationAxis, const nsSimdFloat& fAngle)
{
  ///\todo optimize
  const nsAngle halfAngle = nsAngle::MakeFromRadian(fAngle) * 0.5f;
  float s = nsMath::Sin(halfAngle);
  float c = nsMath::Cos(halfAngle);

  nsSimdQuat res;
  res.m_v = vRotationAxis * s;
  res.m_v.SetW(c);
  return res;
}

NS_ALWAYS_INLINE void nsSimdQuat::Normalize()
{
  m_v.Normalize<4>();
}

inline nsResult nsSimdQuat::GetRotationAxisAndAngle(nsSimdVec4f& ref_vAxis, nsSimdFloat& ref_fAngle, const nsSimdFloat& fEpsilon) const
{
  ///\todo optimize
  const nsAngle acos = nsMath::ACos(m_v.w().Max(-1).Min(1));
  const float d = nsMath::Sin(acos);

  if (d < fEpsilon)
  {
    ref_vAxis.Set(1.0f, 0.0f, 0.0f, 0.0f);
  }
  else
  {
    ref_vAxis = m_v / d;
  }

  ref_fAngle = acos * 2;

  return NS_SUCCESS;
}

NS_ALWAYS_INLINE nsSimdMat4f nsSimdQuat::GetAsMat4() const
{
  const nsSimdVec4f xyz = m_v;
  const nsSimdVec4f x2y2z2 = xyz + xyz;
  const nsSimdVec4f xx2yy2zz2 = x2y2z2.CompMul(xyz);

  // diagonal terms
  // 1 - (yy2 + zz2)
  // 1 - (xx2 + zz2)
  // 1 - (xx2 + yy2)
  const nsSimdVec4f yy2_xx2_xx2 = xx2yy2zz2.Get<nsSwizzle::YXXX>();
  const nsSimdVec4f zz2_zz2_yy2 = xx2yy2zz2.Get<nsSwizzle::ZZYX>();
  nsSimdVec4f diagonal = nsSimdVec4f(1.0f) - (yy2_xx2_xx2 + zz2_zz2_yy2);
  diagonal.SetW(nsSimdFloat::MakeZero());

  // non diagonal terms
  // xy2 +- wz2
  // yz2 +- wx2
  // xz2 +- wy2
  const nsSimdVec4f x_y_x = xyz.Get<nsSwizzle::XYXX>();
  const nsSimdVec4f y2_z2_z2 = x2y2z2.Get<nsSwizzle::YZZX>();
  const nsSimdVec4f base = x_y_x.CompMul(y2_z2_z2);

  const nsSimdVec4f z2_x2_y2 = x2y2z2.Get<nsSwizzle::ZXYX>();
  const nsSimdVec4f offset = z2_x2_y2 * m_v.w();

  const nsSimdVec4f adds = base + offset;
  const nsSimdVec4f subs = base - offset;

  // final matrix layout
  // col0 = (diaX, addX, subZ, diaW)
  const nsSimdVec4f addX_u_diaX_u = adds.GetCombined<nsSwizzle::XXXX>(diagonal);
  const nsSimdVec4f subZ_u_diaW_u = subs.GetCombined<nsSwizzle::ZXWX>(diagonal);
  const nsSimdVec4f col0 = addX_u_diaX_u.GetCombined<nsSwizzle::ZXXZ>(subZ_u_diaW_u);

  // col1 = (subX, diaY, addY, diaW)
  const nsSimdVec4f subX_u_diaY_u = subs.GetCombined<nsSwizzle::XXYX>(diagonal);
  const nsSimdVec4f addY_u_diaW_u = adds.GetCombined<nsSwizzle::YXWX>(diagonal);
  const nsSimdVec4f col1 = subX_u_diaY_u.GetCombined<nsSwizzle::XZXZ>(addY_u_diaW_u);

  // col2 = (addZ, subY, diaZ, diaW)
  const nsSimdVec4f addZ_u_subY_u = adds.GetCombined<nsSwizzle::ZXYX>(subs);
  const nsSimdVec4f col2 = addZ_u_subY_u.GetCombined<nsSwizzle::XZZW>(diagonal);

  return nsSimdMat4f::MakeFromColumns(col0, col1, col2, nsSimdVec4f(0, 0, 0, 1));
}

NS_ALWAYS_INLINE bool nsSimdQuat::IsValid(const nsSimdFloat& fEpsilon) const
{
  return m_v.IsNormalized<4>(fEpsilon);
}

NS_ALWAYS_INLINE bool nsSimdQuat::IsNaN() const
{
  return m_v.IsNaN<4>();
}

NS_ALWAYS_INLINE nsSimdQuat nsSimdQuat::operator-() const
{
  return nsSimdQuat(m_v.FlipSign(nsSimdVec4b(true, true, true, false)));
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdQuat::operator*(const nsSimdVec4f& v) const
{
  nsSimdVec4f t = m_v.CrossRH(v);
  t += t;
  return v + t * m_v.w() + m_v.CrossRH(t);
}

NS_ALWAYS_INLINE nsSimdQuat nsSimdQuat::operator*(const nsSimdQuat& q2) const
{
  nsSimdQuat q;

  q.m_v = q2.m_v * m_v.w() + m_v * q2.m_v.w() + m_v.CrossRH(q2.m_v);
  q.m_v.SetW(m_v.w() * q2.m_v.w() - m_v.Dot<3>(q2.m_v));

  return q;
}

NS_ALWAYS_INLINE bool nsSimdQuat::operator==(const nsSimdQuat& q2) const
{
  return (m_v == q2.m_v).AllSet<4>();
}

NS_ALWAYS_INLINE bool nsSimdQuat::operator!=(const nsSimdQuat& q2) const
{
  return (m_v != q2.m_v).AnySet<4>();
}
