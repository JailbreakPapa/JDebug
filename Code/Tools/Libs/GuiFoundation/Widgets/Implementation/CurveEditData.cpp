#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Math/Math.h>
#include <Foundation/Tracks/Curve1D.h>
#include <GuiFoundation/Widgets/CurveEditData.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsCurveTangentMode, 1)
NS_ENUM_CONSTANTS(nsCurveTangentMode::Bnsier, nsCurveTangentMode::FixedLength, nsCurveTangentMode::Linear, nsCurveTangentMode::Auto)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCurveControlPointData, 5, nsRTTIDefaultAllocator<nsCurveControlPointData>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Tick", m_iTick),
    NS_MEMBER_PROPERTY("Value", m_fValue),
    NS_MEMBER_PROPERTY("LeftTangent", m_LeftTangent)->AddAttributes(new nsDefaultValueAttribute(nsVec2(-0.1f, 0))),
    NS_MEMBER_PROPERTY("RightTangent", m_RightTangent)->AddAttributes(new nsDefaultValueAttribute(nsVec2(+0.1f, 0))),
    NS_MEMBER_PROPERTY("Linked", m_bTangentsLinked)->AddAttributes(new nsDefaultValueAttribute(true)),
    NS_ENUM_MEMBER_PROPERTY("LeftTangentMode", nsCurveTangentMode, m_LeftTangentMode),
    NS_ENUM_MEMBER_PROPERTY("RightTangentMode", nsCurveTangentMode, m_RightTangentMode),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSingleCurveData, 3, nsRTTIDefaultAllocator<nsSingleCurveData>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Color", m_CurveColor)->AddAttributes(new nsDefaultValueAttribute(nsColorScheme::LightUI(nsColorScheme::Lime))),
    NS_ARRAY_MEMBER_PROPERTY("ControlPoints", m_ControlPoints),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCurveGroupData, 2, nsRTTIDefaultAllocator<nsCurveGroupData>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("FPS", m_uiFramesPerSecond)->AddAttributes(new nsDefaultValueAttribute(60)),
    NS_ARRAY_MEMBER_PROPERTY("Curves", m_Curves)->AddFlags(nsPropertyFlags::PointerOwner),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCurveExtentsAttribute, 1, nsRTTIDefaultAllocator<nsCurveExtentsAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("LowerExtent", m_fLowerExtent),
    NS_MEMBER_PROPERTY("UpperExtent", m_fUpperExtent),
    NS_MEMBER_PROPERTY("LowerExtentFixed", m_bLowerExtentFixed),
    NS_MEMBER_PROPERTY("UpperExtentFixed", m_bUpperExtentFixed),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(float, bool, float, bool),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsCurveExtentsAttribute::nsCurveExtentsAttribute(double fLowerExtent, bool bLowerExtentFixed, double fUpperExtent, bool bUpperExtentFixed)
{
  m_fLowerExtent = fLowerExtent;
  m_fUpperExtent = fUpperExtent;
  m_bLowerExtentFixed = bLowerExtentFixed;
  m_bUpperExtentFixed = bUpperExtentFixed;
}

void nsCurveControlPointData::SetTickFromTime(nsTime time, nsInt64 iFps)
{
  const nsUInt32 uiTicksPerStep = 4800 / iFps;
  m_iTick = (nsInt64)nsMath::RoundToMultiple(time.GetSeconds() * 4800.0, (double)uiTicksPerStep);
}

nsCurveGroupData::~nsCurveGroupData()
{
  Clear();
}

void nsCurveGroupData::CloneFrom(const nsCurveGroupData& rhs)
{
  Clear();

  m_bOwnsData = true;
  m_uiFramesPerSecond = rhs.m_uiFramesPerSecond;
  m_Curves.SetCount(rhs.m_Curves.GetCount());

  for (nsUInt32 i = 0; i < m_Curves.GetCount(); ++i)
  {
    m_Curves[i] = NS_DEFAULT_NEW(nsSingleCurveData);
    *m_Curves[i] = *(rhs.m_Curves[i]);
  }
}

void nsCurveGroupData::Clear()
{
  m_uiFramesPerSecond = 60;

  if (m_bOwnsData)
  {
    m_bOwnsData = false;

    for (nsUInt32 i = 0; i < m_Curves.GetCount(); ++i)
    {
      NS_DEFAULT_DELETE(m_Curves[i]);
    }
  }

  m_Curves.Clear();
}

nsInt64 nsCurveGroupData::TickFromTime(nsTime time) const
{
  const nsUInt32 uiTicksPerStep = 4800 / m_uiFramesPerSecond;
  return (nsInt64)nsMath::RoundToMultiple(time.GetSeconds() * 4800.0, (double)uiTicksPerStep);
}

static void ConvertControlPoint(const nsCurveControlPointData& cp, nsCurve1D& out_result)
{
  auto& ccp = out_result.AddControlPoint(cp.GetTickAsTime().GetSeconds());
  ccp.m_Position.y = cp.m_fValue;
  ccp.m_LeftTangent = cp.m_LeftTangent;
  ccp.m_RightTangent = cp.m_RightTangent;
  ccp.m_TangentModeLeft = cp.m_LeftTangentMode;
  ccp.m_TangentModeRight = cp.m_RightTangentMode;
}

void nsSingleCurveData::ConvertToRuntimeData(nsCurve1D& out_result) const
{
  out_result.Clear();

  for (const auto& cp : m_ControlPoints)
  {
    ConvertControlPoint(cp, out_result);
  }
}

double nsSingleCurveData::Evaluate(nsInt64 iTick) const
{
  nsCurve1D temp;
  const nsCurveControlPointData* llhs = nullptr;
  const nsCurveControlPointData* lhs = nullptr;
  const nsCurveControlPointData* rhs = nullptr;
  const nsCurveControlPointData* rrhs = nullptr;
  FindNearestControlPoints(m_ControlPoints.GetArrayPtr(), iTick, llhs, lhs, rhs, rrhs);

  if (llhs)
    ConvertControlPoint(*llhs, temp);
  if (lhs)
    ConvertControlPoint(*lhs, temp);
  if (rhs)
    ConvertControlPoint(*rhs, temp);
  if (rrhs)
    ConvertControlPoint(*rrhs, temp);

  // #TODO: This is rather slow as we eval lots of points but only need one
  temp.CreateLinearApproximation();
  return temp.Evaluate(iTick / 4800.0);
}

void nsCurveGroupData::ConvertToRuntimeData(nsUInt32 uiCurveIdx, nsCurve1D& out_result) const
{
  m_Curves[uiCurveIdx]->ConvertToRuntimeData(out_result);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class nsCurve1DControlPoint_2_3 : public nsGraphPatch
{
public:
  nsCurve1DControlPoint_2_3()
    : nsGraphPatch("nsCurve1DControlPoint", 3)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    auto* pPoint = pNode->FindProperty("Point");
    if (pPoint && pPoint->m_Value.IsA<nsVec2>())
    {
      nsVec2 pt = pPoint->m_Value.Get<nsVec2>();
      pNode->AddProperty("Time", (double)nsMath::Max(0.0f, pt.x));
      pNode->AddProperty("Value", (double)pt.y);
      pNode->AddProperty("LeftTangentMode", (nsUInt32)nsCurveTangentMode::Bnsier);
      pNode->AddProperty("RightTangentMode", (nsUInt32)nsCurveTangentMode::Bnsier);
    }
  }
};

nsCurve1DControlPoint_2_3 g_nsCurve1DControlPoint_2_3;

//////////////////////////////////////////////////////////////////////////

class nsCurve1DControlPoint_3_4 : public nsGraphPatch
{
public:
  nsCurve1DControlPoint_3_4()
    : nsGraphPatch("nsCurve1DControlPoint", 4)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    auto* pPoint = pNode->FindProperty("Time");
    if (pPoint && pPoint->m_Value.IsA<double>())
    {
      const double fTime = pPoint->m_Value.Get<double>();
      pNode->AddProperty("Tick", (nsInt64)nsMath::RoundToMultiple(fTime * 4800.0, 4800.0 / 60.0));
    }
  }
};

nsCurve1DControlPoint_3_4 g_nsCurve1DControlPoint_3_4;

//////////////////////////////////////////////////////////////////////////

class nsCurve1DControlPoint_4_5 : public nsGraphPatch
{
public:
  nsCurve1DControlPoint_4_5()
    : nsGraphPatch("nsCurve1DControlPoint", 5)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    ref_context.RenameClass("nsCurveControlPointData");
  }
};

nsCurve1DControlPoint_4_5 g_nsCurve1DControlPoint_4_5;

//////////////////////////////////////////////////////////////////////////

class nsCurve1DData_2_3 : public nsGraphPatch
{
public:
  nsCurve1DData_2_3()
    : nsGraphPatch("nsCurve1DData", 3)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    ref_context.RenameClass("nsSingleCurveData");
  }
};

nsCurve1DData_2_3 g_nsCurve1DData_2_3;

//////////////////////////////////////////////////////////////////////////

class nsCurve1DAssetData_1_2 : public nsGraphPatch
{
public:
  nsCurve1DAssetData_1_2()
    : nsGraphPatch("nsCurve1DAssetData", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    ref_context.RenameClass("nsCurveGroupData");
  }
};

nsCurve1DAssetData_1_2 g_nsCurve1DAssetData_1_2;
