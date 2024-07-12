#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Serialization/GraphVersioning.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Utilities/EnumerableClass.h>

class nsRTTI;
class nsAbstractObjectNode;
class nsAbstractObjectGraph;
class nsGraphVersioning;
class nsGraphPatchContext;

/// \brief Patch base class for nsAbstractObjectGraph patches.
///
/// Create static instance of derived class to automatically patch graphs on load.
class NS_FOUNDATION_DLL nsGraphPatch : public nsEnumerable<nsGraphPatch>
{
public:
  enum class PatchType : nsUInt8
  {
    NodePatch,  ///< Patch applies to a node of a certain type and version
    GraphPatch, ///< Patch applies to an entire graph without any restrictions.
  };

  /// \brief Constructor. pType is the type to patch. uiTypeVersion is the version to patch to.
  ///
  /// Patches are executed in order from version uiTypeVersion-1 to uiTypeVersion. If no patch exists for previous versions
  /// the input to the patch function can potentially be of a lower version than uiTypeVersion-1.
  /// If type is PatchType::NodePatch, the patch is executed for each instance of the given type.
  /// If type is PatchType::GraphPatch, the patch is executed once for the entire graph. In this case
  /// szType and uiTypeVersion are ignored and the patch function has to figure out what to do by itself.
  nsGraphPatch(const char* szType, nsUInt32 uiTypeVersion, PatchType type = PatchType::NodePatch);

  /// \brief Patch function. If type == PatchType::NodePatch, the implementation needs to patch pNode in pGraph to m_uiTypeVersion.
  ///  If type == PatchType::GraphPatch, pNode will be nullptr and the implementation has to figure out what to patch in pGraph on its own.
  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const = 0;
  /// \brief Returns the type to patch.
  const char* GetType() const;
  /// \brief Returns the type version to patch to.
  nsUInt32 GetTypeVersion() const;
  PatchType GetPatchType() const;

  NS_DECLARE_ENUMERABLE_CLASS(nsGraphPatch);

private:
  const char* m_szType = nullptr;
  nsUInt32 m_uiTypeVersion;
  PatchType m_PatchType;
};
