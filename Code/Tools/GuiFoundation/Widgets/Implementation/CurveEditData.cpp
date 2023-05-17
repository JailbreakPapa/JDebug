#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Math/Math.h>
#include <Foundation/Tracks/Curve1D.h>
#include <GuiFoundation/Widgets/CurveEditData.h>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_ENUM(wdCurveTangentMode, 1)
WD_ENUM_CONSTANTS(wdCurveTangentMode::Bwdier, wdCurveTangentMode::FixedLength, wdCurveTangentMode::Linear, wdCurveTangentMode::Auto)
WD_END_STATIC_REFLECTED_ENUM;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdCurveControlPointData, 5, wdRTTIDefaultAllocator<wdCurveControlPointData>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Tick", m_iTick),
    WD_MEMBER_PROPERTY("Value", m_fValue),
    WD_MEMBER_PROPERTY("LeftTangent", m_LeftTangent)->AddAttributes(new wdDefaultValueAttribute(wdVec2(-0.1f, 0))),
    WD_MEMBER_PROPERTY("RightTangent", m_RightTangent)->AddAttributes(new wdDefaultValueAttribute(wdVec2(+0.1f, 0))),
    WD_MEMBER_PROPERTY("Linked", m_bTangentsLinked)->AddAttributes(new wdDefaultValueAttribute(true)),
    WD_ENUM_MEMBER_PROPERTY("LeftTangentMode", wdCurveTangentMode, m_LeftTangentMode),
    WD_ENUM_MEMBER_PROPERTY("RightTangentMode", wdCurveTangentMode, m_RightTangentMode),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdSingleCurveData, 3, wdRTTIDefaultAllocator<wdSingleCurveData>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Color", m_CurveColor)->AddAttributes(new wdDefaultValueAttribute(wdColorScheme::LightUI(wdColorScheme::Lime))),
    WD_ARRAY_MEMBER_PROPERTY("ControlPoints", m_ControlPoints),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdCurveGroupData, 2, wdRTTIDefaultAllocator<wdCurveGroupData>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("FPS", m_uiFramesPerSecond)->AddAttributes(new wdDefaultValueAttribute(60)),
    WD_ARRAY_MEMBER_PROPERTY("Curves", m_Curves)->AddFlags(wdPropertyFlags::PointerOwner),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdCurveExtentsAttribute, 1, wdRTTIDefaultAllocator<wdCurveExtentsAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("LowerExtent", m_fLowerExtent),
    WD_MEMBER_PROPERTY("UpperExtent", m_fUpperExtent),
    WD_MEMBER_PROPERTY("LowerExtentFixed", m_bLowerExtentFixed),
    WD_MEMBER_PROPERTY("UpperExtentFixed", m_bUpperExtentFixed),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(float, bool, float, bool),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdCurveExtentsAttribute::wdCurveExtentsAttribute(double fLowerExtent, bool bLowerExtentFixed, double fUpperExtent, bool bUpperExtentFixed)
{
  m_fLowerExtent = fLowerExtent;
  m_fUpperExtent = fUpperExtent;
  m_bLowerExtentFixed = bLowerExtentFixed;
  m_bUpperExtentFixed = bUpperExtentFixed;
}

void wdCurveControlPointData::SetTickFromTime(wdTime time, wdInt64 iFps)
{
  const wdUInt32 uiTicksPerStep = 4800 / iFps;
  m_iTick = (wdInt64)wdMath::RoundToMultiple(time.GetSeconds() * 4800.0, (double)uiTicksPerStep);
}

wdCurveGroupData::~wdCurveGroupData()
{
  Clear();
}

void wdCurveGroupData::CloneFrom(const wdCurveGroupData& rhs)
{
  Clear();

  m_bOwnsData = true;
  m_uiFramesPerSecond = rhs.m_uiFramesPerSecond;
  m_Curves.SetCount(rhs.m_Curves.GetCount());

  for (wdUInt32 i = 0; i < m_Curves.GetCount(); ++i)
  {
    m_Curves[i] = WD_DEFAULT_NEW(wdSingleCurveData);
    *m_Curves[i] = *(rhs.m_Curves[i]);
  }
}

void wdCurveGroupData::Clear()
{
  m_uiFramesPerSecond = 60;

  if (m_bOwnsData)
  {
    m_bOwnsData = false;

    for (wdUInt32 i = 0; i < m_Curves.GetCount(); ++i)
    {
      WD_DEFAULT_DELETE(m_Curves[i]);
    }
  }

  m_Curves.Clear();
}

wdInt64 wdCurveGroupData::TickFromTime(wdTime time) const
{
  const wdUInt32 uiTicksPerStep = 4800 / m_uiFramesPerSecond;
  return (wdInt64)wdMath::RoundToMultiple(time.GetSeconds() * 4800.0, (double)uiTicksPerStep);
}

static void ConvertControlPoint(const wdCurveControlPointData& cp, wdCurve1D& out_result)
{
  auto& ccp = out_result.AddControlPoint(cp.GetTickAsTime().GetSeconds());
  ccp.m_Position.y = cp.m_fValue;
  ccp.m_LeftTangent = cp.m_LeftTangent;
  ccp.m_RightTangent = cp.m_RightTangent;
  ccp.m_TangentModeLeft = cp.m_LeftTangentMode;
  ccp.m_TangentModeRight = cp.m_RightTangentMode;
}

void wdSingleCurveData::ConvertToRuntimeData(wdCurve1D& out_result) const
{
  out_result.Clear();

  for (const auto& cp : m_ControlPoints)
  {
    ConvertControlPoint(cp, out_result);
  }
}

double wdSingleCurveData::Evaluate(wdInt64 iTick) const
{
  wdCurve1D temp;
  const wdCurveControlPointData* llhs = nullptr;
  const wdCurveControlPointData* lhs = nullptr;
  const wdCurveControlPointData* rhs = nullptr;
  const wdCurveControlPointData* rrhs = nullptr;
  FindNearestControlPoints(m_ControlPoints.GetArrayPtr(), iTick, llhs, lhs, rhs, rrhs);

  if (llhs)
    ConvertControlPoint(*llhs, temp);
  if (lhs)
    ConvertControlPoint(*lhs, temp);
  if (rhs)
    ConvertControlPoint(*rhs, temp);
  if (rrhs)
    ConvertControlPoint(*rrhs, temp);

  //#TODO: This is rather slow as we eval lots of points but only need one
  temp.CreateLinearApproximation();
  return temp.Evaluate(iTick / 4800.0);
}

void wdCurveGroupData::ConvertToRuntimeData(wdUInt32 uiCurveIdx, wdCurve1D& out_result) const
{
  m_Curves[uiCurveIdx]->ConvertToRuntimeData(out_result);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/GraphPatch.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>

class wdCurve1DControlPoint_2_3 : public wdGraphPatch
{
public:
  wdCurve1DControlPoint_2_3()
    : wdGraphPatch("wdCurve1DControlPoint", 3)
  {
  }

  virtual void Patch(wdGraphPatchContext& ref_context, wdAbstractObjectGraph* pGraph, wdAbstractObjectNode* pNode) const override
  {
    auto* pPoint = pNode->FindProperty("Point");
    if (pPoint && pPoint->m_Value.IsA<wdVec2>())
    {
      wdVec2 pt = pPoint->m_Value.Get<wdVec2>();
      pNode->AddProperty("Time", (double)wdMath::Max(0.0f, pt.x));
      pNode->AddProperty("Value", (double)pt.y);
      pNode->AddProperty("LeftTangentMode", (wdUInt32)wdCurveTangentMode::Bwdier);
      pNode->AddProperty("RightTangentMode", (wdUInt32)wdCurveTangentMode::Bwdier);
    }
  }
};

wdCurve1DControlPoint_2_3 g_wdCurve1DControlPoint_2_3;

//////////////////////////////////////////////////////////////////////////

class wdCurve1DControlPoint_3_4 : public wdGraphPatch
{
public:
  wdCurve1DControlPoint_3_4()
    : wdGraphPatch("wdCurve1DControlPoint", 4)
  {
  }

  virtual void Patch(wdGraphPatchContext& ref_context, wdAbstractObjectGraph* pGraph, wdAbstractObjectNode* pNode) const override
  {
    auto* pPoint = pNode->FindProperty("Time");
    if (pPoint && pPoint->m_Value.IsA<double>())
    {
      const double fTime = pPoint->m_Value.Get<double>();
      pNode->AddProperty("Tick", (wdInt64)wdMath::RoundToMultiple(fTime * 4800.0, 4800.0 / 60.0));
    }
  }
};

wdCurve1DControlPoint_3_4 g_wdCurve1DControlPoint_3_4;

//////////////////////////////////////////////////////////////////////////

class wdCurve1DControlPoint_4_5 : public wdGraphPatch
{
public:
  wdCurve1DControlPoint_4_5()
    : wdGraphPatch("wdCurve1DControlPoint", 5)
  {
  }

  virtual void Patch(wdGraphPatchContext& ref_context, wdAbstractObjectGraph* pGraph, wdAbstractObjectNode* pNode) const override
  {
    ref_context.RenameClass("wdCurveControlPointData");
  }
};

wdCurve1DControlPoint_4_5 g_wdCurve1DControlPoint_4_5;

//////////////////////////////////////////////////////////////////////////

class wdCurve1DData_2_3 : public wdGraphPatch
{
public:
  wdCurve1DData_2_3()
    : wdGraphPatch("wdCurve1DData", 3)
  {
  }

  virtual void Patch(wdGraphPatchContext& ref_context, wdAbstractObjectGraph* pGraph, wdAbstractObjectNode* pNode) const override
  {
    ref_context.RenameClass("wdSingleCurveData");
  }
};

wdCurve1DData_2_3 g_wdCurve1DData_2_3;

//////////////////////////////////////////////////////////////////////////

class wdCurve1DAssetData_1_2 : public wdGraphPatch
{
public:
  wdCurve1DAssetData_1_2()
    : wdGraphPatch("wdCurve1DAssetData", 2)
  {
  }

  virtual void Patch(wdGraphPatchContext& ref_context, wdAbstractObjectGraph* pGraph, wdAbstractObjectNode* pNode) const override
  {
    ref_context.RenameClass("wdCurveGroupData");
  }
};

wdCurve1DAssetData_1_2 g_wdCurve1DAssetData_1_2;
