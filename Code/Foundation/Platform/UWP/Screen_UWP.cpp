#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)

#  include <Foundation/Basics/Platform/uwp/UWPUtils.h>
#  include <Foundation/System/Screen.h>
#  include <windows.graphics.display.h>

nsResult nsScreen::EnumerateScreens(nsDynamicArray<nsScreenInfo>& out_Screens)
{
  out_Screens.Clear();

  ComPtr<ABI::Windows::Graphics::Display::IDisplayInformationStatics> displayInformationStatics;
  NS_HRESULT_TO_FAILURE_LOG(ABI::Windows::Foundation::GetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Display_DisplayInformation).Get(), &displayInformationStatics));

  // Get information for current screen. Todo: How to get information for secondary screen?
  ComPtr<ABI::Windows::Graphics::Display::IDisplayInformation> currentDisplayInformation;
  NS_HRESULT_TO_FAILURE_LOG(displayInformationStatics->GetForCurrentView(currentDisplayInformation.GetAddressOf()));
  ComPtr<ABI::Windows::Graphics::Display::IDisplayInformation4> currentDisplayInformation4;
  NS_HRESULT_TO_FAILURE(currentDisplayInformation.As(&currentDisplayInformation4));

  nsScreenInfo& currentScreen = out_Screens.ExpandAndGetRef();
  currentScreen.m_sDisplayName = "Current Display";
  currentScreen.m_iOffsetX = 0;
  currentScreen.m_iOffsetY = 0;
  currentScreen.m_iResolutionX = 0;
  currentScreen.m_iResolutionY = 0;
  currentScreen.m_bIsPrimary = true;

  UINT rawPixelWidth, rawPixelHeight;
  NS_HRESULT_TO_FAILURE(currentDisplayInformation4->get_ScreenHeightInRawPixels(&rawPixelHeight));
  NS_HRESULT_TO_FAILURE(currentDisplayInformation4->get_ScreenWidthInRawPixels(&rawPixelWidth));

  currentScreen.m_iResolutionX = static_cast<nsInt32>(rawPixelWidth);
  currentScreen.m_iResolutionY = static_cast<nsInt32>(rawPixelHeight);

  return NS_SUCCESS;
}

#endif
