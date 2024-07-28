
NS_ALWAYS_INLINE nsWorld* nsWorldModule::GetWorld()
{
  return m_pWorld;
}

NS_ALWAYS_INLINE const nsWorld* nsWorldModule::GetWorld() const
{
  return m_pWorld;
}

//////////////////////////////////////////////////////////////////////////

template <typename ModuleType, typename RTTIType>
nsWorldModuleTypeId nsWorldModuleFactory::RegisterWorldModule()
{
  struct Helper
  {
    static nsWorldModule* Create(nsAllocator* pAllocator, nsWorld* pWorld) { return NS_NEW(pAllocator, ModuleType, pWorld); }
  };

  const nsRTTI* pRtti = nsGetStaticRTTI<RTTIType>();
  return RegisterWorldModule(pRtti, &Helper::Create);
}
