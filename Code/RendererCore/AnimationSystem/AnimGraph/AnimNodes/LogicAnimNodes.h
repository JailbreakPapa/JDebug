#pragma once

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class NS_RENDERERCORE_DLL nsLogicAndAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsLogicAndAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsLogicAndAnimNode

public:
  nsLogicAndAnimNode();
  ~nsLogicAndAnimNode();

private:
  nsUInt8 m_uiBoolCount = 2;                          // [ property ]
  nsHybridArray<nsAnimGraphBoolInputPin, 2> m_InBool; // [ property ]
  nsAnimGraphBoolOutputPin m_OutIsTrue;               // [ property ]
  nsAnimGraphBoolOutputPin m_OutIsFalse;              // [ property ]
};

class NS_RENDERERCORE_DLL nsLogicEventAndAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsLogicEventAndAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsLogicEventAndAnimNode

public:
  nsLogicEventAndAnimNode();
  ~nsLogicEventAndAnimNode();

private:
  nsAnimGraphTriggerInputPin m_InActivate;      // [ property ]
  nsAnimGraphBoolInputPin m_InBool;             // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnActivated; // [ property ]
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsLogicOrAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsLogicOrAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsLogicOrAnimNode

public:
  nsLogicOrAnimNode();
  ~nsLogicOrAnimNode();

private:
  nsUInt8 m_uiBoolCount = 2;                          // [ property ]
  nsHybridArray<nsAnimGraphBoolInputPin, 2> m_InBool; // [ property ]
  nsAnimGraphBoolOutputPin m_OutIsTrue;               // [ property ]
  nsAnimGraphBoolOutputPin m_OutIsFalse;              // [ property ]
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsLogicNotAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsLogicNotAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsLogicNotAnimNode

public:
  nsLogicNotAnimNode();
  ~nsLogicNotAnimNode();

private:
  nsAnimGraphBoolInputPin m_InBool;   // [ property ]
  nsAnimGraphBoolOutputPin m_OutBool; // [ property ]
};
