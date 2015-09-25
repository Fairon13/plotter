#ifndef PLOTMANAGERCLASS_H
#define PLOTMANAGERCLASS_H

#include <QTreeWidget>
#include <QHash>
#include <QSettings>

class PlotViewClass;
class QCPGraph;
class DataCollectorClass;
class PlotManagerClass : public QTreeWidget
{
    Q_OBJECT

    QHash<PlotViewClass*, QTreeWidgetItem*>  _plotToItem;
    QHash<QCPGraph*, QTreeWidgetItem*>       _graphToItem;

public:
    static  PlotManagerClass*   plotManager;

    PlotManagerClass(QWidget *parent = 0);

    void    updateView();
    void    setPlotName(PlotViewClass* plot, QString name);

    void    prepareToSave(QHash<QCPGraph*, int> &graphToIdx);
    void    saveState(QSettings &pSetup, QHash<DataCollectorClass *, int> &collectorToIdx, QHash<QCPGraph *, int> &graphToIdx);
    void    loadState(QSettings &pSetup);

    void    linkStage();

    PlotViewClass*  addPlot();
    void    removeGraphs(QList<QCPGraph *> removeList);

protected:
    void    dropEvent(QDropEvent *event);
    void    dragEnterEvent(QDragEnterEvent *event);
    void    dragMoveEvent(QDragMoveEvent *event);
    void    mouseDoubleClickEvent(QMouseEvent *event);

signals:

public slots:
    void    onAddPlot();
    void    onRemove();
};

#endif // PLOTMANAGERCLASS_H
