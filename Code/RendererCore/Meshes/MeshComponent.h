#pragma once

#include <RendererCore/Meshes/MeshComponentBase.h>

struct nsMsgExtractGeometry;
using nsMeshComponentManager = nsComponentManager<class nsMeshComponent, nsBlockStorageType::Compact>;

/// \brief Renders a single instance of a static mesh.
///
/// This is the main component to use for rendering regular meshes.
class NS_RENDERERCORE_DLL nsMeshComponent : public nsMeshComponentBase
{
  NS_DECLARE_COMPONENT_TYPE(nsMeshComponent, nsMeshComponentBase, nsMeshComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsMeshComponent

public:
  nsMeshComponent();
  ~nsMeshComponent();

  /// \brief Extracts the render geometry for export etc.
  void OnMsgExtractGeometry(nsMsgExtractGeometry& ref_msg) const; // [ msg handler ]
};
