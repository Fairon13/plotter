#include "pipethread.h"
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <QTcpSocket>

#include "pipedata.h"

PipeThread::PipeThread(QString serverName, const quint16 port, const qint64 bufferSize) : QObject(0)
{
    _serverNameComputer = serverName;
    _port = port;
    _bufferSize = bufferSize;
    _pSocket = 0;
    _BufferRead = 0;
    _BufferWrite = 0;

    _pThread = new QThread();
    moveToThread(_pThread);
    _pThread->start();

    QMetaObject::invokeMethod(this, "preStart", Qt::QueuedConnection);
}

PipeThread::~PipeThread()
{
    delete[] _BufferRead;
    delete[] _BufferWrite;
    delete   _pSocket;
}

void PipeThread::deletePipe()
{
    if(_state == PIPE_STATE_DISCONNECTED)
    {
        _state = PIPE_STATE_DELETING;
        return;
    }

    delete this;
}

void PipeThread::preStart()
{
    _BufferRead  = new char[_bufferSize];
    _BufferWrite = new char[_bufferSize];

    _state = PIPE_STATE_DISCONNECTED;

    _pSocket = new QTcpSocket();

    connect(_pSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(slotStateChanged(QAbstractSocket::SocketState)));
    connect(_pSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(_pSocket, SIGNAL(bytesWritten(qint64)), SLOT(slotBytesWritten(qint64)));
}

void PipeThread::start()
{
    QMetaObject::invokeMethod(this, "slotConnect", Qt::QueuedConnection);
}

void PipeThread::sendData(const qint64 numBytesToSend)
{
    _numBytesToWrite = qMin(numBytesToSend, _bufferSize);
    _pSocket->write(_BufferWritePointer, _numBytesToWrite);
}

void PipeThread::clearBufferState()
{
    memset(_BufferRead,  0, _bufferSize);
    memset(_BufferWrite, 0, _bufferSize);

    _BufferWritePointer = _BufferWrite;
    _BufferReadPointer = _BufferRead;

    _numBytesToRead = -1;
    _numBytesToWrite = 0;

    _nDublicate = 0;
}

void PipeThread::exchangeCompleted()
{
    emit exchangeFinished();
}

void PipeThread::slotBeginExhange()
{
    if(_state == PIPE_STATE_CONNECTED)
        beginExchange();
//    else
//        exchangeCompleted();
}

void PipeThread::slotConnect()
{
    _pSocket->connectToHost(_serverNameComputer, _port);
    if(_pSocket->waitForConnected(-1) == false)
    {
        if(_state == PIPE_STATE_DELETING){
            delete this;
            return;
        }

        QTimer::singleShot(1000, this, SLOT(slotConnect()));
        return;
    }

    clearBufferState();
    _state = PIPE_STATE_CONNECTED;
    initialize();
    emit connected();
}

void PipeThread::slotStateChanged(QAbstractSocket::SocketState socketState)
{
    if(socketState == QAbstractSocket::UnconnectedState)
    {
        if(_state == PIPE_STATE_CONNECTED)
        {
            exchangeCompleted();
            _state = PIPE_STATE_DISCONNECTED;
        }
        QTimer::singleShot(1000, this, SLOT(slotConnect()));
   }
}

void PipeThread::slotReadyRead()
{
    qint64  numBytesToRead = qMin(_pSocket->bytesAvailable(), _bufferSize);
    _pSocket->read(_BufferReadPointer, numBytesToRead);
    _BufferReadPointer += numBytesToRead;

    if(_numBytesToRead == -1)
    {
        qint64  temp = getNumBytesToRead();
        if(temp > _bufferSize)
        {
            char*   pNewBuffer = new char[temp];
            memcpy(pNewBuffer, _BufferRead, _bufferSize);
            delete[] _BufferRead;
            _BufferRead = pNewBuffer;
            _BufferReadPointer = pNewBuffer + numBytesToRead;
            _bufferSize = temp;
        }

        qint64  needBytesToRead = qMin(temp, _bufferSize);
        _numBytesToRead = qMax(needBytesToRead - numBytesToRead, qint64(0));

    } else
    {
        _numBytesToRead -= numBytesToRead;
        _numBytesToRead = qMax(_numBytesToRead, qint64(0));
    }

    if(_numBytesToRead == 0)
    {
        _BufferReadPointer = _BufferRead;
        _numBytesToRead = -1;
        readingCompleted();
    }

}

void PipeThread::slotBytesWritten(qint64 numBytes)
{
    _BufferWritePointer += numBytes;
    _numBytesToWrite -= numBytes;
    _numBytesToWrite = qMax(_numBytesToWrite, qint64(0));

    if(_numBytesToWrite)
        _pSocket->write(_BufferWritePointer, _numBytesToWrite);
    else
    {
        _BufferWritePointer = _BufferWrite;
        sendingCompleted();
    }
}
