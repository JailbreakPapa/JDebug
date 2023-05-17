#pragma once

/// \file

#include <Foundation/Basics.h>

#if WD_ENABLED(WD_MATH_CHECK_FOR_NAN)
#  define WD_NAN_ASSERT(obj) (obj)->AssertNotNaN();
#else
#  define WD_NAN_ASSERT(obj)
#endif

/// \brief Simple helper union to store ints and floats to modify their bit patterns.
union wdIntFloatUnion
{
  constexpr wdIntFloatUnion(float fInit)
    : f(fInit)
  {
  }

  constexpr wdIntFloatUnion(wdUInt32 uiInit)
    : i(uiInit)
  {
  }

  wdUInt32 i;
  float f;
};

/// \brief Simple helper union to store ints and doubles to modify their bit patterns.
union wdInt64DoubleUnion
{

  constexpr wdInt64DoubleUnion(double fInit)
    : f(fInit)
  {
  }
  constexpr wdInt64DoubleUnion(wdUInt64 uiInit)
    : i(uiInit)
  {
  }

  wdUInt64 i;
  double f;
};

/// \brief Enum to describe which memory layout is used to store a matrix in a float array.
///
/// All wdMatX classes use column-major format internally. That means they contain one array
/// of, e.g. 16 elements, and the first elements represent the first column, then the second column, etc.
/// So the data is stored column by column and is thus column-major.
/// Some other libraries, such as OpenGL or DirectX require data represented either in column-major
/// or row-major format. wdMatrixLayout allows to retrieve the data from an wdMatX class in the proper format,
/// and it also allows to pass matrix data as an array back in the wdMatX class, and have it converted properly.
/// That means, if you need to pass the content of an wdMatX to a function that requires the data in row-major
/// format, you specify that you want to convert the matrix to wdMatrixLayout::RowMajor format and you will get
/// the data properly transposed. If a function requires data in column-major format, you specify
/// wdMatrixLayout::ColumnMajor and you get it in column-major format (which is simply a memcpy).
struct wdMatrixLayout
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
struct wdClipSpaceDepthRange
{
  enum Enum
  {
    MinusOneToOne, ///< Near plane at -1, far plane at +1
    ZeroToOne,     ///< Near plane at 0, far plane at 1
  };

  /// \brief Holds the default value for the projection depth range on each platform.
  /// This can be overridden by renderers to ensure the proper range is used when they become active.
  /// On Windows/D3D this is initialized with 'ZeroToOne' by default on all other platforms/OpenGL it is initialized with 'MinusOneToOne' by default.
  WD_FOUNDATION_DLL static Enum Default;
};

/// \brief Specifies whether a projection matrix should flip the result along the Y axis or not.
///
/// Mostly needed to compensate for differing Y texture coordinate conventions. Ie. on some platforms
/// the Y texture coordinate origin is at the lower left and on others on the upper left. To prevent having
/// to modify content to compensate, instead textures are simply flipped along Y on texture load.
/// The same has to be done for all render targets, ie. content has to be rendered upside-down.
///
/// Use wdClipSpaceYMode::RenderToTextureDefault when rendering to a texture, to always get the correct
/// projection matrix.
struct wdClipSpaceYMode
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
  WD_FOUNDATION_DLL static Enum RenderToTextureDefault;
};

/// \brief For selecting a left-handed or right-handed convention
struct wdHandedness
{
  enum Enum
  {
    LeftHanded,
    RightHanded,
  };

  /// \brief Holds the default handedness value to use. wd uses 'LeftHanded' by default.
  WD_FOUNDATION_DLL static Enum Default /*= wdHandedness::LeftHanded*/;
};

// forward declarations
template <typename Type>
class wdVec2Template;

using wdVec2 = wdVec2Template<float>;
using wdVec2d = wdVec2Template<double>;
using wdVec2I32 = wdVec2Template<wdInt32>;
using wdVec2U32 = wdVec2Template<wdUInt32>;

template <typename Type>
class wdVec3Template;

using wdVec3 = wdVec3Template<float>;
using wdVec3d = wdVec3Template<double>;
using wdVec3I32 = wdVec3Template<wdInt32>;
using wdVec3U32 = wdVec3Template<wdUInt32>;

template <typename Type>
class wdVec4Template;

using wdVec4 = wdVec4Template<float>;
using wdVec4d = wdVec4Template<double>;
using wdVec4I32 = wdVec4Template<wdInt32>;
using wdVec4I16 = wdVec4Template<wdInt16>;
using wdVec4I8 = wdVec4Template<wdInt8>;
using wdVec4U32 = wdVec4Template<wdUInt32>;
using wdVec4U16 = wdVec4Template<wdUInt16>;
using wdVec4U8 = wdVec4Template<wdUInt8>;

template <typename Type>
class wdMat3Template;

using wdMat3 = wdMat3Template<float>;
using wdMat3d = wdMat3Template<double>;

template <typename Type>
class wdMat4Template;

using wdMat4 = wdMat4Template<float>;
using wdMat4d = wdMat4Template<double>;

template <typename Type>
struct wdPlaneTemplate;

using wdPlane = wdPlaneTemplate<float>;
using wdPlaned = wdPlaneTemplate<double>;

template <typename Type>
class wdQuatTemplate;

using wdQuat = wdQuatTemplate<float>;
using wdQuatd = wdQuatTemplate<double>;

template <typename Type>
class wdBoundingBoxTemplate;

using wdBoundingBox = wdBoundingBoxTemplate<float>;
using wdBoundingBoxd = wdBoundingBoxTemplate<double>;
using wdBoundingBoxu32 = wdBoundingBoxTemplate<wdUInt32>;

template <typename Type>
class wdBoundingBoxSphereTemplate;

using wdBoundingBoxSphere = wdBoundingBoxSphereTemplate<float>;
using wdBoundingBoxSphered = wdBoundingBoxSphereTemplate<double>;

template <typename Type>
class wdBoundingSphereTemplate;

using wdBoundingSphere = wdBoundingSphereTemplate<float>;
using wdBoundingSphered = wdBoundingSphereTemplate<double>;

template <wdUInt8 DecimalBits>
class wdFixedPoint;

class wdAngle;

template <typename Type>
class wdTransformTemplate;

using wdTransform = wdTransformTemplate<float>;
using wdTransformd = wdTransformTemplate<double>;

class wdColor;
class wdColorLinearUB;
class wdColorGammaUB;

class wdRandom;


/// \brief An enum that allows to select on of the six main axis (positive / negative)
struct WD_FOUNDATION_DLL wdBasisAxis
{
  using StorageType = wdInt8;

  /// \brief An enum that allows to select on of the six main axis (positive / negative)
  enum Enum : wdInt8
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
  static wdVec3 GetBasisVector(wdBasisAxis::Enum basisAxis);

  /// \brief Computes a matrix representing the transformation. 'Forward' represents the X axis, 'Right' the Y axis and 'Up' the Z axis.
  static wdMat3 CalculateTransformationMatrix(wdBasisAxis::Enum forwardDir, wdBasisAxis::Enum rightDir, wdBasisAxis::Enum dir, float fUniformScale = 1.0f, float fScaleX = 1.0f, float fScaleY = 1.0f, float fScaleZ = 1.0f);

  /// \brief Returns a quaternion that rotates from 'identity' to 'axis'
  static wdQuat GetBasisRotation(wdBasisAxis::Enum identity, wdBasisAxis::Enum axis);

  /// \brief Returns a quaternion that rotates from 'PositiveX' to 'axis'
  static wdQuat GetBasisRotation_PosX(wdBasisAxis::Enum axis);

  /// \brief Returns the axis that is orthogonal to axis1 and axis2. If 'flip' is set, it returns the negated axis.
  ///
  /// If axis1 and axis2 are not orthogonal to each other, the value of axis1 is returned as the result.
  static wdBasisAxis::Enum GetOrthogonalAxis(wdBasisAxis::Enum axis1, wdBasisAxis::Enum axis2, bool bFlip);
};

/// \brief An enum that represents the operator of a comparison
struct WD_FOUNDATION_DLL wdComparisonOperator
{
  using StorageType = wdUInt8;

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

  static bool Compare(wdComparisonOperator::Enum cmp, double f1, double f2);
};
