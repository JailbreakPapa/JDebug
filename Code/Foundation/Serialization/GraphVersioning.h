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

class nsRTTI;
class nsAbstractObjectNode;
class nsAbstractObjectGraph;
class nsGraphPatch;
class nsGraphPatchContext;
class nsGraphVersioning;

/// \brief Tuple used for identifying patches and tracking patch progression.
struct nsVersionKey
{
  nsVersionKey() = default;
  nsVersionKey(nsStringView sType, nsUInt32 uiTypeVersion)
  {
    m_sType.Assign(sType);
    m_uiTypeVersion = uiTypeVersion;
  }
  NS_DECLARE_POD_TYPE();
  nsHashedString m_sType;
  nsUInt32 m_uiTypeVersion;
};

/// \brief Hash helper class for nsVersionKey
struct nsGraphVersioningHash
{
  NS_FORCE_INLINE static nsUInt32 Hash(const nsVersionKey& a)
  {
    auto typeNameHash = a.m_sType.GetHash();
    nsUInt32 uiHash = nsHashingUtils::xxHash32(&typeNameHash, sizeof(typeNameHash));
    uiHash = nsHashingUtils::xxHash32(&a.m_uiTypeVersion, sizeof(a.m_uiTypeVersion), uiHash);
    return uiHash;
  }

  NS_ALWAYS_INLINE static bool Equal(const nsVersionKey& a, const nsVersionKey& b)
  {
    return a.m_sType == b.m_sType && a.m_uiTypeVersion == b.m_uiTypeVersion;
  }
};

/// \brief A class that overlaps nsReflectedTypeDescriptor with the properties needed for patching.
struct NS_FOUNDATION_DLL nsTypeVersionInfo
{
  const char* GetTypeName() const;
  void SetTypeName(const char* szName);
  const char* GetParentTypeName() const;
  void SetParentTypeName(const char* szName);

  nsHashedString m_sTypeName;
  nsHashedString m_sParentTypeName;
  nsUInt32 m_uiTypeVersion;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsTypeVersionInfo);

/// \brief Handles the patching of a node. Is passed into the patch
///  classes to provide utility functions and track the node's patching progress.
class NS_FOUNDATION_DLL nsGraphPatchContext
{
public:
  /// \brief Ensures that the base class named szType is at version uiTypeVersion.
  ///  If bForcePatch is set, the current version of the base class is reset back to force the execution
  ///  of this patch if necessary. This is mainly necessary for backwards compatibility with patches that
  ///  were written before the type information of all base classes was written to the doc.
  void PatchBaseClass(const char* szType, nsUInt32 uiTypeVersion, bool bForcePatch = false); // [tested]

  /// \brief Renames current class type.
  void RenameClass(const char* szTypeName); // [tested]

  /// \brief Renames current class type.
  void RenameClass(const char* szTypeName, nsUInt32 uiVersion);

  /// \brief Changes the base class hierarchy to the given one.
  void ChangeBaseClass(nsArrayPtr<nsVersionKey> baseClasses); // [tested]

private:
  friend class nsGraphVersioning;
  nsGraphPatchContext(nsGraphVersioning* pParent, nsAbstractObjectGraph* pGraph, nsAbstractObjectGraph* pTypesGraph);
  void Patch(nsAbstractObjectNode* pNode);
  void Patch(nsUInt32 uiBaseClassIndex, nsUInt32 uiTypeVersion, bool bForcePatch);
  void UpdateBaseClasses();

private:
  nsGraphVersioning* m_pParent = nullptr;
  nsAbstractObjectGraph* m_pGraph = nullptr;
  nsAbstractObjectNode* m_pNode = nullptr;
  nsDynamicArray<nsVersionKey> m_BaseClasses;
  nsUInt32 m_uiBaseClassIndex = 0;
  mutable nsHashTable<nsHashedString, nsTypeVersionInfo> m_TypeToInfo;
};

/// \brief Singleton that allows version patching of nsAbstractObjectGraph.
///
/// Patching is automatically executed of nsAbstractObjectGraph de-serialize functions.
class NS_FOUNDATION_DLL nsGraphVersioning
{
  NS_DECLARE_SINGLETON(nsGraphVersioning);

public:
  nsGraphVersioning();
  ~nsGraphVersioning();

  /// \brief Patches all nodes inside pGraph to the current version. pTypesGraph is the graph of serialized
  /// used types in pGraph at the time of saving. If not provided, any base class is assumed to be at max version.
  void PatchGraph(nsAbstractObjectGraph* pGraph, nsAbstractObjectGraph* pTypesGraph = nullptr);

private:
  friend class nsGraphPatchContext;

  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, GraphVersioning);

  void PluginEventHandler(const nsPluginEvent& EventData);
  void UpdatePatches();
  nsUInt32 GetMaxPatchVersion(const nsHashedString& sType) const;

  nsHashTable<nsHashedString, nsUInt32> m_MaxPatchVersion; ///< Max version the given type can be patched to.
  nsDynamicArray<const nsGraphPatch*> m_GraphPatches;
  nsHashTable<nsVersionKey, const nsGraphPatch*, nsGraphVersioningHash> m_NodePatches;
};
