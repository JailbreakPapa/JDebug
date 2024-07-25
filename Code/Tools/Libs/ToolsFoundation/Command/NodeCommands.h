#pragma once

#include <ToolsFoundation/Command/Command.h>

class nsDocumentObject;
class nsCommandHistory;
class nsPin;

class NS_TOOLSFOUNDATION_DLL nsRemoveNodeCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsRemoveNodeCommand, nsCommand);

public:
  nsRemoveNodeCommand();

public: // Properties
  nsUuid m_Object;

private:
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override;

private:
  nsDocumentObject* m_pObject = nullptr;
};


class NS_TOOLSFOUNDATION_DLL nsMoveNodeCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsMoveNodeCommand, nsCommand);

public:
  nsMoveNodeCommand();

public: // Properties
  nsUuid m_Object;
  nsVec2 m_NewPos = nsVec2::MakeZero();

private:
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  nsDocumentObject* m_pObject = nullptr;
  nsVec2 m_vOldPos = nsVec2::MakeZero();
};


class NS_TOOLSFOUNDATION_DLL nsConnectNodePinsCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsConnectNodePinsCommand, nsCommand);

public:
  nsConnectNodePinsCommand();

public: // Properties
  nsUuid m_ConnectionObject;
  nsUuid m_ObjectSource;
  nsUuid m_ObjectTarget;
  nsString m_sSourcePin;
  nsString m_sTargetPin;

private:
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  nsDocumentObject* m_pConnectionObject = nullptr;
  nsDocumentObject* m_pObjectSource = nullptr;
  nsDocumentObject* m_pObjectTarget = nullptr;
};


class NS_TOOLSFOUNDATION_DLL nsDisconnectNodePinsCommand : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsDisconnectNodePinsCommand, nsCommand);

public:
  nsDisconnectNodePinsCommand();

public: // Properties
  nsUuid m_ConnectionObject;

private:
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  nsDocumentObject* m_pConnectionObject = nullptr;
  const nsDocumentObject* m_pObjectSource = nullptr;
  const nsDocumentObject* m_pObjectTarget = nullptr;
  nsString m_sSourcePin;
  nsString m_sTargetPin;
};


class NS_TOOLSFOUNDATION_DLL nsNodeCommands
{
public:
  static nsStatus AddAndConnectCommand(nsCommandHistory* pHistory, const nsRTTI* pConnectionType, const nsPin& sourcePin, const nsPin& targetPin);
  static nsStatus DisconnectAndRemoveCommand(nsCommandHistory* pHistory, const nsUuid& connectionObject);
};
