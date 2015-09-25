#include "plotviewclass.h"
#include <QMenu>
#include <QInputDialog>
#include "plotmanagerclass.h"
#include "setrangedialog.h"
#include "assetclass.h"

QList<PlotViewClass*>                   PlotViewClass::_visiblePlots;
QList<PlotViewClass*>                   PlotViewClass::_plots;
QHash<int, QCPGraph*>                   PlotViewClass::_idxToGraphs;
QHash<QCPGraph*, DataCollectorClass*>   PlotViewClass::_graphToData;

bool    PlotViewClass::_needRangeChange = true;
bool    PlotViewClass::_autoscale = true;
bool    PlotViewClass::_was_autoscale = false;
bool    PlotViewClass::_isFixed_X = false;
bool    PlotViewClass::_isLowQuality = false;
double  PlotViewClass::_range_X = 100.;

void PlotViewClass::updateLegend()
{
    //rearrage elements to fill grid
    QList<QCPLayoutElement*>    elems;

    //take all items from legend
    int maxElemWidth = 0;
    int numElements = legend->elementCount();
    for(int n = 0; n < numElements ; ++n)
    {
        QCPLayoutElement*   pElem = legend->takeAt(n);

        if(pElem)
            if(pElem != &_layouElem)
            {
                elems.append(pElem);
                maxElemWidth = qMax(maxElemWidth, pElem->minimumSizeHint().width());
            }
    }

    //clear legend from empty cells
    legend->simplify();

    //fill legend with items
    int col = 0, row = 0;
    for(int n = 0; n<elems.size() ; ++n)
    {
        legend->addElement(row, col, elems.at(n));
        legend->setColumnStretchFactor(col, 0.01);

        ++col;
        if((col + 1)*maxElemWidth > width())
        {
            col = 0;
            ++row;
        }
    }

    if(row == 0)
        legend->addElement(0, col, &_layouElem);
}

PlotViewClass::PlotViewClass(QWidget *parent) : QCustomPlot(parent)
{
    mPlottingHints |= QCP::phFastPolylines;
    setInteraction(QCP::iSelectLegend, true);

    _plots.append(this);
    _isFixed_Y = false;
    _needUpdateLegend = false;

    _min_Y = -1.;
    _max_Y =  1.;

     plotLayout()->addElement(1, 0, legend);
     plotLayout()->setRowStretchFactor(1, 0.01);
     legend->setMaximumSize(QWIDGETSIZE_MAX - 1, QWIDGETSIZE_MAX);
     legend->setBorderPen(QPen(Qt::NoPen));
     legend->setVisible(true);
     legend->setSelectableParts(QCPLegend::spItems);

     connect(this, SIGNAL(legendClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), SLOT(legendItemClicked(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)));
     connect(xAxis, SIGNAL(rangeChanged(QCPRange)), SLOT(rangeTimeChanged(QCPRange)));
}

void PlotViewClass::updatePlots()
{
    foreach(PlotViewClass* pPlot, _plots)
    {
        pPlot->updatePlot();
    }
}

void PlotViewClass::updatePlotsData(AssetClass *pAsset)
{
    foreach(PlotViewClass* pPlot, _plots)
    {
        if(pPlot->isVisible())
        {
            for(int k=0; k<pPlot->graphCount(); ++k)
            {
                QCPGraph*   pGraph = pPlot->graph(k);
                DataCollectorClass* pData = _graphToData.value(pGraph, 0);

                if(pData && pAsset == pData->_asset)
                    pGraph->addData(pData->_asset->_curTime, pData->_value);
            }
        }
    }
}

void PlotViewClass::reloadPlotsData()
{
    foreach(PlotViewClass* pPlot, _plots)
    {
        if(pPlot->isVisible())
        {
            for(int k=0; k<pPlot->graphCount(); ++k)
            {
                QCPGraph*   pGraph = pPlot->graph(k);
                DataCollectorClass* pData = _graphToData.value(pGraph, 0);

                pGraph->clearData();
                if(pData)
                    pGraph->addData(pData->times(), pData->values());
            }
        }
    }
}

void PlotViewClass::closePlots()
{
    hidePlots();
    _visiblePlots.clear();
}

void PlotViewClass::hidePlots()
{
    foreach(PlotViewClass* pPlot, _plots)
    {
        if(pPlot->isVisible())
        {
            _visiblePlots.append(pPlot);
            pPlot->hide();
        }
    }
}

void PlotViewClass::showPlots()
{
    foreach(PlotViewClass* pPlot, _visiblePlots)
        pPlot->show();
    _visiblePlots.clear();
}

void PlotViewClass::resetPlots()
{
    foreach(PlotViewClass* pPlot, _plots)
    {
        if(pPlot->isVisible())
        {
            for(int k=0; k<pPlot->graphCount(); ++k)
            {
                QCPGraph*   pGraph = pPlot->graph(k);
                pGraph->clearData();
            }
            pPlot->replot();
        }
    }
}

void PlotViewClass::setLowQuality(const bool isLow)
{
    foreach(PlotViewClass* pPlot, _plots)
    {
        for(int k=0; k<pPlot->graphCount(); ++k)
        {
            QCPGraph*   pGraph = pPlot->graph(k);
            QPen    pPen = pGraph->pen();

            if(isLow)
                pPen.setWidth(1);
            else
                pPen.setWidth(2);

            pGraph->setPen(pPen);
            pGraph->setAntialiased(!isLow);
        }
    }
}

void PlotViewClass::saveState(QSettings &pSetup, QHash<DataCollectorClass *, int> &collectorToIdx, QHash<QCPGraph *, int> &graphToIdx)
{
    pSetup.setValue("name", windowTitle());
    pSetup.setValue("pos", pos());
    pSetup.setValue("size", size());
    pSetup.setValue("visible", isVisible());
    pSetup.setValue("autoscale", _autoscale);
    pSetup.setValue("yAxisName", yAxis->label());
    pSetup.setValue("yAxisFixed", _isFixed_Y);
    pSetup.setValue("yAxisMin", _min_Y);
    pSetup.setValue("yAxisMax", _max_Y);

    pSetup.beginWriteArray("graphs", graphCount());
    for(int k=0; k < graphCount(); ++k)
    {
        QCPGraph*   pGraph = graph(k);
        pSetup.setArrayIndex(k);
        pSetup.setValue("idx", graphToIdx.value(pGraph, -1));
        pSetup.setValue("name", pGraph->name());
        pSetup.setValue("color", pGraph->pen().color());
    }

    pSetup.endArray();
}

void PlotViewClass::loadState(QSettings &pSetup)
{
    setWindowTitle(pSetup.value("name", "Plot").toString());
    move(pSetup.value("pos", QPoint(0, 0)).toPoint());
    resize(pSetup.value("size", QSize(200, 200)).toSize());
    setVisible(pSetup.value("visible", true).toBool());
    _autoscale = pSetup.value("autoscale", true).toBool();
    yAxis->setLabel(pSetup.value("yAxisName").toString());
    _isFixed_Y = pSetup.value("yAxisFixed", false).toBool();
    _min_Y = pSetup.value("yAxisMin", -1.).toDouble();
    _max_Y = pSetup.value("yAxisMax",  1.).toDouble();

    int num = pSetup.beginReadArray("graphs");
    for(int k=0; k<num; ++k)
    {
        pSetup.setArrayIndex(k);
        QCPGraph*   pGraph = addGraph();
        pGraph->setName(pSetup.value("name", "Value").toString());
        pGraph->setPen(QPen(QBrush(pSetup.value("color").value<QColor>()), 2.));
        int  idx = pSetup.value("idx").toInt();
        _idxToGraphs.insert(idx, pGraph);
    }
    pSetup.endArray();

    _needUpdateLegend = true;

    replot();
}

void PlotViewClass::updatePlot()
{
    if(isVisible())
    {
        bool    oldRangeChange = _needRangeChange;

        _needRangeChange = false;

        if(_autoscale)
        {
            if(!_isFixed_Y)
                yAxis->rescale();

            xAxis->rescale();
            if(_isFixed_X)
                xAxis->setRangeLower(xAxis->range().upper - _range_X);
        }

        _needRangeChange = oldRangeChange;
        replot();
    }
}

void PlotViewClass::closeEvent(QCloseEvent *event)
{
    hide();
}

void PlotViewClass::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        _was_autoscale = _autoscale;
        _autoscale = false;
    }

    QCustomPlot::mousePressEvent(event);
}

void PlotViewClass::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && (mMousePressPos - event->pos()).manhattanLength() < 5)
        _autoscale = _was_autoscale;

    QCustomPlot::mouseReleaseEvent(event);
}

void PlotViewClass::mouseDoubleClickEvent(QMouseEvent *event)
{
    _autoscale = true;
    _was_autoscale = true;
    if(_isFixed_Y)
    {
        yAxis->setRangeLower(_min_Y);
        yAxis->setRangeUpper(_max_Y);
    };

    updatePlot();
}

void PlotViewClass::wheelEvent(QWheelEvent *event)
{
    _autoscale = false;
    QCustomPlot::wheelEvent(event);
}

void PlotViewClass::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu   pMenu;
    QAction* pSetName = pMenu.addAction(tr("Plot name"));
    QAction* pLeftName = pMenu.addAction(tr("Y name"));
    QAction* pLowQuality = pMenu.addAction(tr("Low quality"));
    pLowQuality->setCheckable(true); pLowQuality->setChecked(_isLowQuality);
    pMenu.addSeparator();
    QAction* pShowAll_X = pMenu.addAction(tr("X: all"));
    QAction* pShowRange_X = pMenu.addAction(tr("X: range"));
    pShowAll_X->setCheckable(true); pShowAll_X->setChecked(!_isFixed_X);
    pShowRange_X->setCheckable(true); pShowRange_X->setChecked(_isFixed_X);
    pMenu.addSeparator();
    QAction* pLeftAll = pMenu.addAction(tr("Y: all"));
    QAction* pLeftRange = pMenu.addAction(tr("Y: range"));
    pLeftAll->setCheckable(true); pLeftAll->setChecked(!_isFixed_Y);
    pLeftRange->setCheckable(true); pLeftRange->setChecked(_isFixed_Y);
    pMenu.addSeparator();
    QAction* pAutoScale = pMenu.addAction(tr("Auto Scale"));
    pAutoScale->setCheckable(true); pAutoScale->setChecked(_autoscale);
    pMenu.addSeparator();
    QAction* pClose = pMenu.addAction(tr("Close"));

    QAction* pAct = pMenu.exec(event->globalPos());

    if(pAct == pAutoScale)
    {
        _autoscale = true;
        if(_isFixed_Y)
        {
            yAxis->setRangeLower(_min_Y);
            yAxis->setRangeUpper(_max_Y);
        };

        updatePlot();

    } else if(pAct == pClose)
        setVisible(false);
    else if(pAct == pSetName)
    {
        QString text = QInputDialog::getText(this, tr("Plot Name"), tr("Input new name"),QLineEdit::Normal, windowTitle());
        if(!text.isEmpty())
        {
            setWindowTitle(text);
            PlotManagerClass::plotManager->setPlotName(this, text);
            updatePlot();
        }
    } else if(pAct == pLeftName)
    {
        QString text = QInputDialog::getText(this, tr("Y axis name"), tr("Input new name for Y axis"), QLineEdit::Normal, yAxis->label());
        if(!text.isEmpty())
        {
            yAxis->setLabel(text);
            updatePlot();
        }
    } else if(pAct == pLeftRange)
    {
        SetRangeDialog  pDialog(this);
        pDialog.setValue(_min_Y, _max_Y);

        if(pDialog.exec() == QDialog::Accepted)
        {
            _min_Y = pDialog.minValue();
            _max_Y = pDialog.maxValue();

            if(_min_Y > _max_Y)
            {
                double  tmp = _min_Y;
                _min_Y = _max_Y;
                _max_Y = tmp;
            }

            _isFixed_Y = true;

            yAxis->setRangeLower(_min_Y);
            yAxis->setRangeUpper(_max_Y);
            updatePlot();
        }
    } else if(pAct == pLeftAll)
    {
        _isFixed_Y = false;
        updatePlot();
    } else  if(pAct == pShowRange_X)
    {
        SetRangeDialog  pDialog(this);
        pDialog.setValue(qMax(0., xAxis->range().lower), qMax(0., xAxis->range().upper));

        if(pDialog.exec() == QDialog::Accepted)
        {
            double _min_R = pDialog.minValue();
            double _max_R = pDialog.maxValue();

            if(_min_R > _max_R)
            {
                double  tmp = _min_R;
                _min_R = _max_R;
                _max_R = tmp;
            }

            _isFixed_X = true;
            _range_X = _max_R - _min_R;

            xAxis->setRangeUpper(_max_R);
            updatePlot();
        }
    } else if(pAct == pShowAll_X)
    {
        _isFixed_X = false;
        updatePlot();
    } else if(pAct == pLowQuality)
    {
        _isLowQuality = !_isLowQuality;
        setLowQuality(_isLowQuality);
        updatePlot();
    }
}

void PlotViewClass::showEvent(QShowEvent *event)
{
    for(int k=0; k < graphCount(); ++k)
    {
        QCPGraph*   pGraph = graph(k);
        DataCollectorClass* pData = _graphToData.value(pGraph, 0);
        if(pData)
            pGraph->addData(pData->times(), pData->values());
    }

    updatePlot();
    QCustomPlot::showEvent(event);
}

void PlotViewClass::hideEvent(QHideEvent *event)
{
    for(int k=0; k < graphCount(); ++k)
        graph(k)->clearData();

    QCustomPlot::hideEvent(event);
}

void PlotViewClass::resizeEvent(QResizeEvent *event)
{
    QCustomPlot::resizeEvent(event);
    _needUpdateLegend = true;
}

void PlotViewClass::draw(QCPPainter *painter)
{
    if(_needUpdateLegend)
    {
        updateLegend();
        _needUpdateLegend = false;
    }

    QCustomPlot::draw(painter);
}

void PlotViewClass::rangeTimeChanged(QCPRange pRange)
{
    if(_needRangeChange)
    {
        _needRangeChange = false;
        foreach(PlotViewClass* pPlot, _plots)
        {
            if(pPlot != this)
            {
                pPlot->xAxis->setRange(pRange);
                pPlot->updatePlot();
            }
        }

        _needRangeChange = true;
    }
}

void PlotViewClass::legendItemClicked(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event)
{
    QCPPlottableLegendItem *legendItem = qobject_cast<QCPPlottableLegendItem*>(item);
    if(legendItem)
    {
        if(legendItem->selected())
        {
            legendItem->plottable()->setVisible(true);
            legendItem->setSelected(false);
        } else
        {
            legendItem->plottable()->setVisible(false);
            legendItem->setSelected(true);
        }
        replot();
    }
}

QList<PlotViewClass *> PlotViewClass::visiblePlots()
{
    QList<PlotViewClass *>  res;

    foreach(PlotViewClass* pView, _plots)
    {
        if(pView->isVisible())
            res.append(pView);
    }

    return res;
}
