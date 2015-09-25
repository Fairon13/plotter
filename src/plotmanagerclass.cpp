#include "plotmanagerclass.h"
#include "datacollectorclass.h"
#include <QDropEvent>
#include <QMimeData>
#include "plotviewclass.h"
#include "mainwindow.h"

PlotManagerClass*   PlotManagerClass::plotManager = 0;

#define PlotManagerType_Plot     QTreeWidgetItem::UserType
#define PlotManagerType_Graph    QTreeWidgetItem::UserType + 1

PlotManagerClass::PlotManagerClass(QWidget *parent) : QTreeWidget(parent)
{
    plotManager = this;
}

void PlotManagerClass::updateView()
{
    PlotViewClass::updatePlots();
}

void PlotManagerClass::setPlotName(PlotViewClass *plot, QString name)
{
    QTreeWidgetItem*    pItem = _plotToItem.value(plot, 0);
    if(pItem == 0)
        return;

    pItem->setText(0, name);
}

void PlotManagerClass::prepareToSave(QHash<QCPGraph *, int> &graphToIdx)
{
    foreach(PlotViewClass* pView, PlotViewClass::_plots)
    {
        for(int n = 0; n<pView->graphCount(); ++n)
        {
            int idx = graphToIdx.size();
            graphToIdx.insert(pView->graph(n), idx);
        }
    }
}

void PlotManagerClass::saveState(QSettings &pSetup, QHash<DataCollectorClass *, int> &collectorToIdx, QHash<QCPGraph *, int> &graphToIdx)
{
    pSetup.beginWriteArray("plot");
    int idx = 0;
    foreach(PlotViewClass* pView, PlotViewClass::_plots)
    {
        pSetup.setArrayIndex(idx);
        pView->saveState(pSetup, collectorToIdx, graphToIdx);
        ++idx;
    }
    pSetup.endArray();
}

void PlotManagerClass::loadState(QSettings &pSetup)
{
    PlotViewClass::_idxToGraphs.clear();
    int num = pSetup.beginReadArray("plot");
    for(int k=0; k<num; ++k)
    {
        pSetup.setArrayIndex(k);
        PlotViewClass* pPlot = addPlot();
        pPlot->loadState(pSetup);

        QTreeWidgetItem*    pRootItem = _plotToItem.value(pPlot, 0);
        if(pRootItem)
        {
            pRootItem->setText(0, pPlot->windowTitle());
            int numGraphs = pPlot->graphCount();
            for(int k=0; k<numGraphs; ++k)
            {
                QTreeWidgetItem*    pItem = new QTreeWidgetItem(pRootItem, PlotManagerType_Graph);
                pItem->setText(0, pPlot->graph(k)->name());
                pItem->setIcon(0, QIcon(":/chart"));
                pItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<void*>(pPlot->graph(k))));
                _graphToItem.insert(pPlot->graph(k), pItem);
            }
        }
    }
    pSetup.endArray();
}

void PlotManagerClass::linkStage()
{

}

PlotViewClass *PlotManagerClass::addPlot()
{
    QTreeWidgetItem*    pItem = new QTreeWidgetItem(this, PlotManagerType_Plot);
    pItem->setText(0, "Plot");
    pItem->setIcon(0, QIcon(":/chart_curve"));

    PlotViewClass*    pPlot = new PlotViewClass();
    pPlot->setWindowTitle("Plot");
    pPlot->setWindowIcon(QIcon(":/chart_curve"));
    pPlot->resize(400, 400);
    pPlot->xAxis->setLabel(tr("time, sec"));
    pPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    pItem->setData(0, Qt::UserRole, QVariant::fromValue((void*)pPlot));
    _plotToItem.insert(pPlot, pItem);

    return pPlot;
}

void PlotManagerClass::removeGraphs(QList<QCPGraph *> removeList)
{
    for(int k =0; k<removeList.size(); ++k)
    {
        QCPGraph*   pGraph = removeList.at(k);
        QTreeWidgetItem* pItem = _graphToItem.value(pGraph, 0);
        if(pItem)
        {
            delete pItem;
            _graphToItem.remove(pGraph);
        }

        PlotViewClass::_graphToData.remove(pGraph);

        QCustomPlot*  pPlot = pGraph->parentPlot();
        pPlot->removeGraph(pGraph);
    }
}

void PlotManagerClass::dropEvent(QDropEvent *event)
{
    QByteArray arr = event->mimeData()->data("plotter/collector");
    DataCollectorClass**  pData = (DataCollectorClass**)arr.constData();

    QTreeWidgetItem* pItem = itemAt(event->pos());
    PlotViewClass*   pPlot = (PlotViewClass*)pItem->data(0, Qt::UserRole).value<void*>();

    int        graphIdx = pPlot->graphCount();
    int        red = (graphIdx & 1)*128;
    int        green = ((graphIdx >> 1) & 1)*128;
    int        blue = ((graphIdx >> 2) & 1)*128;

    QCPGraph*  pGraph = pPlot->addGraph();
    pGraph->setPen(QPen(QBrush(QColor(red, green, blue)), 2.));
//    pGraph->setAntialiased(false);

    if((*pData)->_alias.isEmpty())
        pGraph->setName((*pData)->_name);
    else
        pGraph->setName((*pData)->_alias);

    (*pData)->setGraph(pGraph);
    PlotViewClass::_graphToData.insert(pGraph, *pData);

    QTreeWidgetItem* pCollector = new QTreeWidgetItem(pItem, PlotManagerType_Graph);
    pCollector->setText(0, (*pData)->_name);
    pCollector->setIcon(0, QIcon(":/chart"));
    pCollector->setData(0, Qt::UserRole,     QVariant::fromValue(reinterpret_cast<void*>(pGraph)));

    _graphToItem.insert(pGraph, pCollector);

    pPlot->needUpdateLegend();
    pPlot->reloadPlotsData();
    pPlot->replot();
}

void PlotManagerClass::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasFormat("plotter/collector"))
        event->acceptProposedAction();
}

void PlotManagerClass::dragMoveEvent(QDragMoveEvent *event)
{
    QTreeWidgetItem* pItem = itemAt(event->pos());
    if(pItem == 0)
    {
        event->ignore();
        return;
    }

    if(pItem->type() != PlotManagerType_Plot)
    {
        event->ignore();
        return;
    }

    event->accept();
}

void PlotManagerClass::mouseDoubleClickEvent(QMouseEvent *event)
{
    QTreeWidgetItem*    pItem = itemAt(event->pos());
    if(pItem == 0)
        return;

    if(pItem->type() == PlotManagerType_Plot)
    {
        PlotViewClass*    pPlot = (PlotViewClass*)pItem->data(0, Qt::UserRole).value<void*>();
        pPlot->setVisible(true);
        pPlot->raise();
    } else if(pItem->type() == PlotManagerType_Graph)
    {
        QCPGraph* pGraph = (QCPGraph*)pItem->data(0, Qt::UserRole).value<void*>();
        DataCollectorClass* pData = PlotViewClass::_graphToData.value(pGraph, 0);
        MainWindow::pWin->setSelected(pData);
    }

}

void PlotManagerClass::onAddPlot()
{
    addPlot();
}

void PlotManagerClass::onRemove()
{
    QList<QTreeWidgetItem*>  pItems = selectedItems();
    if(pItems.count() == 0)
        return;

    foreach(QTreeWidgetItem* pItem, pItems)
    {
        if(pItem->type() == PlotManagerType_Plot)
        {
            PlotViewClass*    pPlot = (PlotViewClass*)pItem->data(0, Qt::UserRole).value<void*>();
            _plotToItem.remove(pPlot);

            int num = pPlot->graphCount();
            for(int k=0; k<num; ++k)
            {
                DataCollectorClass* pData = PlotViewClass::_graphToData.value(pPlot->graph(k), 0);
                if(pData)
                {
                    pData->removeGraph(pPlot->graph(k));
                    PlotViewClass::_graphToData.remove(pPlot->graph(k));
                    _graphToItem.remove(pPlot->graph(k));
                }
            }

            delete pPlot;

        } else if(pItem->type() == PlotManagerType_Graph)
        {
            PlotViewClass*      pPlot = (PlotViewClass*)pItem->parent()->data(0, Qt::UserRole).value<void*>();
            QCPGraph*           pGraph = (QCPGraph*)pItem->data(0, Qt::UserRole).value<void*>();
            DataCollectorClass* pData = PlotViewClass::_graphToData.value(pGraph, 0);

            _graphToItem.remove(pGraph);

            pData->removeGraph(pGraph);
            pPlot->removeGraph(pGraph);
            pPlot->needUpdateLegend();
            pPlot->replot();
        }

        delete pItem;
    }

    update();
}
