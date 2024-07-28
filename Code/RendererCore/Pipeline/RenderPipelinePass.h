#pragma once

#include <Foundation/Containers/HashTable.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/UniquePtr.h>
#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Pipeline/RenderDataBatch.h>
#include <RendererCore/Pipeline/RenderPipelineNode.h>

struct nsGALTextureCreationDescription;
class nsStreamWriter;

/// \brief Passed to nsRenderPipelinePass::InitRenderPipelinePass to inform about
/// existing connections on each input / output pin index.
struct nsRenderPipelinePassConnection
{
  nsRenderPipelinePassConnection() { m_pOutput = nullptr; }

  nsGALTextureCreationDescription m_Desc;
  nsGALTextureHandle m_TextureHandle;
  const nsRenderPipelineNodePin* m_pOutput;                  ///< The output pin that this connection spawns from.
  nsHybridArray<const nsRenderPipelineNodePin*, 4> m_Inputs; ///< The various input pins this connection is connected to.
};

class NS_RENDERERCORE_DLL nsRenderPipelinePass : public nsRenderPipelineNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsRenderPipelinePass, nsRenderPipelineNode);
  NS_DISALLOW_COPY_AND_ASSIGN(nsRenderPipelinePass);

public:
  nsRenderPipelinePass(const char* szName, bool bIsStereoAware = false);
  ~nsRenderPipelinePass();

  /// \brief Sets the name of the pass.
  void SetName(const char* szName);

  /// \brief returns the name of the pass.
  const char* GetName() const;

  /// \brief True if the render pipeline pass can handle stereo cameras correctly.
  bool IsStereoAware() const { return m_bIsStereoAware; }

  /// \brief For a given input pin configuration, provide the output configuration of this node.
  /// Outputs is already resized to the number of output pins.
  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) = 0;

  /// \brief After GetRenderTargetDescriptions was called successfully for each pass, this function is called
  /// with the inputs and outputs for review. Disconnected pins have a nullptr value in the passed in arrays.
  /// This is the time to create additional resources that are not covered by the pins automatically, e.g. a picking texture or eye
  /// adaptation buffer.
  virtual void InitRenderPipelinePass(const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs);

  /// \brief Render into outputs. Both inputs and outputs are passed in with actual texture handles.
  /// Disconnected pins have a nullptr value in the passed in arrays. You can now create views and render target setups on the fly and
  /// fill the output targets with data.
  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) = 0;

  virtual void ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs);

  /// \brief Allows for the pass to write data back using nsView::SetRenderPassReadBackProperty. E.g. picking results etc.
  virtual void ReadBackProperties(nsView* pView);

  virtual nsResult Serialize(nsStreamWriter& inout_stream) const;
  virtual nsResult Deserialize(nsStreamReader& inout_stream);

  void RenderDataWithCategory(const nsRenderViewContext& renderViewContext, nsRenderData::Category category, nsRenderDataBatch::Filter filter = nsRenderDataBatch::Filter());

  NS_ALWAYS_INLINE nsRenderPipeline* GetPipeline() { return m_pPipeline; }
  NS_ALWAYS_INLINE const nsRenderPipeline* GetPipeline() const { return m_pPipeline; }

private:
  friend class nsRenderPipeline;

  bool m_bActive = true;

  const bool m_bIsStereoAware;
  nsHashedString m_sName;

  nsRenderPipeline* m_pPipeline = nullptr;
};
