#pragma once

#include <ToolsFoundation/Command/Command.h>

class wdDocumentObject;
class wdCommandHistory;
class wdPin;

class WD_TOOLSFOUNDATION_DLL wdRemoveNodeCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdRemoveNodeCommand, wdCommand);

public:
  wdRemoveNodeCommand();

public: // Properties
  wdUuid m_Object;

private:
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override;

private:
  wdDocumentObject* m_pObject = nullptr;
};


class WD_TOOLSFOUNDATION_DLL wdMoveNodeCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdMoveNodeCommand, wdCommand);

public:
  wdMoveNodeCommand();

public: // Properties
  wdUuid m_Object;
  wdVec2 m_NewPos = wdVec2::ZeroVector();

private:
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  wdDocumentObject* m_pObject = nullptr;
  wdVec2 m_vOldPos = wdVec2::ZeroVector();
};


class WD_TOOLSFOUNDATION_DLL wdConnectNodePinsCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdConnectNodePinsCommand, wdCommand);

public:
  wdConnectNodePinsCommand();

public: // Properties
  wdUuid m_ConnectionObject;
  wdUuid m_ObjectSource;
  wdUuid m_ObjectTarget;
  wdString m_sSourcePin;
  wdString m_sTargetPin;

private:
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  wdDocumentObject* m_pConnectionObject = nullptr;
  wdDocumentObject* m_pObjectSource = nullptr;
  wdDocumentObject* m_pObjectTarget = nullptr;
};


class WD_TOOLSFOUNDATION_DLL wdDisconnectNodePinsCommand : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdDisconnectNodePinsCommand, wdCommand);

public:
  wdDisconnectNodePinsCommand();

public: // Properties
  wdUuid m_ConnectionObject;

private:
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

private:
  wdDocumentObject* m_pConnectionObject = nullptr;
  const wdDocumentObject* m_pObjectSource = nullptr;
  const wdDocumentObject* m_pObjectTarget = nullptr;
  wdString m_sSourcePin;
  wdString m_sTargetPin;
};


class WD_TOOLSFOUNDATION_DLL wdNodeCommands
{
public:
  static wdStatus AddAndConnectCommand(wdCommandHistory* pHistory, const wdRTTI* pConnectionType, const wdPin& sourcePin, const wdPin& targetPin);
  static wdStatus DisconnectAndRemoveCommand(wdCommandHistory* pHistory, const wdUuid& connectionObject);
};
