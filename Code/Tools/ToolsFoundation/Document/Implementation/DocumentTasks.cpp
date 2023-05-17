#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Serialization/DdlSerializer.h>
#include <ToolsFoundation/Document/DocumentTasks.h>

wdSaveDocumentTask::wdSaveDocumentTask()
{
  ConfigureTask("wdSaveDocumentTask", wdTaskNesting::Maybe);
}

wdSaveDocumentTask::~wdSaveDocumentTask() = default;

void wdSaveDocumentTask::Execute()
{
  wdAbstractGraphDdlSerializer::WriteDocument(file, &headerGraph, &objectGraph, &typesGraph, false);

  if (file.Close() == WD_FAILURE)
  {
    m_document->m_LastSaveResult = wdStatus(wdFmt("Unable to open file '{0}' for writing!", m_document->m_sDocumentPath));
  }
  else
  {
    m_document->m_LastSaveResult = wdStatus(WD_SUCCESS);
  }
}

wdAfterSaveDocumentTask::wdAfterSaveDocumentTask()
{
  ConfigureTask("wdAfterSaveDocumentTask", wdTaskNesting::Maybe);
}

wdAfterSaveDocumentTask::~wdAfterSaveDocumentTask() = default;

void wdAfterSaveDocumentTask::Execute()
{
  if (m_document->m_LastSaveResult.Succeeded())
  {
    wdDocumentEvent e;
    e.m_pDocument = m_document;
    e.m_Type = wdDocumentEvent::Type::DocumentSaved;
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
