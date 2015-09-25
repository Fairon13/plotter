#include "dataviewclass.h"
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QDir>
#include <QMimeData>
#include <QInputDialog>
#include <QMenu>

#include "pipeasset.h"
#include "busasset.h"
#include "datacollectorclass.h"
#include "plotviewclass.h"
#include "addingassetdialog.h"
#include "dataeditorclass.h"

#define  DataViewType_Asset  QTreeWidgetItem::UserType
#define  DataViewType_Data   QTreeWidgetItem::UserType + 1

DataViewClass::DataViewClass(AssetClass* pAsset, QWidget *parent) : QTreeWidget(parent)
{
    _asset = pAsset;

    setColumnCount(7);

    QStringList     columsNames;
    columsNames.append(tr("Param name"));
    columsNames.append(tr("Description"));
    columsNames.append(tr("Channel"));
    columsNames.append(tr("Value"));
    columsNames.append("A");
    columsNames.append("B");
    columsNames.append(tr("Alias"));
    setHeaderLabels(columsNames);

    setAlternatingRowColors(true);
    setSortingEnabled(true);
    setUniformRowHeights(true);

    setItemDelegate(new DataEditorClass());
    addAsset(pAsset);
    //    setExpandsOnDoubleClick(false);
}

void DataViewClass::addAsset(AssetClass *pAsset)
{
    QTreeWidgetItem*    pRootItem = new QTreeWidgetItem(this, DataViewType_Asset);
//    pRootItem->setText(0, pAsset->name());
    pRootItem->setText(0, pAsset->description());
//    pRootItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<void*>(pAsset)));

    QHash<unsigned, DataCollectorClass*>::const_iterator    iter = pAsset->_collectors.constBegin();
    while(iter != pAsset->_collectors.constEnd())
    {
        QTreeWidgetItem*    pItem = new QTreeWidgetItem(this, DataViewType_Data);
        pItem->setText(0, iter.value()->_name);
        pItem->setText(1, iter.value()->_param);
        if(iter.value()->_isInteger)
        {
            pItem->setText(2, QString("I:%1").arg(iter.value()->_channel, 6));
            pItem->setText(3, QString::number(int(iter.value()->_value)));
        }
        else
        {
            pItem->setText(2, QString("F:%1").arg(iter.value()->_channel, 6));
            pItem->setText(3, QString::number(iter.value()->_value));
        }

        pItem->setText(4, QString::number(iter.value()->_scale));
        pItem->setText(5, QString::number(iter.value()->_offset));
        pItem->setText(6, iter.value()->_alias);
        pItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<void*>(iter.value())));
        pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
        _DataToItem.insert(iter.value(), pItem);
        ++iter;
    }

    sortByColumn(2, Qt::AscendingOrder);
}

void DataViewClass::updateView()
{
    QHash<unsigned, DataCollectorClass*>::const_iterator    iter = _asset->_collectors.constBegin();
    while(iter != _asset->_collectors.constEnd())
    {
        QTreeWidgetItem*    pItem = _DataToItem.value(iter.value(), 0);
        if(pItem)
        {
            if(iter.value()->_isInteger)
                pItem->setText(3, QString::number(int(iter.value()->_value)));
            else
                pItem->setText(3, QString::number(iter.value()->_value));
        }
        ++iter;
    }
}

bool DataViewClass::setSelected(DataCollectorClass *pData)
{
    QTreeWidgetItem*    pItem = _DataToItem.value(pData, 0);

    if(pItem){
        setCurrentItem(pItem);
        return true;
    }

    return false;
}

void DataViewClass::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();

    QTreeWidget::mousePressEvent(event);
}

void DataViewClass::contextMenuEvent(QContextMenuEvent *event)
{
    QTreeWidgetItem*        pItem = itemAt(event->pos());
    if(pItem == 0)
        return;
    if(pItem->type() != DataViewType_Data)
        return;
    DataCollectorClass*     pData = (DataCollectorClass*)pItem->data(0, Qt::UserRole).value<void*>();

    QMenu   pMenu;
    QAction* pSetAlias = pMenu.addAction(tr("Set Alias"));
    QAction* pSetA = pMenu.addAction(tr("Set A"));
    QAction* pSetB = pMenu.addAction(tr("Set B"));
    QAction* pAct = pMenu.exec(event->globalPos());

    if(pAct == pSetAlias)
    {
        QString newAlias = QInputDialog::getText(this, tr("Input new alias for view"), tr("New alias for view:"), QLineEdit::Normal, pData->_alias );
        if(!newAlias.isEmpty())
        {
            pData->setAlias(newAlias);
            PlotViewClass::updatePlots();
        }
    } else if(pAct == pSetA)
    {
        double  val = QInputDialog::getDouble(this, tr("Input new A for view"), tr("New A for view:"), pData->_scale, -1.e30, 1.e30, 5);
        pData->setLinear(val, pData->_offset);

        pItem->setText(3, QString::number(pData->_value));
        pItem->setText(4, QString::number(pData->_scale));

        PlotViewClass::updatePlots();
    } else if(pAct == pSetB)
    {
        double  val = QInputDialog::getDouble(this, tr("Input new B for view"), tr("New B for view:"), pData->_offset, -1.e30, 1.e30, 5);
        pData->setLinear(pData->_scale, val);

        pItem->setText(3, QString::number(pData->_value));
        pItem->setText(5, QString::number(pData->_offset));
        PlotViewClass::updatePlots();
    }
}

void DataViewClass::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    QTreeWidgetItem*        pItem = itemAt(event->pos());
    if(pItem == 0)
        return;

    if(pItem->type() != DataViewType_Data)
        return;

    DataCollectorClass*     pData = (DataCollectorClass*)pItem->data(0, Qt::UserRole).value<void*>();

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setData("plotter/collector", QByteArray((const char*)&pData, sizeof(DataCollectorClass*)));
    drag->setMimeData(mimeData);

    drag->exec( Qt::LinkAction );
}
