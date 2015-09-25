#include "addingbusassetdialog.h"
#include <QFileDialog>
#include <QMessageBox>

#include "busasset.h"

AddingBusAssetDialog::AddingBusAssetDialog(BusAsset *pAsset, QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

    if(pAsset)
    {
        ui.nameAsset->setText(pAsset->_name);
        ui.nameFederate->setText(pAsset->_nameFederate);
        ui.nameFederation->setText(pAsset->_nameFederation);
        ui.urlFederation->setText(pAsset->_urlFederate);
        ui.protoPath->setText(pAsset->_protoPath);
    }
}

void AddingBusAssetDialog::fillAsset(BusAsset *pAsset)
{
    pAsset->_name = ui.nameAsset->text();
    pAsset->_nameFederate = ui.nameFederate->text();
    pAsset->_nameFederation = ui.nameFederation->text();
    pAsset->_urlFederate = ui.urlFederation->text();
    pAsset->_protoPath = ui.protoPath->text();
}

void AddingBusAssetDialog::choosePath()
{
    ui.protoPath->setText(QFileDialog::getOpenFileName(this, tr("Choose proto file to load"), QDir::currentPath(), "Proto file (*.proto)", 0, QFileDialog::DontUseNativeDialog));
}

void AddingBusAssetDialog::onAccept()
{
    if(ui.nameAsset->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Input Error"), tr("Name is empty"));
        return;
    }

    if(ui.protoPath->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Input Error"), tr("Path to proto file is empty"));
        return;
    }

    if(ui.nameFederate->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Input Error"), tr("Federate name is empty"));
        return;
    }

    if(ui.nameFederation->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Input Error"), tr("Federation name is empty"));
        return;
    }

    if(ui.urlFederation->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Input Error"), tr("Federation URL is empty"));
        return;
    }

    accept();
}
