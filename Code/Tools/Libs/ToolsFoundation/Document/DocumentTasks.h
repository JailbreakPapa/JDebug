#pragma once

#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Threading/TaskSystem.h>
#include <Foundation/Types/Status.h>
#include <ToolsFoundation/Document/Document.h>

class nsSaveDocumentTask final : public nsTask
{
public:
  nsSaveDocumentTask();
  ~nsSaveDocumentTask();

  nsDeferredFileWriter file;
  nsAbstractObjectGraph headerGraph;
  nsAbstractObjectGraph objectGraph;
  nsAbstractObjectGraph typesGraph;
  nsDocument* m_document = nullptr;

  virtual void Execute() override;
};

class nsAfterSaveDocumentTask final : public nsTask
{
public:
  nsAfterSaveDocumentTask();
  ~nsAfterSaveDocumentTask();

  nsDocument* m_document = nullptr;
  nsDocument::AfterSaveCallback m_callback;

  virtual void Execute() override;
};
