#pragma once

#include <Foundation/Math/Declarations.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>
#include <RendererCore/Components/RenderComponent.h>
#include <RendererCore/Debug/DebugRenderer.h>

struct nsMsgQueryAnimationSkeleton;

using nsVisualizeSkeletonComponentManager = nsComponentManagerSimple<class nsSkeletonComponent, nsComponentUpdateType::Always, nsBlockStorageType::Compact>;

/// \brief Uses debug rendering to visualize various aspects of an animation skeleton.
///
/// This is meant for visually inspecting skeletons. It is used by the main skeleton editor,
/// but can also be added to a scene or added to an animated mesh on-demand.
///
/// There are different options what to visualize and also to highlight certain bones.
class NS_RENDERERCORE_DLL nsSkeletonComponent : public nsRenderComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsSkeletonComponent, nsRenderComponent, nsVisualizeSkeletonComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  virtual void OnActivated() override;


  //////////////////////////////////////////////////////////////////////////
  // nsRenderComponent

public:
  virtual nsResult GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg) override;

  //////////////////////////////////////////////////////////////////////////
  // nsSkeletonComponent

public:
  nsSkeletonComponent();
  ~nsSkeletonComponent();

  void SetSkeletonFile(const char* szFile); // [ property ]
  const char* GetSkeletonFile() const;      // [ property ]

  void SetSkeleton(const nsSkeletonResourceHandle& hResource);
  const nsSkeletonResourceHandle& GetSkeleton() const { return m_hSkeleton; }

  /// \brief Sets a semicolon-separated list of bone names that should be highlighted.
  ///
  /// Set it to "*" to highlight all bones.
  /// Set it to empty to not highlight any bone.
  /// Set it to "BoneA;BoneB" to highlight the bones with name "BoneA" and "BoneB".
  void SetBonesToHighlight(const char* szFilter); // [ property ]
  const char* GetBonesToHighlight() const;        // [ property ]

  bool m_bVisualizeBones = true;                  // [ property ]
  bool m_bVisualizeColliders = false;             // [ property ]
  bool m_bVisualizeJoints = false;                // [ property ]
  bool m_bVisualizeSwingLimits = false;           // [ property ]
  bool m_bVisualizeTwistLimits = false;           // [ property ]

protected:
  void Update();
  void VisualizeSkeletonDefaultState();
  void OnAnimationPoseUpdated(nsMsgAnimationPoseUpdated& msg); // [ msg handler ]

  void BuildSkeletonVisualization(nsMsgAnimationPoseUpdated& msg);
  void BuildColliderVisualization(nsMsgAnimationPoseUpdated& msg);
  void BuildJointVisualization(nsMsgAnimationPoseUpdated& msg);

  void OnQueryAnimationSkeleton(nsMsgQueryAnimationSkeleton& msg);
  nsDebugRenderer::Line& AddLine(const nsVec3& vStart, const nsVec3& vEnd, const nsColor& color);

  nsSkeletonResourceHandle m_hSkeleton;
  nsTransform m_RootTransform = nsTransform::MakeIdentity();
  nsUInt32 m_uiSkeletonChangeCounter = 0;
  nsString m_sBonesToHighlight;

  nsBoundingBox m_MaxBounds;
  nsDynamicArray<nsDebugRenderer::Line> m_LinesSkeleton;

  struct SphereShape
  {
    nsTransform m_Transform;
    nsBoundingSphere m_Shape;
    nsColor m_Color;
  };

  struct BoxShape
  {
    nsTransform m_Transform;
    nsBoundingBox m_Shape;
    nsColor m_Color;
  };

  struct CapsuleShape
  {
    nsTransform m_Transform;
    float m_fLength;
    float m_fRadius;
    nsColor m_Color;
  };

  struct AngleShape
  {
    nsTransform m_Transform;
    nsColor m_Color;
    nsAngle m_StartAngle;
    nsAngle m_EndAngle;
  };

  struct ConeLimitShape
  {
    nsTransform m_Transform;
    nsColor m_Color;
    nsAngle m_Angle1;
    nsAngle m_Angle2;
  };

  struct CylinderShape
  {
    nsTransform m_Transform;
    nsColor m_Color;
    float m_fRadius1;
    float m_fRadius2;
    float m_fLength;
  };

  nsDynamicArray<SphereShape> m_SpheresShapes;
  nsDynamicArray<BoxShape> m_BoxShapes;
  nsDynamicArray<CapsuleShape> m_CapsuleShapes;
  nsDynamicArray<AngleShape> m_AngleShapes;
  nsDynamicArray<ConeLimitShape> m_ConeLimitShapes;
  nsDynamicArray<CylinderShape> m_CylinderShapes;
};
