#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Reflection/Reflection.h>

#include <Foundation/Types/RefCounted.h>
#include <ozz/base/maths/soa_transform.h>

class nsAnimGraphInstance;
class nsAnimController;
class nsStreamWriter;
class nsStreamReader;
struct nsAnimGraphPinDataBoneWeights;
struct nsAnimGraphPinDataLocalTransforms;
struct nsAnimGraphPinDataModelTransforms;

struct nsAnimGraphSharedBoneWeights : public nsRefCounted
{
  nsDynamicArray<ozz::math::SimdFloat4, nsAlignedAllocatorWrapper> m_Weights;
};

using nsAnimPoseGeneratorLocalPoseID = nsUInt32;
using nsAnimPoseGeneratorModelPoseID = nsUInt32;
using nsAnimPoseGeneratorCommandID = nsUInt32;

class NS_RENDERERCORE_DLL nsAnimGraphPin : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphPin, nsReflectedClass);

public:
  enum Type : nsUInt8
  {
    Invalid,
    Trigger,
    Number,
    Bool,
    BoneWeights,
    LocalPose,
    ModelPose,
    // EXTEND THIS if a new type is introduced

    ENUM_COUNT
  };

  bool IsConnected() const
  {
    return m_iPinIndex != -1;
  }

  virtual nsAnimGraphPin::Type GetPinType() const = 0;

  nsResult Serialize(nsStreamWriter& inout_stream) const;
  nsResult Deserialize(nsStreamReader& inout_stream);

protected:
  friend class nsAnimGraph;

  nsInt16 m_iPinIndex = -1;
  nsUInt8 m_uiNumConnections = 0;
};

class NS_RENDERERCORE_DLL nsAnimGraphInputPin : public nsAnimGraphPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphInputPin, nsAnimGraphPin);

public:
};

class NS_RENDERERCORE_DLL nsAnimGraphOutputPin : public nsAnimGraphPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphOutputPin, nsAnimGraphPin);

public:
};

//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsAnimGraphTriggerInputPin : public nsAnimGraphInputPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphTriggerInputPin, nsAnimGraphInputPin);

public:
  virtual nsAnimGraphPin::Type GetPinType() const override { return nsAnimGraphPin::Trigger; }

  bool IsTriggered(nsAnimGraphInstance& ref_graph) const;
  bool AreAllTriggered(nsAnimGraphInstance& ref_graph) const;
};

class NS_RENDERERCORE_DLL nsAnimGraphTriggerOutputPin : public nsAnimGraphOutputPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphTriggerOutputPin, nsAnimGraphOutputPin);

public:
  virtual nsAnimGraphPin::Type GetPinType() const override { return nsAnimGraphPin::Trigger; }

  /// \brief Sets this output pin to the triggered state for this frame.
  ///
  /// All pin states are reset before every graph update, so this only needs to be called
  /// when a pin should be set to the triggered state, but then it must be called every frame.
  void SetTriggered(nsAnimGraphInstance& ref_graph) const;
};

//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsAnimGraphNumberInputPin : public nsAnimGraphInputPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphNumberInputPin, nsAnimGraphInputPin);

public:
  virtual nsAnimGraphPin::Type GetPinType() const override { return nsAnimGraphPin::Number; }

  double GetNumber(nsAnimGraphInstance& ref_graph, double fFallback = 0.0) const;
};

class NS_RENDERERCORE_DLL nsAnimGraphNumberOutputPin : public nsAnimGraphOutputPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphNumberOutputPin, nsAnimGraphOutputPin);

public:
  virtual nsAnimGraphPin::Type GetPinType() const override { return nsAnimGraphPin::Number; }

  void SetNumber(nsAnimGraphInstance& ref_graph, double value) const;
};

//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsAnimGraphBoolInputPin : public nsAnimGraphInputPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphBoolInputPin, nsAnimGraphInputPin);

public:
  virtual nsAnimGraphPin::Type GetPinType() const override { return nsAnimGraphPin::Bool; }

  bool GetBool(nsAnimGraphInstance& ref_graph, bool bFallback = false) const;
};

class NS_RENDERERCORE_DLL nsAnimGraphBoolOutputPin : public nsAnimGraphOutputPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphBoolOutputPin, nsAnimGraphOutputPin);

public:
  virtual nsAnimGraphPin::Type GetPinType() const override { return nsAnimGraphPin::Bool; }

  void SetBool(nsAnimGraphInstance& ref_graph, bool bValue) const;
};

//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsAnimGraphBoneWeightsInputPin : public nsAnimGraphInputPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphBoneWeightsInputPin, nsAnimGraphInputPin);

public:
  virtual nsAnimGraphPin::Type GetPinType() const override { return nsAnimGraphPin::BoneWeights; }

  nsAnimGraphPinDataBoneWeights* GetWeights(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph) const;
};

class NS_RENDERERCORE_DLL nsAnimGraphBoneWeightsOutputPin : public nsAnimGraphOutputPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphBoneWeightsOutputPin, nsAnimGraphOutputPin);

public:
  virtual nsAnimGraphPin::Type GetPinType() const override { return nsAnimGraphPin::BoneWeights; }

  void SetWeights(nsAnimGraphInstance& ref_graph, nsAnimGraphPinDataBoneWeights* pWeights) const;
};

//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsAnimGraphLocalPoseInputPin : public nsAnimGraphInputPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphLocalPoseInputPin, nsAnimGraphInputPin);

public:
  virtual nsAnimGraphPin::Type GetPinType() const override { return nsAnimGraphPin::LocalPose; }

  nsAnimGraphPinDataLocalTransforms* GetPose(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph) const;
};

class NS_RENDERERCORE_DLL nsAnimGraphLocalPoseMultiInputPin : public nsAnimGraphInputPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphLocalPoseMultiInputPin, nsAnimGraphInputPin);

public:
  virtual nsAnimGraphPin::Type GetPinType() const override { return nsAnimGraphPin::LocalPose; }

  void GetPoses(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsDynamicArray<nsAnimGraphPinDataLocalTransforms*>& out_poses) const;
};

class NS_RENDERERCORE_DLL nsAnimGraphLocalPoseOutputPin : public nsAnimGraphOutputPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphLocalPoseOutputPin, nsAnimGraphOutputPin);

public:
  virtual nsAnimGraphPin::Type GetPinType() const override { return nsAnimGraphPin::LocalPose; }

  void SetPose(nsAnimGraphInstance& ref_graph, nsAnimGraphPinDataLocalTransforms* pPose) const;
};

//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsAnimGraphModelPoseInputPin : public nsAnimGraphInputPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphModelPoseInputPin, nsAnimGraphInputPin);

public:
  virtual nsAnimGraphPin::Type GetPinType() const override { return nsAnimGraphPin::ModelPose; }

  nsAnimGraphPinDataModelTransforms* GetPose(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph) const;
};

class NS_RENDERERCORE_DLL nsAnimGraphModelPoseOutputPin : public nsAnimGraphOutputPin
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphModelPoseOutputPin, nsAnimGraphOutputPin);

public:
  virtual nsAnimGraphPin::Type GetPinType() const override { return nsAnimGraphPin::ModelPose; }

  void SetPose(nsAnimGraphInstance& ref_graph, nsAnimGraphPinDataModelTransforms* pPose) const;
};
