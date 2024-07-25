#pragma once

#include <GuiFoundation/Action/Action.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QIcon>

///
class NS_GUIFOUNDATION_DLL nsNamedAction : public nsAction
{
  NS_ADD_DYNAMIC_REFLECTION(nsNamedAction, nsAction);

public:
  nsNamedAction(const nsActionContext& context, const char* szName, const char* szIconPath)
    : nsAction(context)
    , m_sName(szName)
    , m_sIconPath(szIconPath)
  {
  }

  const char* GetName() const { return m_sName; }

  nsStringView GetAdditionalDisplayString() { return m_sAdditionalDisplayString; }
  void SetAdditionalDisplayString(nsStringView sString, bool bTriggerUpdate = true)
  {
    m_sAdditionalDisplayString = sString;
    if (bTriggerUpdate)
      TriggerUpdate();
  }

  const char* GetIconPath() const { return m_sIconPath; }
  void SetIconPath(const char* szIconPath) { m_sIconPath = szIconPath; }

protected:
  nsString m_sName;
  nsString m_sAdditionalDisplayString; // to add some context to the current action
  nsString m_sIconPath;
};

///
class NS_GUIFOUNDATION_DLL nsCategoryAction : public nsAction
{
  NS_ADD_DYNAMIC_REFLECTION(nsCategoryAction, nsAction);

public:
  nsCategoryAction(const nsActionContext& context)
    : nsAction(context)
  {
  }

  virtual void Execute(const nsVariant& value) override {};
};

///
class NS_GUIFOUNDATION_DLL nsMenuAction : public nsNamedAction
{
  NS_ADD_DYNAMIC_REFLECTION(nsMenuAction, nsNamedAction);

public:
  nsMenuAction(const nsActionContext& context, const char* szName, const char* szIconPath)
    : nsNamedAction(context, szName, szIconPath)
  {
  }

  virtual void Execute(const nsVariant& value) override {};
};

///
class NS_GUIFOUNDATION_DLL nsDynamicMenuAction : public nsMenuAction
{
  NS_ADD_DYNAMIC_REFLECTION(nsDynamicMenuAction, nsMenuAction);

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
      using StorageType = nsUInt8;

      enum Enum
      {
        Default = 0,
        Separator = NS_BIT(0),
      };
      struct Bits
      {
        StorageType Separator : 1;
      };
    };

    Item() { m_CheckState = CheckMark::NotCheckable; }

    nsString m_sDisplay;
    QIcon m_Icon;
    CheckMark m_CheckState;
    nsBitflags<ItemFlags> m_ItemFlags;
    nsVariant m_UserValue;
  };

  nsDynamicMenuAction(const nsActionContext& context, const char* szName, const char* szIconPath)
    : nsMenuAction(context, szName, szIconPath)
  {
  }
  virtual void GetEntries(nsHybridArray<Item, 16>& out_entries) = 0;
};

///
class NS_GUIFOUNDATION_DLL nsDynamicActionAndMenuAction : public nsDynamicMenuAction
{
  NS_ADD_DYNAMIC_REFLECTION(nsDynamicActionAndMenuAction, nsDynamicMenuAction);

public:
  nsDynamicActionAndMenuAction(const nsActionContext& context, const char* szName, const char* szIconPath);

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
class NS_GUIFOUNDATION_DLL nsEnumerationMenuAction : public nsDynamicMenuAction
{
  NS_ADD_DYNAMIC_REFLECTION(nsEnumerationMenuAction, nsDynamicMenuAction);

public:
  nsEnumerationMenuAction(const nsActionContext& context, const char* szName, const char* szIconPath);
  void InitEnumerationType(const nsRTTI* pEnumerationType);
  virtual void GetEntries(nsHybridArray<nsDynamicMenuAction::Item, 16>& out_entries) override;
  virtual nsInt64 GetValue() const = 0;

protected:
  const nsRTTI* m_pEnumerationType;
};

///
class NS_GUIFOUNDATION_DLL nsButtonAction : public nsNamedAction
{
  NS_ADD_DYNAMIC_REFLECTION(nsButtonAction, nsNamedAction);

public:
  nsButtonAction(const nsActionContext& context, const char* szName, bool bCheckable, const char* szIconPath);

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


class NS_GUIFOUNDATION_DLL nsSliderAction : public nsNamedAction
{
  NS_ADD_DYNAMIC_REFLECTION(nsSliderAction, nsNamedAction);

public:
  nsSliderAction(const nsActionContext& context, const char* szName);

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

  void GetRange(nsInt32& out_iMin, nsInt32& out_iMax) const
  {
    out_iMin = m_iMinValue;
    out_iMax = m_iMaxValue;
  }

  void SetRange(nsInt32 iMin, nsInt32 iMax, bool bTriggerUpdate = true);

  nsInt32 GetValue() const { return m_iCurValue; }
  void SetValue(nsInt32 iVal, bool bTriggerUpdate = true);

protected:
  bool m_bEnabled;
  bool m_bVisible;
  nsInt32 m_iMinValue;
  nsInt32 m_iMaxValue;
  nsInt32 m_iCurValue;
};
