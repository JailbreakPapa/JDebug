#pragma once

#include <QWidget>
#include "ui_JDWorldWidget.h"

class JDWorldWidget : public QWidget
{
	Q_OBJECT

public:
	JDWorldWidget(QWidget *parent = nullptr);
	~JDWorldWidget();

private:
	Ui::JDWorldWidgetClass ui;
};
