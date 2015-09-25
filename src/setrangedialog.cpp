#include "setrangedialog.h"
#include "ui_setrangedialog.h"

void SetRangeDialog::setValue(double valA, double valB)
{
    double  minVal = qMin(valA, valB);
    double  maxVal = qMax(valA, valB);

    ui->valueMin->setText(QString::number(minVal));
    ui->valueMax->setText(QString::number(maxVal));
}

double SetRangeDialog::minValue()
{
    return ui->valueMin->text().toDouble();
}

double SetRangeDialog::maxValue()
{
    return ui->valueMax->text().toDouble();
}

SetRangeDialog::SetRangeDialog(QWidget *parent) : QDialog(parent), ui(new Ui::SetRangeDialog)
{
    ui->setupUi(this);
}

SetRangeDialog::~SetRangeDialog()
{
    delete ui;
}
