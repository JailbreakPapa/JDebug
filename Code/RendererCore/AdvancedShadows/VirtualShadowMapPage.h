#pragma once

#include <Foundation/Basics.h>
#include <RendererCore/AdvancedShadows/VirtualShadowMapCore.h>
#include <RendererCore/RendererCoreDLL.h>
#include <Foundation/Containers/DynamicArray.h>

NS_RENDERERCORE_DLL class nsVirtualShadowMapPage
{
    public:
        nsUInt32 Page_X;
        nsUInt32 Page_Y;
        nsUInt32 Page_MIP;

        explicit nsVirtualShadowMapPage(nsUInt32 InPage_X, nsUInt32 InPage_Y, nsUInt32 InPage_MIP)
            : Page_X(InPage_X)
            , Page_Y(InPage_Y)
            , Page_MIP(InPage_MIP)
        {
        }
};