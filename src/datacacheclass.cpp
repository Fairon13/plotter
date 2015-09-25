#include "datacacheclass.h"
#include <QFile>

void DataCacheClass::append(const double value)
{
    _dataCache[_dataIdx] = value;
    ++_dataIdx;

    if(_dataIdx > DATA_CACHE_SECTOR_SIZE)
    {
        _dataCachePrev = _dataCache[_dataIdx - 1];

        _dataOffsets.append(_dataFile->pos());
        _dataFile->write((const char*)_dataCache, DATA_CACHE_SECTOR_SIZE*sizeof(double));
        _dataIdx = 0;
    }
}

double DataCacheClass::lastValue()
{
    if(_dataIdx)
        return _dataCache[_dataIdx - 1];
    else
        return _dataCachePrev;
}

int DataCacheClass::numValues()
{
    return _dataOffsets.size()*DATA_CACHE_SECTOR_SIZE + _dataIdx;
}

QVector<double> DataCacheClass::values()
{
    if(_dataFile == 0)
        return QVector<double>();

    int numValues = _dataIdx + _dataOffsets.size() * DATA_CACHE_SECTOR_SIZE;
    QVector<double>     res_values(numValues);
    qint64  prevPos = _dataFile->pos();

    for(int k = 0; k<_dataOffsets.size(); ++k)
    {
        _dataFile->seek(_dataOffsets.at(k));
        _dataFile->read((char*)(res_values.data() + k*DATA_CACHE_SECTOR_SIZE), DATA_CACHE_SECTOR_SIZE*sizeof(double));
    }

    _dataFile->seek(prevPos);

    if(_dataIdx > 0)
        memcpy(res_values.data() + _dataOffsets.size()*DATA_CACHE_SECTOR_SIZE, _dataCache, _dataIdx*sizeof(double));

    return res_values;
}

void DataCacheClass::reset()
{
    _dataOffsets.clear();
    _dataIdx = 0;
    _dataCachePrev = 0.;
}

DataCacheClass::DataCacheClass()
{
    _dataFile = 0;
    _dataIdx = 0;
    _dataCachePrev = 0.;
}

void DataCacheClass::setDataFile(QFile *dataFile)
{
    _dataFile = dataFile;
}

