#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Serialization/DdlSerializer.h>
#include <ToolsFoundation/Document/DocumentTasks.h>

nsSaveDocumentTask::nsSaveDocumentTask()
{
  ConfigureTask("nsSaveDocumentTask", nsTaskNesting::Maybe);
}

nsSaveDocumentTask::~nsSaveDocumentTask() = default;

void nsSaveDocumentTask::Execute()
{
  nsAbstractGraphDdlSerializer::WriteDocument(file, &headerGraph, &objectGraph, &typesGraph, false);

  if (file.Close() == NS_FAILURE)
  {
    m_document->m_LastSaveResult = nsStatus(nsFmt("Unable to open file '{0}' for writing!", m_document->m_sDocumentPath));
  }
  else
  {
    m_document->m_LastSaveResult = nsStatus(NS_SUCCESS);
  }
}

nsAfterSaveDocumentTask::nsAfterSaveDocumentTask()
{
  ConfigureTask("nsAfterSaveDocumentTask", nsTaskNesting::Maybe);
}

nsAfterSaveDocumentTask::~nsAfterSaveDocumentTask() = default;

void nsAfterSaveDocumentTask::Execute()
{
  if (m_document->m_LastSaveResult.Succeeded())
  {
    nsDocumentEvent e;
    e.m_pDocument = m_document;
    e.m_Type = nsDocumentEvent::Type::DocumentSaved;
    m_document->m_EventsOne.Broadcast(e);
    m_document->s_EventsAny.Broadcast(e);

    m_document->SetModified(false);

    // after saving once, this information is pointless
    m_document->m_uiUnknownObjectTypeInstances = 0;
    m_document->m_UnknownObjectTypes.Clear();
  }

  if (m_document->m_LastSaveResult.Succeeded())
  {
    m_document->InternalAfterSaveDocument();
  }
  if (m_callback.IsValid())
  {
    m_callback(m_document, m_document->m_LastSaveResult);
  }
  m_document->m_ActiveSaveTask.Invalidate();
}
