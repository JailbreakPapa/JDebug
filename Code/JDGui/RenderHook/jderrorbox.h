#pragma once

#include <QDialog>
#include "ui_jderrorbox.h"

class JDErrorBox : public QDialog
{
	Q_OBJECT

public:
	JDErrorBox(QWidget *parent = nullptr);
	~JDErrorBox();

private:
	Ui::JDErrorBoxClass ui;
};
