
/*
 *   Copyright (c) 2023 Watch Dogs LLC
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <GuiFoundation/Theme/ApplicationTheme.h>



void wdApplicationTheme::LoadBaseStyleSheet(QApplication& Application)
{
  // NOTE: All style sheets should be included in the resources.qrc!
  QFile f(":/StyleSheets/basedesolationtheme.qss");
  f.open(QFile::ReadOnly);

  // Apply style sheet to the listed application.
  QString style(f.readAll());
  Application.setStyleSheet(style);
}

void wdApplicationTheme::LoadBasePalette(QApplication& Applicaiton)
{
  // STYLE SHEETS
  QApplication::setStyle(QStyleFactory::create("fusion"));
  QPalette palette;

  palette.setColor(QPalette::WindowText, QColor(200, 200, 200, 255));
  palette.setColor(QPalette::Button, QColor(50, 50, 50, 255));
  palette.setColor(QPalette::Light, QColor(30, 30, 30, 255));
  palette.setColor(QPalette::Midlight, QColor(30, 30, 30, 255));
  palette.setColor(QPalette::Dark, QColor(30, 30, 30, 255));
  palette.setColor(QPalette::Mid, QColor(30, 30, 30, 255));
  palette.setColor(QPalette::Text, QColor(200, 200, 200, 255));
  palette.setColor(QPalette::BrightText, QColor(37, 37, 37, 255));
  palette.setColor(QPalette::ButtonText, QColor(200, 200, 200, 255));
  palette.setColor(QPalette::Base, QColor(30, 30, 30, 255));
  palette.setColor(QPalette::Window, QColor(30, 30, 30, 255));
  palette.setColor(QPalette::Shadow, QColor(0, 0, 0, 255));
  palette.setColor(QPalette::Highlight, QColor(70, 100, 222, 255));
  palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255, 255));
  palette.setColor(QPalette::Link, QColor(0, 148, 255, 255));
  palette.setColor(QPalette::LinkVisited, QColor(255, 0, 220, 255));
  palette.setColor(QPalette::AlternateBase, QColor(30, 30, 30, 255));
  QBrush NoRoleBrush(QColor(0, 0, 0, 255), Qt::NoBrush);
  palette.setBrush(QPalette::NoRole, NoRoleBrush);
  palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220, 255));
  palette.setColor(QPalette::ToolTipText, QColor(0, 0, 0, 255));

  palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128, 255));
  palette.setColor(QPalette::Disabled, QPalette::Button, QColor(70, 100, 222, 255));
  palette.setColor(QPalette::Disabled, QPalette::Text, QColor(105, 105, 105, 255));
  palette.setColor(QPalette::Disabled, QPalette::BrightText, QColor(255, 255, 255, 255));
  palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128, 255));
  palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(70, 100, 222, 255));

  QApplication::setPalette(palette);
  // STYLE SHEETS
}
