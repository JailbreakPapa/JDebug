#pragma once

#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Threading/TaskSystem.h>
#include <Foundation/Types/Status.h>
#include <ToolsFoundation/Document/Document.h>

class wdSaveDocumentTask final : public wdTask
{
public:
  wdSaveDocumentTask();
  ~wdSaveDocumentTask();

  wdDeferredFileWriter file;
  wdAbstractObjectGraph headerGraph;
  wdAbstractObjectGraph objectGraph;
  wdAbstractObjectGraph typesGraph;
  wdDocument* m_document = nullptr;

  virtual void Execute() override;
};

class wdAfterSaveDocumentTask final : public wdTask
{
public:
  wdAfterSaveDocumentTask();
  ~wdAfterSaveDocumentTask();

  wdDocument* m_document = nullptr;
  wdDocument::AfterSaveCallback m_callback;

  virtual void Execute() override;
};
