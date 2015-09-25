#include "pipesignals.h"

char*           PipeSignals::pSavedStateName = 0;

PipeSignals::PipeSignals(QString serverName, const quint16 port, const qint64 bufferSize) : PipeThread(serverName, port, bufferSize)
{
    _wasReading = false;
    _wasBegining = false;
    _wasLoadState = false;
}

void PipeSignals::beginExchange()
{
    if(_wasReading)
    {
        _wasReading = false;
        _wasBegining = false;
        clientAfterRead();
    } else _wasBegining = true;
}

void PipeSignals::clientAfterRead()
{
    signalLayout*   pData = (signalLayout*)_BufferRead;

    if(pData[0].channel > 0)
    {
        for(unsigned n=0; n < pData[0].channel; ++n)
            if(pData[n + 1].channel == 17 && pData[n + 1].value == 2)
                _wasLoadState = true;
    }

    pData = (signalLayout*)_BufferWrite;
    pData[0].channel =   0;
    pData[0].value =     0;

    qint64  sizeSend = pData[0].channel + 1;
            sizeSend *= sizeof(signalLayout);
            sizeSend += 257;

            sendData(sizeSend);
}

int PipeSignals::numData()
{
    signalLayout*   pData = (signalLayout*)_BufferRead;
    return    pData->channel;
}

unsigned PipeSignals::channel(const int idx)
{
    signalLayout*   pData = (signalLayout*)_BufferRead;
    return pData[idx + 1].channel;
}

int PipeSignals::value(const int idx)
{
    signalLayout*   pData = (signalLayout*)_BufferRead;
    return pData[idx + 1].value;
}

void PipeSignals::initialize()
{
    _wasReading = false;
    _wasBegining = false;
    _wasLoadState = false;
}

qint64 PipeSignals::getNumBytesToRead()
{
    signalLayout*   pData = (signalLayout*)_BufferRead;

    qint64  bytesToRead = pData[0].channel + 1;
            bytesToRead *= sizeof(signalLayout);
            bytesToRead += 257;

    return  bytesToRead;
}

void PipeSignals::readingCompleted()
{
    if(_wasBegining)
    {
        _wasBegining = false;
        _wasReading = false;
        clientAfterRead();
    } else _wasReading = true;
}

void PipeSignals::sendingCompleted()
{
    exchangeCompleted();
}
