#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Math/Vec2.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Tracks/Curve1D.h>
#include <GuiFoundation/GuiFoundationDLL.h>

class nsCurve1D;

NS_DECLARE_REFLECTABLE_TYPE(NS_GUIFOUNDATION_DLL, nsCurveTangentMode);

template <typename T>
void FindNearestControlPoints(nsArrayPtr<T> cps, nsInt64 iTick, T*& ref_pLlhs, T*& lhs, T*& rhs, T*& ref_pRrhs)
{
  ref_pLlhs = nullptr;
  lhs = nullptr;
  rhs = nullptr;
  ref_pRrhs = nullptr;
  nsInt64 lhsTick = nsMath::MinValue<nsInt64>();
  nsInt64 llhsTick = nsMath::MinValue<nsInt64>();
  nsInt64 rhsTick = nsMath::MaxValue<nsInt64>();
  nsInt64 rrhsTick = nsMath::MaxValue<nsInt64>();

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

class NS_GUIFOUNDATION_DLL nsCurveControlPointData : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsCurveControlPointData, nsReflectedClass);

public:
  nsTime GetTickAsTime() const { return nsTime::MakeFromSeconds(m_iTick / 4800.0); }
  void SetTickFromTime(nsTime time, nsInt64 iFps);

  nsInt64 m_iTick; // 4800 ticks per second
  double m_fValue;
  nsVec2 m_LeftTangent = nsVec2(-0.1f, 0.0f);
  nsVec2 m_RightTangent = nsVec2(+0.1f, 0.0f);
  bool m_bTangentsLinked = true;
  nsEnum<nsCurveTangentMode> m_LeftTangentMode;
  nsEnum<nsCurveTangentMode> m_RightTangentMode;
};

class NS_GUIFOUNDATION_DLL nsSingleCurveData : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsSingleCurveData, nsReflectedClass);

public:
  nsColorGammaUB m_CurveColor;
  nsDynamicArray<nsCurveControlPointData> m_ControlPoints;

  void ConvertToRuntimeData(nsCurve1D& out_result) const;
  double Evaluate(nsInt64 iTick) const;
};

class NS_GUIFOUNDATION_DLL nsCurveExtentsAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsCurveExtentsAttribute, nsPropertyAttribute);

public:
  nsCurveExtentsAttribute() = default;
  nsCurveExtentsAttribute(double fLowerExtent, bool bLowerExtentFixed, double fUpperExtent, bool bUpperExtentFixed);

  double m_fLowerExtent = 0.0;
  double m_fUpperExtent = 1.0;
  bool m_bLowerExtentFixed = false;
  bool m_bUpperExtentFixed = false;
};


class NS_GUIFOUNDATION_DLL nsCurveGroupData : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsCurveGroupData, nsReflectedClass);

public:
  nsCurveGroupData() = default;
  nsCurveGroupData(const nsCurveGroupData& rhs) = delete;
  ~nsCurveGroupData();
  nsCurveGroupData& operator=(const nsCurveGroupData& rhs) = delete;

  /// \brief Makes a deep copy of rhs.
  void CloneFrom(const nsCurveGroupData& rhs);

  /// \brief Clears the curve and deallocates the curve data, if it is owned (e.g. if it was created through CloneFrom())
  void Clear();

  /// Can be set to false for cases where the instance is only supposed to act like a container for passing curve pointers around
  bool m_bOwnsData = true;
  nsDynamicArray<nsSingleCurveData*> m_Curves;
  nsUInt16 m_uiFramesPerSecond = 60;

  nsInt64 TickFromTime(nsTime time) const;

  void ConvertToRuntimeData(nsUInt32 uiCurveIdx, nsCurve1D& out_result) const;
};

struct NS_GUIFOUNDATION_DLL nsSelectedCurveCP
{
  NS_DECLARE_POD_TYPE();

  nsUInt16 m_uiCurve;
  nsUInt16 m_uiPoint;
};
