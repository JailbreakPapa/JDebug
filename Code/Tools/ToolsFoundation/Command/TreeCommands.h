#pragma once

#include <ToolsFoundation/Command/Command.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class WD_TOOLSFOUNDATION_DLL wdAddObjectCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdAddObjectCommand, wdCommand);

public:
  wdAddObjectCommand();

public: // Properties
  void SetType(const char* szType);
  const char* GetType() const;

  const wdRTTI* m_pType;
  wdUuid m_Parent;
  wdString m_sParentProperty;
  wdVariant m_Index;
  wdUuid m_NewObjectGuid; ///< This is optional. If not filled out, a new guid is assigned automatically.

private:
  virtual bool HasReturnValues() const override { return true; }
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override;

private:
  wdDocumentObject* m_pObject;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


class WD_TOOLSFOUNDATION_DLL wdPasteObjectsCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdPasteObjectsCommand, wdCommand);

public:
  wdPasteObjectsCommand();

public: // Properties
  wdUuid m_Parent;
  wdString m_sGraphTextFormat;
  wdString m_sMimeType;
  bool m_bAllowPickedPosition = true;

private:
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override;

private:
  struct PastedObject
  {
    wdDocumentObject* m_pObject;
    wdDocumentObject* m_pParent;
    wdString m_sParentProperty;
    wdVariant m_Index;
  };

  wdHybridArray<PastedObject, 4> m_PastedObjects;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class WD_TOOLSFOUNDATION_DLL wdInstantiatePrefabCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdInstantiatePrefabCommand, wdCommand);

public:
  wdInstantiatePrefabCommand();

public: // Properties
  wdUuid m_Parent;
  wdInt32 m_Index = -1;
  wdUuid m_CreateFromPrefab;
  wdUuid m_RemapGuid;
  wdString m_sBasePrefabGraph;
  wdString m_sObjectGraph;
  wdUuid m_CreatedRootObject;
  bool m_bAllowPickedPosition;

private:
  virtual bool HasReturnValues() const override { return true; }
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override;

private:
  struct PastedObject
  {
    wdDocumentObject* m_pObject;
    wdDocumentObject* m_pParent;
    wdString m_sParentProperty;
    wdVariant m_Index;
  };

  // at the moment this array always only holds a single item, the group node for the prefab
  wdHybridArray<PastedObject, 4> m_PastedObjects;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class WD_TOOLSFOUNDATION_DLL wdUnlinkPrefabCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdUnlinkPrefabCommand, wdCommand);

public:
  wdUnlinkPrefabCommand() {}

  wdUuid m_Object;

private:
  virtual bool HasReturnValues() const override { return false; }
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  wdUuid m_OldCreateFromPrefab;
  wdUuid m_OldRemapGuid;
  wdString m_sOldGraphTextFormat;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class WD_TOOLSFOUNDATION_DLL wdRemoveObjectCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdRemoveObjectCommand, wdCommand);

public:
  wdRemoveObjectCommand();

public: // Properties
  wdUuid m_Object;

private:
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override;

private:
  wdDocumentObject* m_pParent;
  wdString m_sParentProperty;
  wdVariant m_Index;
  wdDocumentObject* m_pObject;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class WD_TOOLSFOUNDATION_DLL wdMoveObjectCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdMoveObjectCommand, wdCommand);

public:
  wdMoveObjectCommand();

public: // Properties
  wdUuid m_Object;
  wdUuid m_NewParent;
  wdString m_sParentProperty;
  wdVariant m_Index;

private:
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  wdDocumentObject* m_pObject;
  wdDocumentObject* m_pOldParent;
  wdDocumentObject* m_pNewParent;
  wdString m_sOldParentProperty;
  wdVariant m_OldIndex;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class WD_TOOLSFOUNDATION_DLL wdSetObjectPropertyCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdSetObjectPropertyCommand, wdCommand);

public:
  wdSetObjectPropertyCommand();

public: // Properties
  wdUuid m_Object;
  wdVariant m_NewValue;
  wdVariant m_Index;
  wdString m_sProperty;

private:
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  wdDocumentObject* m_pObject;
  wdVariant m_OldValue;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class WD_TOOLSFOUNDATION_DLL wdResizeAndSetObjectPropertyCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdResizeAndSetObjectPropertyCommand, wdCommand);

public:
  wdResizeAndSetObjectPropertyCommand();

public: // Properties
  wdUuid m_Object;
  wdVariant m_NewValue;
  wdVariant m_Index;
  wdString m_sProperty;

private:
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override { return wdStatus(WD_SUCCESS); }
  virtual void CleanupInternal(CommandState state) override {}

  wdDocumentObject* m_pObject;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class WD_TOOLSFOUNDATION_DLL wdInsertObjectPropertyCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdInsertObjectPropertyCommand, wdCommand);

public:
  wdInsertObjectPropertyCommand();

public: // Properties
  wdUuid m_Object;
  wdVariant m_NewValue;
  wdVariant m_Index;
  wdString m_sProperty;

private:
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  wdDocumentObject* m_pObject;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class WD_TOOLSFOUNDATION_DLL wdRemoveObjectPropertyCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdRemoveObjectPropertyCommand, wdCommand);

public:
  wdRemoveObjectPropertyCommand();

public: // Properties
  wdUuid m_Object;
  wdVariant m_Index;
  wdString m_sProperty;

private:
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  wdDocumentObject* m_pObject;
  wdVariant m_OldValue;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class WD_TOOLSFOUNDATION_DLL wdMoveObjectPropertyCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdMoveObjectPropertyCommand, wdCommand);

public:
  wdMoveObjectPropertyCommand();

public: // Properties
  wdUuid m_Object;
  wdVariant m_OldIndex;
  wdVariant m_NewIndex;
  wdString m_sProperty;

private:
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  wdDocumentObject* m_pObject;
};
