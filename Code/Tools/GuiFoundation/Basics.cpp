#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Basics.h>
#include <QWidget>

wdQtScopedUpdatesDisabled::wdQtScopedUpdatesDisabled(
  QWidget* pWidget1, QWidget* pWidget2, QWidget* pWidget3, QWidget* pWidget4, QWidget* pWidget5, QWidget* pWidget6)
{
  QWidget* pWidgets[] = {pWidget1, pWidget2, pWidget3, pWidget4, pWidget5, pWidget6};

  for (int i = 0; i < WD_ARRAY_SIZE(pWidgets); ++i)
  {
    if (pWidgets[i] != nullptr && pWidgets[i]->updatesEnabled())
    {
      m_pWidgets[i] = pWidgets[i];
      m_pWidgets[i]->setUpdatesEnabled(false);
    }
    else
    {
      m_pWidgets[i] = nullptr;
    }
  }
}

wdQtScopedUpdatesDisabled::~wdQtScopedUpdatesDisabled()
{
  for (int i = WD_ARRAY_SIZE(m_pWidgets) - 1; i >= 0; --i)
  {
    if (m_pWidgets[i] != nullptr)
    {
      m_pWidgets[i]->setUpdatesEnabled(true);
    }
  }
}

wdQtScopedBlockSignals::wdQtScopedBlockSignals(
  QObject* pObject1, QObject* pObject2, QObject* pObject3, QObject* pObject4, QObject* pObject5, QObject* pObject6)
{
  QObject* pObjects[] = {pObject1, pObject2, pObject3, pObject4, pObject5, pObject6};

  for (int i = 0; i < WD_ARRAY_SIZE(pObjects); ++i)
  {
    if (pObjects[i] != nullptr && !pObjects[i]->signalsBlocked())
    {
      m_pObjects[i] = pObjects[i];
      m_pObjects[i]->blockSignals(true);
    }
    else
    {
      m_pObjects[i] = nullptr;
    }
  }
}

wdQtScopedBlockSignals::~wdQtScopedBlockSignals()
{
  for (int i = WD_ARRAY_SIZE(m_pObjects) - 1; i >= 0; --i)
  {
    if (m_pObjects[i] != nullptr)
    {
      m_pObjects[i]->blockSignals(false);
    }
  }
}
