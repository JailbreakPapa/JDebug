#include <ToolsFoundationTest/ToolsFoundationTestPCH.h>

#include <ToolsFoundationTest/Object/TestObjectManager.h>

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTestDocument, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsTestDocumentObjectManager::nsTestDocumentObjectManager() = default;
nsTestDocumentObjectManager::~nsTestDocumentObjectManager() = default;

nsTestDocument::nsTestDocument(nsStringView sDocumentPath, bool bUseIPCObjectMirror /*= false*/)
  : nsDocument(sDocumentPath, NS_DEFAULT_NEW(nsTestDocumentObjectManager))
  , m_bUseIPCObjectMirror(bUseIPCObjectMirror)
{
}

nsTestDocument::~nsTestDocument()
{
  if (m_bUseIPCObjectMirror)
  {
    m_ObjectMirror.Clear();
    m_ObjectMirror.DeInit();
  }
}

void nsTestDocument::InitializeAfterLoading(bool bFirstTimeCreation)
{
  SUPER::InitializeAfterLoading(bFirstTimeCreation);

  if (m_bUseIPCObjectMirror)
  {
    m_ObjectMirror.InitSender(GetObjectManager());
    m_ObjectMirror.InitReceiver(&m_Context);
    m_ObjectMirror.SendDocument();
  }
}

void nsTestDocument::ApplyNativePropertyChangesToObjectManager(nsDocumentObject* pObject)
{
  // Create native object graph
  nsAbstractObjectGraph graph;
  nsAbstractObjectNode* pRootNode = nullptr;
  {
    nsRttiConverterWriter rttiConverter(&graph, &m_Context, true, true);
    pRootNode = rttiConverter.AddObjectToGraph(pObject->GetType(), m_ObjectMirror.GetNativeObjectPointer(pObject), "Object");
  }

  // Create object manager graph
  nsAbstractObjectGraph origGraph;
  nsAbstractObjectNode* pOrigRootNode = nullptr;
  {
    nsDocumentObjectConverterWriter writer(&origGraph, GetObjectManager());
    pOrigRootNode = writer.AddObjectToGraph(pObject);
  }

  // Remap native guids so they match the object manager (stuff like embedded classes will not have a guid on the native side).
  graph.ReMapNodeGuidsToMatchGraph(pRootNode, origGraph, pOrigRootNode);
  nsDeque<nsAbstractGraphDiffOperation> diffResult;

  graph.CreateDiffWithBaseGraph(origGraph, diffResult);

  // As we messed up the native side the object mirror is no longer synced and needs to be destroyed.
  m_ObjectMirror.Clear();
  m_ObjectMirror.DeInit();

  // Apply diff while object mirror is down.
  GetObjectAccessor()->StartTransaction("Apply Native Property Changes to Object");
  nsDocumentObjectConverterReader::ApplyDiffToObject(GetObjectAccessor(), pObject, diffResult);
  GetObjectAccessor()->FinishTransaction();

  // Restart mirror from scratch.
  m_ObjectMirror.InitSender(GetObjectManager());
  m_ObjectMirror.InitReceiver(&m_Context);
  m_ObjectMirror.SendDocument();
}

nsDocumentInfo* nsTestDocument::CreateDocumentInfo()
{
  return NS_DEFAULT_NEW(nsDocumentInfo);
}
