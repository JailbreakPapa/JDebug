
template <typename T>
nsCustomDataResource<T>::nsCustomDataResource() = default;

template <typename T>
nsCustomDataResource<T>::~nsCustomDataResource() = default;

template <typename T>
void nsCustomDataResource<T>::CreateAndLoadData(nsAbstractObjectGraph& ref_graph, nsRttiConverterContext& ref_context, const nsAbstractObjectNode* pRootNode)
{
  T* pData = reinterpret_cast<T*>(m_Data);

  if (GetLoadingState() == nsResourceState::Loaded)
  {
    nsMemoryUtils::Destruct(pData);
  }

  nsMemoryUtils::Construct<SkipTrivialTypes>(pData);

  if (pRootNode)
  {
    // pRootNode is empty when the resource file is empty
    // no need to attempt to load it then
    pData->Load(ref_graph, ref_context, pRootNode);
  }
}

template <typename T>
nsResourceLoadDesc nsCustomDataResource<T>::UnloadData(Unload WhatToUnload)
{
  if (GetData() != nullptr)
  {
    nsMemoryUtils::Destruct(GetData());
  }

  return nsCustomDataResourceBase::UnloadData(WhatToUnload);
}

template <typename T>
nsResourceLoadDesc nsCustomDataResource<T>::UpdateContent(nsStreamReader* Stream)
{
  return UpdateContent_Internal(Stream, *nsGetStaticRTTI<T>());
}

template <typename T>
void nsCustomDataResource<T>::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsCustomDataResource<T>);
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}
