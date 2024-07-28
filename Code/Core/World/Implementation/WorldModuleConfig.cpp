#include <Core/CorePCH.h>

#include <Core/World/WorldModule.h>
#include <Core/World/WorldModuleConfig.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>

nsResult nsWorldModuleConfig::Save()
{
  m_InterfaceImpls.Sort();

  nsStringBuilder sPath;
  sPath = ":project/WorldModules.ddl";

  nsFileWriter file;
  if (file.Open(sPath).Failed())
    return NS_FAILURE;

  nsOpenDdlWriter writer;
  writer.SetOutputStream(&file);
  writer.SetCompactMode(false);
  writer.SetPrimitiveTypeStringMode(nsOpenDdlWriter::TypeStringMode::Compliant);

  for (auto& interfaceImpl : m_InterfaceImpls)
  {
    writer.BeginObject("InterfaceImpl");

    nsOpenDdlUtils::StoreString(writer, interfaceImpl.m_sInterfaceName, "Interface");
    nsOpenDdlUtils::StoreString(writer, interfaceImpl.m_sImplementationName, "Implementation");

    writer.EndObject();
  }

  return NS_SUCCESS;
}

void nsWorldModuleConfig::Load()
{
  const char* szPath = ":project/WorldModules.ddl";

  NS_LOG_BLOCK("nsWorldModuleConfig::Load()", szPath);

  m_InterfaceImpls.Clear();

  nsFileReader file;
  if (file.Open(szPath).Failed())
  {
    nsLog::Dev("World module config file is not available: '{0}'", szPath);
    return;
  }
  else
  {
    nsLog::Success("World module config file is available: '{0}'", szPath);
  }

  nsOpenDdlReader reader;
  if (reader.ParseDocument(file, 0, nsLog::GetThreadLocalLogSystem()).Failed())
  {
    nsLog::Error("Failed to parse world module config file '{0}'", szPath);
    return;
  }

  const nsOpenDdlReaderElement* pTree = reader.GetRootElement();

  for (const nsOpenDdlReaderElement* pInterfaceImpl = pTree->GetFirstChild(); pInterfaceImpl != nullptr;
       pInterfaceImpl = pInterfaceImpl->GetSibling())
  {
    if (!pInterfaceImpl->IsCustomType("InterfaceImpl"))
      continue;

    const nsOpenDdlReaderElement* pInterface = pInterfaceImpl->FindChildOfType(nsOpenDdlPrimitiveType::String, "Interface");
    const nsOpenDdlReaderElement* pImplementation = pInterfaceImpl->FindChildOfType(nsOpenDdlPrimitiveType::String, "Implementation");

    // this prevents duplicates
    AddInterfaceImplementation(pInterface->GetPrimitivesString()[0], pImplementation->GetPrimitivesString()[0]);
  }
}

void nsWorldModuleConfig::Apply()
{
  NS_LOG_BLOCK("nsWorldModuleConfig::Apply");

  for (const auto& interfaceImpl : m_InterfaceImpls)
  {
    nsWorldModuleFactory::GetInstance()->RegisterInterfaceImplementation(interfaceImpl.m_sInterfaceName, interfaceImpl.m_sImplementationName);
  }
}

void nsWorldModuleConfig::AddInterfaceImplementation(nsStringView sInterfaceName, nsStringView sImplementationName)
{
  for (auto& interfaceImpl : m_InterfaceImpls)
  {
    if (interfaceImpl.m_sInterfaceName == sInterfaceName)
    {
      interfaceImpl.m_sImplementationName = sImplementationName;
      return;
    }
  }

  m_InterfaceImpls.PushBack({sInterfaceName, sImplementationName});
}

void nsWorldModuleConfig::RemoveInterfaceImplementation(nsStringView sInterfaceName)
{
  for (nsUInt32 i = 0; i < m_InterfaceImpls.GetCount(); ++i)
  {
    if (m_InterfaceImpls[i].m_sInterfaceName == sInterfaceName)
    {
      m_InterfaceImpls.RemoveAtAndCopy(i);
      return;
    }
  }
}
