#ifndef ADDINGBUSASSETDIALOG_H
#define ADDINGBUSASSETDIALOG_H

#include "ui_addingbusassetdialog.h"

class BusAsset;
class AddingBusAssetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddingBusAssetDialog(BusAsset* pAsset = 0, QWidget *parent = 0);

    void    fillAsset(BusAsset*    pAsset);

public slots:
    void    choosePath();
    void    onAccept();

private:
    Ui::AddingBusAssetDialog ui;
};

#endif // ADDINGBUSASSETDIALOG_H
