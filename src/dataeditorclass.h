#ifndef DATAEDITORCLASS_H
#define DATAEDITORCLASS_H

#include <QStyledItemDelegate>

class DataEditorClass : public QStyledItemDelegate
{

public:
    explicit DataEditorClass();
//    void     setInstpector(InspectorView*  inspector) { _inspector = inspector; }

    virtual QWidget    *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void        setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void        setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

#endif // DATAEDITORCLASS_H
