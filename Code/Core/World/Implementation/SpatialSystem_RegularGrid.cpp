#include <Core/CorePCH.h>

#include <Core/World/SpatialSystem_RegularGrid.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/SimdMath/SimdConversion.h>
#include <Foundation/Time/Stopwatch.h>

nsCVarInt cvar_SpatialQueriesCachingThreshold("Spatial.Queries.CachingThreshold", 100, nsCVarFlags::Default, "Number of objects that are tested for a query before it is considered for caching");

struct PlaneData
{
  nsSimdVec4f m_x0x1x2x3;
  nsSimdVec4f m_y0y1y2y3;
  nsSimdVec4f m_z0z1z2z3;
  nsSimdVec4f m_w0w1w2w3;

  nsSimdVec4f m_x4x5x4x5;
  nsSimdVec4f m_y4y5y4y5;
  nsSimdVec4f m_z4z5z4z5;
  nsSimdVec4f m_w4w5w4w5;
};

namespace
{
  enum
  {
    MAX_CELL_INDEX = (1 << 20) - 1,
    CELL_INDEX_MASK = (1 << 21) - 1
  };

  NS_ALWAYS_INLINE nsSimdVec4f ToVec3(const nsSimdVec4i& v)
  {
    return v.ToFloat();
  }

  NS_ALWAYS_INLINE nsSimdVec4i ToVec3I32(const nsSimdVec4f& v)
  {
    nsSimdVec4f vf = v.Floor();
    return nsSimdVec4i::Truncate(vf);
  }

  NS_ALWAYS_INLINE nsUInt64 GetCellKey(nsInt32 x, nsInt32 y, nsInt32 z)
  {
    nsUInt64 sx = (x + MAX_CELL_INDEX) & CELL_INDEX_MASK;
    nsUInt64 sy = (y + MAX_CELL_INDEX) & CELL_INDEX_MASK;
    nsUInt64 sz = (z + MAX_CELL_INDEX) & CELL_INDEX_MASK;

    return (sx << 42) | (sy << 21) | sz;
  }

  NS_ALWAYS_INLINE nsSimdBBox ComputeCellBoundingBox(const nsSimdVec4i& vCellIndex, const nsSimdVec4i& vCellSize)
  {
    nsSimdVec4i overlapSize = vCellSize >> 2;
    nsSimdVec4i minPos = vCellIndex.CompMul(vCellSize);

    nsSimdVec4f bmin = ToVec3(minPos - overlapSize);
    nsSimdVec4f bmax = ToVec3(minPos + overlapSize + vCellSize);

    return nsSimdBBox(bmin, bmax);
  }

  NS_ALWAYS_INLINE bool AreTagSetsEqual(const nsTagSet& a, const nsTagSet* pB)
  {
    if (pB != nullptr)
    {
      return a == *pB;
    }

    return a.IsEmpty();
  }

  NS_ALWAYS_INLINE bool FilterByTags(const nsTagSet& tags, const nsTagSet* pIncludeTags, const nsTagSet* pExcludeTags)
  {
    if (pExcludeTags != nullptr && !pExcludeTags->IsEmpty() && pExcludeTags->IsAnySet(tags))
      return true;

    if (pIncludeTags != nullptr && !pIncludeTags->IsEmpty() && !pIncludeTags->IsAnySet(tags))
      return true;

    return false;
  }

  NS_ALWAYS_INLINE bool CanBeCached(nsSpatialData::Category category)
  {
    return nsSpatialData::GetCategoryFlags(category).IsSet(nsSpatialData::Flags::FrequentChanges) == false;
  }


#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  void TagsToString(const nsTagSet& tags, nsStringBuilder& out_sSb)
  {
    out_sSb.Append("{ ");

    bool first = true;
    for (auto it = tags.GetIterator(); it.IsValid(); ++it)
    {
      if (!first)
      {
        out_sSb.Append(", ");
        first = false;
      }
      out_sSb.Append(it->GetTagString().GetView());
    }

    out_sSb.Append(" }");
  }
#endif

  NS_FORCE_INLINE bool SphereFrustumIntersect(const nsSimdBSphere& sphere, const PlaneData& planeData)
  {
    nsSimdVec4f pos_xxxx(sphere.m_CenterAndRadius.x());
    nsSimdVec4f pos_yyyy(sphere.m_CenterAndRadius.y());
    nsSimdVec4f pos_zzzz(sphere.m_CenterAndRadius.z());
    nsSimdVec4f pos_rrrr(sphere.m_CenterAndRadius.w());

    nsSimdVec4f dot_0123;
    dot_0123 = nsSimdVec4f::MulAdd(pos_xxxx, planeData.m_x0x1x2x3, planeData.m_w0w1w2w3);
    dot_0123 = nsSimdVec4f::MulAdd(pos_yyyy, planeData.m_y0y1y2y3, dot_0123);
    dot_0123 = nsSimdVec4f::MulAdd(pos_zzzz, planeData.m_z0z1z2z3, dot_0123);

    nsSimdVec4f dot_4545;
    dot_4545 = nsSimdVec4f::MulAdd(pos_xxxx, planeData.m_x4x5x4x5, planeData.m_w4w5w4w5);
    dot_4545 = nsSimdVec4f::MulAdd(pos_yyyy, planeData.m_y4y5y4y5, dot_4545);
    dot_4545 = nsSimdVec4f::MulAdd(pos_zzzz, planeData.m_z4z5z4z5, dot_4545);

    nsSimdVec4b cmp_0123 = dot_0123 > pos_rrrr;
    nsSimdVec4b cmp_4545 = dot_4545 > pos_rrrr;
    return (cmp_0123 || cmp_4545).NoneSet<4>();
  }

  NS_FORCE_INLINE nsUInt32 SphereFrustumIntersect(const nsSimdBSphere& sphereA, const nsSimdBSphere& sphereB, const PlaneData& planeData)
  {
    nsSimdVec4f posA_xxxx(sphereA.m_CenterAndRadius.x());
    nsSimdVec4f posA_yyyy(sphereA.m_CenterAndRadius.y());
    nsSimdVec4f posA_zzzz(sphereA.m_CenterAndRadius.z());
    nsSimdVec4f posA_rrrr(sphereA.m_CenterAndRadius.w());

    nsSimdVec4f dotA_0123;
    dotA_0123 = nsSimdVec4f::MulAdd(posA_xxxx, planeData.m_x0x1x2x3, planeData.m_w0w1w2w3);
    dotA_0123 = nsSimdVec4f::MulAdd(posA_yyyy, planeData.m_y0y1y2y3, dotA_0123);
    dotA_0123 = nsSimdVec4f::MulAdd(posA_zzzz, planeData.m_z0z1z2z3, dotA_0123);

    nsSimdVec4f posB_xxxx(sphereB.m_CenterAndRadius.x());
    nsSimdVec4f posB_yyyy(sphereB.m_CenterAndRadius.y());
    nsSimdVec4f posB_zzzz(sphereB.m_CenterAndRadius.z());
    nsSimdVec4f posB_rrrr(sphereB.m_CenterAndRadius.w());

    nsSimdVec4f dotB_0123;
    dotB_0123 = nsSimdVec4f::MulAdd(posB_xxxx, planeData.m_x0x1x2x3, planeData.m_w0w1w2w3);
    dotB_0123 = nsSimdVec4f::MulAdd(posB_yyyy, planeData.m_y0y1y2y3, dotB_0123);
    dotB_0123 = nsSimdVec4f::MulAdd(posB_zzzz, planeData.m_z0z1z2z3, dotB_0123);

    nsSimdVec4f posAB_xxxx = posA_xxxx.GetCombined<nsSwizzle::XXXX>(posB_xxxx);
    nsSimdVec4f posAB_yyyy = posA_yyyy.GetCombined<nsSwizzle::XXXX>(posB_yyyy);
    nsSimdVec4f posAB_zzzz = posA_zzzz.GetCombined<nsSwizzle::XXXX>(posB_zzzz);
    nsSimdVec4f posAB_rrrr = posA_rrrr.GetCombined<nsSwizzle::XXXX>(posB_rrrr);

    nsSimdVec4f dot_A45B45;
    dot_A45B45 = nsSimdVec4f::MulAdd(posAB_xxxx, planeData.m_x4x5x4x5, planeData.m_w4w5w4w5);
    dot_A45B45 = nsSimdVec4f::MulAdd(posAB_yyyy, planeData.m_y4y5y4y5, dot_A45B45);
    dot_A45B45 = nsSimdVec4f::MulAdd(posAB_zzzz, planeData.m_z4z5z4z5, dot_A45B45);

    nsSimdVec4b cmp_A0123 = dotA_0123 > posA_rrrr;
    nsSimdVec4b cmp_B0123 = dotB_0123 > posB_rrrr;
    nsSimdVec4b cmp_A45B45 = dot_A45B45 > posAB_rrrr;

    nsSimdVec4b cmp_A45 = cmp_A45B45.Get<nsSwizzle::XYXY>();
    nsSimdVec4b cmp_B45 = cmp_A45B45.Get<nsSwizzle::ZWZW>();

    nsUInt32 result = (cmp_A0123 || cmp_A45).NoneSet<4>() ? 1 : 0;
    result |= (cmp_B0123 || cmp_B45).NoneSet<4>() ? 2 : 0;

    return result;
  }
} // namespace

//////////////////////////////////////////////////////////////////////////

struct CellDataMapping
{
  NS_DECLARE_POD_TYPE();

  nsUInt32 m_uiCellIndex = nsInvalidIndex;
  nsUInt32 m_uiCellDataIndex = nsInvalidIndex;
};

struct nsSpatialSystem_RegularGrid::Cell
{
  Cell(nsAllocator* pAlignedAlloctor, nsAllocator* pAllocator)
    : m_BoundingSpheres(pAlignedAlloctor)
    , m_BoundingBoxHalfExtents(pAlignedAlloctor)
    , m_TagSets(pAllocator)
    , m_ObjectPointers(pAllocator)
    , m_DataIndices(pAllocator)
  {
  }

  NS_FORCE_INLINE nsUInt32 AddData(const nsSimdBBoxSphere& bounds, const nsTagSet& tags, nsGameObject* pObject, nsUInt64 uiLastVisibleFrameIdxAndVisType, nsUInt32 uiDataIndex)
  {
    m_BoundingSpheres.PushBack(bounds.GetSphere());
    m_BoundingBoxHalfExtents.PushBack(bounds.m_BoxHalfExtents);
    m_TagSets.PushBack(tags);
    m_ObjectPointers.PushBack(pObject);
    m_DataIndices.PushBack(uiDataIndex);
    m_LastVisibleFrameIdxAndVisType.PushBack(uiLastVisibleFrameIdxAndVisType);

    return m_BoundingSpheres.GetCount() - 1;
  }

  // Returns the data index of the moved data
  NS_FORCE_INLINE nsUInt32 RemoveData(nsUInt32 uiCellDataIndex)
  {
    nsUInt32 uiMovedDataIndex = m_DataIndices.PeekBack();

    m_BoundingSpheres.RemoveAtAndSwap(uiCellDataIndex);
    m_BoundingBoxHalfExtents.RemoveAtAndSwap(uiCellDataIndex);
    m_TagSets.RemoveAtAndSwap(uiCellDataIndex);
    m_ObjectPointers.RemoveAtAndSwap(uiCellDataIndex);
    m_DataIndices.RemoveAtAndSwap(uiCellDataIndex);
    m_LastVisibleFrameIdxAndVisType.RemoveAtAndSwap(uiCellDataIndex);

    NS_ASSERT_DEBUG(m_DataIndices.GetCount() == uiCellDataIndex || m_DataIndices[uiCellDataIndex] == uiMovedDataIndex, "Implementation error");

    return uiMovedDataIndex;
  }

  NS_ALWAYS_INLINE nsBoundingBox GetBoundingBox() const { return nsSimdConversion::ToBBoxSphere(m_Bounds).GetBox(); }

  nsSimdBBoxSphere m_Bounds;

  nsDynamicArray<nsSimdBSphere> m_BoundingSpheres;
  nsDynamicArray<nsSimdVec4f> m_BoundingBoxHalfExtents;
  nsDynamicArray<nsTagSet> m_TagSets;
  nsDynamicArray<nsGameObject*> m_ObjectPointers;
  mutable nsDynamicArray<nsAtomicInteger64> m_LastVisibleFrameIdxAndVisType;
  nsDynamicArray<nsUInt32> m_DataIndices;
};

//////////////////////////////////////////////////////////////////////////

struct CellKeyHashHelper
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(nsUInt64 value)
  {
    // return nsUInt32(value * 2654435761U);
    return nsHashHelper<nsUInt64>::Hash(value);
  }

  NS_ALWAYS_INLINE static bool Equal(nsUInt64 a, nsUInt64 b) { return a == b; }
};

//////////////////////////////////////////////////////////////////////////

struct nsSpatialSystem_RegularGrid::Grid
{
  Grid(nsSpatialSystem_RegularGrid& ref_system, nsSpatialData::Category category)
    : m_System(ref_system)
    , m_Cells(&ref_system.m_Allocator)
    , m_CellKeyToCellIndex(&ref_system.m_Allocator)
    , m_Category(category)
    , m_bCanBeCached(CanBeCached(category))
  {
    const nsSimdBBox overflowBox = nsSimdBBox::MakeFromCenterAndHalfExtents(nsSimdVec4f::MakeZero(), nsSimdVec4f((float)(ref_system.m_vCellSize.x() * MAX_CELL_INDEX)));

    auto pOverflowCell = NS_NEW(&m_System.m_AlignedAllocator, Cell, &m_System.m_AlignedAllocator, &m_System.m_Allocator);
    pOverflowCell->m_Bounds = overflowBox;

    m_Cells.PushBack(pOverflowCell);
  }

  nsUInt32 GetOrCreateCell(const nsSimdBBoxSphere& bounds)
  {
    nsSimdVec4i cellIndex = ToVec3I32(bounds.m_CenterAndRadius * m_System.m_fInvCellSize);
    nsSimdBBox cellBox = ComputeCellBoundingBox(cellIndex, m_System.m_vCellSize);

    if (cellBox.Contains(bounds.GetBox()))
    {
      nsUInt64 cellKey = GetCellKey(cellIndex.x(), cellIndex.y(), cellIndex.z());

      nsUInt32 uiCellIndex = 0;
      if (m_CellKeyToCellIndex.TryGetValue(cellKey, uiCellIndex))
      {
        return uiCellIndex;
      }

      uiCellIndex = m_Cells.GetCount();
      m_CellKeyToCellIndex.Insert(cellKey, uiCellIndex);

      auto pNewCell = NS_NEW(&m_System.m_AlignedAllocator, Cell, &m_System.m_AlignedAllocator, &m_System.m_Allocator);
      pNewCell->m_Bounds = cellBox;

      m_Cells.PushBack(pNewCell);

      return uiCellIndex;
    }
    else
    {
      return m_uiOverflowCellIndex;
    }
  }

  void AddSpatialData(const nsSimdBBoxSphere& bounds, const nsTagSet& tags, nsGameObject* pObject, nsUInt64 uiLastVisibleFrameIdxAndVisType, const nsSpatialDataHandle& hData)
  {
    nsUInt32 uiDataIndex = hData.GetInternalID().m_InstanceIndex;

    nsUInt32 uiCellIndex = GetOrCreateCell(bounds);
    nsUInt32 uiCellDataIndex = m_Cells[uiCellIndex]->AddData(bounds, tags, pObject, uiLastVisibleFrameIdxAndVisType, uiDataIndex);

    m_CellDataMappings.EnsureCount(uiDataIndex + 1);
    NS_ASSERT_DEBUG(m_CellDataMappings[uiDataIndex].m_uiCellIndex == nsInvalidIndex, "data has already been added to a cell");
    m_CellDataMappings[uiDataIndex] = {uiCellIndex, uiCellDataIndex};
  }

  void RemoveSpatialData(const nsSpatialDataHandle& hData)
  {
    nsUInt32 uiDataIndex = hData.GetInternalID().m_InstanceIndex;

    auto& mapping = m_CellDataMappings[uiDataIndex];
    nsUInt32 uiMovedDataIndex = m_Cells[mapping.m_uiCellIndex]->RemoveData(mapping.m_uiCellDataIndex);
    if (uiMovedDataIndex != uiDataIndex)
    {
      m_CellDataMappings[uiMovedDataIndex].m_uiCellDataIndex = mapping.m_uiCellDataIndex;
    }

    mapping = {};
  }

  bool MigrateSpatialDataFromOtherGrid(nsUInt32 uiDataIndex, const Grid& other)
  {
    // Data has already been added
    if (uiDataIndex < m_CellDataMappings.GetCount() && m_CellDataMappings[uiDataIndex].m_uiCellIndex != nsInvalidIndex)
      return false;

    auto& mapping = other.m_CellDataMappings[uiDataIndex];
    if (mapping.m_uiCellIndex == nsInvalidIndex)
      return false;

    auto& pOtherCell = other.m_Cells[mapping.m_uiCellIndex];

    const nsTagSet& tags = pOtherCell->m_TagSets[mapping.m_uiCellDataIndex];
    if (FilterByTags(tags, &m_IncludeTags, &m_ExcludeTags))
      return false;

    nsSimdBBoxSphere bounds;
    bounds.m_CenterAndRadius = pOtherCell->m_BoundingSpheres[mapping.m_uiCellDataIndex].m_CenterAndRadius;
    bounds.m_BoxHalfExtents = pOtherCell->m_BoundingBoxHalfExtents[mapping.m_uiCellDataIndex];
    nsGameObject* objectPointer = pOtherCell->m_ObjectPointers[mapping.m_uiCellDataIndex];
    const nsUInt64 uiLastVisibleFrameIdxAndVisType = pOtherCell->m_LastVisibleFrameIdxAndVisType[mapping.m_uiCellDataIndex];

    NS_ASSERT_DEBUG(pOtherCell->m_DataIndices[mapping.m_uiCellDataIndex] == uiDataIndex, "Implementation error");
    nsSpatialDataHandle hData = nsSpatialDataHandle(nsSpatialDataId(uiDataIndex, 1));

    AddSpatialData(bounds, tags, objectPointer, uiLastVisibleFrameIdxAndVisType, hData);
    return true;
  }

  NS_ALWAYS_INLINE bool CachingCompleted() const { return m_uiLastMigrationIndex == nsInvalidIndex; }

  template <typename Functor>
  NS_FORCE_INLINE void ForEachCellInBox(const nsSimdBBox& box, Functor func) const
  {
    nsSimdVec4i minIndex = ToVec3I32((box.m_Min - m_System.m_vOverlapSize) * m_System.m_fInvCellSize);
    nsSimdVec4i maxIndex = ToVec3I32((box.m_Max + m_System.m_vOverlapSize) * m_System.m_fInvCellSize);

    NS_ASSERT_DEBUG((minIndex.Abs() < nsSimdVec4i(MAX_CELL_INDEX)).AllSet<3>(), "Position is too big");
    NS_ASSERT_DEBUG((maxIndex.Abs() < nsSimdVec4i(MAX_CELL_INDEX)).AllSet<3>(), "Position is too big");

    const nsInt32 iMinX = minIndex.x();
    const nsInt32 iMinY = minIndex.y();
    const nsInt32 iMinZ = minIndex.z();

    const nsSimdVec4i diff = maxIndex - minIndex + nsSimdVec4i(1);
    const nsInt32 iDiffX = diff.x();
    const nsInt32 iDiffY = diff.y();
    const nsInt32 iDiffZ = diff.z();
    const nsInt32 iNumIterations = iDiffX * iDiffY * iDiffZ;

    for (nsInt32 i = 0; i < iNumIterations; ++i)
    {
      nsInt32 index = i;
      nsInt32 z = i / (iDiffX * iDiffY);
      index -= z * iDiffX * iDiffY;
      nsInt32 y = index / iDiffX;
      nsInt32 x = index - (y * iDiffX);

      x += iMinX;
      y += iMinY;
      z += iMinZ;

      nsUInt64 cellKey = GetCellKey(x, y, z);
      nsUInt32 cellIndex = 0;
      if (m_CellKeyToCellIndex.TryGetValue(cellKey, cellIndex))
      {
        const Cell& constCell = *m_Cells[cellIndex];
        if (func(constCell) == nsVisitorExecution::Stop)
          return;
      }
    }

    const Cell& overflowCell = *m_Cells[m_uiOverflowCellIndex];
    func(overflowCell);
  }

  nsSpatialSystem_RegularGrid& m_System;
  nsDynamicArray<nsUniquePtr<Cell>> m_Cells;

  nsHashTable<nsUInt64, nsUInt32, CellKeyHashHelper> m_CellKeyToCellIndex;
  static constexpr nsUInt32 m_uiOverflowCellIndex = 0;

  nsDynamicArray<CellDataMapping> m_CellDataMappings;

  const nsSpatialData::Category m_Category;
  const bool m_bCanBeCached;

  nsTagSet m_IncludeTags;
  nsTagSet m_ExcludeTags;

  nsUInt32 m_uiLastMigrationIndex = 0;
};

//////////////////////////////////////////////////////////////////////////

struct nsSpatialSystem_RegularGrid::Stats
{
  nsUInt32 m_uiNumObjectsTested = 0;
  nsUInt32 m_uiNumObjectsPassed = 0;
  nsUInt32 m_uiNumObjectsFiltered = 0;
};

//////////////////////////////////////////////////////////////////////////

namespace nsInternal
{
  struct QueryHelper
  {
    template <typename T>
    struct ShapeQueryData
    {
      T m_Shape;
      nsSpatialSystem::QueryCallback m_Callback;
    };

    template <typename T, bool UseTagsFilter>
    static nsVisitorExecution::Enum ShapeQueryCallback(const nsSpatialSystem_RegularGrid::Cell& cell, const nsSpatialSystem::QueryParams& queryParams, nsSpatialSystem_RegularGrid::Stats& ref_stats, void* pUserData, nsVisibilityState visType)
    {
      auto pQueryData = static_cast<const ShapeQueryData<T>*>(pUserData);
      T shape = pQueryData->m_Shape;

      nsSimdBBox cellBox = cell.m_Bounds.GetBox();
      if (!cellBox.Overlaps(shape))
        return nsVisitorExecution::Continue;

      auto boundingSpheres = cell.m_BoundingSpheres.GetData();
      auto tagSets = cell.m_TagSets.GetData();
      auto objectPointers = cell.m_ObjectPointers.GetData();

      const nsUInt32 numSpheres = cell.m_BoundingSpheres.GetCount();
      ref_stats.m_uiNumObjectsTested += numSpheres;

      for (nsUInt32 i = 0; i < numSpheres; ++i)
      {
        if (!shape.Overlaps(boundingSpheres[i]))
          continue;

        if constexpr (UseTagsFilter)
        {
          if (FilterByTags(tagSets[i], queryParams.m_pIncludeTags, queryParams.m_pExcludeTags))
          {
            ref_stats.m_uiNumObjectsFiltered++;
            continue;
          }
        }

        ref_stats.m_uiNumObjectsPassed++;

        if (pQueryData->m_Callback(objectPointers[i]) == nsVisitorExecution::Stop)
          return nsVisitorExecution::Stop;
      }

      return nsVisitorExecution::Continue;
    }

    struct FrustumQueryData
    {
      PlaneData m_PlaneData;
      nsDynamicArray<const nsGameObject*>* m_pOutObjects;
      nsUInt64 m_uiFrameCounter;
      nsSpatialSystem::IsOccludedFunc m_IsOccludedCB;
    };

    template <bool UseTagsFilter, bool UseOcclusionCallback>
    static nsVisitorExecution::Enum FrustumQueryCallback(const nsSpatialSystem_RegularGrid::Cell& cell, const nsSpatialSystem::QueryParams& queryParams, nsSpatialSystem_RegularGrid::Stats& ref_stats, void* pUserData, nsVisibilityState visType)
    {
      auto pQueryData = static_cast<FrustumQueryData*>(pUserData);
      PlaneData planeData = pQueryData->m_PlaneData;

      nsSimdBSphere cellSphere = cell.m_Bounds.GetSphere();
      if (!SphereFrustumIntersect(cellSphere, planeData))
        return nsVisitorExecution::Continue;

      if constexpr (UseOcclusionCallback)
      {
        if (pQueryData->m_IsOccludedCB(cell.m_Bounds.GetBox()))
        {
          return nsVisitorExecution::Continue;
        }
      }

      auto boundingSpheres = cell.m_BoundingSpheres.GetData();
      auto boundingBoxHalfExtents = cell.m_BoundingBoxHalfExtents.GetData();
      auto tagSets = cell.m_TagSets.GetData();
      auto objectPointers = cell.m_ObjectPointers.GetData();
      auto lastVisibleFrameIdxAndVisType = cell.m_LastVisibleFrameIdxAndVisType.GetData();

      const nsUInt32 numSpheres = cell.m_BoundingSpheres.GetCount();
      ref_stats.m_uiNumObjectsTested += numSpheres;

      nsUInt32 currentIndex = 0;
      const nsUInt64 uiFrameIdxAndType = (pQueryData->m_uiFrameCounter << 4) | static_cast<nsUInt64>(visType);

      while (currentIndex < numSpheres)
      {
        if (numSpheres - currentIndex >= 32)
        {
          nsUInt32 mask = 0;

          for (nsUInt32 i = 0; i < 32; i += 2)
          {
            auto& objectSphereA = boundingSpheres[currentIndex + i + 0];
            auto& objectSphereB = boundingSpheres[currentIndex + i + 1];

            mask |= SphereFrustumIntersect(objectSphereA, objectSphereB, planeData) << i;
          }

          while (mask > 0)
          {
            nsUInt32 i = nsMath::FirstBitLow(mask) + currentIndex;
            mask &= mask - 1;

            if constexpr (UseTagsFilter)
            {
              if (FilterByTags(tagSets[i], queryParams.m_pIncludeTags, queryParams.m_pExcludeTags))
              {
                ref_stats.m_uiNumObjectsFiltered++;
                continue;
              }
            }

            if constexpr (UseOcclusionCallback)
            {
              const nsSimdBBox bbox = nsSimdBBox::MakeFromCenterAndHalfExtents(boundingSpheres[i].GetCenter(), boundingBoxHalfExtents[i]);
              if (pQueryData->m_IsOccludedCB(bbox))
              {
                continue;
              }
            }

            lastVisibleFrameIdxAndVisType[i].Max(uiFrameIdxAndType);
            pQueryData->m_pOutObjects->PushBack(objectPointers[i]);

            ref_stats.m_uiNumObjectsPassed++;
          }

          currentIndex += 32;
        }
        else
        {
          nsUInt32 i = currentIndex;
          ++currentIndex;

          if (!SphereFrustumIntersect(boundingSpheres[i], planeData))
            continue;

          if constexpr (UseTagsFilter)
          {
            if (FilterByTags(tagSets[i], queryParams.m_pIncludeTags, queryParams.m_pExcludeTags))
            {
              ref_stats.m_uiNumObjectsFiltered++;
              continue;
            }
          }

          if constexpr (UseOcclusionCallback)
          {
            const nsSimdBBox bbox = nsSimdBBox::MakeFromCenterAndHalfExtents(boundingSpheres[i].GetCenter(), boundingBoxHalfExtents[i]);

            if (pQueryData->m_IsOccludedCB(bbox))
            {
              continue;
            }
          }

          lastVisibleFrameIdxAndVisType[i].Max(uiFrameIdxAndType);
          pQueryData->m_pOutObjects->PushBack(objectPointers[i]);

          ref_stats.m_uiNumObjectsPassed++;
        }
      }

      return nsVisitorExecution::Continue;
    }
  };
} // namespace nsInternal

//////////////////////////////////////////////////////////////////////////

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSpatialSystem_RegularGrid, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsSpatialSystem_RegularGrid::nsSpatialSystem_RegularGrid(nsUInt32 uiCellSize /*= 128*/)
  : m_AlignedAllocator("Spatial System Aligned", nsFoundation::GetAlignedAllocator())
  , m_vCellSize(uiCellSize)
  , m_vOverlapSize(uiCellSize / 4.0f)
  , m_fInvCellSize(1.0f / uiCellSize)
  , m_Grids(&m_Allocator)
  , m_DataTable(&m_Allocator)
{
  NS_CHECK_AT_COMPILETIME(sizeof(Data) == 8);

  m_Grids.SetCount(MAX_NUM_GRIDS);

  cvar_SpatialQueriesCachingThreshold.m_CVarEvents.AddEventHandler([&](const nsCVarEvent& e)
    {
    if (e.m_EventType == nsCVarEvent::ValueChanged)
    {
      RemoveAllCachedGrids();
    } });
}

nsSpatialSystem_RegularGrid::~nsSpatialSystem_RegularGrid() = default;

nsResult nsSpatialSystem_RegularGrid::GetCellBoxForSpatialData(const nsSpatialDataHandle& hData, nsBoundingBox& out_boundingBox) const
{
  Data* pData = nullptr;
  if (!m_DataTable.TryGetValue(hData.GetInternalID(), pData))
    return NS_FAILURE;

  ForEachGrid(*pData, hData,
    [&](Grid& ref_grid, const CellDataMapping& mapping)
    {
      auto& pCell = ref_grid.m_Cells[mapping.m_uiCellIndex];

      out_boundingBox = pCell->GetBoundingBox();
      return nsVisitorExecution::Stop;
    });

  return NS_SUCCESS;
}

template <>
struct nsHashHelper<nsBoundingBox>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const nsBoundingBox& value) { return nsHashingUtils::xxHash32(&value, sizeof(nsBoundingBox)); }

  NS_ALWAYS_INLINE static bool Equal(const nsBoundingBox& a, const nsBoundingBox& b) { return a == b; }
};

void nsSpatialSystem_RegularGrid::GetAllCellBoxes(nsDynamicArray<nsBoundingBox>& out_boundingBoxes, nsSpatialData::Category filterCategory /*= nsInvalidSpatialDataCategory*/) const
{
  if (filterCategory != nsInvalidSpatialDataCategory)
  {
    nsUInt32 uiGridIndex = filterCategory.m_uiValue;
    auto& pGrid = m_Grids[uiGridIndex];
    if (pGrid != nullptr)
    {
      for (auto& pCell : pGrid->m_Cells)
      {
        out_boundingBoxes.ExpandAndGetRef() = pCell->GetBoundingBox();
      }
    }
  }
  else
  {
    nsHashSet<nsBoundingBox> boundingBoxes;

    for (auto& pGrid : m_Grids)
    {
      if (pGrid != nullptr)
      {
        for (auto& pCell : pGrid->m_Cells)
        {
          boundingBoxes.Insert(pCell->GetBoundingBox());
        }
      }
    }

    for (auto boundingBox : boundingBoxes)
    {
      out_boundingBoxes.PushBack(boundingBox);
    }
  }
}

void nsSpatialSystem_RegularGrid::StartNewFrame()
{
  SUPER::StartNewFrame();

  m_SortedCacheCandidates.Clear();

  {
    NS_LOCK(m_CacheCandidatesMutex);

    for (nsUInt32 i = 0; i < m_CacheCandidates.GetCount(); ++i)
    {
      auto& cacheCandidate = m_CacheCandidates[i];

      const float fScore = cacheCandidate.m_fQueryCount + cacheCandidate.m_fFilteredRatio * 100.0f;
      m_SortedCacheCandidates.PushBack({i, fScore});

      // Query has to be issued at least once every 10 frames to keep a stable value
      cacheCandidate.m_fQueryCount = nsMath::Max(cacheCandidate.m_fQueryCount - 0.1f, 0.0f);
    }
  }

  m_SortedCacheCandidates.Sort();

  // First remove all cached grids that don't make it into the top MAX_NUM_CACHED_GRIDS to make space for new grids
  if (m_SortedCacheCandidates.GetCount() > MAX_NUM_CACHED_GRIDS)
  {
    for (nsUInt32 i = MAX_NUM_CACHED_GRIDS; i < m_SortedCacheCandidates.GetCount(); ++i)
    {
      RemoveCachedGrid(m_SortedCacheCandidates[i].m_uiIndex);
    }
  }

  // Then take the MAX_NUM_CACHED_GRIDS candidates with the highest score and migrate the data
  for (nsUInt32 i = 0; i < nsMath::Min<nsUInt32>(m_SortedCacheCandidates.GetCount(), MAX_NUM_CACHED_GRIDS); ++i)
  {
    MigrateCachedGrid(m_SortedCacheCandidates[i].m_uiIndex);
  }
}

nsSpatialDataHandle nsSpatialSystem_RegularGrid::CreateSpatialData(const nsSimdBBoxSphere& bounds, nsGameObject* pObject, nsUInt32 uiCategoryBitmask, const nsTagSet& tags)
{
  if (uiCategoryBitmask == 0)
    return nsSpatialDataHandle();

  return AddSpatialDataToGrids(bounds, pObject, uiCategoryBitmask, tags, false);
}

nsSpatialDataHandle nsSpatialSystem_RegularGrid::CreateSpatialDataAlwaysVisible(nsGameObject* pObject, nsUInt32 uiCategoryBitmask, const nsTagSet& tags)
{
  if (uiCategoryBitmask == 0)
    return nsSpatialDataHandle();

  const nsSimdBBox hugeBox = nsSimdBBox::MakeFromCenterAndHalfExtents(nsSimdVec4f::MakeZero(), nsSimdVec4f((float)(m_vCellSize.x() * MAX_CELL_INDEX)));

  return AddSpatialDataToGrids(hugeBox, pObject, uiCategoryBitmask, tags, true);
}

void nsSpatialSystem_RegularGrid::DeleteSpatialData(const nsSpatialDataHandle& hData)
{
  Data oldData;
  NS_VERIFY(m_DataTable.Remove(hData.GetInternalID(), &oldData), "Invalid spatial data handle");

  ForEachGrid(oldData, hData,
    [&](Grid& ref_grid, const CellDataMapping& mapping)
    {
      ref_grid.RemoveSpatialData(hData);
      return nsVisitorExecution::Continue;
    });
}

void nsSpatialSystem_RegularGrid::UpdateSpatialDataBounds(const nsSpatialDataHandle& hData, const nsSimdBBoxSphere& bounds)
{
  Data* pData = nullptr;
  NS_VERIFY(m_DataTable.TryGetValue(hData.GetInternalID(), pData), "Invalid spatial data handle");

  // No need to update bounds for always visible data
  if (IsAlwaysVisibleData(*pData))
    return;

  ForEachGrid(*pData, hData,
    [&](Grid& ref_grid, const CellDataMapping& mapping)
    {
      auto& pOldCell = ref_grid.m_Cells[mapping.m_uiCellIndex];

      if (pOldCell->m_Bounds.GetBox().Contains(bounds.GetBox()))
      {
        pOldCell->m_BoundingSpheres[mapping.m_uiCellDataIndex] = bounds.GetSphere();
        pOldCell->m_BoundingBoxHalfExtents[mapping.m_uiCellDataIndex] = bounds.m_BoxHalfExtents;
      }
      else
      {
        const nsTagSet tags = pOldCell->m_TagSets[mapping.m_uiCellDataIndex];
        nsGameObject* objectPointer = pOldCell->m_ObjectPointers[mapping.m_uiCellDataIndex];

        const nsUInt64 uiLastVisibleFrameIdxAndVisType = pOldCell->m_LastVisibleFrameIdxAndVisType[mapping.m_uiCellDataIndex];

        ref_grid.RemoveSpatialData(hData);

        ref_grid.AddSpatialData(bounds, tags, objectPointer, uiLastVisibleFrameIdxAndVisType, hData);
      }

      return nsVisitorExecution::Continue;
    });
}

void nsSpatialSystem_RegularGrid::UpdateSpatialDataObject(const nsSpatialDataHandle& hData, nsGameObject* pObject)
{
  Data* pData = nullptr;
  NS_VERIFY(m_DataTable.TryGetValue(hData.GetInternalID(), pData), "Invalid spatial data handle");

  ForEachGrid(*pData, hData,
    [&](Grid& ref_grid, const CellDataMapping& mapping)
    {
      auto& pCell = ref_grid.m_Cells[mapping.m_uiCellIndex];
      pCell->m_ObjectPointers[mapping.m_uiCellDataIndex] = pObject;
      return nsVisitorExecution::Continue;
    });
}

void nsSpatialSystem_RegularGrid::FindObjectsInSphere(const nsBoundingSphere& sphere, const QueryParams& queryParams, QueryCallback callback) const
{
  NS_PROFILE_SCOPE("FindObjectsInSphere");

  nsSimdBSphere simdSphere(nsSimdConversion::ToVec3(sphere.m_vCenter), sphere.m_fRadius);

  const nsSimdBBox simdBox = nsSimdBBox::MakeFromCenterAndHalfExtents(simdSphere.m_CenterAndRadius, simdSphere.m_CenterAndRadius.Get<nsSwizzle::WWWW>());

  nsInternal::QueryHelper::ShapeQueryData<nsSimdBSphere> queryData = {simdSphere, callback};

  ForEachCellInBoxInMatchingGrids(simdBox, queryParams,
    &nsInternal::QueryHelper::ShapeQueryCallback<nsSimdBSphere, false>,
    &nsInternal::QueryHelper::ShapeQueryCallback<nsSimdBSphere, true>,
    &queryData, nsVisibilityState::Indirect);
}

void nsSpatialSystem_RegularGrid::FindObjectsInBox(const nsBoundingBox& box, const QueryParams& queryParams, QueryCallback callback) const
{
  NS_PROFILE_SCOPE("FindObjectsInBox");

  nsSimdBBox simdBox(nsSimdConversion::ToVec3(box.m_vMin), nsSimdConversion::ToVec3(box.m_vMax));

  nsInternal::QueryHelper::ShapeQueryData<nsSimdBBox> queryData = {simdBox, callback};

  ForEachCellInBoxInMatchingGrids(simdBox, queryParams,
    &nsInternal::QueryHelper::ShapeQueryCallback<nsSimdBBox, false>,
    &nsInternal::QueryHelper::ShapeQueryCallback<nsSimdBBox, true>,
    &queryData, nsVisibilityState::Indirect);
}

void nsSpatialSystem_RegularGrid::FindVisibleObjects(const nsFrustum& frustum, const QueryParams& queryParams, nsDynamicArray<const nsGameObject*>& out_Objects, nsSpatialSystem::IsOccludedFunc IsOccluded, nsVisibilityState visType) const
{
  NS_PROFILE_SCOPE("FindVisibleObjects");

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  nsStopwatch timer;
#endif

  nsVec3 cornerPoints[8];
  frustum.ComputeCornerPoints(cornerPoints).AssertSuccess();

  nsSimdVec4f simdCornerPoints[8];
  for (nsUInt32 i = 0; i < 8; ++i)
  {
    simdCornerPoints[i] = nsSimdConversion::ToVec3(cornerPoints[i]);
  }

  const nsSimdBBox simdBox = nsSimdBBox::MakeFromPoints(simdCornerPoints, 8);

  nsInternal::QueryHelper::FrustumQueryData queryData;
  {
    // Compiler is too stupid to properly unroll a constant loop so we do it by hand
    nsSimdVec4f plane0 = nsSimdConversion::ToVec4(*reinterpret_cast<const nsVec4*>(&(frustum.GetPlane(0).m_vNormal.x)));
    nsSimdVec4f plane1 = nsSimdConversion::ToVec4(*reinterpret_cast<const nsVec4*>(&(frustum.GetPlane(1).m_vNormal.x)));
    nsSimdVec4f plane2 = nsSimdConversion::ToVec4(*reinterpret_cast<const nsVec4*>(&(frustum.GetPlane(2).m_vNormal.x)));
    nsSimdVec4f plane3 = nsSimdConversion::ToVec4(*reinterpret_cast<const nsVec4*>(&(frustum.GetPlane(3).m_vNormal.x)));
    nsSimdVec4f plane4 = nsSimdConversion::ToVec4(*reinterpret_cast<const nsVec4*>(&(frustum.GetPlane(4).m_vNormal.x)));
    nsSimdVec4f plane5 = nsSimdConversion::ToVec4(*reinterpret_cast<const nsVec4*>(&(frustum.GetPlane(5).m_vNormal.x)));

    nsSimdMat4f helperMat;
    helperMat.SetRows(plane0, plane1, plane2, plane3);

    queryData.m_PlaneData.m_x0x1x2x3 = helperMat.m_col0;
    queryData.m_PlaneData.m_y0y1y2y3 = helperMat.m_col1;
    queryData.m_PlaneData.m_z0z1z2z3 = helperMat.m_col2;
    queryData.m_PlaneData.m_w0w1w2w3 = helperMat.m_col3;

    helperMat.SetRows(plane4, plane5, plane4, plane5);

    queryData.m_PlaneData.m_x4x5x4x5 = helperMat.m_col0;
    queryData.m_PlaneData.m_y4y5y4y5 = helperMat.m_col1;
    queryData.m_PlaneData.m_z4z5z4z5 = helperMat.m_col2;
    queryData.m_PlaneData.m_w4w5w4w5 = helperMat.m_col3;

    queryData.m_pOutObjects = &out_Objects;
    queryData.m_uiFrameCounter = m_uiFrameCounter;

    queryData.m_IsOccludedCB = IsOccluded;
  }

  if (IsOccluded.IsValid())
  {
    ForEachCellInBoxInMatchingGrids(simdBox, queryParams,
      &nsInternal::QueryHelper::FrustumQueryCallback<false, true>,
      &nsInternal::QueryHelper::FrustumQueryCallback<true, true>,
      &queryData, visType);
  }
  else
  {
    ForEachCellInBoxInMatchingGrids(simdBox, queryParams,
      &nsInternal::QueryHelper::FrustumQueryCallback<false, false>,
      &nsInternal::QueryHelper::FrustumQueryCallback<true, false>,
      &queryData, visType);
  }

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  if (queryParams.m_pStats != nullptr)
  {
    queryParams.m_pStats->m_TimeTaken = timer.GetRunningTotal();
  }
#endif
}

nsVisibilityState nsSpatialSystem_RegularGrid::GetVisibilityState(const nsSpatialDataHandle& hData, nsUInt32 uiNumFramesBeforeInvisible) const
{
  Data* pData = nullptr;
  NS_VERIFY(m_DataTable.TryGetValue(hData.GetInternalID(), pData), "Invalid spatial data handle");

  if (IsAlwaysVisibleData(*pData))
    return nsVisibilityState::Direct;

  nsUInt64 uiLastVisibleFrameIdxAndVisType = 0;
  ForEachGrid(*pData, hData,
    [&](const Grid& grid, const CellDataMapping& mapping)
    {
      auto& pCell = grid.m_Cells[mapping.m_uiCellIndex];
      uiLastVisibleFrameIdxAndVisType = nsMath::Max<nsUInt64>(uiLastVisibleFrameIdxAndVisType, pCell->m_LastVisibleFrameIdxAndVisType[mapping.m_uiCellDataIndex]);
      return nsVisitorExecution::Continue;
    });

  const nsUInt64 uiLastVisibleFrameIdx = (uiLastVisibleFrameIdxAndVisType >> 4);
  const nsUInt64 uiLastVisibilityType = (uiLastVisibleFrameIdxAndVisType & static_cast<nsUInt64>(15)); // mask out lower 4 bits

  if (m_uiFrameCounter > uiLastVisibleFrameIdx + uiNumFramesBeforeInvisible)
    return nsVisibilityState::Invisible;

  return static_cast<nsVisibilityState>(uiLastVisibilityType);
}

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
void nsSpatialSystem_RegularGrid::GetInternalStats(nsStringBuilder& sb) const
{
  NS_LOCK(m_CacheCandidatesMutex);

  nsUInt32 uiNumActiveGrids = 0;
  for (auto& pGrid : m_Grids)
  {
    uiNumActiveGrids += (pGrid != nullptr) ? 1 : 0;
  }

  sb.SetFormat("Num Grids: {}\n", uiNumActiveGrids);

  for (auto& pGrid : m_Grids)
  {
    if (pGrid == nullptr)
      continue;

    sb.AppendFormat(" \nCategory: {}, CanBeCached: {}\nIncludeTags: ", nsSpatialData::GetCategoryName(pGrid->m_Category), pGrid->m_bCanBeCached);
    TagsToString(pGrid->m_IncludeTags, sb);
    sb.Append(", ExcludeTags: ");
    TagsToString(pGrid->m_ExcludeTags, sb);
    sb.Append("\n");
  }

  sb.Append("\nCache Candidates:\n");

  for (auto& sortedCandidate : m_SortedCacheCandidates)
  {
    auto& candidate = m_CacheCandidates[sortedCandidate.m_uiIndex];
    const nsUInt32 uiGridIndex = candidate.m_uiGridIndex;
    Grid* pGrid = nullptr;

    if (uiGridIndex != nsInvalidIndex)
    {
      pGrid = m_Grids[uiGridIndex].Borrow();
      if (pGrid->CachingCompleted())
      {
        continue;
      }
    }

    sb.AppendFormat(" \nCategory: {}\nIncludeTags: ", nsSpatialData::GetCategoryName(candidate.m_Category));
    TagsToString(candidate.m_IncludeTags, sb);
    sb.Append(", ExcludeTags: ");
    TagsToString(candidate.m_ExcludeTags, sb);
    sb.AppendFormat("\nScore: {}", nsArgF(sortedCandidate.m_fScore, 2));

    if (pGrid != nullptr)
    {
      const nsUInt32 uiNumObjectsMigrated = pGrid->m_uiLastMigrationIndex;
      sb.AppendFormat("\nMigrationStatus: {}%%\n", nsArgF(float(uiNumObjectsMigrated) / m_DataTable.GetCount() * 100.0f, 2));
    }
  }
}
#endif

NS_ALWAYS_INLINE bool nsSpatialSystem_RegularGrid::IsAlwaysVisibleData(const Data& data) const
{
  return data.m_uiAlwaysVisible != 0;
}

nsSpatialDataHandle nsSpatialSystem_RegularGrid::AddSpatialDataToGrids(const nsSimdBBoxSphere& bounds, nsGameObject* pObject, nsUInt32 uiCategoryBitmask, const nsTagSet& tags, bool bAlwaysVisible)
{
  Data data;
  data.m_uiGridBitmask = uiCategoryBitmask;
  data.m_uiAlwaysVisible = bAlwaysVisible ? 1 : 0;

  // find matching cached grids and add them to data.m_uiGridBitmask
  for (nsUInt32 uiCachedGridIndex = m_uiFirstCachedGridIndex; uiCachedGridIndex < m_Grids.GetCount(); ++uiCachedGridIndex)
  {
    auto& pGrid = m_Grids[uiCachedGridIndex];
    if (pGrid == nullptr)
      continue;

    if ((pGrid->m_Category.GetBitmask() & uiCategoryBitmask) == 0 ||
        FilterByTags(tags, &pGrid->m_IncludeTags, &pGrid->m_ExcludeTags))
      continue;

    data.m_uiGridBitmask |= NS_BIT(uiCachedGridIndex);
  }

  auto hData = nsSpatialDataHandle(m_DataTable.Insert(data));

  nsUInt64 uiGridBitmask = data.m_uiGridBitmask;
  while (uiGridBitmask > 0)
  {
    nsUInt32 uiGridIndex = nsMath::FirstBitLow(uiGridBitmask);
    uiGridBitmask &= uiGridBitmask - 1;

    auto& pGrid = m_Grids[uiGridIndex];
    if (pGrid == nullptr)
    {
      pGrid = NS_NEW(&m_Allocator, Grid, *this, nsSpatialData::Category(static_cast<nsUInt16>(uiGridIndex)));
    }

    pGrid->AddSpatialData(bounds, tags, pObject, m_uiFrameCounter, hData);
  }

  return hData;
}

template <typename Functor>
NS_FORCE_INLINE void nsSpatialSystem_RegularGrid::ForEachGrid(const Data& data, const nsSpatialDataHandle& hData, Functor func) const
{
  nsUInt64 uiGridBitmask = data.m_uiGridBitmask;
  nsUInt32 uiDataIndex = hData.GetInternalID().m_InstanceIndex;

  while (uiGridBitmask > 0)
  {
    nsUInt32 uiGridIndex = nsMath::FirstBitLow(uiGridBitmask);
    uiGridBitmask &= uiGridBitmask - 1;

    auto& grid = *m_Grids[uiGridIndex];
    auto& mapping = grid.m_CellDataMappings[uiDataIndex];

    if (func(grid, mapping) == nsVisitorExecution::Stop)
      break;
  }
}

void nsSpatialSystem_RegularGrid::ForEachCellInBoxInMatchingGrids(const nsSimdBBox& box, const QueryParams& queryParams, CellCallback noFilterCallback, CellCallback filterByTagsCallback, void* pUserData, nsVisibilityState visType) const
{
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  if (queryParams.m_pStats != nullptr)
  {
    queryParams.m_pStats->m_uiTotalNumObjects = m_DataTable.GetCount();
  }
#endif

  nsUInt32 uiGridBitmask = queryParams.m_uiCategoryBitmask;

  // search for cached grids that match the exact query params first
  for (nsUInt32 uiCachedGridIndex = m_uiFirstCachedGridIndex; uiCachedGridIndex < m_Grids.GetCount(); ++uiCachedGridIndex)
  {
    auto& pGrid = m_Grids[uiCachedGridIndex];
    if (pGrid == nullptr || pGrid->CachingCompleted() == false)
      continue;

    if ((pGrid->m_Category.GetBitmask() & uiGridBitmask) == 0 ||
        AreTagSetsEqual(pGrid->m_IncludeTags, queryParams.m_pIncludeTags) == false ||
        AreTagSetsEqual(pGrid->m_ExcludeTags, queryParams.m_pExcludeTags) == false)
      continue;

    uiGridBitmask &= ~pGrid->m_Category.GetBitmask();

    Stats stats;
    pGrid->ForEachCellInBox(box,
      [&](const Cell& cell)
      {
        return noFilterCallback(cell, queryParams, stats, pUserData, visType);
      });

    UpdateCacheCandidate(queryParams.m_pIncludeTags, queryParams.m_pExcludeTags, pGrid->m_Category, 0.0f);

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
    if (queryParams.m_pStats != nullptr)
    {
      queryParams.m_pStats->m_uiNumObjectsTested += stats.m_uiNumObjectsTested;
      queryParams.m_pStats->m_uiNumObjectsPassed += stats.m_uiNumObjectsPassed;
    }
#endif
  }

  // then search for the rest
  const bool useTagsFilter = (queryParams.m_pIncludeTags && queryParams.m_pIncludeTags->IsEmpty() == false) || (queryParams.m_pExcludeTags && queryParams.m_pExcludeTags->IsEmpty() == false);
  CellCallback cellCallback = useTagsFilter ? filterByTagsCallback : noFilterCallback;

  while (uiGridBitmask > 0)
  {
    nsUInt32 uiGridIndex = nsMath::FirstBitLow(uiGridBitmask);
    uiGridBitmask &= uiGridBitmask - 1;

    auto& pGrid = m_Grids[uiGridIndex];
    if (pGrid == nullptr)
      continue;

    Stats stats;
    pGrid->ForEachCellInBox(box,
      [&](const Cell& cell)
      {
        return cellCallback(cell, queryParams, stats, pUserData, visType);
      });

    if (pGrid->m_bCanBeCached && useTagsFilter)
    {
      const nsUInt32 totalNumObjectsAfterSpatialTest = stats.m_uiNumObjectsFiltered + stats.m_uiNumObjectsPassed;
      const nsUInt32 cacheThreshold = nsUInt32(nsMath::Max(cvar_SpatialQueriesCachingThreshold.GetValue(), 1));

      // 1.0 => all objects filtered, 0.0 => no object filtered by tags
      const float filteredRatio = float(double(stats.m_uiNumObjectsFiltered) / totalNumObjectsAfterSpatialTest);

      // Doesn't make sense to cache if there are only few objects in total or only few objects have been filtered
      if (totalNumObjectsAfterSpatialTest > cacheThreshold && filteredRatio > 0.1f)
      {
        UpdateCacheCandidate(queryParams.m_pIncludeTags, queryParams.m_pExcludeTags, pGrid->m_Category, filteredRatio);
      }
    }

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
    if (queryParams.m_pStats != nullptr)
    {
      queryParams.m_pStats->m_uiNumObjectsTested += stats.m_uiNumObjectsTested;
      queryParams.m_pStats->m_uiNumObjectsPassed += stats.m_uiNumObjectsPassed;
    }
#endif
  }
}

void nsSpatialSystem_RegularGrid::MigrateCachedGrid(nsUInt32 uiCandidateIndex)
{
  nsUInt32 uiTargetGridIndex = nsInvalidIndex;
  nsUInt32 uiSourceGridIndex = nsInvalidIndex;

  {
    NS_LOCK(m_CacheCandidatesMutex);

    auto& cacheCandidate = m_CacheCandidates[uiCandidateIndex];
    uiTargetGridIndex = cacheCandidate.m_uiGridIndex;
    uiSourceGridIndex = cacheCandidate.m_Category.m_uiValue;

    if (uiTargetGridIndex == nsInvalidIndex)
    {
      for (nsUInt32 i = m_Grids.GetCount() - 1; i >= MAX_NUM_REGULAR_GRIDS; --i)
      {
        if (m_Grids[i] == nullptr)
        {
          uiTargetGridIndex = i;
          break;
        }
      }

      NS_ASSERT_DEBUG(uiTargetGridIndex != nsInvalidIndex, "No free cached grid");
      cacheCandidate.m_uiGridIndex = uiTargetGridIndex;

      auto pGrid = NS_NEW(&m_Allocator, Grid, *this, cacheCandidate.m_Category);
      pGrid->m_IncludeTags = cacheCandidate.m_IncludeTags;
      pGrid->m_ExcludeTags = cacheCandidate.m_ExcludeTags;

      m_Grids[uiTargetGridIndex] = pGrid;

      m_uiFirstCachedGridIndex = nsMath::Min(m_uiFirstCachedGridIndex, uiTargetGridIndex);
    }
  }

  MigrateSpatialData(uiTargetGridIndex, uiSourceGridIndex);
}

void nsSpatialSystem_RegularGrid::MigrateSpatialData(nsUInt32 uiTargetGridIndex, nsUInt32 uiSourceGridIndex)
{
  auto& pTargetGrid = m_Grids[uiTargetGridIndex];
  if (pTargetGrid->CachingCompleted())
    return;

  auto& pSourceGrid = m_Grids[uiSourceGridIndex];

  constexpr nsUInt32 uiNumObjectsPerStep = 64;
  nsUInt32& uiLastMigrationIndex = pTargetGrid->m_uiLastMigrationIndex;
  const nsUInt32 uiSourceCount = pSourceGrid->m_CellDataMappings.GetCount();
  const nsUInt32 uiEndIndex = nsMath::Min(uiLastMigrationIndex + uiNumObjectsPerStep, uiSourceCount);

  for (nsUInt32 i = uiLastMigrationIndex; i < uiEndIndex; ++i)
  {
    if (pTargetGrid->MigrateSpatialDataFromOtherGrid(i, *pSourceGrid))
    {
      m_DataTable.GetValueUnchecked(i).m_uiGridBitmask |= NS_BIT(uiTargetGridIndex);
    }
  }

  uiLastMigrationIndex = (uiEndIndex == uiSourceCount) ? nsInvalidIndex : uiEndIndex;
}

void nsSpatialSystem_RegularGrid::RemoveCachedGrid(nsUInt32 uiCandidateIndex)
{
  nsUInt32 uiGridIndex;

  {
    NS_LOCK(m_CacheCandidatesMutex);

    auto& cacheCandidate = m_CacheCandidates[uiCandidateIndex];
    uiGridIndex = cacheCandidate.m_uiGridIndex;

    if (uiGridIndex == nsInvalidIndex)
      return;

    cacheCandidate.m_fQueryCount = 0.0f;
    cacheCandidate.m_fFilteredRatio = 0.0f;
    cacheCandidate.m_uiGridIndex = nsInvalidIndex;
  }

  m_Grids[uiGridIndex] = nullptr;
}

void nsSpatialSystem_RegularGrid::RemoveAllCachedGrids()
{
  NS_LOCK(m_CacheCandidatesMutex);

  for (nsUInt32 i = 0; i < m_CacheCandidates.GetCount(); ++i)
  {
    RemoveCachedGrid(i);
  }
}

void nsSpatialSystem_RegularGrid::UpdateCacheCandidate(const nsTagSet* pIncludeTags, const nsTagSet* pExcludeTags, nsSpatialData::Category category, float filteredRatio) const
{
  NS_LOCK(m_CacheCandidatesMutex);

  CacheCandidate* pCacheCandiate = nullptr;
  for (auto& cacheCandidate : m_CacheCandidates)
  {
    if (cacheCandidate.m_Category == category &&
        AreTagSetsEqual(cacheCandidate.m_IncludeTags, pIncludeTags) &&
        AreTagSetsEqual(cacheCandidate.m_ExcludeTags, pExcludeTags))
    {
      pCacheCandiate = &cacheCandidate;
      break;
    }
  }

  if (pCacheCandiate != nullptr)
  {
    pCacheCandiate->m_fQueryCount = nsMath::Min(pCacheCandiate->m_fQueryCount + 1.0f, 100.0f);
    pCacheCandiate->m_fFilteredRatio = nsMath::Max(pCacheCandiate->m_fFilteredRatio, filteredRatio);
  }
  else
  {
    auto& cacheCandidate = m_CacheCandidates.ExpandAndGetRef();
    cacheCandidate.m_Category = category;
    cacheCandidate.m_fQueryCount = 1;
    cacheCandidate.m_fFilteredRatio = filteredRatio;

    if (pIncludeTags != nullptr)
    {
      cacheCandidate.m_IncludeTags = *pIncludeTags;
    }
    if (pExcludeTags != nullptr)
    {
      cacheCandidate.m_ExcludeTags = *pExcludeTags;
    }
  }
}

NS_STATICLINK_FILE(Core, Core_World_Implementation_SpatialSystem_RegularGrid);
