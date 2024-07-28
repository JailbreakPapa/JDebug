 /*
  * Reference(s):
  * - Sparse Virtual Textures by Sean Barrett
  *   http://web.archive.org/web/20190103162611/http://silverspaceship.com/src/svt/
  * - Based on Virtual Texture Demo by Brad Blanchard
  *   http://web.archive.org/web/20190103162638/http://linedef.com/virtual-texture-demo.html
  * - Mars texture
  *   http://web.archive.org/web/20190103162730/http://www.celestiamotherlode.net/catalog/mars.php
  */
/// Heavily modified version of BGFX's VirtualTextures: https://github.com/bkaradzic/bgfx/blob/4f1728dfb88600d771929eb8f9e21ac0fc4bb2d3/examples/40-svt/vt.h
/// Licensed Under the BSD-2-Clause License.
#pragma once

#include <Foundation/Types/Delegate.h>
#include <RendererFoundation/Resources/Texture.h>
#include <RendererCore/RendererCoreDLL.h>


class NS_RENDERERCORE_DLL nsVirtualTexturePage
{
public:
  nsVirtualTexturePage(nsUInt32 PageID, nsUInt8 QualityLevel, nsDelegate<void()> Callback);
  ~nsVirtualTexturePage() = default;

  nsUInt32 GetPageID() const { return m_PageID; }
  nsUInt8 GetQualityLevel() const { return m_QualityLevel; }
  nsUInt32 GetXSize() const { return m_xsize; }
  nsUInt32 GetYSize() const { return m_ysize; }
  nsDelegate<void()> GetCallback() const { return m_Callback; }

private:
  nsUInt32 m_PageID;
  nsUInt8 m_QualityLevel;
  nsUInt32 m_xsize;
  nsUInt32 m_ysize;
  nsDelegate<void()> m_Callback;
};

class NS_RENDERERCORE_DLL nsVirtualTextureStagingPool
{
    nsVirtualTextureStagingPool(nsUInt32 _width, nsUInt32 _height, nsUInt32 _count, bool _readBack);
	~nsVirtualTextureStagingPool();

	void Grow(nsUInt32 count);

	nsGALTexture GetSPTexture();
	void				Next();

private:
	nsDynamicArray<nsGALTexture>  m_stagingTextures;
	nsUInt32			m_stagingTextureIndex;
	nsUInt32			m_width;
	nsUInt32			m_height;
	nsUInt64	m_flags;
};
// VirtualTextureInfo
struct VirtualTextureInfo
{
	VirtualTextureInfo();
	int GetPageSize() const;
	int GetPageTableSize() const;

	int m_virtualTextureSize = 0;
	int m_tileSize = 0;
	int m_borderSize = 0;
};
// PageIndexer
struct nsVirtualTexturePageIndexer
{
public:
	nsVirtualTexturePageIndexer(VirtualTextureInfo* _info);

	int  getIndexFromPage(nsVirtualTexturePage page);
	nsVirtualTexturePage getPageFromIndex(int index);

	bool isValid(nsVirtualTexturePage page);
	int  getCount() const;
	int  getMipCount() const;

private:
	VirtualTextureInfo* m_info;
	int                 m_mipcount;
	int					m_count;

	nsDynamicArray<int>    m_offsets; // This stores the offsets to the first page of the start of a mipmap level
	nsDynamicArray<int>    m_sizes; // This stores the sizes of various mip levels
	nsDynamicArray<nsVirtualTexturePage>   m_reverse;
};

// PageTable
class PageTable
{
public:
	PageTable(PageCache* _cache, VirtualTextureInfo* _info, PageIndexer* _indexer);
	~PageTable();

	void update(bgfx::ViewId blitViewId);
	nsGALTextureHandle getTexture();

private:
	VirtualTextureInfo* m_info;
	nsGALTextureHandle  m_texture;
	nsVirtualTexturePageIndexer*		m_indexer;
	nsVirtualTextureQuadtree*			m_quadtree;
	bool				m_quadtreeDirty;

	nsDynamicArray<SimpleImage*>			m_images;
	nsDynamicArray<bgfx::TextureHandle>	m_stagingTextures;
};