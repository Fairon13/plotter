#ifndef PIPESIGNALS_H
#define PIPESIGNALS_H

#include "pipethread.h"

struct  signalLayout
{
    unsigned    channel;
    int         value;
};

class PipeSignals : public PipeThread
{
    Q_OBJECT

    bool    _wasReading;    //was reading process
    bool    _wasBegining;   //was slotBeginExchange

    void    clientAfterRead();

public:
    bool    _wasLoadState;  //was Load state command;

    int         numData();
    unsigned    channel(const int idx);
    int         value(const int idx);

    static  char*           pSavedStateName;

    virtual     void      initialize();         //initialize pipe exchange
    virtual     void      beginExchange();           //Begin exchange data
    virtual     qint64    getNumBytesToRead();  //must return num bytes to read, including entire buffer
    virtual     void      readingCompleted();   //reading complete.
    virtual     void      sendingCompleted();   //send data complete


    PipeSignals(QString serverName, const quint16 port, const qint64 bufferSize);
signals:

public slots:

};

#endif // PIPESIGNALS_H
