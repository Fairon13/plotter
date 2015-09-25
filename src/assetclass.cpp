#include "assetclass.h"
#include "mainwindow.h"
#include "plotviewclass.h"

void AssetClass::upExchangeComplete()
{
    //Выравниваем значения всех коллекторов чтобы количество значений было у всех одинаково
    int numTimes = _cache.numValues();
    foreach(DataCollectorClass* pData, _collectors)
    {
        if(numTimes > pData->numValues())
            pData->copyLastValue(numTimes - pData->numValues());
    }

    MainWindow::pWin->exchangeComplete(this);
}

AssetClass::AssetClass(QObject *parent) : QObject(parent)
{
    _type = ASSET_TYPE_UNKNOWN;
}

AssetClass::~AssetClass()
{
    foreach(DataCollectorClass* pData, _collectors)
        delete pData;

    _tempFile.close();
    _tempFile.remove();
}

void AssetClass::start()
{
    if(_tempFile.isOpen())
        reset();
    else
    {
        QString tempName = _name;
        tempName.append(".cache");

        _tempFile.setFileName(tempName);
        _tempFile.open(QIODevice::ReadWrite | QIODevice::Truncate);
        _cache.setDataFile(&_tempFile);
    }
}

void AssetClass::prepareToSave(QHash<DataCollectorClass *, int> &collectorToIdx)
{
    foreach(DataCollectorClass* pData, _collectors)
    {
        int idx = collectorToIdx.size();
        collectorToIdx.insert(pData, idx);
    }
}

void AssetClass::saveState(QSettings &pSetup, QHash<DataCollectorClass *, int> &collectorToIdx, QHash<QCPGraph *, int> &graphToIdx)
{
    pSetup.setValue("idx",          quint64(this));
    pSetup.setValue("name",         _name);
    pSetup.setValue("path",         _path);

    saveParams(pSetup);

    pSetup.beginWriteArray("dataCollector");
    int idx = 0;
    foreach(DataCollectorClass* pData, _collectors)
    {
        pSetup.setArrayIndex(idx);
        pData->saveState(pSetup, collectorToIdx, graphToIdx);
        ++idx;
    }
    pSetup.endArray();
}

void AssetClass::loadState(QSettings &pSetup)
{
    _name = pSetup.value("name").toString();
    _path = pSetup.value("path").toString();

    loadParams(pSetup);

    DataCollectorClass::_idxToData.clear();

    int num = pSetup.beginReadArray("dataCollector");
    for(int k=0; k<num; ++k)
    {
        pSetup.setArrayIndex(k);
        DataCollectorClass* pData = new DataCollectorClass(this);
        pData->loadState(pSetup);
        if(pData->_isInteger)
            _collectors.insert(pData->_channel | 0x80000000u, pData);
        else
            _collectors.insert(pData->_channel, pData);
    }
    pSetup.endArray();
}

void AssetClass::saveData(QString path)
{
    path.append(".values");
    QFile::remove(path);

    _tempFile.close();
    _tempFile.copy(path);
    _tempFile.open(QIODevice::ReadWrite | QIODevice::Append);

    QFile   file(path);
    file.open(QIODevice::WriteOnly | QIODevice::Append);

    qint64  offset = file.size();

    QDataStream strmOut(&file);
    strmOut.setVersion(QDataStream::Qt_4_8);

    QHash<unsigned, DataCollectorClass*>::const_iterator    iter = _collectors.constBegin();
    while(iter != _collectors.constEnd())
    {
        DataCacheClass  &cache = iter.value()->_cache;
        strmOut << iter.value()->_name;
        strmOut << cache._dataIdx;
        for(int n = 0; n < cache._dataIdx; ++n)
            strmOut << cache._dataCache[n];
        strmOut << cache._dataOffsets;
        ++iter;
    }

    strmOut << offset;
    strmOut.setDevice(0);
    file.close();
}

void AssetClass::loadData(QString path)
{
    /*
    QFile       file;
    QString  fileName;
    QByteArray arrData;

    fileName = path;
    fileName.append("/time.value");

    file.setFileName(fileName);
    if(file.open(QIODevice::ReadOnly))
    {
        arrData = file.readAll(); file.close();

        _timeFile.resize(0);
        _timeFile.write(arrData.constData(), arrData.size());
    }

    QHash<unsigned, DataCollectorClass*>::const_iterator    iter = _collectors.constBegin();
    while(iter != _collectors.constEnd())
    {
        fileName = path;
        fileName.append('/');
        fileName.append(iter.value()->_name);
        fileName.append(".value");

        file.setFileName(fileName);
        if(file.open(QIODevice::ReadOnly))
        {
            iter.value()->updateData(file.readAll());
            file.close();
        }

        ++iter;
    }
    */
}

void AssetClass::linkStage()
{
    foreach(DataCollectorClass* pData, _collectors)
        pData->linkStage();
}

void AssetClass::addTime(const double time)
{
    _curTime = time - _startTime;
    _cache.append(_curTime);
}

void AssetClass::addValue(const unsigned channel, const double value)
{
    DataCollectorClass*  pData = _collectors.value(channel, 0);
    if(pData == 0)
        return;

    pData->addValue(value);
}

void AssetClass::addValue(const unsigned channel, const int value)
{
    DataCollectorClass*  pData = _collectors.value(channel | 0x80000000u, 0);
    if(pData == 0)
        return;

    pData->addValue(value);
}

QVector<double> AssetClass::times()
{
    return _cache.values();
}

void AssetClass::reset()
{
    foreach(DataCollectorClass* pData, _collectors)
        pData->reset();

    _cache.reset();
    _tempFile.resize(0);

    _noValue = true;
    PlotViewClass::resetPlots();
}

QList<DataCollectorClass *> AssetClass::collectors()
{
    return _collectors.values();
}

QList<QCPGraph *> AssetClass::graphs()
{
    QList<QCPGraph *>   listGraphs;

    foreach(DataCollectorClass* pData, _collectors)
        listGraphs.append(pData->_graphs);

    return listGraphs;
}
