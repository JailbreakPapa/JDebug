#pragma once

#include <ToolsFoundation/Command/Command.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_TOOLSFOUNDATION_DLL nsAddObjectCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsAddObjectCommand, nsCommand);

public:
  nsAddObjectCommand();

public: // Properties
  void SetType(nsStringView sType);
  nsStringView GetType() const;

  const nsRTTI* m_pType = nullptr;
  nsUuid m_Parent;
  nsString m_sParentProperty;
  nsVariant m_Index;
  nsUuid m_NewObjectGuid; ///< This is optional. If not filled out, a new guid is assigned automatically.

private:
  virtual bool HasReturnValues() const override { return true; }
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override;

private:
  nsDocumentObject* m_pObject = nullptr;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


class NS_TOOLSFOUNDATION_DLL nsPasteObjectsCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsPasteObjectsCommand, nsCommand);

public:
  nsPasteObjectsCommand();

public: // Properties
  nsUuid m_Parent;
  nsString m_sGraphTextFormat;
  nsString m_sMimeType;
  bool m_bAllowPickedPosition = true;

private:
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override;

private:
  struct PastedObject
  {
    nsDocumentObject* m_pObject;
    nsDocumentObject* m_pParent;
    nsString m_sParentProperty;
    nsVariant m_Index;
  };

  nsHybridArray<PastedObject, 4> m_PastedObjects;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_TOOLSFOUNDATION_DLL nsInstantiatePrefabCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsInstantiatePrefabCommand, nsCommand);

public:
  nsInstantiatePrefabCommand();

public: // Properties
  nsUuid m_Parent;
  nsInt32 m_Index = -1;
  nsUuid m_CreateFromPrefab;
  nsUuid m_RemapGuid;
  nsString m_sBasePrefabGraph;
  nsString m_sObjectGraph;
  nsUuid m_CreatedRootObject;
  bool m_bAllowPickedPosition;

private:
  virtual bool HasReturnValues() const override { return true; }
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override;

private:
  struct PastedObject
  {
    nsDocumentObject* m_pObject;
    nsDocumentObject* m_pParent;
    nsString m_sParentProperty;
    nsVariant m_Index;
  };

  // at the moment this array always only holds a single item, the group node for the prefab
  nsHybridArray<PastedObject, 4> m_PastedObjects;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_TOOLSFOUNDATION_DLL nsUnlinkPrefabCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsUnlinkPrefabCommand, nsCommand);

public:
  nsUnlinkPrefabCommand() = default;

  nsUuid m_Object;

private:
  virtual bool HasReturnValues() const override { return false; }
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  nsUuid m_OldCreateFromPrefab;
  nsUuid m_OldRemapGuid;
  nsString m_sOldGraphTextFormat;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_TOOLSFOUNDATION_DLL nsRemoveObjectCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsRemoveObjectCommand, nsCommand);

public:
  nsRemoveObjectCommand();

public: // Properties
  nsUuid m_Object;

private:
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override;

private:
  nsDocumentObject* m_pParent = nullptr;
  nsString m_sParentProperty;
  nsVariant m_Index;
  nsDocumentObject* m_pObject = nullptr;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_TOOLSFOUNDATION_DLL nsMoveObjectCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsMoveObjectCommand, nsCommand);

public:
  nsMoveObjectCommand();

public: // Properties
  nsUuid m_Object;
  nsUuid m_NewParent;
  nsString m_sParentProperty;
  nsVariant m_Index;

private:
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  nsDocumentObject* m_pObject;
  nsDocumentObject* m_pOldParent;
  nsDocumentObject* m_pNewParent;
  nsString m_sOldParentProperty;
  nsVariant m_OldIndex;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_TOOLSFOUNDATION_DLL nsSetObjectPropertyCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsSetObjectPropertyCommand, nsCommand);

public:
  nsSetObjectPropertyCommand();

public: // Properties
  nsUuid m_Object;
  nsVariant m_NewValue;
  nsVariant m_Index;
  nsString m_sProperty;

private:
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  nsDocumentObject* m_pObject;
  nsVariant m_OldValue;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_TOOLSFOUNDATION_DLL nsResizeAndSetObjectPropertyCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsResizeAndSetObjectPropertyCommand, nsCommand);

public:
  nsResizeAndSetObjectPropertyCommand();

public: // Properties
  nsUuid m_Object;
  nsVariant m_NewValue;
  nsVariant m_Index;
  nsString m_sProperty;

private:
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override { return nsStatus(NS_SUCCESS); }
  virtual void CleanupInternal(CommandState state) override {}

  nsDocumentObject* m_pObject;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_TOOLSFOUNDATION_DLL nsInsertObjectPropertyCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsInsertObjectPropertyCommand, nsCommand);

public:
  nsInsertObjectPropertyCommand();

public: // Properties
  nsUuid m_Object;
  nsVariant m_NewValue;
  nsVariant m_Index;
  nsString m_sProperty;

private:
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  nsDocumentObject* m_pObject;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_TOOLSFOUNDATION_DLL nsRemoveObjectPropertyCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsRemoveObjectPropertyCommand, nsCommand);

public:
  nsRemoveObjectPropertyCommand();

public: // Properties
  nsUuid m_Object;
  nsVariant m_Index;
  nsString m_sProperty;

private:
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  nsDocumentObject* m_pObject;
  nsVariant m_OldValue;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class NS_TOOLSFOUNDATION_DLL nsMoveObjectPropertyCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsMoveObjectPropertyCommand, nsCommand);

public:
  nsMoveObjectPropertyCommand();

public: // Properties
  nsUuid m_Object;
  nsVariant m_OldIndex;
  nsVariant m_NewIndex;
  nsString m_sProperty;

private:
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  nsDocumentObject* m_pObject;
};
