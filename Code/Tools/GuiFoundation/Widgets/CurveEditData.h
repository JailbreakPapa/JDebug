#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Math/Vec2.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Tracks/Curve1D.h>
#include <GuiFoundation/GuiFoundationDLL.h>

class wdCurve1D;

WD_DECLARE_REFLECTABLE_TYPE(WD_GUIFOUNDATION_DLL, wdCurveTangentMode);

template <typename T>
void FindNearestControlPoints(wdArrayPtr<T> cps, wdInt64 iTick, T*& ref_pLlhs, T*& lhs, T*& rhs, T*& ref_pRrhs)
{
  ref_pLlhs = nullptr;
  lhs = nullptr;
  rhs = nullptr;
  ref_pRrhs = nullptr;
  wdInt64 lhsTick = wdMath::MinValue<wdInt64>();
  wdInt64 llhsTick = wdMath::MinValue<wdInt64>();
  wdInt64 rhsTick = wdMath::MaxValue<wdInt64>();
  wdInt64 rrhsTick = wdMath::MaxValue<wdInt64>();

  for (decltype(auto) cp : cps)
  {
    if (cp.m_iTick <= iTick)
    {
      if (cp.m_iTick > lhsTick)
      {
        ref_pLlhs = lhs;
        llhsTick = lhsTick;

        lhs = &cp;
        lhsTick = cp.m_iTick;
      }
      else if (cp.m_iTick > llhsTick)
      {
        ref_pLlhs = &cp;
        llhsTick = cp.m_iTick;
      }
    }

    if (cp.m_iTick > iTick)
    {
      if (cp.m_iTick < rhsTick)
      {
        ref_pRrhs = rhs;
        rrhsTick = rhsTick;

        rhs = &cp;
        rhsTick = cp.m_iTick;
      }
      else if (cp.m_iTick < rrhsTick)
      {
        ref_pRrhs = &cp;
        rrhsTick = cp.m_iTick;
      }
    }
  }
}

class WD_GUIFOUNDATION_DLL wdCurveControlPointData : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdCurveControlPointData, wdReflectedClass);

public:
  wdTime GetTickAsTime() const { return wdTime::Seconds(m_iTick / 4800.0); }
  void SetTickFromTime(wdTime time, wdInt64 iFps);

  wdInt64 m_iTick; // 4800 ticks per second
  double m_fValue;
  wdVec2 m_LeftTangent = wdVec2(-0.1f, 0.0f);
  wdVec2 m_RightTangent = wdVec2(+0.1f, 0.0f);
  bool m_bTangentsLinked = true;
  wdEnum<wdCurveTangentMode> m_LeftTangentMode;
  wdEnum<wdCurveTangentMode> m_RightTangentMode;
};

class WD_GUIFOUNDATION_DLL wdSingleCurveData : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdSingleCurveData, wdReflectedClass);

public:
  wdColorGammaUB m_CurveColor;
  wdDynamicArray<wdCurveControlPointData> m_ControlPoints;

  void ConvertToRuntimeData(wdCurve1D& out_result) const;
  double Evaluate(wdInt64 iTick) const;
};

class WD_GUIFOUNDATION_DLL wdCurveExtentsAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdCurveExtentsAttribute, wdPropertyAttribute);

public:
  wdCurveExtentsAttribute() = default;
  wdCurveExtentsAttribute(double fLowerExtent, bool bLowerExtentFixed, double fUpperExtent, bool bUpperExtentFixed);

  double m_fLowerExtent = 0.0;
  double m_fUpperExtent = 1.0;
  bool m_bLowerExtentFixed = false;
  bool m_bUpperExtentFixed = false;
};


class WD_GUIFOUNDATION_DLL wdCurveGroupData : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdCurveGroupData, wdReflectedClass);

public:
  wdCurveGroupData() = default;
  wdCurveGroupData(const wdCurveGroupData& rhs) = delete;
  ~wdCurveGroupData();
  wdCurveGroupData& operator=(const wdCurveGroupData& rhs) = delete;

  /// \brief Makes a deep copy of rhs.
  void CloneFrom(const wdCurveGroupData& rhs);

  /// \brief Clears the curve and deallocates the curve data, if it is owned (e.g. if it was created through CloneFrom())
  void Clear();

  /// Can be set to false for cases where the instance is only supposed to act like a container for passing curve pointers around
  bool m_bOwnsData = true;
  wdDynamicArray<wdSingleCurveData*> m_Curves;
  wdUInt16 m_uiFramesPerSecond = 60;

  wdInt64 TickFromTime(wdTime time) const;

  void ConvertToRuntimeData(wdUInt32 uiCurveIdx, wdCurve1D& out_result) const;
};

struct WD_GUIFOUNDATION_DLL wdSelectedCurveCP
{
  WD_DECLARE_POD_TYPE();

  wdUInt16 m_uiCurve;
  wdUInt16 m_uiPoint;
};
