#pragma once

#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/UniquePtr.h>

#include <Core/World/CoordinateSystem.h>
#include <Core/World/SpatialSystem.h>

class nsTimeStepSmoothing;

/// \brief Describes the initial state of a world.
struct nsWorldDesc
{
  NS_DECLARE_POD_TYPE();

  nsWorldDesc(nsStringView sWorldName) { m_sName.Assign(sWorldName); }

  nsHashedString m_sName;
  nsUInt64 m_uiRandomNumberGeneratorSeed = 0;

  nsUniquePtr<nsSpatialSystem> m_pSpatialSystem;
  bool m_bAutoCreateSpatialSystem = true;                ///< automatically create a default spatial system if none is set

  nsSharedPtr<nsCoordinateSystemProvider> m_pCoordinateSystemProvider;
  nsUniquePtr<nsTimeStepSmoothing> m_pTimeStepSmoothing; ///< if nullptr, nsDefaultTimeStepSmoothing will be used

  bool m_bReportErrorWhenStaticObjectMoves = true;

  nsTime m_MaxComponentInitializationTimePerFrame = nsTime::MakeFromHours(10000); // max time to spend on component initialization per frame
};
