#pragma once

#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/Implementation/ReflectionProbeData.h>
#include <RendererFoundation/RendererFoundationDLL.h>

/// \brief Event generated on mapping changes.
/// \sa nsReflectionProbeMapping::m_Events
struct nsReflectionProbeMappingEvent
{
  enum class Type
  {
    ProbeMapped,          ///< The given probe was mapped to the atlas.
    ProbeUnmapped,        ///<  The given probe was unmapped from the atlas.
    ProbeUpdateRequested, ///< The given probe needs to be updated after which nsReflectionProbeMapping::ProbeUpdateFinished must be called.
  };

  nsReflectionProbeId m_Id;
  Type m_Type;
};

/// \brief This class creates a reflection probe atlas and controls the mapping of added probes to the available atlas indices.
class nsReflectionProbeMapping
{
public:
  /// \brief Creates a reflection probe atlas and mapping of the given size.
  /// \param uiAtlasSize How many probes the atlas can contain.
  nsReflectionProbeMapping(nsUInt32 uiAtlasSize);
  ~nsReflectionProbeMapping();

  /// \name Probe management
  ///@{

  /// \brief Adds a probe that will be considered for mapping into the atlas.
  void AddProbe(nsReflectionProbeId probe, nsBitflags<nsProbeFlags> flags);

  /// \brief Marks previously added probe as dirty and potentially changes its flags.
  void UpdateProbe(nsReflectionProbeId probe, nsBitflags<nsProbeFlags> flags);

  /// \brief Should be called once a requested nsReflectionProbeMappingEvent::Type::ProbeUpdateRequested event has been completed.
  /// \param probe The probe that has finished its update.
  void ProbeUpdateFinished(nsReflectionProbeId probe);

  /// \brief Removes a probe. If the probe was mapped, nsReflectionProbeMappingEvent::Type::ProbeUnmapped will be fired when calling this function.
  void RemoveProbe(nsReflectionProbeId probe);

  ///@}
  /// \name Render helpers
  ///@{

  /// \brief Returns the index at which a given probe is mapped.
  /// \param probe The probe that is being queried.
  /// \param bForExtraction If set, returns whether the index can be used for using the probe during rendering. If the probe was just mapped but not updated yet, -1 will be returned for bForExtraction = true but a valid index for bForExtraction = false so that the index can be rendered into.
  /// \return Returns the mapped index in the atlas or -1 of the probe is not mapped.
  nsInt32 GetReflectionIndex(nsReflectionProbeId probe, bool bForExtraction = false) const;

  /// \brief Returns the atlas texture.
  /// \return The texture handle of the cube map atlas.
  nsGALTextureHandle GetTexture() const { return m_hReflectionSpecularTexture; }

  ///@}
  /// \name Compute atlas mapping
  ///@{

  /// \brief Should be called in the PreExtraction phase. This will reset all probe weights.
  void PreExtraction();

  /// \brief Adds weight to a probe. Should be called during extraction of the probe. The mapping will map the probes with the highest weights in the atlas over time. This can be called multiple times in a frame for a probe if it is visible in multiple views. The maximum weight is then taken.
  void AddWeight(nsReflectionProbeId probe, float fPriority);

  /// \brief Should be called in the PostExtraction phase. This will compute the best probe mapping and potentially fire nsReflectionProbeMappingEvent events to map / unmap or request updates of probes.
  void PostExtraction();

  ///@}

public:
  nsEvent<const nsReflectionProbeMappingEvent&> m_Events;

private:
  struct nsProbeMappingFlags
  {
    using StorageType = nsUInt8;

    enum Enum
    {
      SkyLight = nsProbeFlags::SkyLight,
      HasCustomCubeMap = nsProbeFlags::HasCustomCubeMap,
      Sphere = nsProbeFlags::Sphere,
      Box = nsProbeFlags::Box,
      Dynamic = nsProbeFlags::Dynamic,
      Dirty = NS_BIT(5),
      Usable = NS_BIT(6),
      Default = 0
    };

    struct Bits
    {
      StorageType SkyLight : 1;
      StorageType HasCustomCubeMap : 1;
      StorageType Sphere : 1;
      StorageType Box : 1;
      StorageType Dynamic : 1;
      StorageType Dirty : 1;
      StorageType Usable : 1;
    };
  };

  // NS_DECLARE_FLAGS_OPERATORS(nsProbeMappingFlags);

  struct SortedProbes
  {
    NS_DECLARE_POD_TYPE();

    NS_ALWAYS_INLINE bool operator<(const SortedProbes& other) const
    {
      if (m_fPriority > other.m_fPriority) // we want to sort descending (higher priority first)
        return true;

      return m_uiIndex < other.m_uiIndex;
    }

    nsReflectionProbeId m_uiIndex;
    float m_fPriority = 0.0f;
  };

  struct ProbeDataInternal
  {
    nsBitflags<nsProbeMappingFlags> m_Flags;
    nsInt32 m_uiReflectionIndex = -1;
    float m_fPriority = 0.0f;
    nsReflectionProbeId m_id;
  };

private:
  void MapProbe(nsReflectionProbeId id, nsInt32 iReflectionIndex);
  void UnmapProbe(nsReflectionProbeId id);

private:
  nsDynamicArray<ProbeDataInternal> m_RegisteredProbes;
  nsReflectionProbeId m_SkyLight;

  nsUInt32 m_uiAtlasSize = 32;
  nsDynamicArray<nsReflectionProbeId> m_MappedCubes;

  // GPU Data
  nsGALTextureHandle m_hReflectionSpecularTexture;

  // Cleared every frame:
  nsDynamicArray<SortedProbes> m_SortedProbes; // All probes exiting in the scene, sorted by priority.
  nsDynamicArray<SortedProbes> m_ActiveProbes; // Probes that are currently mapped in the atlas.
  nsDynamicArray<nsInt32> m_UnusedProbeSlots;  // Probe slots are are currently unused in the atlas.
  nsDynamicArray<SortedProbes> m_AddProbes;    // Probes that should be added to the atlas
};
