#ifndef PIPEASSET_H
#define PIPEASSET_H

#include <QObject>
#include "assetclass.h"
#include "pipedata.h"
#include "pipesignals.h"

class PipeAsset : public AssetClass
{
    Q_OBJECT

    friend class AddingAssetDialog;

    PipeData*               _pipeData;
    PipeSignals*            _pipeSignals;

    QString                 _pipeServer;
    quint16                 _dataPort;
    quint16                 _signalsPort;
    qint64                  _pipeSize;

    QString                 _viewName;
    QString                 _viewParam;
    unsigned                _viewChannel;
    bool                    _isSign;
    bool                    _isInteger;

    bool                    _dataPipeReady;
    bool                    _signalsPipeReady;

    bool   readView(const char *&pData, const char *pEnd);

    void   processData();
    void   invokeExchange();
    void   readPitons();

    void    saveParams(QSettings &pSetup);
    void    loadParams(QSettings &pSetup);

    bool    pipeNotReady(QObject *pipe);

public:
    PipeAsset(QObject* parent = 0);
    ~PipeAsset();

    bool    load();
    bool    edit();
    void    start();
    void    stop();
    QString description();

public slots:
    void      exchangeCompleted();
    void      connected();

};

#endif // PIPEASSET_H
