#ifndef DATAVIEWCLASS_H
#define DATAVIEWCLASS_H

#include <QTreeWidget>
#include <QHash>
#include <QSettings>

class AssetClass;
class DataCollectorClass;
class QCPGraph;
class DataViewClass : public QTreeWidget
{
    Q_OBJECT

    QPoint   dragStartPosition;

    AssetClass*     _asset;
    QHash<DataCollectorClass*, QTreeWidgetItem*>    _DataToItem;

    void    addAsset(AssetClass* pAsset);

public:
    explicit DataViewClass(AssetClass* pAsset, QWidget *parent = 0);

    void    prepareToSave(QHash<DataCollectorClass*, int> &collectorToIdx);
    void    saveState(QSettings &pSetup, QHash<DataCollectorClass*, int> &collectorToIdx, QHash<QCPGraph*, int> &graphToIdx);
    void    loadState(QSettings &pSetup);
    void    linkStage();
    void    connect();
    void    disconnect();
    void    saveData(QString path);
    void    loadData(QString path);

    AssetClass*     asset() { return _asset; }

    void    updateView();
    bool    setSelected(DataCollectorClass *pData);

protected:
    void    mousePressEvent(QMouseEvent *event);
    void    mouseMoveEvent(QMouseEvent *event);
    void    contextMenuEvent(QContextMenuEvent *event);

signals:

public slots:
};

#endif // DATAVIEWCLASS_H
