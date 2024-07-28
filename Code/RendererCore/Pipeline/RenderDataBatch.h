#pragma once

#include <RendererCore/Pipeline/Declarations.h>

class nsRenderDataBatch
{
private:
  struct SortableRenderData
  {
    NS_DECLARE_POD_TYPE();

    const nsRenderData* m_pRenderData;
    nsUInt64 m_uiSortingKey;
  };

public:
  // NS_DECLARE_POD_TYPE(); // nsDelegate has a destructor and therefore nsRenderDataBatch can't be POD

  /// \brief This function should return true if the given render data should be filtered and not rendered.
  using Filter = nsDelegate<bool(const nsRenderData*)>;

  template <typename T>
  class Iterator
  {
  public:
    const T& operator*() const;
    const T* operator->() const;

    operator const T*() const;

    void Next();

    bool IsValid() const;

    void operator++();

  private:
    friend class nsRenderDataBatch;

    Iterator(const SortableRenderData* pStart, const SortableRenderData* pEnd, Filter filter);

    Filter m_Filter;
    const SortableRenderData* m_pCurrent;
    const SortableRenderData* m_pEnd;
  };

  nsUInt32 GetCount() const;

  template <typename T>
  const T* GetFirstData() const;

  template <typename T>
  Iterator<T> GetIterator(nsUInt32 uiStartIndex = 0, nsUInt32 uiCount = nsInvalidIndex) const;

private:
  friend class nsExtractedRenderData;
  friend class nsRenderDataBatchList;

  Filter m_Filter;
  nsArrayPtr<SortableRenderData> m_Data;
};

class nsRenderDataBatchList
{
public:
  nsUInt32 GetBatchCount() const;

  nsRenderDataBatch GetBatch(nsUInt32 uiIndex) const;

private:
  friend class nsExtractedRenderData;

  nsRenderDataBatch::Filter m_Filter;
  nsArrayPtr<const nsRenderDataBatch> m_Batches;
};

#include <RendererCore/Pipeline/Implementation/RenderDataBatch_inl.h>
