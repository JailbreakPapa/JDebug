#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Vec2.h>
#include <Texture/TextureDLL.h>

class NS_TEXTURE_DLL nsTexturePacker
{
public:
  struct Texture
  {
    NS_DECLARE_POD_TYPE();

    nsVec2U32 m_Size;
    nsVec2U32 m_Position;
    nsInt32 m_Priority = 0;
  };

  nsTexturePacker();
  ~nsTexturePacker();

  void SetTextureSize(nsUInt32 uiWidth, nsUInt32 uiHeight, nsUInt32 uiReserveTextures = 0);

  void AddTexture(nsUInt32 uiWidth, nsUInt32 uiHeight);

  const nsDynamicArray<Texture>& GetTextures() const { return m_Textures; }

  nsResult PackTextures();

private:
  bool CanPlaceAt(nsVec2U32 pos, nsVec2U32 size);
  bool TryPlaceAt(nsVec2U32 pos, nsVec2U32 size);
  nsUInt32 PosToIndex(nsUInt32 x, nsUInt32 y) const;
  bool TryPlaceTexture(nsUInt32 idx);

  nsUInt32 m_uiWidth = 0;
  nsUInt32 m_uiHeight = 0;

  nsDynamicArray<Texture> m_Textures;
  nsDynamicArray<bool> m_Grid;
};
