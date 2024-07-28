#pragma once

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class NS_RENDERERCORE_DLL nsControllerInputAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsControllerInputAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsControllerInputAnimNode

private:
  nsAnimGraphNumberOutputPin m_OutLeftStickX;   // [ property ]
  nsAnimGraphNumberOutputPin m_OutLeftStickY;   // [ property ]
  nsAnimGraphNumberOutputPin m_OutRightStickX;  // [ property ]
  nsAnimGraphNumberOutputPin m_OutRightStickY;  // [ property ]

  nsAnimGraphNumberOutputPin m_OutLeftTrigger;  // [ property ]
  nsAnimGraphNumberOutputPin m_OutRightTrigger; // [ property ]

  nsAnimGraphBoolOutputPin m_OutButtonA;        // [ property ]
  nsAnimGraphBoolOutputPin m_OutButtonB;        // [ property ]
  nsAnimGraphBoolOutputPin m_OutButtonX;        // [ property ]
  nsAnimGraphBoolOutputPin m_OutButtonY;        // [ property ]

  nsAnimGraphBoolOutputPin m_OutLeftShoulder;   // [ property ]
  nsAnimGraphBoolOutputPin m_OutRightShoulder;  // [ property ]

  nsAnimGraphBoolOutputPin m_OutPadLeft;        // [ property ]
  nsAnimGraphBoolOutputPin m_OutPadRight;       // [ property ]
  nsAnimGraphBoolOutputPin m_OutPadUp;          // [ property ]
  nsAnimGraphBoolOutputPin m_OutPadDown;        // [ property ]
};
