#ifndef PLOTVIEWWRAPPER_H
#define PLOTVIEWWRAPPER_H

#include <QObject>
#include <QPoint>
#include <QSize>

#define     MAX_PLOT_VIEWS       32
#define     PLOT_BUFFER_SIZE     256

#define     PLOT_STATE_NONE     0
#define     PLOT_STATE_OK       1
#define     PLOT_STATE_CLOSED   2

#define     PLOT_CMD_SET_POS    3
#define     PLOT_CMD_SET_SIZE   4
#define     PLOT_CMD_SET_TITLE  5
#define     PLOT_CMD_GET_POS    6
#define     PLOT_CMD_GET_SIZE   7
#define     PLOT_CMD_GET_TITLE  8
#define     PLOT_CMD_RESET_DATA 9
#define     PLOT_CMD_UPDATE     10
#define     PLOT_CMD_ADD_GRAPH  11
#define     PLOT_CMD_DEL_GRAPH  12
#define     PLOT_CMD_CLOSE      13

struct ControlWord{
    union{
        char    asBytes[PLOT_BUFFER_SIZE];
        int     asInt[PLOT_BUFFER_SIZE/sizeof(int)];
        float   asFloat[PLOT_BUFFER_SIZE/sizeof(float)];
    } data;
    char    control;
};

class PlotViewWrapper : public QObject
{
    Q_OBJECT

    static ControlWord*    pControl;

    int         _idx;
    ControlWord _queue[4];
    int         _numQueue;

public:
    PlotViewWrapper();
    ~PlotViewWrapper();

    void    setPos(const int x, const int y);
    void    setSize(const int width, const int height);
    void    setTitle(const QString title);
    QPoint  getPos();
    QSize   getSize();
    QString getTitle();
    void    resetData();
    void    update();
    void    close();

public slots:
    void    postCommand();
    void    sendCommand();

};

#endif // PLOTVIEWWRAPPER_H
