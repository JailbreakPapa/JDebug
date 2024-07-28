#pragma once

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

// class NS_RENDERERCORE_DLL nsLocalToModelPoseAnimNode : public nsAnimGraphNode
//{
//   NS_ADD_DYNAMIC_REFLECTION(nsLocalToModelPoseAnimNode, nsAnimGraphNode);
//
//   //////////////////////////////////////////////////////////////////////////
//   // nsAnimGraphNode
//
// protected:
//   virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
//   virtual nsResult DeserializeNode(nsStreamReader& stream) override;
//
//   virtual void Step(nsAnimGraphExecutor& executor, nsAnimGraphInstance& graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
//
//   //////////////////////////////////////////////////////////////////////////
//   // nsLocalToModelPoseAnimNode
//
// public:
//   nsLocalToModelPoseAnimNode();
//   ~nsLocalToModelPoseAnimNode();
//
// private:
//   nsAnimGraphLocalPoseInputPin m_LocalPosePin;  // [ property ]
//   nsAnimGraphModelPoseOutputPin m_ModelPosePin; // [ property ]
// };
