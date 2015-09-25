#ifndef BUSASSET_H
#define BUSASSET_H

#include <QObject>
#include <QStringList>
#include "assetclass.h"

#include "busadaptor.h"

class BusAsset : public AssetClass
{
    Q_OBJECT

    friend class AddingBusAssetDialog;

    BusAdaptor*   _busAdaptor;

    QString     _nameFederate;
    QString     _nameFederation;
    QString     _urlFederate;
    QString     _protoPath;
    QString     _pathFdsc;

    QList<ProtoMessage*> _messages;

    double      _timeStepSize;

    bool    readProto();
    void    processData();
    void    invokeExchange();

    void    saveParams(QSettings &pSetup);
    void    loadParams(QSettings &pSetup);

public:

    BusAsset(QObject* parent = 0);
    ~BusAsset();

    bool    load();
    bool    edit();
    void    start();
    void    stop();
    QString description();

public slots:
    void      exchangeCompleted();
    void      connected();

};

#endif // BUSASSET_H
