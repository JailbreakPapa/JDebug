#pragma once

#include <Core/CoreDLL.h>
#include <Core/World/SpatialData.h>
#include <Foundation/Communication/Message.h>
#include <Foundation/Math/BoundingBoxSphere.h>

struct NS_CORE_DLL nsMsgUpdateLocalBounds : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgUpdateLocalBounds, nsMessage);

  NS_ALWAYS_INLINE void AddBounds(const nsBoundingBoxSphere& bounds, nsSpatialData::Category category)
  {
    m_ResultingLocalBounds.ExpandToInclude(bounds);
    m_uiSpatialDataCategoryBitmask |= category.GetBitmask();
  }

  ///\brief Enforces the object to be always visible. Note that you can't set this flag to false again,
  ///  because the same message is sent to multiple components and should accumulate the bounds.
  NS_ALWAYS_INLINE void SetAlwaysVisible(nsSpatialData::Category category)
  {
    m_bAlwaysVisible = true;
    m_uiSpatialDataCategoryBitmask |= category.GetBitmask();
  }

private:
  friend class nsGameObject;

  nsBoundingBoxSphere m_ResultingLocalBounds;
  nsUInt32 m_uiSpatialDataCategoryBitmask = 0;
  bool m_bAlwaysVisible = false;
};
