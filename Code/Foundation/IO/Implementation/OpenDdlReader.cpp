#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OpenDdlReader.h>

wdOpenDdlReader::wdOpenDdlReader()
{
  m_pCurrentChunk = nullptr;
  m_uiBytesInChunkLeft = 0;
}

wdOpenDdlReader::~wdOpenDdlReader()
{
  ClearDataChunks();
}

wdResult wdOpenDdlReader::ParseDocument(wdStreamReader& inout_stream, wdUInt32 uiFirstLineOffset, wdLogInterface* pLog, wdUInt32 uiCacheSizeInKB)
{
  WD_ASSERT_DEBUG(m_ObjectStack.IsEmpty(), "A reader can only be used once.");

  SetLogInterface(pLog);
  SetCacheSize(uiCacheSizeInKB);
  SetInputStream(inout_stream, uiFirstLineOffset);

  m_TempCache.Reserve(s_uiChunkSize);

  wdOpenDdlReaderElement* pElement = &m_Elements.ExpandAndGetRef();
  pElement->m_pFirstChild = nullptr;
  pElement->m_pLastChild = nullptr;
  pElement->m_PrimitiveType = wdOpenDdlPrimitiveType::Custom;
  pElement->m_pSiblingElement = nullptr;
  pElement->m_szCustomType = CopyString("root");
  pElement->m_szName = nullptr;
  pElement->m_uiNumChildElements = 0;

  m_ObjectStack.PushBack(pElement);

  return ParseAll();
}

const wdOpenDdlReaderElement* wdOpenDdlReader::GetRootElement() const
{
  WD_ASSERT_DEBUG(!m_ObjectStack.IsEmpty(), "The reader has not parsed any document yet or an error occurred during parsing.");

  return m_ObjectStack[0];
}


const wdOpenDdlReaderElement* wdOpenDdlReader::FindElement(const char* szGlobalName) const
{
  return m_GlobalNames.GetValueOrDefault(szGlobalName, nullptr);
}

const char* wdOpenDdlReader::CopyString(const wdStringView& string)
{
  if (string.IsEmpty())
    return nullptr;

  // no idea how to make this more efficient without running into lots of other problems
  m_Strings.PushBack(string);
  return m_Strings.PeekBack().GetData();
}

wdOpenDdlReaderElement* wdOpenDdlReader::CreateElement(wdOpenDdlPrimitiveType type, const char* szType, const char* szName, bool bGlobalName)
{
  wdOpenDdlReaderElement* pElement = &m_Elements.ExpandAndGetRef();
  pElement->m_pFirstChild = nullptr;
  pElement->m_pLastChild = nullptr;
  pElement->m_PrimitiveType = type;
  pElement->m_pSiblingElement = nullptr;
  pElement->m_szCustomType = szType;
  pElement->m_szName = CopyString(szName);
  pElement->m_uiNumChildElements = 0;

  if (bGlobalName)
  {
    pElement->m_uiNumChildElements = WD_BIT(31);
  }

  if (bGlobalName && !wdStringUtils::IsNullOrEmpty(szName))
  {
    m_GlobalNames[szName] = pElement;
  }

  wdOpenDdlReaderElement* pParent = m_ObjectStack.PeekBack();
  pParent->m_uiNumChildElements++;

  if (pParent->m_pFirstChild == nullptr)
  {
    pParent->m_pFirstChild = pElement;
    pParent->m_pLastChild = pElement;
  }
  else
  {
    ((wdOpenDdlReaderElement*)pParent->m_pLastChild)->m_pSiblingElement = pElement;
    pParent->m_pLastChild = pElement;
  }

  m_ObjectStack.PushBack(pElement);

  return pElement;
}


void wdOpenDdlReader::OnBeginObject(const char* szType, const char* szName, bool bGlobalName)
{
  CreateElement(wdOpenDdlPrimitiveType::Custom, CopyString(szType), szName, bGlobalName);
}

void wdOpenDdlReader::OnEndObject()
{
  m_ObjectStack.PopBack();
}

void wdOpenDdlReader::OnBeginPrimitiveList(wdOpenDdlPrimitiveType type, const char* szName, bool bGlobalName)
{
  CreateElement(type, nullptr, szName, bGlobalName);

  m_TempCache.Clear();
}

void wdOpenDdlReader::OnEndPrimitiveList()
{
  // if we had to temporarily store the primitive data, copy it into a new destination
  if (!m_TempCache.IsEmpty())
  {
    wdUInt8* pTarget = AllocateBytes(m_TempCache.GetCount());
    m_ObjectStack.PeekBack()->m_pFirstChild = pTarget;

    wdMemoryUtils::Copy(pTarget, m_TempCache.GetData(), m_TempCache.GetCount());
  }

  m_ObjectStack.PopBack();
}

void wdOpenDdlReader::StorePrimitiveData(bool bThisIsAll, wdUInt32 bytecount, const wdUInt8* pData)
{
  wdUInt8* pTarget = nullptr;

  if (!bThisIsAll || !m_TempCache.IsEmpty())
  {
    // if this is not all, accumulate the data in a temp buffer
    wdUInt32 offset = m_TempCache.GetCount();
    m_TempCache.SetCountUninitialized(m_TempCache.GetCount() + bytecount);
    pTarget = &m_TempCache[offset]; // have to index m_TempCache after the resize, otherwise it could be empty and not like it
  }
  else
  {
    // otherwise, allocate the final storage immediately
    pTarget = AllocateBytes(bytecount);
    m_ObjectStack.PeekBack()->m_pFirstChild = pTarget;
  }

  wdMemoryUtils::Copy(pTarget, pData, bytecount);
}


void wdOpenDdlReader::OnPrimitiveBool(wdUInt32 count, const bool* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(bool) * count, (const wdUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void wdOpenDdlReader::OnPrimitiveInt8(wdUInt32 count, const wdInt8* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(wdInt8) * count, (const wdUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void wdOpenDdlReader::OnPrimitiveInt16(wdUInt32 count, const wdInt16* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(wdInt16) * count, (const wdUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void wdOpenDdlReader::OnPrimitiveInt32(wdUInt32 count, const wdInt32* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(wdInt32) * count, (const wdUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void wdOpenDdlReader::OnPrimitiveInt64(wdUInt32 count, const wdInt64* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(wdInt64) * count, (const wdUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void wdOpenDdlReader::OnPrimitiveUInt8(wdUInt32 count, const wdUInt8* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(wdUInt8) * count, (const wdUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void wdOpenDdlReader::OnPrimitiveUInt16(wdUInt32 count, const wdUInt16* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(wdUInt16) * count, (const wdUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void wdOpenDdlReader::OnPrimitiveUInt32(wdUInt32 count, const wdUInt32* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(wdUInt32) * count, (const wdUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void wdOpenDdlReader::OnPrimitiveUInt64(wdUInt32 count, const wdUInt64* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(wdUInt64) * count, (const wdUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void wdOpenDdlReader::OnPrimitiveFloat(wdUInt32 count, const float* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(float) * count, (const wdUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void wdOpenDdlReader::OnPrimitiveDouble(wdUInt32 count, const double* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(double) * count, (const wdUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void wdOpenDdlReader::OnPrimitiveString(wdUInt32 count, const wdStringView* pData, bool bThisIsAll)
{
  const wdUInt32 uiDataSize = count * sizeof(wdStringView);

  const wdUInt32 offset = m_TempCache.GetCount();
  m_TempCache.SetCountUninitialized(m_TempCache.GetCount() + uiDataSize);
  wdStringView* pTarget = (wdStringView*)&m_TempCache[offset];

  for (wdUInt32 i = 0; i < count; ++i)
  {
    const char* szStart = CopyString(pData[i]);
    pTarget[i] = wdStringView(szStart, szStart + pData[i].GetElementCount());
  }

  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}


void wdOpenDdlReader::OnParsingError(const char* szMessage, bool bFatal, wdUInt32 uiLine, wdUInt32 uiColumn)
{
  if (bFatal)
  {
    m_ObjectStack.Clear();
    m_GlobalNames.Clear();
    m_Elements.Clear();

    ClearDataChunks();
  }
}

//////////////////////////////////////////////////////////////////////////

void wdOpenDdlReader::ClearDataChunks()
{
  for (wdUInt32 i = 0; i < m_DataChunks.GetCount(); ++i)
  {
    WD_DEFAULT_DELETE(m_DataChunks[i]);
  }

  m_DataChunks.Clear();
}

wdUInt8* wdOpenDdlReader::AllocateBytes(wdUInt32 uiNumBytes)
{
  uiNumBytes = wdMemoryUtils::AlignSize(uiNumBytes, static_cast<wdUInt32>(WD_ALIGNMENT_MINIMUM));

  // if the requested data is very large, just allocate it as an individual chunk
  if (uiNumBytes > s_uiChunkSize / 2)
  {
    wdUInt8* pResult = WD_DEFAULT_NEW_ARRAY(wdUInt8, uiNumBytes).GetPtr();
    m_DataChunks.PushBack(pResult);
    return pResult;
  }

  // if our current chunk is too small, discard the remaining free bytes and just allocate a new chunk
  if (m_uiBytesInChunkLeft < uiNumBytes)
  {
    m_pCurrentChunk = WD_DEFAULT_NEW_ARRAY(wdUInt8, s_uiChunkSize).GetPtr();
    m_uiBytesInChunkLeft = s_uiChunkSize;
    m_DataChunks.PushBack(m_pCurrentChunk);
  }

  // no fulfill the request from the current chunk
  wdUInt8* pResult = m_pCurrentChunk;
  m_pCurrentChunk += uiNumBytes;
  m_uiBytesInChunkLeft -= uiNumBytes;

  return pResult;
}

//////////////////////////////////////////////////////////////////////////

wdUInt32 wdOpenDdlReaderElement::GetNumChildObjects() const
{
  if (m_PrimitiveType != wdOpenDdlPrimitiveType::Custom)
    return 0;

  return m_uiNumChildElements & (~WD_BIT(31)); // Bit 31 stores whether the name is global
}

wdUInt32 wdOpenDdlReaderElement::GetNumPrimitives() const
{
  if (m_PrimitiveType == wdOpenDdlPrimitiveType::Custom)
    return 0;

  return m_uiNumChildElements & (~WD_BIT(31)); // Bit 31 stores whether the name is global
}


bool wdOpenDdlReaderElement::HasPrimitives(wdOpenDdlPrimitiveType type, wdUInt32 uiMinNumberOfPrimitives /*= 1*/) const
{
  /// \test This is new

  if (m_PrimitiveType != type)
    return false;

  return m_uiNumChildElements >= uiMinNumberOfPrimitives;
}

const wdOpenDdlReaderElement* wdOpenDdlReaderElement::FindChild(const char* szName) const
{
  WD_ASSERT_DEBUG(m_PrimitiveType == wdOpenDdlPrimitiveType::Custom, "Cannot search for a child object in a primitives list");

  const wdOpenDdlReaderElement* pChild = static_cast<const wdOpenDdlReaderElement*>(m_pFirstChild);

  while (pChild)
  {
    if (wdStringUtils::IsEqual(pChild->GetName(), szName))
    {
      return pChild;
    }

    pChild = pChild->GetSibling();
  }

  return nullptr;
}

const wdOpenDdlReaderElement* wdOpenDdlReaderElement::FindChildOfType(wdOpenDdlPrimitiveType type, const char* szName, wdUInt32 uiMinNumberOfPrimitives /* = 1*/) const
{
  /// \test This is new

  WD_ASSERT_DEBUG(m_PrimitiveType == wdOpenDdlPrimitiveType::Custom, "Cannot search for a child object in a primitives list");

  const wdOpenDdlReaderElement* pChild = static_cast<const wdOpenDdlReaderElement*>(m_pFirstChild);

  while (pChild)
  {
    if (pChild->GetPrimitivesType() == type && wdStringUtils::IsEqual(pChild->GetName(), szName))
    {
      if (type == wdOpenDdlPrimitiveType::Custom || pChild->GetNumPrimitives() >= uiMinNumberOfPrimitives)
        return pChild;
    }

    pChild = pChild->GetSibling();
  }

  return nullptr;
}

const wdOpenDdlReaderElement* wdOpenDdlReaderElement::FindChildOfType(const char* szType, const char* szName /*= nullptr*/) const
{
  const wdOpenDdlReaderElement* pChild = static_cast<const wdOpenDdlReaderElement*>(m_pFirstChild);

  while (pChild)
  {
    if (pChild->GetPrimitivesType() == wdOpenDdlPrimitiveType::Custom && wdStringUtils::IsEqual(pChild->GetCustomType(), szType) && (szName == nullptr || wdStringUtils::IsEqual(pChild->GetName(), szName)))
    {
      return pChild;
    }

    pChild = pChild->GetSibling();
  }

  return nullptr;
}


WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_OpenDdlReader);
