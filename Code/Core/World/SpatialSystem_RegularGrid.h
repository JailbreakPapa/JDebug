#pragma once

#include <Core/World/SpatialSystem.h>
#include <Foundation/Containers/IdTable.h>
#include <Foundation/SimdMath/SimdVec4i.h>
#include <Foundation/Types/UniquePtr.h>

namespace nsInternal
{
  struct QueryHelper;
}

class NS_CORE_DLL nsSpatialSystem_RegularGrid : public nsSpatialSystem
{
  NS_ADD_DYNAMIC_REFLECTION(nsSpatialSystem_RegularGrid, nsSpatialSystem);

public:
  nsSpatialSystem_RegularGrid(nsUInt32 uiCellSize = 128);
  ~nsSpatialSystem_RegularGrid();

  /// \brief Returns the bounding box of the cell associated with the given spatial data. Useful for debug visualizations.
  nsResult GetCellBoxForSpatialData(const nsSpatialDataHandle& hData, nsBoundingBox& out_boundingBox) const;

  /// \brief Returns bounding boxes of all existing cells.
  void GetAllCellBoxes(nsDynamicArray<nsBoundingBox>& out_boundingBoxes, nsSpatialData::Category filterCategory = nsInvalidSpatialDataCategory) const;

private:
  friend nsInternal::QueryHelper;

  // nsSpatialSystem implementation
  virtual void StartNewFrame() override;

  nsSpatialDataHandle CreateSpatialData(const nsSimdBBoxSphere& bounds, nsGameObject* pObject, nsUInt32 uiCategoryBitmask, const nsTagSet& tags) override;
  nsSpatialDataHandle CreateSpatialDataAlwaysVisible(nsGameObject* pObject, nsUInt32 uiCategoryBitmask, const nsTagSet& tags) override;

  void DeleteSpatialData(const nsSpatialDataHandle& hData) override;

  void UpdateSpatialDataBounds(const nsSpatialDataHandle& hData, const nsSimdBBoxSphere& bounds) override;
  void UpdateSpatialDataObject(const nsSpatialDataHandle& hData, nsGameObject* pObject) override;

  void FindObjectsInSphere(const nsBoundingSphere& sphere, const QueryParams& queryParams, QueryCallback callback) const override;
  void FindObjectsInBox(const nsBoundingBox& box, const QueryParams& queryParams, QueryCallback callback) const override;

  void FindVisibleObjects(const nsFrustum& frustum, const QueryParams& queryParams, nsDynamicArray<const nsGameObject*>& out_Objects, nsSpatialSystem::IsOccludedFunc IsOccluded, nsVisibilityState visType) const override;

  nsVisibilityState GetVisibilityState(const nsSpatialDataHandle& hData, nsUInt32 uiNumFramesBeforeInvisible) const override;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  virtual void GetInternalStats(nsStringBuilder& sb) const override;
#endif

  nsProxyAllocator m_AlignedAllocator;

  nsSimdVec4i m_vCellSize;
  nsSimdVec4f m_vOverlapSize;
  nsSimdFloat m_fInvCellSize;

  enum
  {
    MAX_NUM_GRIDS = 63,
    MAX_NUM_REGULAR_GRIDS = (sizeof(nsSpatialData::Category::m_uiValue) * 8),
    MAX_NUM_CACHED_GRIDS = MAX_NUM_GRIDS - MAX_NUM_REGULAR_GRIDS
  };

  struct Cell;
  struct Grid;
  nsDynamicArray<nsUniquePtr<Grid>> m_Grids;
  nsUInt32 m_uiFirstCachedGridIndex = MAX_NUM_GRIDS;

  struct Data
  {
    NS_DECLARE_POD_TYPE();

    nsUInt64 m_uiGridBitmask : MAX_NUM_GRIDS;
    nsUInt64 m_uiAlwaysVisible : 1;
  };

  nsIdTable<nsSpatialDataId, Data, nsLocalAllocatorWrapper> m_DataTable;

  bool IsAlwaysVisibleData(const Data& data) const;

  nsSpatialDataHandle AddSpatialDataToGrids(const nsSimdBBoxSphere& bounds, nsGameObject* pObject, nsUInt32 uiCategoryBitmask, const nsTagSet& tags, bool bAlwaysVisible);

  template <typename Functor>
  void ForEachGrid(const Data& data, const nsSpatialDataHandle& hData, Functor func) const;

  struct Stats;
  using CellCallback = nsDelegate<nsVisitorExecution::Enum(const Cell&, const QueryParams&, Stats&, void*, nsVisibilityState)>;
  void ForEachCellInBoxInMatchingGrids(const nsSimdBBox& box, const QueryParams& queryParams, CellCallback noFilterCallback, CellCallback filterByTagsCallback, void* pUserData, nsVisibilityState visType) const;

  struct CacheCandidate
  {
    nsTagSet m_IncludeTags;
    nsTagSet m_ExcludeTags;
    nsSpatialData::Category m_Category;
    float m_fQueryCount = 0.0f;
    float m_fFilteredRatio = 0.0f;
    nsUInt32 m_uiGridIndex = nsInvalidIndex;
  };

  mutable nsDynamicArray<CacheCandidate> m_CacheCandidates;
  mutable nsMutex m_CacheCandidatesMutex;

  struct SortedCacheCandidate
  {
    nsUInt32 m_uiIndex = 0;
    float m_fScore = 0;

    bool operator<(const SortedCacheCandidate& other) const
    {
      if (m_fScore != other.m_fScore)
        return m_fScore > other.m_fScore; // higher score comes first

      return m_uiIndex < other.m_uiIndex;
    }
  };

  nsDynamicArray<SortedCacheCandidate> m_SortedCacheCandidates;

  void MigrateCachedGrid(nsUInt32 uiCandidateIndex);
  void MigrateSpatialData(nsUInt32 uiTargetGridIndex, nsUInt32 uiSourceGridIndex);

  void RemoveCachedGrid(nsUInt32 uiCandidateIndex);
  void RemoveAllCachedGrids();

  void UpdateCacheCandidate(const nsTagSet* pIncludeTags, const nsTagSet* pExcludeTags, nsSpatialData::Category category, float filteredRatio) const;
};
