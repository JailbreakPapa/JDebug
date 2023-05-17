#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Math/Declarations.h>

class wdStreamWriter;
class wdStreamReader;

/// \brief A color curve for animating colors.
///
/// The gradient consists of a number of control points, for rgb, alpha and intensity.
/// One can evaluate the curve at any x coordinate.
class WD_FOUNDATION_DLL wdColorGradient
{
public:
  /// \brief Color control point. Stores red, green and blue in gamma space.
  struct ColorCP
  {
    WD_DECLARE_POD_TYPE();

    double m_PosX;
    wdUInt8 m_GammaRed;
    wdUInt8 m_GammaGreen;
    wdUInt8 m_GammaBlue;
    float m_fInvDistToNextCp; /// Internal: Optimization for Evaluate to not recalculate 1/distance to the next control point

    WD_ALWAYS_INLINE bool operator<(const ColorCP& rhs) const { return m_PosX < rhs.m_PosX; }
  };

  /// \brief Alpha control point.
  struct AlphaCP
  {
    WD_DECLARE_POD_TYPE();

    double m_PosX;
    wdUInt8 m_Alpha;
    float m_fInvDistToNextCp; /// Internal: Optimization for Evaluate to not recalculate 1/distance to the next control point

    WD_ALWAYS_INLINE bool operator<(const AlphaCP& rhs) const { return m_PosX < rhs.m_PosX; }
  };

  /// \brief Intensity control point. Used to scale rgb for high-dynamic range values.
  struct IntensityCP
  {
    WD_DECLARE_POD_TYPE();

    double m_PosX;
    float m_Intensity;
    float m_fInvDistToNextCp; /// Internal: Optimization for Evaluate to not recalculate 1/distance to the next control point

    WD_ALWAYS_INLINE bool operator<(const IntensityCP& rhs) const { return m_PosX < rhs.m_PosX; }
  };

public:
  wdColorGradient();

  /// \brief Removes all control points.
  void Clear();

  /// \brief Checks whether the curve has any control point.
  bool IsEmpty() const;

  /// \brief Appends a color control point. SortControlPoints() must be called to before evaluating the curve.
  void AddColorControlPoint(double x, const wdColorGammaUB& rgb);

  /// \brief Appends an alpha control point. SortControlPoints() must be called to before evaluating the curve.
  void AddAlphaControlPoint(double x, wdUInt8 uiAlpha);

  /// \brief Appends an intensity control point. SortControlPoints() must be called to before evaluating the curve.
  void AddIntensityControlPoint(double x, float fIntensity);

  /// \brief Determines the min and max x-coordinate value across all control points.
  bool GetExtents(double& ref_fMinx, double& ref_fMaxx) const;

  /// \brief Returns the number of control points of each type.
  void GetNumControlPoints(wdUInt32& ref_uiRgb, wdUInt32& ref_uiAlpha, wdUInt32& ref_uiIntensity) const;

  /// \brief Const access to a control point.
  const ColorCP& GetColorControlPoint(wdUInt32 uiIdx) const { return m_ColorCPs[uiIdx]; }
  /// \brief Const access to a control point.
  const AlphaCP& GetAlphaControlPoint(wdUInt32 uiIdx) const { return m_AlphaCPs[uiIdx]; }
  /// \brief Const access to a control point.
  const IntensityCP& GetIntensityControlPoint(wdUInt32 uiIdx) const { return m_IntensityCPs[uiIdx]; }

  /// \brief Non-const access to a control point. If you modify the x coordinate, SortControlPoints() has to be called before evaluating the
  /// curve.
  ColorCP& ModifyColorControlPoint(wdUInt32 uiIdx) { return m_ColorCPs[uiIdx]; }
  /// \brief Non-const access to a control point. If you modify the x coordinate, SortControlPoints() has to be called before evaluating the
  /// curve.
  AlphaCP& ModifyAlphaControlPoint(wdUInt32 uiIdx) { return m_AlphaCPs[uiIdx]; }
  /// \brief Non-const access to a control point. If you modify the x coordinate, SortControlPoints() has to be called before evaluating the
  /// curve.
  IntensityCP& ModifyIntensityControlPoint(wdUInt32 uiIdx) { return m_IntensityCPs[uiIdx]; }

  /// \brief Sorts the control point arrays by their x-coordinate. The CPs have to be sorted before calling Evaluate(), otherwise the result
  /// will be wrong.
  void SortControlPoints();

  /// \brief Evaluates the curve at the given x-coordinate and returns RGBA and intensity separately.
  ///
  /// The control points have to be sorted, so call SortControlPoints() before, if any modifications where done.
  void Evaluate(double x, wdColorGammaUB& ref_rgba, float& ref_fIntensity) const;

  /// \brief Evaluates the curve and returns RGBA and intensity in one combined wdColor value.
  void Evaluate(double x, wdColor& ref_hdr) const;

  /// \brief Evaluates only the color curve.
  void EvaluateColor(double x, wdColorGammaUB& ref_rgb) const;
  /// \brief Evaluates only the color curve.
  void EvaluateColor(double x, wdColor& ref_rgb) const;
  /// \brief Evaluates only the alpha curve.
  void EvaluateAlpha(double x, wdUInt8& ref_uiAlpha) const;
  /// \brief Evaluates only the intensity curve.
  void EvaluateIntensity(double x, float& ref_fIntensity) const;

  /// \brief How much heap memory the curve uses.
  wdUInt64 GetHeapMemoryUsage() const;

  /// \brief Stores the current state in a stream.
  void Save(wdStreamWriter& inout_stream) const;

  /// \brief Restores the state from a stream.
  void Load(wdStreamReader& inout_stream);

private:
  void PrecomputeLerpNormalizer();

  wdHybridArray<ColorCP, 8> m_ColorCPs;
  wdHybridArray<AlphaCP, 8> m_AlphaCPs;
  wdHybridArray<IntensityCP, 8> m_IntensityCPs;
};
