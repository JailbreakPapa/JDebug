#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Math/Color.h>
#include <Foundation/Math/Vec2.h>
#include <Foundation/Types/Enum.h>

class wdStreamWriter;
class wdStreamReader;

struct WD_FOUNDATION_DLL wdCurveTangentMode
{
  typedef wdUInt8 StorageType;

  enum Enum
  {
    Bwdier,
    FixedLength,
    Linear,
    // Constant,
    Auto,
    Default = Auto
  };
};

/// \brief A 1D curve for animating a single value over time.
class WD_FOUNDATION_DLL wdCurve1D
{
public:
  /// \brief Stores position and tangents to control spline interpolation
  struct ControlPoint
  {
    WD_DECLARE_POD_TYPE();

    ControlPoint();

    /// \brief The position (x,y) of the control point
    wdVec2d m_Position;

    /// \brief The tangent for the curve segment to the left that affects the spline interpolation
    wdVec2 m_LeftTangent;
    /// \brief The tangent for the curve segment to the right that affects the spline interpolation
    wdVec2 m_RightTangent;

    wdEnum<wdCurveTangentMode> m_TangentModeLeft;
    wdEnum<wdCurveTangentMode> m_TangentModeRight;

    wdUInt16 m_uiOriginalIndex;

    WD_ALWAYS_INLINE bool operator<(const ControlPoint& rhs) const { return m_Position.x < rhs.m_Position.x; }
  };

public:
  wdCurve1D();

  /// \brief Removes all control points.
  void Clear();

  /// \brief Checks whether the curve has any control point.
  bool IsEmpty() const;

  /// \brief Appends a control point. SortControlPoints() must be called to before evaluating the curve.
  ControlPoint& AddControlPoint(double fPos);

  /// \brief Updates the min/max X value that can be retrieved through GetExtents().
  ///
  /// This is automatically done when SortControlPoints() is called. It can be called manually, if the information is required without
  /// sorting.
  void RecomputeExtents();

  /// \brief returns the min and max position value across all control points.
  ///
  /// The returned values are only up to date if either SortControlPoints() or RecomputeExtents() was called before.
  /// Otherwise they will contain stale values.
  void QueryExtents(double& ref_fMinx, double& ref_fMaxx) const;

  /// \brief Returns the min and max Y value across the curve.
  /// For this information to be available, the linear approximation of the curve must have been computed, otherwise stale values will be
  /// returned.
  void QueryExtremeValues(double& ref_fMinVal, double& ref_fMaxVal) const;

  /// \brief Returns the number of control points.
  wdUInt32 GetNumControlPoints() const;

  /// \brief Const access to a control point.
  const ControlPoint& GetControlPoint(wdUInt32 uiIdx) const { return m_ControlPoints[uiIdx]; }

  /// \brief Non-const access to a control point. If you modify the position, SortControlPoints() has to be called before evaluating the
  /// curve.
  ControlPoint& ModifyControlPoint(wdUInt32 uiIdx) { return m_ControlPoints[uiIdx]; }

  /// \brief Sorts the control point arrays by their position. The CPs have to be sorted before calling Evaluate(), otherwise the result
  /// will be wrong.
  void SortControlPoints();

  /// \brief Evaluates the curve at the given position (x coordinate) and returns the value Y value at that point.
  ///
  /// This uses the linear approximation of the curve, so CreateLinearApproximation() must have been called first.
  ///
  /// \sa CreateLinearApproximation
  double Evaluate(double fPosition) const;

  /// \brief Takes the normalized x coordinate [0;1] and converts it into a valid position on the curve
  ///
  /// \note This only works when the curve extents are available. See QueryExtents() and RecomputeExtents().
  ///
  /// \sa RecomputeExtents
  /// \sa QueryExtents
  double ConvertNormalizedPos(double fPos) const;

  /// \brief Takes a value (typically returned by Evaluate()) and normalizes it into [0;1] range
  ///
  /// \note This only works when the linear approximation of the curve has been computed first.
  double NormalizeValue(double value) const;

  /// \brief How much heap memory the curve uses.
  wdUInt64 GetHeapMemoryUsage() const;

  /// \brief Stores the current state in a stream.
  void Save(wdStreamWriter& inout_stream) const;

  /// \brief Restores the state from a stream.
  void Load(wdStreamReader& inout_stream);

  /// \brief Pre-computes sample points for linear interpolation that approximate the curve within the allowed error threshold.
  ///
  /// \note All control points must already be in sorted order, so call SortControlPoints() first if necessary.
  void CreateLinearApproximation(double fMaxError = 0.01, wdUInt8 uiMaxSubDivs = 8);

  const wdHybridArray<wdVec2d, 24>& GetLinearApproximation() const { return m_LinearApproximation; }

  /// \brief Adjusts the tangents such that the curve cannot make loopings
  void ClampTangents();

  /// \brief Adjusts the tangents in accordance to the specified tangent modes at each control point
  ///
  /// \note All control points must already be in sorted order, so call SortControlPoints() first if necessary.
  void ApplyTangentModes();

  /// \brief Typically called by ApplyTangentModes() for specific control points. Control points must be in sorted order.
  void MakeFixedLengthTangentLeft(wdUInt32 uiCpIdx);
  /// \brief Typically called by ApplyTangentModes() for specific control points. Control points must be in sorted order.
  void MakeFixedLengthTangentRight(wdUInt32 uiCpIdx);
  /// \brief Typically called by ApplyTangentModes() for specific control points. Control points must be in sorted order.
  void MakeLinearTangentLeft(wdUInt32 uiCpIdx);
  /// \brief Typically called by ApplyTangentModes() for specific control points. Control points must be in sorted order.
  void MakeLinearTangentRight(wdUInt32 uiCpIdx);

  void MakeAutoTangentLeft(wdUInt32 uiCpIdx);
  void MakeAutoTangentRight(wdUInt32 uiCpIdx);

private:
  void RecomputeLinearApproxExtremes();
  void ApproximateMinMaxValues(const ControlPoint& lhs, const ControlPoint& rhs, double& fMinY, double& fMaxY);
  void ApproximateCurve(
    const wdVec2d& p0, const wdVec2d& p1, const wdVec2d& p2, const wdVec2d& p3, double fMaxErrorX, double fMaxErrorY, wdInt32 iSubDivLeft);
  void ApproximateCurvePiece(const wdVec2d& p0, const wdVec2d& p1, const wdVec2d& p2, const wdVec2d& p3, double tLeft, const wdVec2d& pLeft,
    double tRight, const wdVec2d& pRight, double fMaxErrorX, double fMaxErrorY, wdInt32 iSubDivLeft);
  wdInt32 FindApproxControlPoint(double x) const;

  double m_fMinX, m_fMaxX;
  double m_fMinY, m_fMaxY;
  wdHybridArray<ControlPoint, 8> m_ControlPoints;
  wdHybridArray<wdVec2d, 24> m_LinearApproximation;
};
