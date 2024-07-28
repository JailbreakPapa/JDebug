#include <Core/CorePCH.h>

#include <Core/World/CoordinateSystem.h>


nsCoordinateSystemConversion::nsCoordinateSystemConversion()
{
  m_mSourceToTarget.SetIdentity();
  m_mTargetToSource.SetIdentity();
}

void nsCoordinateSystemConversion::SetConversion(const nsCoordinateSystem& source, const nsCoordinateSystem& target)
{
  float fSourceScale = source.m_vForwardDir.GetLengthSquared();
  NS_ASSERT_DEV(nsMath::IsEqual(fSourceScale, source.m_vRightDir.GetLengthSquared(), nsMath::DefaultEpsilon<float>()),
    "Only uniformly scaled coordinate systems are supported");
  NS_ASSERT_DEV(nsMath::IsEqual(fSourceScale, source.m_vUpDir.GetLengthSquared(), nsMath::DefaultEpsilon<float>()),
    "Only uniformly scaled coordinate systems are supported");
  nsMat3 mSourceFromId;
  mSourceFromId.SetColumn(0, source.m_vRightDir);
  mSourceFromId.SetColumn(1, source.m_vUpDir);
  mSourceFromId.SetColumn(2, source.m_vForwardDir);

  float fTargetScale = target.m_vForwardDir.GetLengthSquared();
  NS_ASSERT_DEV(nsMath::IsEqual(fTargetScale, target.m_vRightDir.GetLengthSquared(), nsMath::DefaultEpsilon<float>()),
    "Only uniformly scaled coordinate systems are supported");
  NS_ASSERT_DEV(nsMath::IsEqual(fTargetScale, target.m_vUpDir.GetLengthSquared(), nsMath::DefaultEpsilon<float>()),
    "Only uniformly scaled coordinate systems are supported");
  nsMat3 mTargetFromId;
  mTargetFromId.SetColumn(0, target.m_vRightDir);
  mTargetFromId.SetColumn(1, target.m_vUpDir);
  mTargetFromId.SetColumn(2, target.m_vForwardDir);

  m_mSourceToTarget = mTargetFromId * mSourceFromId.GetInverse();
  m_mSourceToTarget.SetColumn(0, m_mSourceToTarget.GetColumn(0).GetNormalized());
  m_mSourceToTarget.SetColumn(1, m_mSourceToTarget.GetColumn(1).GetNormalized());
  m_mSourceToTarget.SetColumn(2, m_mSourceToTarget.GetColumn(2).GetNormalized());

  m_fWindingSwap = m_mSourceToTarget.GetDeterminant() < 0 ? -1.0f : 1.0f;
  m_fSourceToTargetScale = 1.0f / nsMath::Sqrt(fSourceScale) * nsMath::Sqrt(fTargetScale);
  m_mTargetToSource = m_mSourceToTarget.GetInverse();
  m_fTargetToSourceScale = 1.0f / m_fSourceToTargetScale;
}

nsVec3 nsCoordinateSystemConversion::ConvertSourcePosition(const nsVec3& vPos) const
{
  return m_mSourceToTarget * vPos * m_fSourceToTargetScale;
}

nsQuat nsCoordinateSystemConversion::ConvertSourceRotation(const nsQuat& qOrientation) const
{
  nsVec3 axis = m_mSourceToTarget * qOrientation.GetVectorPart();
  nsQuat rr(axis.x, axis.y, axis.z, qOrientation.w * m_fWindingSwap);
  return rr;
}

float nsCoordinateSystemConversion::ConvertSourceLength(float fLength) const
{
  return fLength * m_fSourceToTargetScale;
}

nsVec3 nsCoordinateSystemConversion::ConvertTargetPosition(const nsVec3& vPos) const
{
  return m_mTargetToSource * vPos * m_fTargetToSourceScale;
}

nsQuat nsCoordinateSystemConversion::ConvertTargetRotation(const nsQuat& qOrientation) const
{
  nsVec3 axis = m_mTargetToSource * qOrientation.GetVectorPart();
  nsQuat rr(axis.x, axis.y, axis.z, qOrientation.w * m_fWindingSwap);
  return rr;
}

float nsCoordinateSystemConversion::ConvertTargetLength(float fLength) const
{
  return fLength * m_fTargetToSourceScale;
}
