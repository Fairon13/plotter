#ifndef ADDINGASSETDIALOG_H
#define ADDINGASSETDIALOG_H

#include <QDialog>

namespace Ui {
class AddingAssetDialog;
}

class PipeAsset;
class AddingAssetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddingAssetDialog(PipeAsset *pAsset = 0, QWidget *parent = 0);
    ~AddingAssetDialog();

    void    fillAsset(PipeAsset*    pAsset);

public slots:
    void    choosePath();
    void    onAccept();

private:
    Ui::AddingAssetDialog *ui;
};

#endif // ADDINGASSETDIALOG_H
