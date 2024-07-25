#pragma once

#include <Foundation/Strings/String.h>
#include <Foundation/Types/Delegate.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class nsDocumentObjectManager;
class nsDocumentObject;

/// \brief Implements visitor pattern for content of the document object manager.
class NS_TOOLSFOUNDATION_DLL nsDocumentObjectVisitor
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
  nsDocumentObjectVisitor(
    const nsDocumentObjectManager* pManager, nsStringView sChildrenProperty = "Children", nsStringView sRootProperty = "Children");

  using VisitorFunction = nsDelegate<bool(const nsDocumentObject*)>;
  /// \brief Executes depth first traversal starting at the given node.
  ///
  /// \param pObject
  ///   Object to start traversal at.
  /// \param bVisitStart
  ///   If true, function will be executed for the start object as well.
  /// \param function
  ///   Functions executed for each visited object. Should true if the object's children should be traversed.
  void Visit(const nsDocumentObject* pObject, bool bVisitStart, VisitorFunction function);

private:
  void TraverseChildren(const nsDocumentObject* pObject, nsStringView sProperty, VisitorFunction& function);

  const nsDocumentObjectManager* m_pManager = nullptr;
  nsString m_sChildrenProperty;
  nsString m_sRootProperty;
};
