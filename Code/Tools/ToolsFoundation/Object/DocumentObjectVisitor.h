#pragma once

#include <Foundation/Strings/String.h>
#include <Foundation/Types/Delegate.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class wdDocumentObjectManager;
class wdDocumentObject;

/// \brief Implements visitor pattern for content of the document object manager.
class WD_TOOLSFOUNDATION_DLL wdDocumentObjectVisitor
{
public:
  /// \brief Constructor
  ///
  /// \param pManager
  ///   Manager that will be iterated through.
  /// \param szChildrenProperty
  ///   Name of the property that is used for finding children on an object.
  /// \param szRootProperty
  ///   Same as szChildrenProperty, but for the root object of the document.
  wdDocumentObjectVisitor(
    const wdDocumentObjectManager* pManager, const char* szChildrenProperty = "Children", const char* szRootProperty = "Children");

  typedef wdDelegate<bool(const wdDocumentObject*)> VisitorFunction;
  /// \brief Executes depth first traversal starting at the given node.
  ///
  /// \param pObject
  ///   Object to start traversal at.
  /// \param bVisitStart
  ///   If true, function will be executed for the start object as well.
  /// \param function
  ///   Functions executed for each visited object. Should true if the object's children should be traversed.
  void Visit(const wdDocumentObject* pObject, bool bVisitStart, VisitorFunction function);

private:
  void TraverseChildren(const wdDocumentObject* pObject, const char* szProperty, VisitorFunction& function);

  const wdDocumentObjectManager* m_pManager = nullptr;
  wdString m_sChildrenProperty;
  wdString m_sRootProperty;
};
