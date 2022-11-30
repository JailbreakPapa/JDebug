#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DebuggerGUI.h"

class DebuggerGUI : public QMainWindow
{
    Q_OBJECT

public:
    DebuggerGUI(QWidget *parent = nullptr);
    ~DebuggerGUI();

private:
    Ui::DebuggerGUIClass ui;
};
