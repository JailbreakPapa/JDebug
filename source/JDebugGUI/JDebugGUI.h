#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_JDebugGUI.h"

class JDebugGUI : public QMainWindow
{
    Q_OBJECT

public:
    JDebugGUI(QWidget *parent = nullptr);
    ~JDebugGUI();

private:
    Ui::JDebugGUIClass ui;
};
