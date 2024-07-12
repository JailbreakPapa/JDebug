#pragma once

#include <Foundation/Math/Vec3.h>
#include <Foundation/Math/Vec4.h>

/// \brief Describes on which side of a plane a point or an object is located.
struct nsPositionOnPlane
{
  enum Enum
  {
    Back,     ///< Something is completely on the back side of a plane
    Front,    ///< Something is completely in front of a plane
    OnPlane,  ///< Something is lying completely on a plane (all points)
    Spanning, ///< Something is spanning a plane, i.e. some points are on the front and some on the back
  };
};

/// \brief A class that represents a mathematical plane.
template <typename Type>
struct nsPlaneTemplate
{
public:
  // Means this object can be copied using memcpy instead of copy construction.
  NS_DECLARE_POD_TYPE();

  using ComponentType = Type;

  // *** Data ***
public:
  nsVec3Template<Type> m_vNormal;
  Type m_fNegDistance;


  // *** Constructors ***
public:
  /// \brief Default constructor. Does not initialize the plane.
  nsPlaneTemplate(); // [tested]

  /// \brief Returns an invalid plane with a zero normal.
  [[nodiscard]] static nsPlaneTemplate<Type> MakeInvalid();

  /// \brief Creates a plane from a normal and a point on the plane.
  ///
  /// \note This function asserts that the normal is normalized.
  [[nodiscard]] static nsPlaneTemplate<Type> MakeFromNormalAndPoint(const nsVec3Template<Type>& vNormal, const nsVec3Template<Type>& vPointOnPlane);

  /// \brief Creates a plane from three points.
  ///
  /// \note Asserts that the 3 points properly form a plane.
  /// Only use this function when you are certain that the input data isn't degenerate.
  /// If the data cannot be trusted, use SetFromPoints() and check the result.
  [[nodiscard]] static nsPlaneTemplate<Type> MakeFromPoints(const nsVec3Template<Type>& v1, const nsVec3Template<Type>& v2, const nsVec3Template<Type>& v3);

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  void AssertNotNaN() const
  {
    NS_ASSERT_ALWAYS(!IsNaN(), "This object contains NaN values. This can happen when you forgot to initialize it before using it. Please check that "
                               "all code-paths properly initialize this object.");
  }
#endif

  /// \brief Returns an nsVec4 with the plane normal in x,y,z and the negative distance in w.
  nsVec4Template<Type> GetAsVec4() const;

  /// \brief Creates the plane-equation from three points on the plane.
  nsResult SetFromPoints(const nsVec3Template<Type>& v1, const nsVec3Template<Type>& v2, const nsVec3Template<Type>& v3); // [tested]

  /// \brief Creates the plane-equation from three points on the plane, given as an array.
  nsResult SetFromPoints(const nsVec3Template<Type>* const pVertices); // [tested]

  /// \brief Creates the plane-equation from a set of unreliable points lying on the same plane. Some points might be equal or too close to each other
  /// for the typical algorithm. Returns false, if no reliable set of points could be found. Does try to create a plane anyway.
  nsResult SetFromPoints(const nsVec3Template<Type>* const pVertices, nsUInt32 uiMaxVertices); // [tested]

  /// \brief Creates a plane from two direction vectors that span the plane, and one point on it.
  nsResult SetFromDirections(const nsVec3Template<Type>& vTangent1, const nsVec3Template<Type>& vTangent2, const nsVec3Template<Type>& vPointOnPlane); // [tested]

  // *** Distance and Position ***
public:
  /// \brief Returns the distance of the point to the plane.
  Type GetDistanceTo(const nsVec3Template<Type>& vPoint) const; // [tested]

  /// \brief Returns the minimum distance that any of the given points had to the plane.
  ///
  /// 'Minimum' means the (non-absolute) distance of a point to the plane. So a point behind the plane will always have a 'lower distance'
  /// than a point in front of the plane, even if that is closer to the plane's surface.
  Type GetMinimumDistanceTo(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsVec3Template<Type>)) const; // [tested]

  /// \brief Returns the minimum distance between given box and a plane
  Type GetMinimumDistanceTo(const nsBoundingBoxTemplate<Type>& box) const; // [tested]

  /// \brief Returns the maximum distance between given box and a plane
  Type GetMaximumDistanceTo(const nsBoundingBoxTemplate<Type>& box) const; // [tested]

  /// \brief Returns the minimum and maximum distance that any of the given points had to the plane.
  ///
  /// 'Minimum' (and 'maximum') means the (non-absolute) distance of a point to the plane. So a point behind the plane will always have a 'lower
  /// distance' than a point in front of the plane, even if that is closer to the plane's surface.
  void GetMinMaxDistanceTo(Type& out_fMin, Type& out_fMax, const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsVec3Template<Type>)) const; // [tested]

  /// \brief Returns on which side of the plane the point lies.
  nsPositionOnPlane::Enum GetPointPosition(const nsVec3Template<Type>& vPoint) const; // [tested]

  /// \brief Returns on which side of the plane the point lies.
  nsPositionOnPlane::Enum GetPointPosition(const nsVec3Template<Type>& vPoint, Type fPlaneHalfWidth) const; // [tested]

  /// \brief Returns on which side of the plane the set of points lies. Might be on both sides.
  nsPositionOnPlane::Enum GetObjectPosition(const nsVec3Template<Type>* const pPoints, nsUInt32 uiVertices) const; // [tested]

  /// \brief Returns on which side of the plane the set of points lies. Might be on both sides.
  nsPositionOnPlane::Enum GetObjectPosition(const nsVec3Template<Type>* const pPoints, nsUInt32 uiVertices, Type fPlaneHalfWidth) const; // [tested]

  /// \brief Returns on which side of the plane the sphere is located.
  nsPositionOnPlane::Enum GetObjectPosition(const nsBoundingSphereTemplate<Type>& sphere) const; // [tested]

  /// \brief Returns on which side of the plane the box is located.
  nsPositionOnPlane::Enum GetObjectPosition(const nsBoundingBoxTemplate<Type>& box) const; // [tested]

  /// \brief Projects a point onto a plane (along the planes normal).
  [[nodiscard]] const nsVec3Template<Type> ProjectOntoPlane(const nsVec3Template<Type>& vPoint) const; // [tested]

  /// \brief Returns the mirrored point. E.g. on the other side of the plane, at the same distance.
  [[nodiscard]] const nsVec3Template<Type> Mirror(const nsVec3Template<Type>& vPoint) const; // [tested]

  /// \brief Take the given direction vector and returns a modified one that is coplanar to the plane.
  const nsVec3Template<Type> GetCoplanarDirection(const nsVec3Template<Type>& vDirection) const; // [tested]

  // *** Comparisons ***
public:
  /// \brief Checks whether this plane and the other are identical.
  bool IsIdentical(const nsPlaneTemplate<Type>& rhs) const; // [tested]

  /// \brief Checks whether this plane and the other are equal within some threshold.
  bool IsEqual(const nsPlaneTemplate<Type>& rhs, Type fEpsilon = nsMath::DefaultEpsilon<Type>()) const; // [tested]

  /// \brief Checks whether the plane has valid values (not NaN, normalized normal).
  bool IsValid() const; // [tested]

  /// \brief Checks whether any component is NaN.
  bool IsNaN() const; // [tested]

  /// \brief Checks whether any component is Infinity.
  bool IsFinite() const; // [tested]

  // *** Modifications ***
public:
  /// \brief Transforms the plane with the given matrix.
  void Transform(const nsMat3Template<Type>& m); // [tested]

  /// \brief Transforms the plane with the given matrix.
  void Transform(const nsMat4Template<Type>& m); // [tested]

  /// \brief Negates Normal/Distance to switch which side of the plane is front and back.
  void Flip(); // [tested]

  /// \brief Negates Normal/Distance to switch which side of the plane is front and back. Returns true, if the plane had to be flipped.
  bool FlipIfNecessary(const nsVec3Template<Type>& vPoint, bool bPlaneShouldFacePoint = true); // [tested]

  // *** Intersection Tests ***
public:
  /// \brief Returns true, if the ray hit the plane. The intersection time describes at which multiple of the ray direction the ray hit the plane.
  ///
  /// An intersection will be reported regardless of whether the ray starts 'behind' or 'in front of' the plane, as long as it points at it.
  /// \a vRayDir does not need to be normalized.\n
  /// out_vIntersection = vRayStartPos + out_fIntersection * vRayDir
  ///
  /// Intersections with \a out_fIntersection less than zero will be discarded and not reported as intersections.
  /// If such intersections are desired, use GetRayIntersectionBiDirectional instead.
  [[nodiscard]] bool GetRayIntersection(const nsVec3Template<Type>& vRayStartPos, const nsVec3Template<Type>& vRayDir, Type* out_pIntersectionDinstance = nullptr, nsVec3Template<Type>* out_pIntersection = nullptr) const; // [tested]

  /// \brief Returns true, if the ray intersects the plane. Intersection time and point are stored in the out-parameters. Allows for intersections at
  /// negative times (shooting into the opposite direction).
  [[nodiscard]] bool GetRayIntersectionBiDirectional(const nsVec3Template<Type>& vRayStartPos, const nsVec3Template<Type>& vRayDir, Type* out_pIntersectionDistance = nullptr, nsVec3Template<Type>* out_pIntersection = nullptr) const; // [tested]

  /// \brief Returns true, if there is any intersection with the plane between the line's start and end position. Returns the fraction along the line
  /// and the actual intersection point.
  [[nodiscard]] bool GetLineSegmentIntersection(const nsVec3Template<Type>& vLineStartPos, const nsVec3Template<Type>& vLineEndPos, Type* out_pHitFraction = nullptr, nsVec3Template<Type>* out_pIntersection = nullptr) const; // [tested]

  /// \brief Computes the one point where all three planes intersect. Returns NS_FAILURE if no such point exists.
  static nsResult GetPlanesIntersectionPoint(const nsPlaneTemplate<Type>& p0, const nsPlaneTemplate<Type>& p1, const nsPlaneTemplate<Type>& p2, nsVec3Template<Type>& out_vResult); // [tested]

  // *** Helper Functions ***
public:
  /// \brief Returns three points from an unreliable set of points, that reliably form a plane. Returns false, if there are none.
  static nsResult FindSupportPoints(const nsVec3Template<Type>* const pVertices, nsInt32 iMaxVertices, nsInt32& out_i1, nsInt32& out_i2, nsInt32& out_i3); // [tested]
};

/// \brief Checks whether this plane and the other are identical.
template <typename Type>
bool operator==(const nsPlaneTemplate<Type>& lhs, const nsPlaneTemplate<Type>& rhs); // [tested]

/// \brief Checks whether this plane and the other are not identical.
template <typename Type>
bool operator!=(const nsPlaneTemplate<Type>& lhs, const nsPlaneTemplate<Type>& rhs); // [tested]

#include <Foundation/Math/Implementation/Plane_inl.h>
