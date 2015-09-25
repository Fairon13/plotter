#include "addingassetdialog.h"
#include "ui_addingassetdialog.h"
#include <QFileDialog>
#include <QMessageBox>

#include "pipeasset.h"

AddingAssetDialog::AddingAssetDialog(PipeAsset *pAsset, QWidget *parent) : QDialog(parent), ui(new Ui::AddingAssetDialog)
{
    ui->setupUi(this);

    if(pAsset){
        ui->lineName->setText(pAsset->_name);
        ui->linePath->setText(pAsset->_path);
        ui->lineServer->setText(pAsset->_pipeServer);
        ui->dataPort->setValue(pAsset->_dataPort);
        ui->signalsPort->setValue(pAsset->_signalsPort);
    }
}

AddingAssetDialog::~AddingAssetDialog()
{
    delete ui;
}

void AddingAssetDialog::fillAsset(PipeAsset *pAsset)
{
    pAsset->_name = ui->lineName->text();
    pAsset->_path = ui->linePath->text();
    pAsset->_pipeServer = ui->lineServer->text();
    pAsset->_dataPort = ui->dataPort->value();
    pAsset->_signalsPort = ui->signalsPort->value();
    pAsset->_pipeSize = 10000;
}

void AddingAssetDialog::choosePath()
{
    ui->linePath->setText(QFileDialog::getExistingDirectory(this, tr("Choose folder with serpent pitons"), QDir::currentPath(), QFileDialog::DontUseNativeDialog));
}

void AddingAssetDialog::onAccept()
{
    if(ui->lineName->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Input Error"), tr("Name is empty"));
        return;
    }

    if(ui->linePath->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Input Error"), tr("Path to pitons is empty"));
        return;
    }

    if(ui->lineServer->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Input Error"), tr("Server name is empty"));
        return;
    }

    accept();
}
