#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Math/Color.h>
#include <Foundation/Math/Vec2.h>
#include <Foundation/Types/Enum.h>

class nsStreamWriter;
class nsStreamReader;

struct NS_FOUNDATION_DLL nsCurveTangentMode
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Bnsier,
    FixedLength,
    Linear,
    // Constant,
    Auto,
    Default = Auto
  };
};

/// \brief A 1D curve for animating a single value over time.
class NS_FOUNDATION_DLL nsCurve1D
{
public:
  /// \brief Stores position and tangents to control spline interpolation
  struct ControlPoint
  {
    NS_DECLARE_POD_TYPE();

    ControlPoint();

    /// \brief The position (x,y) of the control point
    nsVec2d m_Position;

    /// \brief The tangent for the curve segment to the left that affects the spline interpolation
    nsVec2 m_LeftTangent;
    /// \brief The tangent for the curve segment to the right that affects the spline interpolation
    nsVec2 m_RightTangent;

    nsEnum<nsCurveTangentMode> m_TangentModeLeft;
    nsEnum<nsCurveTangentMode> m_TangentModeRight;

    nsUInt16 m_uiOriginalIndex;

    NS_ALWAYS_INLINE bool operator<(const ControlPoint& rhs) const { return m_Position.x < rhs.m_Position.x; }
  };

public:
  nsCurve1D();

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
  nsUInt32 GetNumControlPoints() const;

  /// \brief Const access to a control point.
  const ControlPoint& GetControlPoint(nsUInt32 uiIdx) const { return m_ControlPoints[uiIdx]; }

  /// \brief Non-const access to a control point. If you modify the position, SortControlPoints() has to be called before evaluating the
  /// curve.
  ControlPoint& ModifyControlPoint(nsUInt32 uiIdx) { return m_ControlPoints[uiIdx]; }

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
  nsUInt64 GetHeapMemoryUsage() const;

  /// \brief Stores the current state in a stream.
  void Save(nsStreamWriter& inout_stream) const;

  /// \brief Restores the state from a stream.
  void Load(nsStreamReader& inout_stream);

  /// \brief Pre-computes sample points for linear interpolation that approximate the curve within the allowed error threshold.
  ///
  /// \note All control points must already be in sorted order, so call SortControlPoints() first if necessary.
  void CreateLinearApproximation(double fMaxError = 0.01, nsUInt8 uiMaxSubDivs = 8);

  const nsHybridArray<nsVec2d, 24>& GetLinearApproximation() const { return m_LinearApproximation; }

  /// \brief Adjusts the tangents such that the curve cannot make loopings
  void ClampTangents();

  /// \brief Adjusts the tangents in accordance to the specified tangent modes at each control point
  ///
  /// \note All control points must already be in sorted order, so call SortControlPoints() first if necessary.
  void ApplyTangentModes();

  /// \brief Typically called by ApplyTangentModes() for specific control points. Control points must be in sorted order.
  void MakeFixedLengthTangentLeft(nsUInt32 uiCpIdx);
  /// \brief Typically called by ApplyTangentModes() for specific control points. Control points must be in sorted order.
  void MakeFixedLengthTangentRight(nsUInt32 uiCpIdx);
  /// \brief Typically called by ApplyTangentModes() for specific control points. Control points must be in sorted order.
  void MakeLinearTangentLeft(nsUInt32 uiCpIdx);
  /// \brief Typically called by ApplyTangentModes() for specific control points. Control points must be in sorted order.
  void MakeLinearTangentRight(nsUInt32 uiCpIdx);

  void MakeAutoTangentLeft(nsUInt32 uiCpIdx);
  void MakeAutoTangentRight(nsUInt32 uiCpIdx);

private:
  void RecomputeLinearApproxExtremes();
  void ApproximateMinMaxValues(const ControlPoint& lhs, const ControlPoint& rhs, double& fMinY, double& fMaxY);
  void ApproximateCurve(
    const nsVec2d& p0, const nsVec2d& p1, const nsVec2d& p2, const nsVec2d& p3, double fMaxErrorX, double fMaxErrorY, nsInt32 iSubDivLeft);
  void ApproximateCurvePiece(const nsVec2d& p0, const nsVec2d& p1, const nsVec2d& p2, const nsVec2d& p3, double tLeft, const nsVec2d& pLeft,
    double tRight, const nsVec2d& pRight, double fMaxErrorX, double fMaxErrorY, nsInt32 iSubDivLeft);
  nsInt32 FindApproxControlPoint(double x) const;

  double m_fMinX, m_fMaxX;
  double m_fMinY, m_fMaxY;
  nsHybridArray<ControlPoint, 8> m_ControlPoints;
  nsHybridArray<nsVec2d, 24> m_LinearApproximation;
};
