#include <Texture/TexturePCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Texture/Utils/TextureAtlasDesc.h>

nsResult nsTextureAtlasCreationDesc::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(3);

  if (m_Layers.GetCount() > 255u)
    return NS_FAILURE;

  const nsUInt8 uiNumLayers = static_cast<nsUInt8>(m_Layers.GetCount());
  inout_stream << uiNumLayers;

  for (nsUInt32 l = 0; l < uiNumLayers; ++l)
  {
    inout_stream << m_Layers[l].m_Usage;
    inout_stream << m_Layers[l].m_uiNumChannels;
  }

  inout_stream << m_Items.GetCount();
  for (auto& item : m_Items)
  {
    inout_stream << item.m_uiUniqueID;
    inout_stream << item.m_uiFlags;

    for (nsUInt32 l = 0; l < uiNumLayers; ++l)
    {
      inout_stream << item.m_sLayerInput[l];
    }

    inout_stream << item.m_sAlphaInput;
  }

  return NS_SUCCESS;
}

nsResult nsTextureAtlasCreationDesc::Deserialize(nsStreamReader& inout_stream)
{
  const nsTypeVersion uiVersion = inout_stream.ReadVersion(3);

  nsUInt8 uiNumLayers = 0;
  inout_stream >> uiNumLayers;

  m_Layers.SetCount(uiNumLayers);

  for (nsUInt32 l = 0; l < uiNumLayers; ++l)
  {
    inout_stream >> m_Layers[l].m_Usage;
    inout_stream >> m_Layers[l].m_uiNumChannels;
  }

  nsUInt32 uiNumItems = 0;
  inout_stream >> uiNumItems;
  m_Items.SetCount(uiNumItems);

  for (auto& item : m_Items)
  {
    inout_stream >> item.m_uiUniqueID;
    inout_stream >> item.m_uiFlags;

    for (nsUInt32 l = 0; l < uiNumLayers; ++l)
    {
      inout_stream >> item.m_sLayerInput[l];
    }

    if (uiVersion >= 3)
    {
      inout_stream >> item.m_sAlphaInput;
    }
  }

  return NS_SUCCESS;
}

nsResult nsTextureAtlasCreationDesc::Save(nsStringView sFile) const
{
  nsFileWriter file;
  NS_SUCCEED_OR_RETURN(file.Open(sFile));

  return Serialize(file);
}

nsResult nsTextureAtlasCreationDesc::Load(nsStringView sFile)
{
  nsFileReader file;
  NS_SUCCEED_OR_RETURN(file.Open(sFile));

  return Deserialize(file);
}

void nsTextureAtlasRuntimeDesc::Clear()
{
  m_uiNumLayers = 0;
  m_Items.Clear();
}

nsResult nsTextureAtlasRuntimeDesc::Serialize(nsStreamWriter& inout_stream) const
{
  m_Items.Sort();

  inout_stream << m_uiNumLayers;
  inout_stream << m_Items.GetCount();

  for (nsUInt32 i = 0; i < m_Items.GetCount(); ++i)
  {
    inout_stream << m_Items.GetKey(i);
    inout_stream << m_Items.GetValue(i).m_uiFlags;

    for (nsUInt32 l = 0; l < m_uiNumLayers; ++l)
    {
      const auto& r = m_Items.GetValue(i).m_LayerRects[l];
      inout_stream << r.x;
      inout_stream << r.y;
      inout_stream << r.width;
      inout_stream << r.height;
    }
  }

  return NS_SUCCESS;
}

nsResult nsTextureAtlasRuntimeDesc::Deserialize(nsStreamReader& inout_stream)
{
  Clear();

  inout_stream >> m_uiNumLayers;

  nsUInt32 uiNumItems = 0;
  inout_stream >> uiNumItems;
  m_Items.Reserve(uiNumItems);

  for (nsUInt32 i = 0; i < uiNumItems; ++i)
  {
    nsUInt32 key = 0;
    inout_stream >> key;

    auto& item = m_Items[key];
    inout_stream >> item.m_uiFlags;

    for (nsUInt32 l = 0; l < m_uiNumLayers; ++l)
    {
      auto& r = item.m_LayerRects[l];
      inout_stream >> r.x;
      inout_stream >> r.y;
      inout_stream >> r.width;
      inout_stream >> r.height;
    }
  }

  m_Items.Sort();
  return NS_SUCCESS;
}
