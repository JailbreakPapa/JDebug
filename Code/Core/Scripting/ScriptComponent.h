#pragma once

#include <Core/Scripting/ScriptClassResource.h>
#include <Core/World/EventMessageHandlerComponent.h>
#include <Foundation/Types/RangeView.h>

using nsScriptComponentManager = nsComponentManager<class nsScriptComponent, nsBlockStorageType::FreeList>;

class NS_CORE_DLL nsScriptComponent : public nsEventMessageHandlerComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsScriptComponent, nsEventMessageHandlerComponent, nsScriptComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

protected:
  virtual void SerializeComponent(nsWorldWriter& stream) const override;
  virtual void DeserializeComponent(nsWorldReader& stream) override;
  virtual void Initialize() override;
  virtual void Deinitialize() override;
  virtual void OnActivated() override;
  virtual void OnDeactivated() override;
  virtual void OnSimulationStarted() override;

  //////////////////////////////////////////////////////////////////////////
  // nsScriptComponent
public:
  nsScriptComponent();
  ~nsScriptComponent();

  void SetScriptVariable(const nsHashedString& sName, const nsVariant& value); // [ scriptable ]
  nsVariant GetScriptVariable(const nsHashedString& sName) const;              // [ scriptable ]

  void SetScriptClass(const nsScriptClassResourceHandle& hScript);
  const nsScriptClassResourceHandle& GetScriptClass() const { return m_hScriptClass; }

  void SetScriptClassFile(const char* szFile); // [ property ]
  const char* GetScriptClassFile() const;      // [ property ]

  void SetUpdateInterval(nsTime interval);     // [ property ]
  nsTime GetUpdateInterval() const;            // [ property ]

  //////////////////////////////////////////////////////////////////////////
  // Exposed Parameters
  const nsRangeView<const char*, nsUInt32> GetParameters() const;
  void SetParameter(const char* szKey, const nsVariant& value);
  void RemoveParameter(const char* szKey);
  bool GetParameter(const char* szKey, nsVariant& out_value) const;

  NS_ALWAYS_INLINE nsScriptInstance* GetScriptInstance() { return m_pInstance.Borrow(); }

private:
  void InstantiateScript(bool bActivate);
  void ClearInstance(bool bDeactivate);
  void AddUpdateFunctionToSchedule();
  void RemoveUpdateFunctionToSchedule();

  const nsAbstractFunctionProperty* GetScriptFunction(nsUInt32 uiFunctionIndex);
  void CallScriptFunction(nsUInt32 uiFunctionIndex);

  void ReloadScript();

  nsArrayMap<nsHashedString, nsVariant> m_Parameters;

  nsScriptClassResourceHandle m_hScriptClass;
  nsTime m_UpdateInterval = nsTime::MakeZero();

  nsSharedPtr<nsScriptRTTI> m_pScriptType;
  nsUniquePtr<nsScriptInstance> m_pInstance;
};
