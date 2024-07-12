#pragma once

#include <Foundation/Serialization/RttiConverter.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Object/DocumentObjectMirror.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>


class nsTestDocumentObjectManager : public nsDocumentObjectManager
{
public:
  nsTestDocumentObjectManager();
  ~nsTestDocumentObjectManager();
};


class nsTestDocument : public nsDocument
{
  NS_ADD_DYNAMIC_REFLECTION(nsTestDocument, nsDocument);

public:
  nsTestDocument(nsStringView sDocumentPath, bool bUseIPCObjectMirror = false);
  ~nsTestDocument();

  virtual void InitializeAfterLoading(bool bFirstTimeCreation) override;
  void ApplyNativePropertyChangesToObjectManager(nsDocumentObject* pObject);
  virtual nsDocumentInfo* CreateDocumentInfo() override;

  nsDocumentObjectMirror m_ObjectMirror;
  nsRttiConverterContext m_Context;



private:
  bool m_bUseIPCObjectMirror;
};
