#include "dataviewclass.h"
#include "ui_dataviewclass.h"

#include <QMouseEvent>
#include <QMenu>
#include <QInputDialog>

#include "pipeasset.h"
#include "busasset.h"
#include "datacollectorclass.h"
#include "dataeditorclass.h"
#include "plotviewclass.h"

#define  dv_type_asset  QTreeWidgetItem::UserType
#define  dv_type_data   QTreeWidgetItem::UserType + 1

#define dv_col_param_name    0
#define dv_col_description   1
#define dv_col_channel      2
#define dv_col_value        3
#define dv_col_a            4
#define dv_col_b            5
#define dv_col_alias        6
#define dv_col_total        7

DataViewClass::DataViewClass(AssetClass* pAsset, QWidget *parent) : QWidget(parent), ui(new Ui::DataViewClass)
{
    ui->setupUi(this);
    ui->view->setAsset(pAsset);
}

DataViewClass::~DataViewClass()
{
    delete ui;
}

void DataViewClass::updateView()
{
    ui->view->updateView();
}

bool DataViewClass::setSelected(DataCollectorClass *pData)
{
    ui->view->setSelected(pData);
}

AssetClass *DataViewClass::asset()
{
    return ui->view->asset();
}

void DataWidgetClass::updateView()
{
    QHash<unsigned, DataCollectorClass*>::const_iterator    iter = _asset->_collectors.constBegin();
    while(iter != _asset->_collectors.constEnd())
    {
        QTreeWidgetItem*    pItem = _DataToItem.value(iter.value(), 0);
        if(pItem)
        {
            if(iter.value()->_isInteger)
                pItem->setText(dv_col_value, QString::number(int(iter.value()->_value)));
            else
                pItem->setText(dv_col_value, QString::number(iter.value()->_value));
        }
        ++iter;
    }
}

bool DataWidgetClass::setSelected(DataCollectorClass *pData)
{
    QTreeWidgetItem*    pItem = _DataToItem.value(pData, 0);

    if(pItem){
        setCurrentItem(pItem);
        return true;
    }

    return false;
}

void DataWidgetClass::filterChange(QString txt)
{
    int numItems = topLevelItemCount();
    QTreeWidgetItem* pItem;

    if(txt.isEmpty()){
        for(int n = 0; n<numItems; ++n){
            pItem = topLevelItem(n);
            pItem->setHidden(false);
        }
        return;
    }

    for(int n = 0; n<numItems; ++n){
        pItem = topLevelItem(n);

        bool isShowItem = pItem->text(dv_col_channel).contains(txt) || pItem->text(dv_col_param_name).contains(txt, Qt::CaseInsensitive) || pItem->text(dv_col_alias).contains(txt, Qt::CaseInsensitive);
        pItem->setHidden(!isShowItem);
    }
}

void DataWidgetClass::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();

    QTreeWidget::mousePressEvent(event);
}

void DataWidgetClass::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    QTreeWidgetItem*        pItem = itemAt(event->pos());
    if(pItem == 0)
        return;

    if(pItem->type() != dv_type_data)
        return;

    DataCollectorClass*     pData = (DataCollectorClass*)pItem->data(0, Qt::UserRole).value<void*>();

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setData("plotter/collector", QByteArray((const char*)&pData, sizeof(DataCollectorClass*)));
    drag->setMimeData(mimeData);

    drag->exec( Qt::LinkAction );
}

void DataWidgetClass::contextMenuEvent(QContextMenuEvent *event)
{
    QTreeWidgetItem*        pItem = itemAt(event->pos());
    if(pItem == 0)
        return;

    if(pItem->type() != dv_type_data)
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

void DataWidgetClass::setAsset(AssetClass *pAsset)
{
    _asset = pAsset;

    QTreeWidgetItem*    pRootItem = new QTreeWidgetItem(this, dv_type_asset);
    pRootItem->setText(dv_col_param_name, pAsset->description());

    QHash<unsigned, DataCollectorClass*>::const_iterator    iter = pAsset->_collectors.constBegin();
    while(iter != pAsset->_collectors.constEnd())
    {
        QTreeWidgetItem*    pItem = new QTreeWidgetItem(this, dv_type_data);
        pItem->setText(dv_col_param_name, iter.value()->_name);
        pItem->setText(dv_col_description, iter.value()->_param);
        if(iter.value()->_isInteger)
        {
            pItem->setText(dv_col_channel, QString("I:%1").arg(iter.value()->_channel, 6));
            pItem->setText(dv_col_value, QString::number(int(iter.value()->_value)));
        }
        else
        {
            pItem->setText(dv_col_channel, QString("F:%1").arg(iter.value()->_channel, 6));
            pItem->setText(dv_col_value, QString::number(iter.value()->_value));
        }

        pItem->setText(dv_col_a, QString::number(iter.value()->_scale));
        pItem->setText(dv_col_b, QString::number(iter.value()->_offset));
        pItem->setText(dv_col_alias, iter.value()->_alias);
        pItem->setData(dv_col_param_name, Qt::UserRole, QVariant::fromValue(reinterpret_cast<void*>(iter.value())));
        pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
        _DataToItem.insert(iter.value(), pItem);
        ++iter;
    }

    sortByColumn(dv_col_channel, Qt::AscendingOrder);
}

DataWidgetClass::DataWidgetClass(QWidget *parent) : QTreeWidget(parent)
{
    setItemDelegate(new DataEditorClass());
}
