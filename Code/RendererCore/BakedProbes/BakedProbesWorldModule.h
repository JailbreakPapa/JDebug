#pragma once

#include <Core/Graphics/AmbientCubeBasis.h>
#include <Core/World/WorldModule.h>
#include <RendererCore/Declarations.h>

using nsProbeTreeSectorResourceHandle = nsTypedResourceHandle<class nsProbeTreeSectorResource>;

class NS_RENDERERCORE_DLL nsBakedProbesWorldModule : public nsWorldModule
{
  NS_DECLARE_WORLD_MODULE();
  NS_ADD_DYNAMIC_REFLECTION(nsBakedProbesWorldModule, nsWorldModule);

public:
  nsBakedProbesWorldModule(nsWorld* pWorld);
  ~nsBakedProbesWorldModule();

  virtual void Initialize() override;
  virtual void Deinitialize() override;

  bool HasProbeData() const;

  struct ProbeIndexData
  {
    static constexpr nsUInt32 NumProbes = 8;
    nsUInt32 m_probeIndices[NumProbes];
    float m_probeWeights[NumProbes];
  };

  nsResult GetProbeIndexData(const nsVec3& vGlobalPosition, const nsVec3& vNormal, ProbeIndexData& out_probeIndexData) const;

  nsAmbientCube<float> GetSkyVisibility(const ProbeIndexData& indexData) const;

private:
  friend class nsBakedProbesComponent;

  void SetProbeTreeResourcePrefix(const nsHashedString& prefix);

  nsProbeTreeSectorResourceHandle m_hProbeTree;
};
