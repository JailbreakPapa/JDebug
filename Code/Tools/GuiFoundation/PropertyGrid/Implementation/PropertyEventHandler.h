#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/PropertyGrid/Declarations.h>

class wdQtPropertyGridWidget;
class wdAbstractProperty;

struct wdPropertyEvent
{
  enum class Type
  {
    SingleValueChanged,
    BeginTemporary,
    EndTemporary,
    CancelTemporary,
  };

  Type m_Type;
  const wdAbstractProperty* m_pProperty;
  const wdHybridArray<wdPropertySelection, 8>* m_pItems;
  wdVariant m_Value;
};
