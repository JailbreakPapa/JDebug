#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_jdgui.h"

class JDGui : public QMainWindow
{
    Q_OBJECT

public:
    JDGui(QWidget *parent = nullptr);
    ~JDGui();

private:
    Ui::JDGuiClass ui;
};
