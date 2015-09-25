#ifndef PIPEDATA_H
#define PIPEDATA_H

#include "pipethread.h"

class PipeData : public PipeThread
{
    Q_OBJECT

public:
    unsigned*   viewsIdx;
    float*      viewsValue;

    unsigned    nEx;
    bool        firstStep;

    bool    _wasReading;    //was reading process
    bool    _wasBegining;   //was slotBeginExchange

    void      initialize();         //initialize pipe exchange
    void      beginExchange();      //initialize pipe exchange
    qint64    getNumBytesToRead();  //must return num bytes to read, including entire buffer
    void      readingCompleted();   //reading complete.
    void      sendingCompleted();   //send data complete

    void      checkArrayToSend(char*    pBuffer);

    void      afterRead();
    void      beforeSend();

public:
    PipeData(QString serverName, const quint16 port, const qint64 bufferSize);
    ~PipeData();

signals:

public slots:

};

#endif // PIPEDATA_H
