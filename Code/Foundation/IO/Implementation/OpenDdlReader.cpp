#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OpenDdlReader.h>

nsOpenDdlReader::nsOpenDdlReader()
{
  m_pCurrentChunk = nullptr;
  m_uiBytesInChunkLeft = 0;
}

nsOpenDdlReader::~nsOpenDdlReader()
{
  ClearDataChunks();
}

nsResult nsOpenDdlReader::ParseDocument(nsStreamReader& inout_stream, nsUInt32 uiFirstLineOffset, nsLogInterface* pLog, nsUInt32 uiCacheSizeInKB)
{
  NS_ASSERT_DEBUG(m_ObjectStack.IsEmpty(), "A reader can only be used once.");

  SetLogInterface(pLog);
  SetCacheSize(uiCacheSizeInKB);
  SetInputStream(inout_stream, uiFirstLineOffset);

  m_TempCache.Reserve(s_uiChunkSize);

  nsOpenDdlReaderElement* pElement = &m_Elements.ExpandAndGetRef();
  pElement->m_pFirstChild = nullptr;
  pElement->m_pLastChild = nullptr;
  pElement->m_PrimitiveType = nsOpenDdlPrimitiveType::Custom;
  pElement->m_pSiblingElement = nullptr;
  pElement->m_sCustomType = CopyString("root");
  pElement->m_sName = nullptr;
  pElement->m_uiNumChildElements = 0;

  m_ObjectStack.PushBack(pElement);

  return ParseAll();
}

const nsOpenDdlReaderElement* nsOpenDdlReader::GetRootElement() const
{
  NS_ASSERT_DEBUG(!m_ObjectStack.IsEmpty(), "The reader has not parsed any document yet or an error occurred during parsing.");

  return m_ObjectStack[0];
}


const nsOpenDdlReaderElement* nsOpenDdlReader::FindElement(nsStringView sGlobalName) const
{
  return m_GlobalNames.GetValueOrDefault(sGlobalName, nullptr);
}

nsStringView nsOpenDdlReader::CopyString(const nsStringView& string)
{
  if (string.IsEmpty())
    return {};

  // no idea how to make this more efficient without running into lots of other problems
  m_Strings.PushBack(string);
  return m_Strings.PeekBack();
}

nsOpenDdlReaderElement* nsOpenDdlReader::CreateElement(nsOpenDdlPrimitiveType type, nsStringView sType, nsStringView sName, bool bGlobalName)
{
  nsOpenDdlReaderElement* pElement = &m_Elements.ExpandAndGetRef();
  pElement->m_pFirstChild = nullptr;
  pElement->m_pLastChild = nullptr;
  pElement->m_PrimitiveType = type;
  pElement->m_pSiblingElement = nullptr;
  pElement->m_sCustomType = sType;
  pElement->m_sName = CopyString(sName);
  pElement->m_uiNumChildElements = 0;

  if (bGlobalName)
  {
    pElement->m_uiNumChildElements = NS_BIT(31);
  }

  if (bGlobalName && !sName.IsEmpty())
  {
    m_GlobalNames[sName] = pElement;
  }

  nsOpenDdlReaderElement* pParent = m_ObjectStack.PeekBack();
  pParent->m_uiNumChildElements++;

  if (pParent->m_pFirstChild == nullptr)
  {
    pParent->m_pFirstChild = pElement;
    pParent->m_pLastChild = pElement;
  }
  else
  {
    ((nsOpenDdlReaderElement*)pParent->m_pLastChild)->m_pSiblingElement = pElement;
    pParent->m_pLastChild = pElement;
  }

  m_ObjectStack.PushBack(pElement);

  return pElement;
}


void nsOpenDdlReader::OnBeginObject(nsStringView sType, nsStringView sName, bool bGlobalName)
{
  CreateElement(nsOpenDdlPrimitiveType::Custom, CopyString(sType), sName, bGlobalName);
}

void nsOpenDdlReader::OnEndObject()
{
  m_ObjectStack.PopBack();
}

void nsOpenDdlReader::OnBeginPrimitiveList(nsOpenDdlPrimitiveType type, nsStringView sName, bool bGlobalName)
{
  CreateElement(type, nullptr, sName, bGlobalName);

  m_TempCache.Clear();
}

void nsOpenDdlReader::OnEndPrimitiveList()
{
  // if we had to temporarily store the primitive data, copy it into a new destination
  if (!m_TempCache.IsEmpty())
  {
    nsUInt8* pTarget = AllocateBytes(m_TempCache.GetCount());
    m_ObjectStack.PeekBack()->m_pFirstChild = pTarget;

    nsMemoryUtils::Copy(pTarget, m_TempCache.GetData(), m_TempCache.GetCount());
  }

  m_ObjectStack.PopBack();
}

void nsOpenDdlReader::StorePrimitiveData(bool bThisIsAll, nsUInt32 bytecount, const nsUInt8* pData)
{
  nsUInt8* pTarget = nullptr;

  if (!bThisIsAll || !m_TempCache.IsEmpty())
  {
    // if this is not all, accumulate the data in a temp buffer
    nsUInt32 offset = m_TempCache.GetCount();
    m_TempCache.SetCountUninitialized(m_TempCache.GetCount() + bytecount);
    pTarget = &m_TempCache[offset]; // have to index m_TempCache after the resize, otherwise it could be empty and not like it
  }
  else
  {
    // otherwise, allocate the final storage immediately
    pTarget = AllocateBytes(bytecount);
    m_ObjectStack.PeekBack()->m_pFirstChild = pTarget;
  }

  nsMemoryUtils::Copy(pTarget, pData, bytecount);
}


void nsOpenDdlReader::OnPrimitiveBool(nsUInt32 count, const bool* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(bool) * count, (const nsUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void nsOpenDdlReader::OnPrimitiveInt8(nsUInt32 count, const nsInt8* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(nsInt8) * count, (const nsUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void nsOpenDdlReader::OnPrimitiveInt16(nsUInt32 count, const nsInt16* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(nsInt16) * count, (const nsUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void nsOpenDdlReader::OnPrimitiveInt32(nsUInt32 count, const nsInt32* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(nsInt32) * count, (const nsUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void nsOpenDdlReader::OnPrimitiveInt64(nsUInt32 count, const nsInt64* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(nsInt64) * count, (const nsUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void nsOpenDdlReader::OnPrimitiveUInt8(nsUInt32 count, const nsUInt8* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(nsUInt8) * count, (const nsUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void nsOpenDdlReader::OnPrimitiveUInt16(nsUInt32 count, const nsUInt16* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(nsUInt16) * count, (const nsUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void nsOpenDdlReader::OnPrimitiveUInt32(nsUInt32 count, const nsUInt32* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(nsUInt32) * count, (const nsUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void nsOpenDdlReader::OnPrimitiveUInt64(nsUInt32 count, const nsUInt64* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(nsUInt64) * count, (const nsUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void nsOpenDdlReader::OnPrimitiveFloat(nsUInt32 count, const float* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(float) * count, (const nsUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void nsOpenDdlReader::OnPrimitiveDouble(nsUInt32 count, const double* pData, bool bThisIsAll)
{
  StorePrimitiveData(bThisIsAll, sizeof(double) * count, (const nsUInt8*)pData);
  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}

void nsOpenDdlReader::OnPrimitiveString(nsUInt32 count, const nsStringView* pData, bool bThisIsAll)
{
  const nsUInt32 uiDataSize = count * sizeof(nsStringView);

  const nsUInt32 offset = m_TempCache.GetCount();
  m_TempCache.SetCountUninitialized(m_TempCache.GetCount() + uiDataSize);
  nsStringView* pTarget = (nsStringView*)&m_TempCache[offset];

  for (nsUInt32 i = 0; i < count; ++i)
  {
    pTarget[i] = CopyString(pData[i]);
  }

  m_ObjectStack.PeekBack()->m_uiNumChildElements += count;
}


void nsOpenDdlReader::OnParsingError(nsStringView sMessage, bool bFatal, nsUInt32 uiLine, nsUInt32 uiColumn)
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

void nsOpenDdlReader::ClearDataChunks()
{
  for (nsUInt32 i = 0; i < m_DataChunks.GetCount(); ++i)
  {
    NS_DEFAULT_DELETE(m_DataChunks[i]);
  }

  m_DataChunks.Clear();
}

nsUInt8* nsOpenDdlReader::AllocateBytes(nsUInt32 uiNumBytes)
{
  uiNumBytes = nsMemoryUtils::AlignSize(uiNumBytes, static_cast<nsUInt32>(NS_ALIGNMENT_MINIMUM));

  // if the requested data is very large, just allocate it as an individual chunk
  if (uiNumBytes > s_uiChunkSize / 2)
  {
    nsUInt8* pResult = NS_DEFAULT_NEW_ARRAY(nsUInt8, uiNumBytes).GetPtr();
    m_DataChunks.PushBack(pResult);
    return pResult;
  }

  // if our current chunk is too small, discard the remaining free bytes and just allocate a new chunk
  if (m_uiBytesInChunkLeft < uiNumBytes)
  {
    m_pCurrentChunk = NS_DEFAULT_NEW_ARRAY(nsUInt8, s_uiChunkSize).GetPtr();
    m_uiBytesInChunkLeft = s_uiChunkSize;
    m_DataChunks.PushBack(m_pCurrentChunk);
  }

  // no fulfill the request from the current chunk
  nsUInt8* pResult = m_pCurrentChunk;
  m_pCurrentChunk += uiNumBytes;
  m_uiBytesInChunkLeft -= uiNumBytes;

  return pResult;
}

//////////////////////////////////////////////////////////////////////////

nsUInt32 nsOpenDdlReaderElement::GetNumChildObjects() const
{
  if (m_PrimitiveType != nsOpenDdlPrimitiveType::Custom)
    return 0;

  return m_uiNumChildElements & (~NS_BIT(31)); // Bit 31 stores whether the name is global
}

nsUInt32 nsOpenDdlReaderElement::GetNumPrimitives() const
{
  if (m_PrimitiveType == nsOpenDdlPrimitiveType::Custom)
    return 0;

  return m_uiNumChildElements & (~NS_BIT(31)); // Bit 31 stores whether the name is global
}


bool nsOpenDdlReaderElement::HasPrimitives(nsOpenDdlPrimitiveType type, nsUInt32 uiMinNumberOfPrimitives /*= 1*/) const
{
  /// \test This is new

  if (m_PrimitiveType != type)
    return false;

  return m_uiNumChildElements >= uiMinNumberOfPrimitives;
}

const nsOpenDdlReaderElement* nsOpenDdlReaderElement::FindChild(nsStringView sName) const
{
  NS_ASSERT_DEBUG(m_PrimitiveType == nsOpenDdlPrimitiveType::Custom, "Cannot search for a child object in a primitives list");

  const nsOpenDdlReaderElement* pChild = static_cast<const nsOpenDdlReaderElement*>(m_pFirstChild);

  while (pChild)
  {
    if (pChild->GetName() == sName)
    {
      return pChild;
    }

    pChild = pChild->GetSibling();
  }

  return nullptr;
}

const nsOpenDdlReaderElement* nsOpenDdlReaderElement::FindChildOfType(nsOpenDdlPrimitiveType type, nsStringView sName, nsUInt32 uiMinNumberOfPrimitives /* = 1*/) const
{
  /// \test This is new

  NS_ASSERT_DEBUG(m_PrimitiveType == nsOpenDdlPrimitiveType::Custom, "Cannot search for a child object in a primitives list");

  const nsOpenDdlReaderElement* pChild = static_cast<const nsOpenDdlReaderElement*>(m_pFirstChild);

  while (pChild)
  {
    if (pChild->GetPrimitivesType() == type && pChild->GetName() == sName)
    {
      if (type == nsOpenDdlPrimitiveType::Custom || pChild->GetNumPrimitives() >= uiMinNumberOfPrimitives)
        return pChild;
    }

    pChild = pChild->GetSibling();
  }

  return nullptr;
}

const nsOpenDdlReaderElement* nsOpenDdlReaderElement::FindChildOfType(nsStringView sType, nsStringView sName /*= {}*/) const
{
  const nsOpenDdlReaderElement* pChild = static_cast<const nsOpenDdlReaderElement*>(m_pFirstChild);

  while (pChild)
  {
    if (pChild->GetPrimitivesType() == nsOpenDdlPrimitiveType::Custom && pChild->GetCustomType() == sType && (sName.IsEmpty() || pChild->GetName() == sName))
    {
      return pChild;
    }

    pChild = pChild->GetSibling();
  }

  return nullptr;
}
