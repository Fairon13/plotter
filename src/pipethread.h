#ifndef PIPETHREAD_H
#define PIPETHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QList>
#include <QMutex>
#include <QWaitCondition>

#define PIPE_STATE_DISCONNECTED  0
#define PIPE_STATE_CONNECTED     1
#define PIPE_STATE_DELETING      2

class PipeObject;
class PipeThread : public QObject
{
    Q_OBJECT

public:
    QTcpSocket* _pSocket;
    QThread*    _pThread;

    QString     _serverNameComputer;    // название машины с сервером, если клиент
    quint16     _port;                  //TCP port for connection

    qint64      _bufferSize;

    char*       _BufferRead;
    char*       _BufferWrite;

    char*       _BufferReadPointer;
    char*       _BufferWritePointer;

    qint64      _numBytesToRead;
    qint64      _numBytesToWrite;

    qint64      _flowSizeBuffer;        // текущий размер буфера
    unsigned    _nDublicate;

    quint8      _state;

    virtual     void      initialize() = 0;         //initialize pipe exchange
    virtual     void      beginExchange() = 0;         //initialize pipe exchange
    virtual     qint64    getNumBytesToRead() = 0;  //must return num bytes to read, including entire buffer
    virtual     void      readingCompleted() = 0;   //reading complete.
    virtual     void      sendingCompleted() = 0;   //send data complete

    void        sendData(const qint64 numBytesToSend);

    void        clearBufferState();

    void        exchangeCompleted();
public:
    PipeThread(QString serverName, const quint16 port, const qint64 bufferSize);
    virtual  ~PipeThread();
    void      deletePipe();

    static      void moveArrayToSend();
    static      void generateThread(PipeObject *pObject, QTcpSocket*  pSocket = 0);
signals:

    void        exchangeFinished();
    void        connected();

public slots:
    void    preStart();
    void    start();

    virtual void  slotBeginExhange(); //Begin exchange data

    void    slotConnect();
    void    slotStateChanged(QAbstractSocket::SocketState   socketState);
    void    slotReadyRead();
    void    slotBytesWritten(qint64 numBytes);
};

#endif // PIPETHREAD_H
