/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
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
