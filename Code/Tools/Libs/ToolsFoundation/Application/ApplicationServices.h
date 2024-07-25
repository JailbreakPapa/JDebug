#pragma once

#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Strings/String.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class nsDocument;

class NS_TOOLSFOUNDATION_DLL nsApplicationServices
{
  NS_DECLARE_SINGLETON(nsApplicationServices);

public:
  nsApplicationServices();

  /// \brief A writable folder in which application specific user data may be stored
  nsString GetApplicationUserDataFolder() const;

  /// \brief A read-only folder in which application specific data may be located
  nsString GetApplicationDataFolder() const;

  /// \brief The writable location where the application should store preferences (user specific settings)
  nsString GetApplicationPreferencesFolder() const;

  /// \brief The writable location where preferences for the current nsToolsProject should be stored (user specific settings)
  nsString GetProjectPreferencesFolder() const;

  nsString GetProjectPreferencesFolder(nsStringView sProjectFilePath) const;

  /// \brief The writable location where preferences for the given nsDocument should be stored (user specific settings)
  nsString GetDocumentPreferencesFolder(const nsDocument* pDocument) const;

  /// \brief The read-only folder where pre-compiled binaries for external tools can be found
  nsString GetPrecompiledToolsFolder(bool bUsePrecompiledTools) const;

  /// \brief The folder where under which the sample projects are stored
  nsString GetSampleProjectsFolder() const;
};
