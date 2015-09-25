#include "datacollectorclass.h"
#include "plotviewclass.h"
#include "assetclass.h"
#include <QVector>

QHash<int, DataCollectorClass*> DataCollectorClass::_idxToData;

DataCollectorClass::DataCollectorClass(AssetClass *pAsset)
{
    _asset = pAsset;
    _cache.setDataFile(&pAsset->_tempFile);
    _channel = 0;
    _value = 0.;
    _graphsIdx = 0;
    _numGraphs = 0;
    _noValue = true;
    _isSign = false;
    _scale = 1.;
    _offset = 0.;
    _isInteger = false;
}

DataCollectorClass::DataCollectorClass(AssetClass *pAsset, QString name, QString param, const unsigned channel, bool isSign, bool isInteger)
{
    _asset = pAsset;
    _cache.setDataFile(&pAsset->_tempFile);
    _name = name;
    _param = param;
    _channel = channel;
    _value = 0.;
    _graphsIdx = 0;
    _numGraphs = 0;
    _noValue = true;
    _isSign = isSign;
    _scale = 1.;
    _offset = 0.;
    _isInteger = isInteger;
}

void DataCollectorClass::saveState(QSettings &pSetup, QHash<DataCollectorClass *, int> &collectorToIdx, QHash<QCPGraph *, int> &graphToIdx)
{
    pSetup.setValue("idx", collectorToIdx.value(this));
    pSetup.setValue("name", _name);
    pSetup.setValue("param", _param);
    pSetup.setValue("alias", _alias);
    pSetup.setValue("channel", _channel);
    pSetup.setValue("scale", _scale);
    pSetup.setValue("offset", _offset);
    pSetup.setValue("is_sign", _isSign);
    pSetup.setValue("is_integer", _isInteger);

    pSetup.beginWriteArray("graphs");
    int idx = 0;
    foreach(QCPGraph* pGraph, _graphs)
    {
        pSetup.setArrayIndex(idx);
        pSetup.setValue("idx", graphToIdx.value(pGraph));
        ++idx;
    }
    pSetup.endArray();
}

void DataCollectorClass::loadState(QSettings &pSetup)
{
    _name = pSetup.value("name").toString();
    _param = pSetup.value("param").toString();
    _alias = pSetup.value("alias").toString();
    _channel = pSetup.value("channel").toUInt();
    _scale = pSetup.value("scale", 1.).toDouble();
    _offset = pSetup.value("offset", 0.).toDouble();
    _isSign = pSetup.value("is_sign", false).toBool();
    _isInteger = pSetup.value("is_integer", false).toBool();

    int idx = pSetup.value("idx").toInt();
    _idxToData.insert(idx, this);

    _numGraphs = pSetup.beginReadArray("graphs");
    if(_numGraphs != 0)
        _graphsIdx = new int[_numGraphs];

    for(int k=0; k<_numGraphs; ++k)
    {
        pSetup.setArrayIndex(k);
        _graphsIdx[k] = pSetup.value("idx").toInt();
    }
    pSetup.endArray();
}

void DataCollectorClass::linkStage()
{
    for(int k=0; k<_numGraphs; ++k)
    {
        QCPGraph*   pGraph = PlotViewClass::_idxToGraphs.value(_graphsIdx[k], 0);
        if(pGraph)
        {
            _graphs.append(pGraph);
            PlotViewClass::_graphToData.insert(pGraph, this);
        }
    }

    if(_graphsIdx)
    {
        delete[] _graphsIdx;
        _graphsIdx = 0;
        _numGraphs = 0;
    }
}

QVector<double> DataCollectorClass::values()
{
    QVector<double>     src_values = _cache.values();
    QVector<double>     res_values(src_values.size());

    for(int k = 0; k<src_values.size(); ++k)
        res_values[k] = src_values.at(k)*_scale + _offset;

    return res_values;
}

QVector<double> DataCollectorClass::times()
{
    return _asset->times();
}

void DataCollectorClass::copyLastValue(const int numTimes)
{
    double val = _cache.lastValue();
    for(int k = 0; k < numTimes; ++k)
        _cache.append(val);
}

void DataCollectorClass::addValue(const double value)
{
    _cache.append(value);
    _value = _scale*value + _offset;
}

void DataCollectorClass::reset()
{
    _cache.reset();
}

void DataCollectorClass::updateData(QByteArray arrData)
{
//    _cache.loadData(arrData);
    _cache.reset();
    _value = _cache.lastValue()*_scale + _offset;
}

void DataCollectorClass::setGraph(QCPGraph *graph)
{
    _graphs.append(graph);
}

void DataCollectorClass::setAlias(QString alias)
{
    _alias = alias;
    foreach(QCPGraph* pGraph, _graphs)
    {
        pGraph->setName(alias);
        PlotViewClass *pPlot = static_cast<PlotViewClass*>(pGraph->parentPlot());
        pPlot->needUpdateLegend();
    }
}

void DataCollectorClass::setLinear(const double scale, const double offset)
{
    _scale = scale;
    _offset = offset;
    _value = _cache.lastValue()*_scale + _offset;
}
