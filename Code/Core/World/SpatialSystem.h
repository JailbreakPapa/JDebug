#pragma once

#include <Core/World/SpatialData.h>
#include <Foundation/Math/Frustum.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/SimdMath/SimdBBoxSphere.h>
#include <Foundation/Types/TagSet.h>

class NS_CORE_DLL nsSpatialSystem : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsSpatialSystem, nsReflectedClass);

public:
  nsSpatialSystem();
  ~nsSpatialSystem();

  virtual void StartNewFrame();

  /// \name Spatial Data Functions
  ///@{

  virtual nsSpatialDataHandle CreateSpatialData(const nsSimdBBoxSphere& bounds, nsGameObject* pObject, nsUInt32 uiCategoryBitmask, const nsTagSet& tags) = 0;
  virtual nsSpatialDataHandle CreateSpatialDataAlwaysVisible(nsGameObject* pObject, nsUInt32 uiCategoryBitmask, const nsTagSet& tags) = 0;

  virtual void DeleteSpatialData(const nsSpatialDataHandle& hData) = 0;

  virtual void UpdateSpatialDataBounds(const nsSpatialDataHandle& hData, const nsSimdBBoxSphere& bounds) = 0;
  virtual void UpdateSpatialDataObject(const nsSpatialDataHandle& hData, nsGameObject* pObject) = 0;

  ///@}
  /// \name Simple Queries
  ///@{

  using QueryCallback = nsDelegate<nsVisitorExecution::Enum(nsGameObject*)>;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  struct QueryStats
  {
    nsUInt32 m_uiTotalNumObjects = 0;  ///< The total number of spatial objects in this system.
    nsUInt32 m_uiNumObjectsTested = 0; ///< Number of objects tested for the query condition.
    nsUInt32 m_uiNumObjectsPassed = 0; ///< Number of objects that passed the query condition.
    nsTime m_TimeTaken;                ///< Time taken to execute the query
  };
#endif

  struct QueryParams
  {
    nsUInt32 m_uiCategoryBitmask = 0;
    const nsTagSet* m_pIncludeTags = nullptr;
    const nsTagSet* m_pExcludeTags = nullptr;
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
    QueryStats* m_pStats = nullptr;
#endif
  };

  virtual void FindObjectsInSphere(const nsBoundingSphere& sphere, const QueryParams& queryParams, nsDynamicArray<nsGameObject*>& out_objects) const;
  virtual void FindObjectsInSphere(const nsBoundingSphere& sphere, const QueryParams& queryParams, QueryCallback callback) const = 0;

  virtual void FindObjectsInBox(const nsBoundingBox& box, const QueryParams& queryParams, nsDynamicArray<nsGameObject*>& out_objects) const;
  virtual void FindObjectsInBox(const nsBoundingBox& box, const QueryParams& queryParams, QueryCallback callback) const = 0;

  ///@}
  /// \name Visibility Queries
  ///@{

  using IsOccludedFunc = nsDelegate<bool(const nsSimdBBox&)>;

  virtual void FindVisibleObjects(const nsFrustum& frustum, const QueryParams& queryParams, nsDynamicArray<const nsGameObject*>& out_objects, IsOccludedFunc isOccluded, nsVisibilityState visType) const = 0;

  /// \brief Retrieves a state describing how visible the object is.
  ///
  /// An object may be invisible, fully visible, or indirectly visible (through shadows or reflections).
  ///
  /// \param uiNumFramesBeforeInvisible Used to treat an object that was visible and just became invisible as visible for a few more frames.
  virtual nsVisibilityState GetVisibilityState(const nsSpatialDataHandle& hData, nsUInt32 uiNumFramesBeforeInvisible) const = 0;

  ///@}

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  virtual void GetInternalStats(nsStringBuilder& ref_sSb) const;
#endif

protected:
  nsProxyAllocator m_Allocator;

  nsUInt64 m_uiFrameCounter = 0;
};
