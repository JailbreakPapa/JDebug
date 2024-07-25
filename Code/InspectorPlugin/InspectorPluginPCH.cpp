#include <InspectorPlugin/InspectorPluginPCH.h>

NS_STATICLINK_LIBRARY(InspectorPlugin)
{
  if (bReturn)
    return;

  NS_STATICLINK_REFERENCE(InspectorPlugin_App);
  NS_STATICLINK_REFERENCE(InspectorPlugin_CVars);
  NS_STATICLINK_REFERENCE(InspectorPlugin_GlobalEvents);
  NS_STATICLINK_REFERENCE(InspectorPlugin_Input);
  NS_STATICLINK_REFERENCE(InspectorPlugin_Log);
  NS_STATICLINK_REFERENCE(InspectorPlugin_Main);
  NS_STATICLINK_REFERENCE(InspectorPlugin_Memory);
  NS_STATICLINK_REFERENCE(InspectorPlugin_OSFile);
  NS_STATICLINK_REFERENCE(InspectorPlugin_Plugins);
  NS_STATICLINK_REFERENCE(InspectorPlugin_Startup);
  NS_STATICLINK_REFERENCE(InspectorPlugin_Stats);
  NS_STATICLINK_REFERENCE(InspectorPlugin_Time);
}
