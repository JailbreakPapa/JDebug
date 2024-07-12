#include <Texture/TexturePCH.h>

#include <Texture/Utils/TexturePacker.h>

nsTexturePacker::nsTexturePacker() = default;

nsTexturePacker::~nsTexturePacker() = default;

void nsTexturePacker::SetTextureSize(nsUInt32 uiWidth, nsUInt32 uiHeight, nsUInt32 uiReserveTextures /*= 0*/)
{
  m_uiWidth = uiWidth;
  m_uiHeight = uiHeight;

  m_Textures.Clear();
  m_Textures.Reserve(uiReserveTextures);

  // initializes all values to false
  m_Grid.Clear();
  m_Grid.SetCount(m_uiWidth * m_uiHeight);
}

void nsTexturePacker::AddTexture(nsUInt32 uiWidth, nsUInt32 uiHeight)
{
  Texture& tex = m_Textures.ExpandAndGetRef();
  tex.m_Size.x = uiWidth;
  tex.m_Size.y = uiHeight;
  tex.m_Priority = 2 * uiWidth + 2 * uiHeight;
}

struct sortdata
{
  NS_DECLARE_POD_TYPE();

  nsInt32 m_Priority;
  nsInt32 m_Index;
};

nsResult nsTexturePacker::PackTextures()
{
  nsDynamicArray<sortdata> sorted;
  sorted.SetCountUninitialized(m_Textures.GetCount());

  for (nsUInt32 i = 0; i < m_Textures.GetCount(); ++i)
  {
    sorted[i].m_Index = i;
    sorted[i].m_Priority = m_Textures[i].m_Priority;
  }

  sorted.Sort([](const sortdata& lhs, const sortdata& rhs) -> bool
    { return lhs.m_Priority > rhs.m_Priority; });

  for (nsUInt32 idx = 0; idx < sorted.GetCount(); ++idx)
  {
    if (!TryPlaceTexture(sorted[idx].m_Index))
      return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsUInt32 nsTexturePacker::PosToIndex(nsUInt32 x, nsUInt32 y) const
{
  return (y * m_uiWidth + x);
}

bool nsTexturePacker::TryPlaceTexture(nsUInt32 idx)
{
  Texture& tex = m_Textures[idx];

  for (nsUInt32 y = 0; y < m_uiHeight; ++y)
  {
    for (nsUInt32 x = 0; x < m_uiWidth; ++x)
    {
      if (!TryPlaceAt(nsVec2U32(x, y), tex.m_Size))
        continue;

      tex.m_Position.Set(x, y);
      return true;
    }
  }

  return false;
}

bool nsTexturePacker::CanPlaceAt(nsVec2U32 pos, nsVec2U32 size)
{
  if (pos.x + size.x > m_uiWidth)
    return false;
  if (pos.y + size.y > m_uiHeight)
    return false;

  for (nsUInt32 y = 0; y < size.y; ++y)
  {
    for (nsUInt32 x = 0; x < size.x; ++x)
    {
      if (m_Grid[PosToIndex(pos.x + x, pos.y + y)])
        return false;
    }
  }

  return true;
}

bool nsTexturePacker::TryPlaceAt(nsVec2U32 pos, nsVec2U32 size)
{
  if (!CanPlaceAt(pos, size))
    return false;

  for (nsUInt32 y = 0; y < size.y; ++y)
  {
    for (nsUInt32 x = 0; x < size.x; ++x)
    {
      m_Grid[PosToIndex(pos.x + x, pos.y + y)] = true;
    }
  }

  return true;
}
