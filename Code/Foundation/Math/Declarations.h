#pragma once

/// \file

#include <Foundation/Basics.h>

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
#  define NS_NAN_ASSERT(obj) (obj)->AssertNotNaN();
#else
#  define NS_NAN_ASSERT(obj)
#endif

#define NS_DECLARE_IF_FLOAT_TYPE template <typename = typename std::enable_if<std::is_floating_point_v<Type> == true>>
#define NS_IMPLEMENT_IF_FLOAT_TYPE template <typename ENABLE_IF_FLOAT>

/// \brief Simple helper union to store ints and floats to modify their bit patterns.
union nsIntFloatUnion
{
  constexpr nsIntFloatUnion(float fInit)
    : f(fInit)
  {
  }

  constexpr nsIntFloatUnion(nsUInt32 uiInit)
    : i(uiInit)
  {
  }

  nsUInt32 i;
  float f;
};

/// \brief Simple helper union to store ints and doubles to modify their bit patterns.
union nsInt64DoubleUnion
{

  constexpr nsInt64DoubleUnion(double fInit)
    : f(fInit)
  {
  }
  constexpr nsInt64DoubleUnion(nsUInt64 uiInit)
    : i(uiInit)
  {
  }

  nsUInt64 i;
  double f;
};

/// \brief Enum to describe which memory layout is used to store a matrix in a float array.
///
/// All nsMatX classes use column-major format internally. That means they contain one array
/// of, e.g. 16 elements, and the first elements represent the first column, then the second column, etc.
/// So the data is stored column by column and is thus column-major.
/// Some other libraries, such as OpenGL or DirectX require data represented either in column-major
/// or row-major format. nsMatrixLayout allows to retrieve the data from an nsMatX class in the proper format,
/// and it also allows to pass matrix data as an array back in the nsMatX class, and have it converted properly.
/// That means, if you need to pass the content of an nsMatX to a function that requires the data in row-major
/// format, you specify that you want to convert the matrix to nsMatrixLayout::RowMajor format and you will get
/// the data properly transposed. If a function requires data in column-major format, you specify
/// nsMatrixLayout::ColumnMajor and you get it in column-major format (which is simply a memcpy).
struct nsMatrixLayout
{
  enum Enum
  {
    RowMajor,   ///< The matrix is stored in row-major format.
    ColumnMajor ///< The matrix is stored in column-major format.
  };
};

/// \brief Describes for which depth range a projection matrix is constructed.
///
/// Different Rendering APIs use different depth ranges.
/// E.g. OpenGL uses -1 for the near plane and +1 for the far plane.
/// DirectX uses 0 for the near plane and 1 for the far plane.
struct nsClipSpaceDepthRange
{
  enum Enum
  {
    MinusOneToOne, ///< Near plane at -1, far plane at +1
    ZeroToOne,     ///< Near plane at 0, far plane at 1
  };

  /// \brief Holds the default value for the projection depth range on each platform.
  /// This can be overridden by renderers to ensure the proper range is used when they become active.
  /// On Windows/D3D this is initialized with 'ZeroToOne' by default on all other platforms/OpenGL it is initialized with 'MinusOneToOne' by default.
  NS_FOUNDATION_DLL static Enum Default;
};

/// \brief Specifies whether a projection matrix should flip the result along the Y axis or not.
///
/// Mostly needed to compensate for differing Y texture coordinate conventions. Ie. on some platforms
/// the Y texture coordinate origin is at the lower left and on others on the upper left. To prevent having
/// to modify content to compensate, instead textures are simply flipped along Y on texture load.
/// The same has to be done for all render targets, ie. content has to be rendered upside-down.
///
/// Use nsClipSpaceYMode::RenderToTextureDefault when rendering to a texture, to always get the correct
/// projection matrix.
struct nsClipSpaceYMode
{
  enum Enum
  {
    Regular, ///< Creates a regular projection matrix
    Flipped, ///< Creates a projection matrix that flips the image on its head. On platforms with different Y texture coordinate
             ///< conventions, this can be used to compensate, by rendering images flipped to render targets.
  };

  /// \brief Holds the platform default value for the clip space Y mode when rendering to a texture.
  /// This can be overridden by renderers to ensure the proper mode is used when they become active.
  /// On Windows/D3D this is initialized with 'Regular' by default on all other platforms/OpenGL it is initialized with 'Flipped' by default.
  NS_FOUNDATION_DLL static Enum RenderToTextureDefault;
};

/// \brief For selecting a left-handed or right-handed convention
struct nsHandedness
{
  enum Enum
  {
    LeftHanded,
    RightHanded,
  };

  /// \brief Holds the default handedness value to use. ns uses 'LeftHanded' by default.
  NS_FOUNDATION_DLL static Enum Default /*= nsHandedness::LeftHanded*/;
};

// forward declarations
template <typename Type>
class nsVec2Template;

using nsVec2 = nsVec2Template<float>;
using nsVec2d = nsVec2Template<double>;
using nsVec2I32 = nsVec2Template<nsInt32>;
using nsVec2U32 = nsVec2Template<nsUInt32>;
using nsVec2I64 = nsVec2Template<nsInt64>;
using nsVec2U64 = nsVec2Template<nsUInt64>;

template <typename Type>
class nsVec3Template;

using nsVec3 = nsVec3Template<float>;
using nsVec3d = nsVec3Template<double>;
using nsVec3I32 = nsVec3Template<nsInt32>;
using nsVec3U32 = nsVec3Template<nsUInt32>;
using nsVec3I64 = nsVec3Template<nsInt64>;
using nsVec3U64 = nsVec3Template<nsUInt64>;

template <typename Type>
class nsVec4Template;

using nsVec4 = nsVec4Template<float>;
using nsVec4d = nsVec4Template<double>;
using nsVec4I64 = nsVec4Template<nsInt64>;
using nsVec4I32 = nsVec4Template<nsInt32>;
using nsVec4I16 = nsVec4Template<nsInt16>;
using nsVec4I8 = nsVec4Template<nsInt8>;
using nsVec4U64 = nsVec4Template<nsUInt64>;
using nsVec4U32 = nsVec4Template<nsUInt32>;
using nsVec4U16 = nsVec4Template<nsUInt16>;
using nsVec4U8 = nsVec4Template<nsUInt8>;

template <typename Type>
class nsMat3Template;

using nsMat3 = nsMat3Template<float>;
using nsMat3d = nsMat3Template<double>;

template <typename Type>
class nsMat4Template;

using nsMat4 = nsMat4Template<float>;
using nsMat4d = nsMat4Template<double>;

template <typename Type>
struct nsPlaneTemplate;

using nsPlane = nsPlaneTemplate<float>;
using nsPlaned = nsPlaneTemplate<double>;

template <typename Type>
class nsQuatTemplate;

using nsQuat = nsQuatTemplate<float>;
using nsQuatd = nsQuatTemplate<double>;

template <typename Type>
class nsBoundingBoxTemplate;

using nsBoundingBox = nsBoundingBoxTemplate<float>;
using nsBoundingBoxd = nsBoundingBoxTemplate<double>;
using nsBoundingBoxu32 = nsBoundingBoxTemplate<nsUInt32>;

template <typename Type>
class nsBoundingBoxSphereTemplate;

using nsBoundingBoxSphere = nsBoundingBoxSphereTemplate<float>;
using nsBoundingBoxSphered = nsBoundingBoxSphereTemplate<double>;

template <typename Type>
class nsBoundingSphereTemplate;

using nsBoundingSphere = nsBoundingSphereTemplate<float>;
using nsBoundingSphered = nsBoundingSphereTemplate<double>;

template <nsUInt8 DecimalBits>
class nsFixedPoint;

class nsAngle;

template <typename Type>
class nsTransformTemplate;

using nsTransform = nsTransformTemplate<float>;
using nsTransformd = nsTransformTemplate<double>;

class nsColor;
class nsColorLinearUB;
class nsColorGammaUB;

class nsRandom;

template <typename Type>
class nsRectTemplate;

using nsRectU32 = nsRectTemplate<nsUInt32>;
using nsRectU16 = nsRectTemplate<nsUInt16>;
using nsRectI32 = nsRectTemplate<nsInt32>;
using nsRectI16 = nsRectTemplate<nsInt16>;
using nsRectFloat = nsRectTemplate<float>;
using nsRectDouble = nsRectTemplate<double>;

class nsFrustum;


/// \brief An enum that allows to select on of the six main axis (positive / negative)
struct NS_FOUNDATION_DLL nsBasisAxis
{
  using StorageType = nsInt8;

  /// \brief An enum that allows to select on of the six main axis (positive / negative)
  enum Enum : nsInt8
  {
    PositiveX,
    PositiveY,
    PositiveZ,
    NegativeX,
    NegativeY,
    NegativeZ,

    Default = PositiveX
  };

  /// \brief Returns the vector for the given axis. E.g. (1, 0, 0) or (0, -1, 0), etc.
  static nsVec3 GetBasisVector(nsBasisAxis::Enum basisAxis);

  /// \brief Computes a matrix representing the transformation. 'Forward' represents the X axis, 'Right' the Y axis and 'Up' the Z axis.
  static nsMat3 CalculateTransformationMatrix(nsBasisAxis::Enum forwardDir, nsBasisAxis::Enum rightDir, nsBasisAxis::Enum dir, float fUniformScale = 1.0f, float fScaleX = 1.0f, float fScaleY = 1.0f, float fScaleZ = 1.0f);

  /// \brief Returns a quaternion that rotates from 'identity' to 'axis'
  static nsQuat GetBasisRotation(nsBasisAxis::Enum identity, nsBasisAxis::Enum axis);

  /// \brief Returns a quaternion that rotates from 'PositiveX' to 'axis'
  static nsQuat GetBasisRotation_PosX(nsBasisAxis::Enum axis);

  /// \brief Returns the axis that is orthogonal to axis1 and axis2. If 'flip' is set, it returns the negated axis.
  ///
  /// If axis1 and axis2 are not orthogonal to each other, the value of axis1 is returned as the result.
  static nsBasisAxis::Enum GetOrthogonalAxis(nsBasisAxis::Enum axis1, nsBasisAxis::Enum axis2, bool bFlip);
};

/// \brief An enum that represents the operator of a comparison
struct NS_FOUNDATION_DLL nsComparisonOperator
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    Default = Equal
  };

  /// \brief Compares a to b with the given operator. This function only needs the == and < operator for T.
  template <typename T>
  static bool Compare(nsComparisonOperator::Enum cmp, const T& a, const T& b); // [tested]
};
