#include <Core/CorePCH.h>

#include <Core/Console/QuakeConsole.h>
#include <Core/Input/InputManager.h>

bool nsQuakeConsole::ProcessInputCharacter(nsUInt32 uiChar)
{
  switch (uiChar)
  {
    case 27: // Escape
      ClearInputLine();
      return false;

    case '\b': // backspace
    {
      if (!m_sInputLine.IsEmpty() && m_iCaretPosition > 0)
      {
        RemoveCharacter(m_iCaretPosition - 1);
        MoveCaret(-1);
      }
    }
      return false;

    case '\t':
      if (AutoComplete(m_sInputLine))
      {
        MoveCaret(500);
      }
      return false;

    case 13: // Enter
      AddToInputHistory(m_sInputLine);
      ExecuteCommand(m_sInputLine);
      ClearInputLine();
      return false;
  }

  return true;
}

bool nsQuakeConsole::FilterInputCharacter(nsUInt32 uiChar)
{
  // filter out not only all non-ASCII characters, but also all the non-printable ASCII characters
  // if you want to support full Unicode characters in the console, override this function and change this restriction
  if (uiChar < 32 || uiChar > 126)
    return false;

  return true;
}

void nsQuakeConsole::ClampCaretPosition()
{
  m_iCaretPosition = nsMath::Clamp<nsInt32>(m_iCaretPosition, 0, m_sInputLine.GetCharacterCount());
}

void nsQuakeConsole::MoveCaret(nsInt32 iMoveOffset)
{
  m_iCaretPosition += iMoveOffset;

  ClampCaretPosition();
}

void nsQuakeConsole::Scroll(nsInt32 iLines)
{
  if (m_bUseFilteredStrings)
    m_iScrollPosition = nsMath::Clamp<nsInt32>(m_iScrollPosition + iLines, 0, nsMath::Max<nsInt32>(m_FilteredConsoleStrings.GetCount() - 10, 0));
  else
    m_iScrollPosition = nsMath::Clamp<nsInt32>(m_iScrollPosition + iLines, 0, nsMath::Max<nsInt32>(m_ConsoleStrings.GetCount() - 10, 0));
}

void nsQuakeConsole::ClearInputLine()
{
  m_sInputLine.Clear();
  m_iCaretPosition = 0;
  m_iScrollPosition = 0;
  m_iCurrentInputHistoryElement = -1;

  m_FilteredConsoleStrings.Clear();
  m_bUseFilteredStrings = false;

  InputStringChanged();
}

void nsQuakeConsole::ClearConsoleStrings()
{
  m_ConsoleStrings.Clear();
  m_FilteredConsoleStrings.Clear();
  m_bUseFilteredStrings = false;
  m_iScrollPosition = 0;
}

void nsQuakeConsole::DeleteNextCharacter()
{
  RemoveCharacter(m_iCaretPosition);
}

void nsQuakeConsole::RemoveCharacter(nsUInt32 uiInputLinePosition)
{
  if (uiInputLinePosition >= m_sInputLine.GetCharacterCount())
    return;

  auto it = m_sInputLine.GetIteratorFront();
  it += uiInputLinePosition;

  auto itNext = it;
  ++itNext;

  m_sInputLine.Remove(it.GetData(), itNext.GetData());

  InputStringChanged();
}

void nsQuakeConsole::AddInputCharacter(nsUInt32 uiChar)
{
  if (uiChar == '\0')
    return;

  if (!ProcessInputCharacter(uiChar))
    return;

  if (!FilterInputCharacter(uiChar))
    return;

  ClampCaretPosition();

  auto it = m_sInputLine.GetIteratorFront();
  it += m_iCaretPosition;

  nsUInt32 uiString[2] = {uiChar, 0};

  m_sInputLine.Insert(it.GetData(), nsStringUtf8(uiString).GetData());

  MoveCaret(1);

  InputStringChanged();
}

void nsQuakeConsole::DoDefaultInputHandling(bool bConsoleOpen)
{
  if (!m_bDefaultInputHandlingInitialized)
  {
    m_bDefaultInputHandlingInitialized = true;

    nsInputActionConfig cfg;
    cfg.m_bApplyTimeScaling = true;

    cfg.m_sInputSlotTrigger[0] = nsInputSlot_KeyLeft;
    nsInputManager::SetInputActionConfig("Console", "MoveCaretLeft", cfg, true);

    cfg.m_sInputSlotTrigger[0] = nsInputSlot_KeyRight;
    nsInputManager::SetInputActionConfig("Console", "MoveCaretRight", cfg, true);

    cfg.m_sInputSlotTrigger[0] = nsInputSlot_KeyHome;
    nsInputManager::SetInputActionConfig("Console", "MoveCaretStart", cfg, true);

    cfg.m_sInputSlotTrigger[0] = nsInputSlot_KeyEnd;
    nsInputManager::SetInputActionConfig("Console", "MoveCaretEnd", cfg, true);

    cfg.m_sInputSlotTrigger[0] = nsInputSlot_KeyDelete;
    nsInputManager::SetInputActionConfig("Console", "DeleteCharacter", cfg, true);

    cfg.m_sInputSlotTrigger[0] = nsInputSlot_KeyPageUp;
    nsInputManager::SetInputActionConfig("Console", "ScrollUp", cfg, true);

    cfg.m_sInputSlotTrigger[0] = nsInputSlot_KeyPageDown;
    nsInputManager::SetInputActionConfig("Console", "ScrollDown", cfg, true);

    cfg.m_sInputSlotTrigger[0] = nsInputSlot_KeyUp;
    nsInputManager::SetInputActionConfig("Console", "HistoryUp", cfg, true);

    cfg.m_sInputSlotTrigger[0] = nsInputSlot_KeyDown;
    nsInputManager::SetInputActionConfig("Console", "HistoryDown", cfg, true);

    cfg.m_sInputSlotTrigger[0] = nsInputSlot_KeyF2;
    nsInputManager::SetInputActionConfig("Console", "RepeatLast", cfg, true);

    cfg.m_sInputSlotTrigger[0] = nsInputSlot_KeyF3;
    nsInputManager::SetInputActionConfig("Console", "RepeatSecondLast", cfg, true);

    return;
  }

  if (bConsoleOpen)
  {
    if (nsInputManager::GetInputActionState("Console", "MoveCaretLeft") == nsKeyState::Pressed)
      MoveCaret(-1);
    if (nsInputManager::GetInputActionState("Console", "MoveCaretRight") == nsKeyState::Pressed)
      MoveCaret(1);
    if (nsInputManager::GetInputActionState("Console", "MoveCaretStart") == nsKeyState::Pressed)
      MoveCaret(-1000);
    if (nsInputManager::GetInputActionState("Console", "MoveCaretEnd") == nsKeyState::Pressed)
      MoveCaret(1000);
    if (nsInputManager::GetInputActionState("Console", "DeleteCharacter") == nsKeyState::Pressed)
      DeleteNextCharacter();
    if (nsInputManager::GetInputActionState("Console", "ScrollUp") == nsKeyState::Pressed)
      Scroll(10);
    if (nsInputManager::GetInputActionState("Console", "ScrollDown") == nsKeyState::Pressed)
      Scroll(-10);
    if (nsInputManager::GetInputActionState("Console", "HistoryUp") == nsKeyState::Pressed)
    {
      RetrieveInputHistory(1, m_sInputLine);
      m_iCaretPosition = m_sInputLine.GetCharacterCount();
    }
    if (nsInputManager::GetInputActionState("Console", "HistoryDown") == nsKeyState::Pressed)
    {
      RetrieveInputHistory(-1, m_sInputLine);
      m_iCaretPosition = m_sInputLine.GetCharacterCount();
    }

    const nsUInt32 uiChar = nsInputManager::RetrieveLastCharacter();

    if (uiChar != '\0')
      AddInputCharacter(uiChar);
  }
  else
  {
    const nsUInt32 uiChar = nsInputManager::RetrieveLastCharacter(false);

    char szCmd[16] = "";
    char* szIterator = szCmd;
    nsUnicodeUtils::EncodeUtf32ToUtf8(uiChar, szIterator);
    *szIterator = '\0';
    ExecuteBoundKey(szCmd);
  }

  if (nsInputManager::GetInputActionState("Console", "RepeatLast") == nsKeyState::Pressed)
  {
    if (GetInputHistory().GetCount() >= 1)
      ExecuteCommand(GetInputHistory()[0]);
  }

  if (nsInputManager::GetInputActionState("Console", "RepeatSecondLast") == nsKeyState::Pressed)
  {
    if (GetInputHistory().GetCount() >= 2)
      ExecuteCommand(GetInputHistory()[1]);
  }
}
