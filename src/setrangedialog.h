#ifndef SETRANGEDIALOG_H
#define SETRANGEDIALOG_H

#include <QDialog>

namespace Ui {
class SetRangeDialog;
}

class SetRangeDialog : public QDialog
{
    Q_OBJECT


public:
    explicit SetRangeDialog(QWidget *parent = 0);
    ~SetRangeDialog();

    void    setValue(double valA, double valB);

    double  minValue();
    double  maxValue();

private:
    Ui::SetRangeDialog *ui;

};

#endif // SETRANGEDIALOG_H
