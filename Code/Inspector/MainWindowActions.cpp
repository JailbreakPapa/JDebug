#include <Inspector/InspectorPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Inspector/CVarsWidget.moc.h>
#include <Inspector/DataTransferWidget.moc.h>
#include <Inspector/FileWidget.moc.h>
#include <Inspector/GlobalEventsWidget.moc.h>
#include <Inspector/InputWidget.moc.h>
#include <Inspector/LogDockWidget.moc.h>
#include <Inspector/MainWindow.moc.h>
#include <Inspector/MemoryWidget.moc.h>
#include <Inspector/PluginsWidget.moc.h>
#include <Inspector/ReflectionWidget.moc.h>
#include <Inspector/ResourceWidget.moc.h>
#include <Inspector/SubsystemsWidget.moc.h>
#include <Inspector/TimeWidget.moc.h>

void nsQtMainWindow::on_ActionShowWindowLog_triggered()
{
  nsQtLogDockWidget::s_pWidget->toggleView(ActionShowWindowLog->isChecked());
  nsQtLogDockWidget::s_pWidget->raise();
}

void nsQtMainWindow::on_ActionShowWindowMemory_triggered()
{
  nsQtMemoryWidget::s_pWidget->toggleView(ActionShowWindowMemory->isChecked());
  nsQtMemoryWidget::s_pWidget->raise();
}

void nsQtMainWindow::on_ActionShowWindowTime_triggered()
{
  nsQtTimeWidget::s_pWidget->toggleView(ActionShowWindowTime->isChecked());
  nsQtTimeWidget::s_pWidget->raise();
}

void nsQtMainWindow::on_ActionShowWindowInput_triggered()
{
  nsQtInputWidget::s_pWidget->toggleView(ActionShowWindowInput->isChecked());
  nsQtInputWidget::s_pWidget->raise();
}

void nsQtMainWindow::on_ActionShowWindowCVar_triggered()
{
  nsQtCVarsWidget::s_pWidget->toggleView(ActionShowWindowCVar->isChecked());
  nsQtCVarsWidget::s_pWidget->raise();
}

void nsQtMainWindow::on_ActionShowWindowReflection_triggered()
{
  nsQtReflectionWidget::s_pWidget->toggleView(ActionShowWindowReflection->isChecked());
  nsQtReflectionWidget::s_pWidget->raise();
}

void nsQtMainWindow::on_ActionShowWindowSubsystems_triggered()
{
  nsQtSubsystemsWidget::s_pWidget->toggleView(ActionShowWindowSubsystems->isChecked());
  nsQtSubsystemsWidget::s_pWidget->raise();
}

void nsQtMainWindow::on_ActionShowWindowPlugins_triggered()
{
  nsQtPluginsWidget::s_pWidget->toggleView(ActionShowWindowPlugins->isChecked());
  nsQtPluginsWidget::s_pWidget->raise();
}

void nsQtMainWindow::on_ActionShowWindowFile_triggered()
{
  nsQtFileWidget::s_pWidget->toggleView(ActionShowWindowFile->isChecked());
  nsQtFileWidget::s_pWidget->raise();
}

void nsQtMainWindow::on_ActionShowWindowGlobalEvents_triggered()
{
  nsQtGlobalEventsWidget::s_pWidget->toggleView(ActionShowWindowGlobalEvents->isChecked());
  nsQtGlobalEventsWidget::s_pWidget->raise();
}

void nsQtMainWindow::on_ActionShowWindowData_triggered()
{
  nsQtDataWidget::s_pWidget->toggleView(ActionShowWindowData->isChecked());
  nsQtDataWidget::s_pWidget->raise();
}

void nsQtMainWindow::on_ActionShowWindowResource_triggered()
{
  nsQtResourceWidget::s_pWidget->toggleView(ActionShowWindowResource->isChecked());
  nsQtResourceWidget::s_pWidget->raise();
}

void nsQtMainWindow::on_ActionOnTopWhenConnected_triggered()
{
  SetAlwaysOnTop(WhenConnected);
}

void nsQtMainWindow::on_ActionAlwaysOnTop_triggered()
{
  SetAlwaysOnTop(Always);
}

void nsQtMainWindow::on_ActionNeverOnTop_triggered()
{
  SetAlwaysOnTop(Never);
}
