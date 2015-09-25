#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QDir>
#include "ui_AboutDialog.h"

class AboutDialogClass : public QDialog
{
public:
	AboutDialogClass(QWidget *parent):QDialog(parent) { ui.setupUi(this); }
	~AboutDialogClass() {}

private:
	Ui::dialogAbout		ui;

};


#endif
