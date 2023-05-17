#pragma once

#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Strings/String.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class wdDocument;

class WD_TOOLSFOUNDATION_DLL wdApplicationServices
{
  WD_DECLARE_SINGLETON(wdApplicationServices);

public:
  wdApplicationServices();

  /// \brief A writable folder in which application specific user data may be stored
  wdString GetApplicationUserDataFolder() const;

  /// \brief A read-only folder in which application specific data may be located
  wdString GetApplicationDataFolder() const;

  /// \brief The writable location where the application should store preferences (user specific settings)
  wdString GetApplicationPreferencesFolder() const;

  /// \brief The writable location where preferences for the current wdToolsProject should be stored (user specific settings)
  wdString GetProjectPreferencesFolder() const;

  /// \brief The writable location where preferences for the given wdDocument should be stored (user specific settings)
  wdString GetDocumentPreferencesFolder(const wdDocument* pDocument) const;

  /// \brief The read-only folder where pre-compiled binaries for external tools can be found
  wdString GetPrecompiledToolsFolder(bool bUsePrecompiledTools) const;

  /// \brief The folder where under which the sample projects are stored
  wdString GetSampleProjectsFolder() const;
};
