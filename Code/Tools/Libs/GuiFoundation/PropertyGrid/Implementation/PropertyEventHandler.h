#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/PropertyGrid/Declarations.h>

class nsQtPropertyGridWidget;
class nsAbstractProperty;

struct nsPropertyEvent
{
  enum class Type
  {
    SingleValueChanged,
    BeginTemporary,
    EndTemporary,
    CancelTemporary,
  };

  Type m_Type;
  const nsAbstractProperty* m_pProperty;
  const nsHybridArray<nsPropertySelection, 8>* m_pItems;
  nsVariant m_Value;
};
