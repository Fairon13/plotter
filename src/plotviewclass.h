#ifndef PLOTVIEWCLASS_H
#define PLOTVIEWCLASS_H

#include "qcustomplot.h"
#include <QSettings>
#include <QHash>
#include <QSet>

class AssetClass;
class DataCollectorClass;
class PlotViewClass : public QCustomPlot
{
    Q_OBJECT

    bool    _isFixed_Y;

static  bool    _was_autoscale;
static  bool    _autoscale;
static  bool    _isFixed_X;
static  double  _range_X;
static  bool    _needRangeChange;
static  bool    _isLowQuality;

    bool    _needUpdateLegend;

    double  _min_Y;
    double  _max_Y;

    QCPLayoutElement     _layouElem;

    void    updateLegend();

public:
    PlotViewClass(QWidget *parent = 0);
    ~PlotViewClass() { _plots.removeOne(this); }

    void    needUpdateLegend() { _needUpdateLegend = true; }

    static  QList<PlotViewClass*>                   _plots;
    static  QList<PlotViewClass*>                   _visiblePlots;
    static  QHash<int, QCPGraph*>                   _idxToGraphs;
    static  QHash<QCPGraph*, DataCollectorClass*>   _graphToData;

    static  void    updatePlots();
    static  void    updatePlotsData(AssetClass* pAsset);
    static  void    reloadPlotsData();
    static  void    closePlots();
    static  void    hidePlots();
    static  void    showPlots();
    static  void    resetPlots();
    static  void    setLowQuality(const bool isLow);

    static QList<PlotViewClass *> visiblePlots();

    void    saveState(QSettings &pSetup, QHash<DataCollectorClass *, int> &collectorToIdx, QHash<QCPGraph *, int> &graphToIdx);
    void    loadState(QSettings &pSetup);
    void    updatePlot();

protected:
    void    closeEvent(QCloseEvent *event);
    void    mousePressEvent(QMouseEvent *event);
    void    mouseReleaseEvent(QMouseEvent *event);
    void    mouseDoubleClickEvent(QMouseEvent *event);
    void    wheelEvent(QWheelEvent *event);
    void    contextMenuEvent(QContextMenuEvent *event);
    void    showEvent(QShowEvent *event);
    void    hideEvent(QHideEvent *event);
    void    resizeEvent(QResizeEvent *event);
    void    draw(QCPPainter *painter);

signals:

public slots:
    void    rangeTimeChanged(QCPRange pRange);
    void    legendItemClicked(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event);
};

#endif // PLOTVIEWCLASS_H
