#ifndef DATACOLLECTORCLASS_H
#define DATACOLLECTORCLASS_H

#include <QString>
#include <QHash>
#include <QVector>
#include <QList>
#include <QSettings>
#include <QFile>

#include "datacacheclass.h"

class QCPGraph;
class AssetClass;
class DataCollectorClass
{

public:
    DataCacheClass  _cache;
    QString     _name;
    QString     _param;
    QString     _alias;
    unsigned    _channel;
    double      _value;
    double      _scale;
    double      _offset;
    bool        _noValue;
    bool        _isSign;
    bool        _isInteger;

    QList<QCPGraph*>            _graphs;

    int*        _graphsIdx;
    int         _numGraphs;

    AssetClass* _asset;
public:
    static  QHash<int, DataCollectorClass*> _idxToData;

    DataCollectorClass(AssetClass* pAsset);
    DataCollectorClass(AssetClass* pAsset, QString name, QString param, const unsigned channel, bool isSign = false, bool isInteger = false);

    void    saveState(QSettings &pSetup, QHash<DataCollectorClass *, int> &collectorToIdx, QHash<QCPGraph *, int> &graphToIdx);
    void    loadState(QSettings &pSetup);

    void    linkStage();

    double  value() { return _value; }
    QVector<double> values();
    QVector<double> times();


    void    copyLastValue(const int numTimes);

    int       numValues() { return _cache.numValues(); }
    void      addValue(const double  value);
    void      reset();
    void      updateData(QByteArray arrData);
    void      setGraph(QCPGraph*  graph);
    void      removeGraph(QCPGraph*  graph) { _graphs.removeOne(graph); }
    void      setAlias(QString alias);
    void      setLinear(const double scale, const double offset);
};

#endif // DATACOLLECTORCLASS_H
