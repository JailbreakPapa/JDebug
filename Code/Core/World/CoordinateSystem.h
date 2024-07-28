#pragma once

#include <Foundation/Math/Vec3.h>

#include <Core/World/Declarations.h>
#include <Foundation/Types/RefCounted.h>

struct NS_CORE_DLL nsCoordinateSystem
{
  NS_DECLARE_POD_TYPE();

  nsVec3 m_vForwardDir;
  nsVec3 m_vRightDir;
  nsVec3 m_vUpDir;
};

class NS_CORE_DLL nsCoordinateSystemProvider : public nsRefCounted
{
public:
  nsCoordinateSystemProvider(const nsWorld* pOwnerWorld)
    : m_pOwnerWorld(pOwnerWorld)
  {
  }

  virtual ~nsCoordinateSystemProvider() = default;

  virtual void GetCoordinateSystem(const nsVec3& vGlobalPosition, nsCoordinateSystem& out_coordinateSystem) const = 0;

protected:
  friend class nsWorld;

  const nsWorld* m_pOwnerWorld;
};

/// \brief Helper class to convert between two nsCoordinateSystem spaces.
///
/// All functions will do an identity transform until SetConversion is called to set up
/// the conversion. Afterwards the convert functions can be used to convert between
/// the two systems in both directions.
/// Currently, only uniformly scaled orthogonal coordinate systems are supported.
/// They can however be right handed or left handed.
class NS_CORE_DLL nsCoordinateSystemConversion
{
public:
  /// \brief Creates a new conversion that until set up, does identity conversions.
  nsCoordinateSystemConversion(); // [tested]

  /// \brief Set up the source and target coordinate systems.
  void SetConversion(const nsCoordinateSystem& source, const nsCoordinateSystem& target); // [tested]
  /// \brief Returns the equivalent point in the target coordinate system.
  nsVec3 ConvertSourcePosition(const nsVec3& vPos) const; // [tested]
  /// \brief Returns the equivalent rotation in the target coordinate system.
  nsQuat ConvertSourceRotation(const nsQuat& qOrientation) const; // [tested]
  /// \brief Returns the equivalent length in the target coordinate system.
  float ConvertSourceLength(float fLength) const; // [tested]

  /// \brief Returns the equivalent point in the source coordinate system.
  nsVec3 ConvertTargetPosition(const nsVec3& vPos) const; // [tested]
  /// \brief Returns the equivalent rotation in the source coordinate system.
  nsQuat ConvertTargetRotation(const nsQuat& qOrientation) const; // [tested]
  /// \brief Returns the equivalent length in the source coordinate system.
  float ConvertTargetLength(float fLength) const; // [tested]

private:
  nsMat3 m_mSourceToTarget;
  nsMat3 m_mTargetToSource;
  float m_fWindingSwap = 1.0f;
  float m_fSourceToTargetScale = 1.0f;
  float m_fTargetToSourceScale = 1.0f;
};
