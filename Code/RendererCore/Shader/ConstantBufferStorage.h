#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <RendererCore/RendererCoreDLL.h>
#include <RendererFoundation/RendererFoundationDLL.h>

class NS_RENDERERCORE_DLL nsConstantBufferStorageBase
{
protected:
  friend class nsRenderContext;
  friend class nsMemoryUtils;

  nsConstantBufferStorageBase(nsUInt32 uiSizeInBytes);
  ~nsConstantBufferStorageBase();

public:
  nsArrayPtr<nsUInt8> GetRawDataForWriting();
  nsArrayPtr<const nsUInt8> GetRawDataForReading() const;

  void UploadData(nsGALCommandEncoder* pCommandEncoder);

  NS_ALWAYS_INLINE nsGALBufferHandle GetGALBufferHandle() const { return m_hGALConstantBuffer; }

protected:
  bool m_bHasBeenModified = false;
  nsUInt32 m_uiLastHash = 0;
  nsGALBufferHandle m_hGALConstantBuffer;

  nsArrayPtr<nsUInt8> m_Data;
};

template <typename T>
class nsConstantBufferStorage : public nsConstantBufferStorageBase
{
public:
  NS_FORCE_INLINE T& GetDataForWriting()
  {
    nsArrayPtr<nsUInt8> rawData = GetRawDataForWriting();
    NS_ASSERT_DEV(rawData.GetCount() == sizeof(T), "Invalid data size");
    return *reinterpret_cast<T*>(rawData.GetPtr());
  }

  NS_FORCE_INLINE const T& GetDataForReading() const
  {
    nsArrayPtr<const nsUInt8> rawData = GetRawDataForReading();
    NS_ASSERT_DEV(rawData.GetCount() == sizeof(T), "Invalid data size");
    return *reinterpret_cast<const T*>(rawData.GetPtr());
  }
};

using nsConstantBufferStorageId = nsGenericId<24, 8>;

class nsConstantBufferStorageHandle
{
  NS_DECLARE_HANDLE_TYPE(nsConstantBufferStorageHandle, nsConstantBufferStorageId);

  friend class nsRenderContext;
};
