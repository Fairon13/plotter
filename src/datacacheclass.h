#ifndef DATACACHECLASS_H
#define DATACACHECLASS_H

#include <QVector>

#define DATA_CACHE_SECTOR_SIZE  1024

class QFile;
class DataCacheClass
{

public:
    QVector<qint64>      _dataOffsets;
    double               _dataCache[DATA_CACHE_SECTOR_SIZE];
    double               _dataCachePrev;
    QFile*               _dataFile;
    int                  _dataIdx;

public:
    DataCacheClass();
    void            setDataFile(QFile* dataFile);

    void            reset();

    void            append(const double value);
    double          lastValue();

    int             numValues();
    QVector<double> values();
};

#endif // DATACACHECLASS_H
