#ifndef ASSETCLASS_H
#define ASSETCLASS_H

#include <QString>
#include <QHash>
#include <QSettings>
#include <QFile>

#include "datacacheclass.h"
#include "datacollectorclass.h"

#define ASSET_TYPE_UNKNOWN  0
#define ASSET_TYPE_PIPE     1
#define ASSET_TYPE_BUS      2

class AssetClass : public QObject
{
    Q_OBJECT

public:
    QString                 _name;
    QString                 _path;

    QHash<unsigned, DataCollectorClass*>    _collectors;
    QFile                   _tempFile;
    DataCacheClass          _cache;

    double                  _curTime;
    double                  _startTime;

    quint8                  _type;

    bool                    _noValue;

    void    upExchangeComplete();

    virtual void    saveParams(QSettings &pSetup) = 0;
    virtual void    loadParams(QSettings &pSetup) = 0;

public:
    AssetClass(QObject* parent = 0);
    virtual ~AssetClass();

    virtual   bool    load() = 0;
    virtual   void    start();
    virtual   void    stop() = 0;
            QString name() { return _name; }
    virtual QString description() { return QString(); }

    virtual   bool  edit() = 0;

    void      prepareToSave(QHash<DataCollectorClass*, int> &collectorToIdx);
    void      saveState(QSettings &pSetup, QHash<DataCollectorClass *, int> &collectorToIdx, QHash<QCPGraph *, int> &graphToIdx);
    void      loadState(QSettings &pSetup);
    void      saveData(QString path);
    void      loadData(QString path);

    void      linkStage();

    void      addTime(const double  time);
    void      addValue(const unsigned channel, const double value);
    void      addValue(const unsigned channel, const int value);
    QVector<double> times();

    void      reset();

    void      setSelected(DataCollectorClass* pData);

    QList<DataCollectorClass*>  collectors();
    QList<QCPGraph*>            graphs();

};
#endif // ASSETCLASS_H
