#pragma once

#include <Foundation/Containers/ArrayMap.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Rect.h>
#include <Foundation/Strings/String.h>
#include <Texture/TexConv/TexConvEnums.h>

struct NS_TEXTURE_DLL nsTextureAtlasCreationDesc
{
  struct Layer
  {
    nsEnum<nsTexConvUsage> m_Usage;
    nsUInt8 m_uiNumChannels = 4;
  };

  struct Item
  {
    nsUInt32 m_uiUniqueID;
    nsUInt32 m_uiFlags;
    nsString m_sAlphaInput;
    nsString m_sLayerInput[4];
  };

  nsHybridArray<Layer, 4> m_Layers;
  nsDynamicArray<Item> m_Items;

  nsResult Serialize(nsStreamWriter& inout_stream) const;
  nsResult Deserialize(nsStreamReader& inout_stream);

  nsResult Save(nsStringView sFile) const;
  nsResult Load(nsStringView sFile);
};

struct NS_TEXTURE_DLL nsTextureAtlasRuntimeDesc
{
  struct Item
  {
    nsUInt32 m_uiFlags;
    nsRectU32 m_LayerRects[4];
  };

  nsUInt32 m_uiNumLayers = 0;
  nsArrayMap<nsUInt32, Item> m_Items;

  void Clear();

  nsResult Serialize(nsStreamWriter& inout_stream) const;
  nsResult Deserialize(nsStreamReader& inout_stream);
};
