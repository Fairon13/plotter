#include "plotviewwrapper.h"
#include <QTimer>
#include <QCoreApplication>

ControlWord*    PlotViewWrapper::pControl = 0;

PlotViewWrapper::PlotViewWrapper():QObject(0)
{
    if(pControl == 0)
    {
        memset(pControl, 0, sizeof(ControlWord)*MAX_PLOT_VIEWS);
    }

    for(_idx = 0; _idx < MAX_PLOT_VIEWS; ++_idx)
        if(pControl[_idx].control == PLOT_STATE_NONE)
            break;

    pControl[_idx].control = PLOT_STATE_OK;
    _numQueue = 0;
}

PlotViewWrapper::~PlotViewWrapper()
{
    pControl[_idx].control = PLOT_STATE_NONE;
}

void PlotViewWrapper::setPos(const int x, const int y)
{
    _queue[_numQueue].control = PLOT_CMD_SET_POS;
    _queue[_numQueue].data.asInt[0] = x;
    _queue[_numQueue].data.asInt[1] = y;
    ++_numQueue;

    postCommand();
}

void PlotViewWrapper::setSize(const int width, const int height)
{
    _queue[_numQueue].control = PLOT_CMD_SET_SIZE;
    _queue[_numQueue].data.asInt[0] = width;
    _queue[_numQueue].data.asInt[1] = height;
    ++_numQueue;

    postCommand();
}

void PlotViewWrapper::setTitle(const QString title)
{
    QByteArray arr = title.toUtf8();

    _queue[_numQueue].control = PLOT_CMD_SET_TITLE;
    memcpy(_queue[_numQueue].data.asBytes, arr.constData(), arr.size());
    ++_numQueue;

    postCommand();
}

QPoint PlotViewWrapper::getPos()
{
    QPoint  pos;

    _queue[_numQueue].control = PLOT_CMD_GET_POS;
    ++_numQueue;

    sendCommand();

    pos.setX(pControl[_idx].data.asInt[0]);
    pos.setY(pControl[_idx].data.asInt[1]);
    return pos;
}

QSize PlotViewWrapper::getSize()
{
    QSize  size;

    _queue[_numQueue].control = PLOT_CMD_GET_SIZE;
    ++_numQueue;

    sendCommand();

    size.setWidth(pControl[_idx].data.asInt[0]);
    size.setHeight(pControl[_idx].data.asInt[1]);
    return size;
}

QString PlotViewWrapper::getTitle()
{
    _queue[_numQueue].control = PLOT_CMD_GET_TITLE;
    ++_numQueue;

    sendCommand();

    return QString::fromUtf8(pControl[_idx].data.asBytes);
}

void PlotViewWrapper::resetData()
{
    _queue[_numQueue].control = PLOT_CMD_RESET_DATA;
    ++_numQueue;

    postCommand();
}

void PlotViewWrapper::update()
{
    _queue[_numQueue].control = PLOT_CMD_UPDATE;
    ++_numQueue;

    postCommand();
}

void PlotViewWrapper::close()
{
    _queue[_numQueue].control = PLOT_CMD_CLOSE;
    ++_numQueue;

    postCommand();
}

void PlotViewWrapper::postCommand()
{
    if(pControl[_idx].control != PLOT_STATE_OK)
    {
        QTimer::singleShot(100, this, SLOT(postCommand()));
        return;
    }

    if(_numQueue)
    {
        --_numQueue;
        memcpy(pControl[_idx].data.asBytes, _queue[_numQueue].data.asBytes, PLOT_BUFFER_SIZE);
        pControl[_idx].control = _queue[_numQueue].control;
    }

    if(_numQueue)
        QTimer::singleShot(100, this, SLOT(postCommand()));
}

void PlotViewWrapper::sendCommand()
{
    while(_numQueue){

        --_numQueue;
        memcpy(pControl[_idx].data.asBytes, _queue[_numQueue].data.asBytes, PLOT_BUFFER_SIZE);
        pControl[_idx].control = _queue[_numQueue].control;

        //wait for command complete
        forever {
            QCoreApplication::processEvents();
            if(pControl[_idx].control == PLOT_STATE_OK)
                break;
        }
    }
}

