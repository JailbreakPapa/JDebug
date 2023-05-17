#pragma once

template <typename Type>
WD_FORCE_INLINE wdBoundingBoxSphereTemplate<Type>::wdBoundingBoxSphereTemplate()
{
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  // m_vOrigin and m_vBoxHalfExtends are already initialized to NaN by their own constructor.
  const Type TypeNaN = wdMath::NaN<Type>();
  m_fSphereRadius = TypeNaN;
#endif
}

template <typename Type>
wdBoundingBoxSphereTemplate<Type>::wdBoundingBoxSphereTemplate(
  const wdVec3Template<Type>& vCenter, const wdVec3Template<Type>& vBoxHalfExtents, Type fSphereRadius)
{
  m_vCenter = vCenter;
  m_fSphereRadius = fSphereRadius;
  m_vBoxHalfExtends = vBoxHalfExtents;
}

template <typename Type>
wdBoundingBoxSphereTemplate<Type>::wdBoundingBoxSphereTemplate(const wdBoundingBoxTemplate<Type>& box, const wdBoundingSphereTemplate<Type>& sphere)
{
  m_vCenter = box.GetCenter();
  m_vBoxHalfExtends = box.GetHalfExtents();
  m_fSphereRadius = wdMath::Min(m_vBoxHalfExtends.GetLength(), (sphere.m_vCenter - m_vCenter).GetLength() + sphere.m_fRadius);
}

template <typename Type>
wdBoundingBoxSphereTemplate<Type>::wdBoundingBoxSphereTemplate(const wdBoundingBoxTemplate<Type>& box)
{
  m_vCenter = box.GetCenter();
  m_vBoxHalfExtends = box.GetHalfExtents();
  m_fSphereRadius = m_vBoxHalfExtends.GetLength();
}

template <typename Type>
wdBoundingBoxSphereTemplate<Type>::wdBoundingBoxSphereTemplate(const wdBoundingSphereTemplate<Type>& sphere)
{
  m_vCenter = sphere.m_vCenter;
  m_fSphereRadius = sphere.m_fRadius;
  m_vBoxHalfExtends.Set(m_fSphereRadius);
}

template <typename Type>
WD_FORCE_INLINE void wdBoundingBoxSphereTemplate<Type>::SetInvalid()
{
  m_vCenter.SetZero();
  m_fSphereRadius = -wdMath::SmallEpsilon<Type>();
  m_vBoxHalfExtends.Set(-wdMath::MaxValue<Type>());
}

template <typename Type>
WD_FORCE_INLINE bool wdBoundingBoxSphereTemplate<Type>::IsValid() const
{
  return (m_vCenter.IsValid() && m_fSphereRadius >= 0.0f && m_vBoxHalfExtends.IsValid());
}

template <typename Type>
WD_FORCE_INLINE bool wdBoundingBoxSphereTemplate<Type>::IsNaN() const
{
  return (m_vCenter.IsNaN() || wdMath::IsNaN(m_fSphereRadius) || m_vBoxHalfExtends.IsNaN());
}

template <typename Type>
void wdBoundingBoxSphereTemplate<Type>::SetFromPoints(const wdVec3Template<Type>* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride)
{
  wdBoundingBoxTemplate<Type> box;
  box.SetFromPoints(pPoints, uiNumPoints, uiStride);

  m_vCenter = box.GetCenter();
  m_vBoxHalfExtends = box.GetHalfExtents();

  wdBoundingSphereTemplate<Type> sphere(m_vCenter, 0.0f);
  sphere.ExpandToInclude(pPoints, uiNumPoints, uiStride);

  m_fSphereRadius = sphere.m_fRadius;
}

template <typename Type>
WD_FORCE_INLINE const wdBoundingBoxTemplate<Type> wdBoundingBoxSphereTemplate<Type>::GetBox() const
{
  return wdBoundingBoxTemplate<Type>(m_vCenter - m_vBoxHalfExtends, m_vCenter + m_vBoxHalfExtends);
}

template <typename Type>
WD_FORCE_INLINE const wdBoundingSphereTemplate<Type> wdBoundingBoxSphereTemplate<Type>::GetSphere() const
{
  return wdBoundingSphereTemplate<Type>(m_vCenter, m_fSphereRadius);
}

template <typename Type>
void wdBoundingBoxSphereTemplate<Type>::ExpandToInclude(const wdBoundingBoxSphereTemplate& rhs)
{
  wdBoundingBoxTemplate<Type> box;
  box.m_vMin = m_vCenter - m_vBoxHalfExtends;
  box.m_vMax = m_vCenter + m_vBoxHalfExtends;
  box.ExpandToInclude(rhs.GetBox());

  wdBoundingBoxSphereTemplate<Type> result(box);

  const float fSphereRadiusA = (m_vCenter - result.m_vCenter).GetLength() + m_fSphereRadius;
  const float fSphereRadiusB = (rhs.m_vCenter - result.m_vCenter).GetLength() + rhs.m_fSphereRadius;

  m_vCenter = result.m_vCenter;
  m_fSphereRadius = wdMath::Min(result.m_fSphereRadius, wdMath::Max(fSphereRadiusA, fSphereRadiusB));
  m_vBoxHalfExtends = result.m_vBoxHalfExtends;
}

template <typename Type>
void wdBoundingBoxSphereTemplate<Type>::Transform(const wdMat4Template<Type>& mTransform)
{
  m_vCenter = mTransform.TransformPosition(m_vCenter);
  const wdVec3Template<Type> Scale = mTransform.GetScalingFactors();
  m_fSphereRadius *= wdMath::Max(Scale.x, Scale.y, Scale.z);

  wdMat3Template<Type> mAbsRotation = mTransform.GetRotationalPart();
  for (wdUInt32 i = 0; i < 9; ++i)
  {
    mAbsRotation.m_fElementsCM[i] = wdMath::Abs(mAbsRotation.m_fElementsCM[i]);
  }

  m_vBoxHalfExtends = mAbsRotation.TransformDirection(m_vBoxHalfExtends).CompMin(wdVec3(m_fSphereRadius));
}

template <typename Type>
WD_FORCE_INLINE bool operator==(const wdBoundingBoxSphereTemplate<Type>& lhs, const wdBoundingBoxSphereTemplate<Type>& rhs)
{
  return lhs.m_vCenter == rhs.m_vCenter && lhs.m_vBoxHalfExtends == rhs.m_vBoxHalfExtends && lhs.m_fSphereRadius == rhs.m_fSphereRadius;
}

/// \brief Checks whether this box and the other are not identical.
template <typename Type>
WD_ALWAYS_INLINE bool operator!=(const wdBoundingBoxSphereTemplate<Type>& lhs, const wdBoundingBoxSphereTemplate<Type>& rhs)
{
  return !(lhs == rhs);
}
