#pragma once

#include <Core/World/Declarations.h>
#include <Foundation/Math/Quat.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/TagSet.h>
#include <Foundation/Types/Uuid.h>

/// \brief Describes the initial state of a game object.
struct NS_CORE_DLL nsGameObjectDesc
{
  NS_DECLARE_POD_TYPE();

  bool m_bActiveFlag = true;                       ///< Whether the object should have the 'active flag' set. See nsGameObject::SetActiveFlag().
  bool m_bDynamic = false;                         ///< Whether the object should start out as 'dynamic'. See nsGameObject::MakeDynamic().
  nsUInt16 m_uiTeamID = 0;                         ///< See nsGameObject::GetTeamID().

  nsHashedString m_sName;                          ///< See nsGameObject::SetName().
  nsGameObjectHandle m_hParent;                    ///< An optional parent object to attach this object to as a child.

  nsVec3 m_LocalPosition = nsVec3::MakeZero();     ///< The local position relative to the parent (or the world)
  nsQuat m_LocalRotation = nsQuat::MakeIdentity(); ///< The local rotation relative to the parent (or the world)
  nsVec3 m_LocalScaling = nsVec3(1);               ///< The local scaling relative to the parent (or the world)
  float m_LocalUniformScaling = 1.0f;              ///< An additional local uniform scaling relative to the parent (or the world)
  nsTagSet m_Tags;                                 ///< See nsGameObject::GetTags()
  nsUInt32 m_uiStableRandomSeed = 0xFFFFFFFF;      ///< 0 means the game object gets a random value assigned, 0xFFFFFFFF means that if the object has a parent, the value will be derived deterministically from that one's seed, otherwise it gets a random value, any other value will be used directly
};
