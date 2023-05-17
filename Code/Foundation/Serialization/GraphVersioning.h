#pragma once

/// \file

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Basics.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Serialization/GraphPatch.h>
#include <Foundation/Strings/HashedString.h>

class wdRTTI;
class wdAbstractObjectNode;
class wdAbstractObjectGraph;
class wdGraphPatch;
class wdGraphPatchContext;
class wdGraphVersioning;

/// \brief Tuple used for identifying patches and tracking patch progression.
struct wdVersionKey
{
  wdVersionKey() {}
  wdVersionKey(wdStringView sType, wdUInt32 uiTypeVersion)
  {
    m_sType.Assign(sType);
    m_uiTypeVersion = uiTypeVersion;
  }
  WD_DECLARE_POD_TYPE();
  wdHashedString m_sType;
  wdUInt32 m_uiTypeVersion;
};

/// \brief Hash helper class for wdVersionKey
struct wdGraphVersioningHash
{
  WD_FORCE_INLINE static wdUInt32 Hash(const wdVersionKey& a)
  {
    auto typeNameHash = a.m_sType.GetHash();
    wdUInt32 uiHash = wdHashingUtils::xxHash32(&typeNameHash, sizeof(typeNameHash));
    uiHash = wdHashingUtils::xxHash32(&a.m_uiTypeVersion, sizeof(a.m_uiTypeVersion), uiHash);
    return uiHash;
  }

  WD_ALWAYS_INLINE static bool Equal(const wdVersionKey& a, const wdVersionKey& b)
  {
    return a.m_sType == b.m_sType && a.m_uiTypeVersion == b.m_uiTypeVersion;
  }
};

/// \brief A class that overlaps wdReflectedTypeDescriptor with the properties needed for patching.
struct WD_FOUNDATION_DLL wdTypeVersionInfo
{
  const char* GetTypeName() const;
  void SetTypeName(const char* szName);
  const char* GetParentTypeName() const;
  void SetParentTypeName(const char* szName);

  wdHashedString m_sTypeName;
  wdHashedString m_sParentTypeName;
  wdUInt32 m_uiTypeVersion;
};
WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdTypeVersionInfo);

/// \brief Handles the patching of a node. Is passed into the patch
///  classes to provide utility functions and track the node's patching progress.
class WD_FOUNDATION_DLL wdGraphPatchContext
{
public:
  /// \brief Ensures that the base class named szType is at version uiTypeVersion.
  ///  If bForcePatch is set, the current version of the base class is reset back to force the execution
  ///  of this patch if necessary. This is mainly necessary for backwards compatibility with patches that
  ///  were written before the type information of all base classes was written to the doc.
  void PatchBaseClass(const char* szType, wdUInt32 uiTypeVersion, bool bForcePatch = false); // [tested]

  /// \brief Renames current class type.
  void RenameClass(const char* szTypeName); // [tested]

  /// \brief Renames current class type.
  void RenameClass(const char* szTypeName, wdUInt32 uiVersion);

  /// \brief Changes the base class hierarchy to the given one.
  void ChangeBaseClass(wdArrayPtr<wdVersionKey> baseClasses); // [tested]

private:
  friend class wdGraphVersioning;
  wdGraphPatchContext(wdGraphVersioning* pParent, wdAbstractObjectGraph* pGraph, wdAbstractObjectGraph* pTypesGraph);
  void Patch(wdAbstractObjectNode* pNode);
  void Patch(wdUInt32 uiBaseClassIndex, wdUInt32 uiTypeVersion, bool bForcePatch);
  void UpdateBaseClasses();

private:
  wdGraphVersioning* m_pParent = nullptr;
  wdAbstractObjectGraph* m_pGraph = nullptr;
  wdAbstractObjectNode* m_pNode = nullptr;
  wdDynamicArray<wdVersionKey> m_BaseClasses;
  wdUInt32 m_uiBaseClassIndex = 0;
  mutable wdHashTable<wdHashedString, wdTypeVersionInfo> m_TypeToInfo;
};

/// \brief Singleton that allows version patching of wdAbstractObjectGraph.
///
/// Patching is automatically executed of wdAbstractObjectGraph de-serialize functions.
class WD_FOUNDATION_DLL wdGraphVersioning
{
  WD_DECLARE_SINGLETON(wdGraphVersioning);

public:
  wdGraphVersioning();
  ~wdGraphVersioning();

  /// \brief Patches all nodes inside pGraph to the current version. pTypesGraph is the graph of serialized
  /// used types in pGraph at the time of saving. If not provided, any base class is assumed to be at max version.
  void PatchGraph(wdAbstractObjectGraph* pGraph, wdAbstractObjectGraph* pTypesGraph = nullptr);

private:
  friend class wdGraphPatchContext;

  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, GraphVersioning);

  void PluginEventHandler(const wdPluginEvent& EventData);
  void UpdatePatches();
  wdUInt32 GetMaxPatchVersion(const wdHashedString& sType) const;

  wdHashTable<wdHashedString, wdUInt32> m_MaxPatchVersion; ///< Max version the given type can be patched to.
  wdDynamicArray<const wdGraphPatch*> m_GraphPatches;
  wdHashTable<wdVersionKey, const wdGraphPatch*, wdGraphVersioningHash> m_NodePatches;
};
