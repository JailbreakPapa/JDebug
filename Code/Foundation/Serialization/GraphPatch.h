#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Serialization/GraphVersioning.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Utilities/EnumerableClass.h>

class wdRTTI;
class wdAbstractObjectNode;
class wdAbstractObjectGraph;
class wdGraphVersioning;
class wdGraphPatchContext;

/// \brief Patch base class for wdAbstractObjectGraph patches.
///
/// Create static instance of derived class to automatically patch graphs on load.
class WD_FOUNDATION_DLL wdGraphPatch : public wdEnumerable<wdGraphPatch>
{
public:
  enum class PatchType : wdUInt8
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
  wdGraphPatch(const char* szType, wdUInt32 uiTypeVersion, PatchType type = PatchType::NodePatch);

  /// \brief Patch function. If type == PatchType::NodePatch, the implementation needs to patch pNode in pGraph to m_uiTypeVersion.
  ///  If type == PatchType::GraphPatch, pNode will be nullptr and the implementation has to figure out waht to patch in pGraph on its own.
  virtual void Patch(wdGraphPatchContext& ref_context, wdAbstractObjectGraph* pGraph, wdAbstractObjectNode* pNode) const = 0;
  /// \brief Returns the type to patch.
  const char* GetType() const;
  /// \brief Returns the type version to patch to.
  wdUInt32 GetTypeVersion() const;
  PatchType GetPatchType() const;

  WD_DECLARE_ENUMERABLE_CLASS(wdGraphPatch);

private:
  const char* m_szType = nullptr;
  wdUInt32 m_uiTypeVersion;
  PatchType m_PatchType;
};
