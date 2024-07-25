#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/HybridArray.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

struct NS_TOOLSFOUNDATION_DLL nsActiveDocumentChange
{
  const nsDocument* m_pOldDocument;
  const nsDocument* m_pNewDocument;
};

/// \brief Tracks existing and active nsDocument.
///
/// While the IDocumentManager manages documents of a certain context,
/// this class simply keeps track of the overall number of documents and the currently active one.
class NS_TOOLSFOUNDATION_DLL nsDocumentRegistry
{
public:
  static bool RegisterDocument(const nsDocument* pDocument);
  static bool UnregisterDocument(const nsDocument* pDocument);

  static nsArrayPtr<const nsDocument*> GetDocuments() { return s_Documents; }

  static void SetActiveDocument(const nsDocument* pDocument);
  static const nsDocument* GetActiveDocument();

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(Core, DocumentRegistry);

  static void Startup();
  static void Shutdown();

public:
  // static nsEvent<nsDocumentChange&> m_DocumentAddedEvent;
  // static nsEvent<nsDocumentChange&> m_DocumentRemovedEvent;
  static nsEvent<nsActiveDocumentChange&> m_ActiveDocumentChanged;

private:
  static nsHybridArray<const nsDocument*, 16> s_Documents;
  static nsDocument* s_pActiveDocument;
};
