#include "dataeditorclass.h"
#include <QLineEdit>
#include "datacollectorclass.h"
#include "plotviewclass.h"

DataEditorClass::DataEditorClass(): QStyledItemDelegate(0)
{

}

QWidget *DataEditorClass::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column() == 4 || index.column() == 5 || index.column() == 6)
        return new QLineEdit(parent);

    return 0;
}

void DataEditorClass::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    DataCollectorClass*     pCollector = (DataCollectorClass*)index.sibling(index.row(), 0).data(Qt::UserRole).value<void*>();

    if(pCollector)
    {
        QLineEdit*  pEditor = (QLineEdit*)editor;

        if(index.column() == 4)
            pEditor->setText(QString::number(pCollector->_scale));
        else if(index.column() == 5)
            pEditor->setText(QString::number(pCollector->_offset));
        else if(index.column() == 6)
                pEditor->setText(pCollector->_alias);
    }
}

void DataEditorClass::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    DataCollectorClass*     pCollector = (DataCollectorClass*)index.sibling(index.row(), 0).data(Qt::UserRole).value<void*>();

    if(pCollector)
    {
        QLineEdit*  pEditor = (QLineEdit*)editor;

        if(index.column() == 4)
        {
            pCollector->setLinear(pEditor->text().toDouble(), pCollector->_offset);
            model->setData(index, pEditor->text(), Qt::DisplayRole);

            QModelIndex valIdx = index.sibling(index.row(), 3);
            model->setData(valIdx, QString::number(pCollector->_value), Qt::DisplayRole);

            PlotViewClass::updatePlots();
        } else if(index.column() == 5)
        {
            pCollector->setLinear(pCollector->_scale, pEditor->text().toDouble());
            model->setData(index, pEditor->text(), Qt::DisplayRole);

            QModelIndex valIdx = index.sibling(index.row(), 3);
            model->setData(valIdx, QString::number(pCollector->_value), Qt::DisplayRole);

            PlotViewClass::updatePlots();
        } else if(index.column() == 6)
        {
            pCollector->setAlias(pEditor->text());
            model->setData(index, pEditor->text(), Qt::DisplayRole);

            PlotViewClass::updatePlots();
        }
    }
}

