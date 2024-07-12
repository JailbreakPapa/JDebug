#pragma once

template <typename Type>
NS_FORCE_INLINE nsBoundingBoxSphereTemplate<Type>::nsBoundingBoxSphereTemplate()
{
#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  // m_vOrigin and m_vBoxHalfExtends are already initialized to NaN by their own constructor.
  const Type TypeNaN = nsMath::NaN<Type>();
  m_fSphereRadius = TypeNaN;
#endif
}

template <typename Type>
NS_FORCE_INLINE nsBoundingBoxSphereTemplate<Type>::nsBoundingBoxSphereTemplate(const nsBoundingBoxSphereTemplate& rhs)
{
  m_vCenter = rhs.m_vCenter;
  m_fSphereRadius = rhs.m_fSphereRadius;
  m_vBoxHalfExtends = rhs.m_vBoxHalfExtends;
}

template <typename Type>
void nsBoundingBoxSphereTemplate<Type>::operator=(const nsBoundingBoxSphereTemplate& rhs)
{
  m_vCenter = rhs.m_vCenter;
  m_fSphereRadius = rhs.m_fSphereRadius;
  m_vBoxHalfExtends = rhs.m_vBoxHalfExtends;
}

template <typename Type>
nsBoundingBoxSphereTemplate<Type>::nsBoundingBoxSphereTemplate(const nsBoundingBoxTemplate<Type>& box)
  : m_vCenter(box.GetCenter())
{
  m_vBoxHalfExtends = box.GetHalfExtents();
  m_fSphereRadius = m_vBoxHalfExtends.GetLength();
}

template <typename Type>
nsBoundingBoxSphereTemplate<Type>::nsBoundingBoxSphereTemplate(const nsBoundingSphereTemplate<Type>& sphere)
  : m_vCenter(sphere.m_vCenter)
  , m_fSphereRadius(sphere.m_fRadius)
{
  m_vBoxHalfExtends.Set(m_fSphereRadius);
}


template <typename Type>
NS_FORCE_INLINE nsBoundingBoxSphereTemplate<Type> nsBoundingBoxSphereTemplate<Type>::MakeZero()
{
  nsBoundingBoxSphereTemplate<Type> res;
  res.m_vCenter.SetZero();
  res.m_fSphereRadius = 0;
  res.m_vBoxHalfExtends.SetZero();
  return res;
}

template <typename Type>
NS_FORCE_INLINE nsBoundingBoxSphereTemplate<Type> nsBoundingBoxSphereTemplate<Type>::MakeInvalid()
{
  nsBoundingBoxSphereTemplate<Type> res;
  res.m_vCenter.SetZero();
  res.m_fSphereRadius = -nsMath::SmallEpsilon<Type>(); // has to be very small for ExpandToInclude to work
  res.m_vBoxHalfExtends.Set(-nsMath::MaxValue<Type>());
  return res;
}

template <typename Type>
NS_FORCE_INLINE nsBoundingBoxSphereTemplate<Type> nsBoundingBoxSphereTemplate<Type>::MakeFromCenterExtents(const nsVec3Template<Type>& vCenter, const nsVec3Template<Type>& vBoxHalfExtents, Type fSphereRadius)
{
  nsBoundingBoxSphereTemplate<Type> res;
  res.m_vCenter = vCenter;
  res.m_fSphereRadius = fSphereRadius;
  res.m_vBoxHalfExtends = vBoxHalfExtents;
  return res;
}

template <typename Type>
nsBoundingBoxSphereTemplate<Type> nsBoundingBoxSphereTemplate<Type>::MakeFromPoints(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /*= sizeof(nsVec3Template<Type>)*/)
{
  nsBoundingBoxTemplate<Type> box = nsBoundingBoxTemplate<Type>::MakeFromPoints(pPoints, uiNumPoints, uiStride);

  nsBoundingBoxSphereTemplate<Type> res;
  res.m_vCenter = box.GetCenter();
  res.m_vBoxHalfExtends = box.GetHalfExtents();

  nsBoundingSphereTemplate<Type> sphere = nsBoundingSphereTemplate<Type>::MakeFromCenterAndRadius(res.m_vCenter, 0.0f);
  sphere.ExpandToInclude(pPoints, uiNumPoints, uiStride);

  res.m_fSphereRadius = sphere.m_fRadius;
  return res;
}

template <typename Type>
nsBoundingBoxSphereTemplate<Type> nsBoundingBoxSphereTemplate<Type>::MakeFromBox(const nsBoundingBoxTemplate<Type>& box)
{
  nsBoundingBoxSphereTemplate<Type> res;
  res.m_vCenter = box.GetCenter();
  res.m_vBoxHalfExtends = box.GetHalfExtents();
  res.m_fSphereRadius = res.m_vBoxHalfExtends.GetLength();
  return res;
}

template <typename Type>
nsBoundingBoxSphereTemplate<Type> nsBoundingBoxSphereTemplate<Type>::MakeFromSphere(const nsBoundingSphereTemplate<Type>& sphere)
{
  nsBoundingBoxSphereTemplate<Type> res;
  res.m_vCenter = sphere.m_vCenter;
  res.m_fSphereRadius = sphere.m_fRadius;
  res.m_vBoxHalfExtends.Set(res.m_fSphereRadius);
  return res;
}

template <typename Type>
nsBoundingBoxSphereTemplate<Type> nsBoundingBoxSphereTemplate<Type>::MakeFromBoxAndSphere(const nsBoundingBoxTemplate<Type>& box, const nsBoundingSphereTemplate<Type>& sphere)
{
  nsBoundingBoxSphereTemplate<Type> res;
  res.m_vCenter = box.GetCenter();
  res.m_vBoxHalfExtends = box.GetHalfExtents();
  res.m_fSphereRadius = nsMath::Min(res.m_vBoxHalfExtends.GetLength(), (sphere.m_vCenter - res.m_vCenter).GetLength() + sphere.m_fRadius);
  return res;
}

template <typename Type>
NS_FORCE_INLINE bool nsBoundingBoxSphereTemplate<Type>::IsValid() const
{
  return (m_vCenter.IsValid() && m_fSphereRadius >= 0.0f && m_vBoxHalfExtends.IsValid() && (m_vBoxHalfExtends.x >= 0) && (m_vBoxHalfExtends.y >= 0) && (m_vBoxHalfExtends.z >= 0));
}

template <typename Type>
NS_FORCE_INLINE bool nsBoundingBoxSphereTemplate<Type>::IsNaN() const
{
  return (m_vCenter.IsNaN() || nsMath::IsNaN(m_fSphereRadius) || m_vBoxHalfExtends.IsNaN());
}

template <typename Type>
NS_FORCE_INLINE const nsBoundingBoxTemplate<Type> nsBoundingBoxSphereTemplate<Type>::GetBox() const
{
  return nsBoundingBoxTemplate<Type>::MakeFromMinMax(m_vCenter - m_vBoxHalfExtends, m_vCenter + m_vBoxHalfExtends);
}

template <typename Type>
NS_FORCE_INLINE const nsBoundingSphereTemplate<Type> nsBoundingBoxSphereTemplate<Type>::GetSphere() const
{
  return nsBoundingSphereTemplate<Type>::MakeFromCenterAndRadius(m_vCenter, m_fSphereRadius);
}

template <typename Type>
void nsBoundingBoxSphereTemplate<Type>::ExpandToInclude(const nsBoundingBoxSphereTemplate& rhs)
{
  nsBoundingBoxTemplate<Type> box;
  box.m_vMin = m_vCenter - m_vBoxHalfExtends;
  box.m_vMax = m_vCenter + m_vBoxHalfExtends;
  box.ExpandToInclude(rhs.GetBox());

  nsBoundingBoxSphereTemplate<Type> result = nsBoundingBoxSphereTemplate<Type>::MakeFromBox(box);

  const float fSphereRadiusA = (m_vCenter - result.m_vCenter).GetLength() + m_fSphereRadius;
  const float fSphereRadiusB = (rhs.m_vCenter - result.m_vCenter).GetLength() + rhs.m_fSphereRadius;

  m_vCenter = result.m_vCenter;
  m_fSphereRadius = nsMath::Min(result.m_fSphereRadius, nsMath::Max(fSphereRadiusA, fSphereRadiusB));
  m_vBoxHalfExtends = result.m_vBoxHalfExtends;
}

template <typename Type>
void nsBoundingBoxSphereTemplate<Type>::Transform(const nsMat4Template<Type>& mTransform)
{
  m_vCenter = mTransform.TransformPosition(m_vCenter);
  const nsVec3Template<Type> Scale = mTransform.GetScalingFactors();
  m_fSphereRadius *= nsMath::Max(Scale.x, Scale.y, Scale.z);

  nsMat3Template<Type> mAbsRotation = mTransform.GetRotationalPart();
  for (nsUInt32 i = 0; i < 9; ++i)
  {
    mAbsRotation.m_fElementsCM[i] = nsMath::Abs(mAbsRotation.m_fElementsCM[i]);
  }

  m_vBoxHalfExtends = mAbsRotation.TransformDirection(m_vBoxHalfExtends).CompMin(nsVec3(m_fSphereRadius));
}

template <typename Type>
NS_FORCE_INLINE bool operator==(const nsBoundingBoxSphereTemplate<Type>& lhs, const nsBoundingBoxSphereTemplate<Type>& rhs)
{
  return lhs.m_vCenter == rhs.m_vCenter && lhs.m_vBoxHalfExtends == rhs.m_vBoxHalfExtends && lhs.m_fSphereRadius == rhs.m_fSphereRadius;
}

/// \brief Checks whether this box and the other are not identical.
template <typename Type>
NS_ALWAYS_INLINE bool operator!=(const nsBoundingBoxSphereTemplate<Type>& lhs, const nsBoundingBoxSphereTemplate<Type>& rhs)
{
  return !(lhs == rhs);
}
