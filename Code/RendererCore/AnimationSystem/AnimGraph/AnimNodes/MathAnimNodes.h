#pragma once

#include <Foundation/CodeUtils/MathExpression.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class NS_RENDERERCORE_DLL nsMathExpressionAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsMathExpressionAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
  virtual bool GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsLogicAndAnimNode

public:
  nsMathExpressionAnimNode();
  ~nsMathExpressionAnimNode();

  void SetExpression(nsString sExpr);
  nsString GetExpression() const;

private:
  nsAnimGraphNumberInputPin m_ValueAPin;  // [ property ]
  nsAnimGraphNumberInputPin m_ValueBPin;  // [ property ]
  nsAnimGraphNumberInputPin m_ValueCPin;  // [ property ]
  nsAnimGraphNumberInputPin m_ValueDPin;  // [ property ]
  nsAnimGraphNumberOutputPin m_ResultPin; // [ property ]

  nsString m_sExpression;

  struct InstanceData
  {
    nsMathExpression m_mExpression;
  };
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsCompareNumberAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsCompareNumberAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsCompareNumberAnimNode

public:
  double m_fReferenceValue = 0.0f;           // [ property ]
  nsEnum<nsComparisonOperator> m_Comparison; // [ property ]

private:
  nsAnimGraphNumberInputPin m_InNumber;      // [ property ]
  nsAnimGraphNumberInputPin m_InReference;   // [ property ]
  nsAnimGraphBoolOutputPin m_OutIsTrue;      // [ property ]
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsBoolToNumberAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsBoolToNumberAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsBoolToNumberAnimNode

public:
  nsBoolToNumberAnimNode();
  ~nsBoolToNumberAnimNode();

  double m_fFalseValue = 0.0f;
  double m_fTrueValue = 1.0f;

private:
  nsAnimGraphBoolInputPin m_InValue;      // [ property ]
  nsAnimGraphNumberOutputPin m_OutNumber; // [ property ]
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsBoolToTriggerAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsBoolToTriggerAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
  virtual bool GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsBoolToNumberAnimNode

public:
  nsBoolToTriggerAnimNode();
  ~nsBoolToTriggerAnimNode();

private:
  nsAnimGraphBoolInputPin m_InValue;        // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnTrue;  // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnFalse; // [ property ]

  struct InstanceData
  {
    nsInt8 m_iIsTrue = -1; // -1 == undefined, 0 == false, 1 == true
  };
};
