/*
 *   Copyright (c) 2023 Watch Dogs LLC
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/GuiFoundationPCH.h>
/// @brief Class for loading QT Style-sheets. it is recommended to use lua files for advanced stylesheets.
class WD_GUIFOUNDATION_DLL wdApplicationTheme
{
public:
  wdApplicationTheme() {}
  ~wdApplicationTheme() {}

 public:
  /// <summary>
  /// Loads the current qss stylesheet that is listed in: basedesolationtheme.qss.
  /// </summary>
  /// <param name="Application:"> Application to apply the stored stylesheet to. </param>
  static void LoadBaseStyleSheet(QApplication& Application);
  /// <summary>
  /// Loads the base palette for all applications.
  /// </summary>
  /// <param name="Applicaiton"> Application to apply the stored stylesheet to. </param>
  static void LoadBasePalette(QApplication& Applicaiton);
};
