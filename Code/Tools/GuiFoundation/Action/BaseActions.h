#pragma once

#include <GuiFoundation/Action/Action.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QIcon>

///
class WD_GUIFOUNDATION_DLL wdNamedAction : public wdAction
{
  WD_ADD_DYNAMIC_REFLECTION(wdNamedAction, wdAction);

public:
  wdNamedAction(const wdActionContext& context, const char* szName, const char* szIconPath)
    : wdAction(context)
    , m_sName(szName)
    , m_sIconPath(szIconPath)
  {
  }

  const char* GetName() const { return m_sName; }

  const char* GetAdditionalDisplayString() { return m_sAdditionalDisplayString; }
  void SetAdditionalDisplayString(const char* szString, bool bTriggerUpdate = true)
  {
    m_sAdditionalDisplayString = szString;
    if (bTriggerUpdate)
      TriggerUpdate();
  }

  const char* GetIconPath() const { return m_sIconPath; }
  void SetIconPath(const char* szIconPath) { m_sIconPath = szIconPath; }

protected:
  wdString m_sName;
  wdString m_sAdditionalDisplayString; // to add some context to the current action
  wdString m_sIconPath;
};

///
class WD_GUIFOUNDATION_DLL wdCategoryAction : public wdAction
{
  WD_ADD_DYNAMIC_REFLECTION(wdCategoryAction, wdAction);

public:
  wdCategoryAction(const wdActionContext& context)
    : wdAction(context)
  {
  }

  virtual void Execute(const wdVariant& value) override{};
};

///
class WD_GUIFOUNDATION_DLL wdMenuAction : public wdNamedAction
{
  WD_ADD_DYNAMIC_REFLECTION(wdMenuAction, wdNamedAction);

public:
  wdMenuAction(const wdActionContext& context, const char* szName, const char* szIconPath)
    : wdNamedAction(context, szName, szIconPath)
  {
  }

  virtual void Execute(const wdVariant& value) override{};
};

///
class WD_GUIFOUNDATION_DLL wdDynamicMenuAction : public wdMenuAction
{
  WD_ADD_DYNAMIC_REFLECTION(wdDynamicMenuAction, wdMenuAction);

public:
  struct Item
  {
    enum class CheckMark
    {
      NotCheckable,
      Unchecked,
      Checked
    };

    struct ItemFlags
    {
      typedef wdUInt8 StorageType;

      enum Enum
      {
        Default = 0,
        Separator = WD_BIT(0),
      };
      struct Bits
      {
        StorageType Separator : 1;
      };
    };

    Item() { m_CheckState = CheckMark::NotCheckable; }

    wdString m_sDisplay;
    QIcon m_Icon;
    CheckMark m_CheckState;
    wdBitflags<ItemFlags> m_ItemFlags;
    wdVariant m_UserValue;
  };

  wdDynamicMenuAction(const wdActionContext& context, const char* szName, const char* szIconPath)
    : wdMenuAction(context, szName, szIconPath)
  {
  }
  virtual void GetEntries(wdHybridArray<Item, 16>& out_entries) = 0;
};

///
class WD_GUIFOUNDATION_DLL wdDynamicActionAndMenuAction : public wdDynamicMenuAction
{
  WD_ADD_DYNAMIC_REFLECTION(wdDynamicActionAndMenuAction, wdDynamicMenuAction);

public:
  wdDynamicActionAndMenuAction(const wdActionContext& context, const char* szName, const char* szIconPath);

  bool IsEnabled() const { return m_bEnabled; }
  void SetEnabled(bool bEnable, bool bTriggerUpdate = true)
  {
    m_bEnabled = bEnable;
    if (bTriggerUpdate)
      TriggerUpdate();
  }

  bool IsVisible() const { return m_bVisible; }
  void SetVisible(bool bVisible, bool bTriggerUpdate = true)
  {
    m_bVisible = bVisible;
    if (bTriggerUpdate)
      TriggerUpdate();
  }

protected:
  bool m_bEnabled;
  bool m_bVisible;
};

///
class WD_GUIFOUNDATION_DLL wdEnumerationMenuAction : public wdDynamicMenuAction
{
  WD_ADD_DYNAMIC_REFLECTION(wdEnumerationMenuAction, wdDynamicMenuAction);

public:
  wdEnumerationMenuAction(const wdActionContext& context, const char* szName, const char* szIconPath);
  void InitEnumerationType(const wdRTTI* pEnumerationType);
  virtual void GetEntries(wdHybridArray<wdDynamicMenuAction::Item, 16>& out_entries) override;
  virtual wdInt64 GetValue() const = 0;

protected:
  const wdRTTI* m_pEnumerationType;
};

///
class WD_GUIFOUNDATION_DLL wdButtonAction : public wdNamedAction
{
  WD_ADD_DYNAMIC_REFLECTION(wdButtonAction, wdNamedAction);

public:
  wdButtonAction(const wdActionContext& context, const char* szName, bool bCheckable, const char* szIconPath);

  bool IsEnabled() const { return m_bEnabled; }
  void SetEnabled(bool bEnable, bool bTriggerUpdate = true)
  {
    m_bEnabled = bEnable;
    if (bTriggerUpdate)
      TriggerUpdate();
  }

  bool IsCheckable() const { return m_bCheckable; }
  void SetCheckable(bool bCheckable, bool bTriggerUpdate = true)
  {
    m_bCheckable = bCheckable;
    if (bTriggerUpdate)
      TriggerUpdate();
  }

  bool IsChecked() const { return m_bChecked; }
  void SetChecked(bool bChecked, bool bTriggerUpdate = true)
  {
    m_bChecked = bChecked;
    if (bTriggerUpdate)
      TriggerUpdate();
  }

  bool IsVisible() const { return m_bVisible; }
  void SetVisible(bool bVisible, bool bTriggerUpdate = true)
  {
    m_bVisible = bVisible;
    if (bTriggerUpdate)
      TriggerUpdate();
  }

protected:
  bool m_bCheckable;
  bool m_bChecked;
  bool m_bEnabled;
  bool m_bVisible;
};


class WD_GUIFOUNDATION_DLL wdSliderAction : public wdNamedAction
{
  WD_ADD_DYNAMIC_REFLECTION(wdSliderAction, wdNamedAction);

public:
  wdSliderAction(const wdActionContext& context, const char* szName);

  bool IsEnabled() const { return m_bEnabled; }
  void SetEnabled(bool bEnable, bool bTriggerUpdate = true)
  {
    m_bEnabled = bEnable;
    if (bTriggerUpdate)
      TriggerUpdate();
  }

  bool IsVisible() const { return m_bVisible; }
  void SetVisible(bool bVisible, bool bTriggerUpdate = true)
  {
    m_bVisible = bVisible;
    if (bTriggerUpdate)
      TriggerUpdate();
  }

  void GetRange(wdInt32& out_iMin, wdInt32& out_iMax) const
  {
    out_iMin = m_iMinValue;
    out_iMax = m_iMaxValue;
  }

  void SetRange(wdInt32 iMin, wdInt32 iMax, bool bTriggerUpdate = true);

  wdInt32 GetValue() const { return m_iCurValue; }
  void SetValue(wdInt32 iVal, bool bTriggerUpdate = true);

protected:
  bool m_bEnabled;
  bool m_bVisible;
  wdInt32 m_iMinValue;
  wdInt32 m_iMaxValue;
  wdInt32 m_iCurValue;
};
