#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/HybridArray.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

struct WD_TOOLSFOUNDATION_DLL wdActiveDocumentChange
{
  const wdDocument* m_pOldDocument;
  const wdDocument* m_pNewDocument;
};

/// \brief Tracks existing and active wdDocument.
///
/// While the IDocumentManager manages documents of a certain context,
/// this class simply keeps track of the overall number of documents and the currently active one.
class WD_TOOLSFOUNDATION_DLL wdDocumentRegistry
{
public:
  static bool RegisterDocument(const wdDocument* pDocument);
  static bool UnregisterDocument(const wdDocument* pDocument);

  static wdArrayPtr<const wdDocument*> GetDocuments() { return s_Documents; }

  static void SetActiveDocument(const wdDocument* pDocument);
  static const wdDocument* GetActiveDocument();

private:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(Core, DocumentRegistry);

  static void Startup();
  static void Shutdown();

public:
  // static wdEvent<wdDocumentChange&> m_DocumentAddedEvent;
  // static wdEvent<wdDocumentChange&> m_DocumentRemovedEvent;
  static wdEvent<wdActiveDocumentChange&> m_ActiveDocumentChanged;

private:
  static wdHybridArray<const wdDocument*, 16> s_Documents;
  static wdDocument* s_pActiveDocument;
};
