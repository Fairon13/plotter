#include "pipedata.h"

using namespace std;

void PipeData::initialize()
{
    firstStep = true;
    _flowSizeBuffer = 0;

    _wasReading = false;
    _wasBegining = false;

    if(viewsIdx)
        delete[] viewsIdx;

    viewsIdx = 0;
}

void PipeData::beginExchange()
{
    if(_wasReading)
    {
        _wasReading = false;
        _wasBegining = false;
        afterRead();
        beforeSend();
    } else _wasBegining = true;
}

qint64 PipeData::getNumBytesToRead()
{
    return *((unsigned*)_BufferRead) + sizeof(unsigned);
}

void PipeData::readingCompleted()
{
    if(_wasBegining)
    {
        _wasBegining = false;
        _wasReading = false;

        afterRead();
        beforeSend();

    } else _wasReading = true;
}

void PipeData::sendingCompleted()
{
    if(firstStep)
        firstStep = false;

    exchangeCompleted();
}

void PipeData::afterRead()
{
    if(firstStep)  // заходим один раз
    {
        // переприсваиваем данные согласно списку в фильтре
        // считываем буфер прототип
        nEx = *(unsigned*)_BufferRead;
        viewsIdx = (unsigned*) new char[nEx];
        memcpy(viewsIdx, _BufferRead + sizeof(unsigned), nEx);
        memset(_BufferRead + sizeof(unsigned), 0, nEx);

        nEx /= sizeof(unsigned); // определяем количество элементов в посылке для последующих шагов
    }

    viewsValue = reinterpret_cast<float*>(_BufferRead + sizeof(unsigned));
}

void PipeData::beforeSend()
{
    if(firstStep)  // первый шаг
    {
        // заполнение первой укороченной посылки буфером прототипом
        unsigned numViewsToSend = 0;
        _flowSizeBuffer = numViewsToSend*sizeof(unsigned) + 1;
        *((unsigned*)_BufferWrite) = unsigned(_flowSizeBuffer);
    }
    else   // последующие шаги
    {
        *((unsigned*)_BufferWrite) = _flowSizeBuffer;
//        memcpy(_BufferWrite + sizeof(unsigned), view::getShortViewBuffer(), flowSizeBuffer);
    }
    sendData(_flowSizeBuffer + sizeof(unsigned));
}

PipeData::PipeData(QString serverName, const quint16 port, const qint64 bufferSize) : PipeThread(serverName, port, bufferSize)
{
    firstStep = true;

    viewsValue = 0;
    viewsIdx = 0;

    _wasReading = false;
    _wasBegining = false;
}

PipeData::~PipeData()
{
    if(viewsIdx)
        delete[] viewsIdx;
}
