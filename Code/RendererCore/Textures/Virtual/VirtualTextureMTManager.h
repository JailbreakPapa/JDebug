#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>
#include <Foundation/IO/MemoryStream.h>
#include <RendererCore/Pipeline/Declarations.h>
#include <RendererCore/RenderContext/Implementation/RenderContextStructs.h>
#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/RendererFoundationDLL.h>

/// @brief This class handles the management of virtual textures in a multithreaded environment.
static class NS_RENDERERCORE_DLL nsVirtualTextureMTManager
{
   /// TODO: Implement this class. is the class supposed to handle page queries and cache management? should it send jobs to the worker threads, or should the texture do it itself? 
   nsVirtualTextureMTManager();
   virtual ~nsVirtualTextureMTManager() = default;
public:
   /// @brief This function is called by the texture to request a page to be loaded.
   /// @param[in] PageID The ID of the page to be loaded.
   /// @param[in] QualityLevel The quality level of the page to be loaded.
   /// @param[in] Callback The callback to be called when the page is loaded.
   void RequestPage(nsUInt32 PageID, nsUInt8 QualityLevel, nsDelegate<void() Callback);
   /// @brief This function is called by the texture to request a page to be unloaded.
   /// @param[in] PageID The ID of the page to be unloaded.
   void UnloadPage(nsUInt32 PageID);


};