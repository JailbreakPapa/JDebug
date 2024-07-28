#pragma once

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class NS_RENDERERCORE_DLL nsSetBlackboardNumberAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsSetBlackboardNumberAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsSetBlackboardNumberAnimNode

public:
  void SetBlackboardEntry(const char* szEntry); // [ property ]
  const char* GetBlackboardEntry() const;       // [ property ]

  double m_fNumber = 0.0f;                      // [ property ]

private:
  nsAnimGraphTriggerInputPin m_InActivate;      // [ property ]
  nsAnimGraphNumberInputPin m_InNumber;         // [ property ]
  nsHashedString m_sBlackboardEntry;            // [ property ]
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsGetBlackboardNumberAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsGetBlackboardNumberAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsGetBlackboardNumberAnimNode

public:
  void SetBlackboardEntry(const char* szEntry); // [ property ]
  const char* GetBlackboardEntry() const;       // [ property ]

private:
  nsHashedString m_sBlackboardEntry;            // [ property ]
  nsAnimGraphNumberOutputPin m_OutNumber;       // [ property ]
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsCompareBlackboardNumberAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsCompareBlackboardNumberAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
  virtual bool GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsCompareBlackboardNumberAnimNode

public:
  void SetBlackboardEntry(const char* szEntry); // [ property ]
  const char* GetBlackboardEntry() const;       // [ property ]

  double m_fReferenceValue = 0.0;               // [ property ]
  nsEnum<nsComparisonOperator> m_Comparison;    // [ property ]

private:
  nsHashedString m_sBlackboardEntry;            // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnTrue;      // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnFalse;     // [ property ]
  nsAnimGraphBoolOutputPin m_OutIsTrue;         // [ property ]

  struct InstanceData
  {
    nsInt8 m_iIsTrue = -1; // -1 == undefined, 0 == false, 1 == true
  };
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsCheckBlackboardBoolAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsCheckBlackboardBoolAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
  virtual bool GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsCheckBlackboardBoolAnimNode

public:
  void SetBlackboardEntry(const char* szEntry); // [ property ]
  const char* GetBlackboardEntry() const;       // [ property ]

private:
  nsHashedString m_sBlackboardEntry;            // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnTrue;      // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnFalse;     // [ property ]
  nsAnimGraphBoolOutputPin m_OutBool;           // [ property ]

  struct InstanceData
  {
    nsInt8 m_iIsTrue = -1; // -1 == undefined, 0 == false, 1 == true
  };
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsSetBlackboardBoolAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsSetBlackboardBoolAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsSetBlackboardBoolAnimNode

public:
  void SetBlackboardEntry(const char* szEntry); // [ property ]
  const char* GetBlackboardEntry() const;       // [ property ]

  bool m_bBool = false;                         // [ property ]

private:
  nsHashedString m_sBlackboardEntry;            // [ property ]
  nsAnimGraphTriggerInputPin m_InActivate;      // [ property ]
  nsAnimGraphBoolInputPin m_InBool;             // [ property ]
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsGetBlackboardBoolAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsGetBlackboardBoolAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsGetBlackboardBoolAnimNode

public:
  void SetBlackboardEntry(const char* szEntry); // [ property ]
  const char* GetBlackboardEntry() const;       // [ property ]

private:
  nsHashedString m_sBlackboardEntry;            // [ property ]
  nsAnimGraphBoolOutputPin m_OutBool;           // [ property ]
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsOnBlackboardValueChangedAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsOnBlackboardValueChangedAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
  virtual bool GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsOnBlackboardValuechangedAnimNode

public:
  void SetBlackboardEntry(const char* szEntry);    // [ property ]
  const char* GetBlackboardEntry() const;          // [ property ]

private:
  nsHashedString m_sBlackboardEntry;               // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnValueChanged; // [ property ]

  struct InstanceData
  {
    nsUInt32 m_uiChangeCounter = nsInvalidIndex;
  };
};
