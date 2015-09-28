#ifndef DATAVIEWCLASS_H
#define DATAVIEWCLASS_H

#include <QWidget>
#include <QHash>
#include <QTreeWidget>

namespace Ui {
class DataViewClass;
}

class AssetClass;
class DataCollectorClass;
class DataViewClass : public QWidget
{
    Q_OBJECT

public:
    explicit DataViewClass(AssetClass *pAsset, QWidget *parent = 0);
    ~DataViewClass();

    void            updateView();
    bool            setSelected(DataCollectorClass *pData);
    AssetClass*     asset();

private:
    Ui::DataViewClass *ui;
};

class DataWidgetClass : public QTreeWidget
{
    Q_OBJECT

    AssetClass*                                     _asset;
    QHash<DataCollectorClass*, QTreeWidgetItem*>    _DataToItem;
    QPoint   dragStartPosition;

public:
    DataWidgetClass(QWidget *parent = 0);

    void    setAsset(AssetClass *pAsset);
    AssetClass*     asset() { return _asset; }

    void    updateView();
    bool    setSelected(DataCollectorClass *pData);

public slots:
    void    filterChange(QString    txt);

protected:
    void    mousePressEvent(QMouseEvent *event);
    void    mouseMoveEvent(QMouseEvent *event);
    void    contextMenuEvent(QContextMenuEvent *event);
};

#endif // DATAVIEWCLASS_H
