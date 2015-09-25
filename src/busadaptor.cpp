#include "busadaptor.h"
#include <QLibrary>
#include <QDebug>
#include <QFile>
#include "busasset.h"

// загрузка busMT.dll
QLibrary    *BusAdaptor::pLib = (QLibrary*)1;

BUS_CONNECT					BusAdaptor::busConnect = 0;
BUS_DISCONNECT				BusAdaptor::busDisconnect = 0;
BUS_SETCURRENT				BusAdaptor::busSetCurrent = 0;
BUS_GETCURRENT				BusAdaptor::busGetCurrent = 0;
BUS_GETMSGTYPEID			BusAdaptor::busGetMsgTypeID = 0;
BUS_SETMSGTYPEPARAMNAMES	BusAdaptor::busSetMsgTypeParamNames = 0;
BUS_GETMSGTYPEPARAMNAMES	BusAdaptor::busGetMsgTypeParamNames = 0;
BUS_SENDPARAMSI				BusAdaptor::busSendParamsI = 0;
BUS_SENDPARAMSD				BusAdaptor::busSendParamsD = 0;
BUS_SETPARAMHANLDERI		BusAdaptor::busSetParamHandlerI = 0;
BUS_SETPARAMHANLDERD		BusAdaptor::busSetParamHandlerD = 0;
BUS_SETLOGLEVEL				BusAdaptor::busSetLogLevel = 0;
BUS_ADVANCETIME				BusAdaptor::busAdvanceTime = 0;
BUS_SETTIMEHANDLER			BusAdaptor::busSetTimeHandler = 0;

QHash<BUS_HANDLE, BusAdaptor*>  BusAdaptor::busToAdaptor;

void BusAdaptor::paramHandlerI(BUS_HANDLE hBus, BUS_MSG_TYPE_ID msgTypeId, const int *pData, const char *pFlags, int numDataElems)
{
    BusAdaptor* pBus = busToAdaptor.value(hBus, 0);
    if(pBus == 0)
        return;

    ProtoMessage* pMsg = pBus->msgToProto.value(msgTypeId, 0);
    if(pMsg == 0)
        return;

    int* msgData = pMsg->asInt;

    for(int k=0; k<numDataElems; ++k)
        if(pFlags[k])
            msgData[k] = pData[k];

    pBus->isValidData = true;
}

void BusAdaptor::paramHandlerD(BUS_HANDLE hBus, BUS_MSG_TYPE_ID msgTypeId, const double *pData, const char *pFlags, int numDataElems)
{
    BusAdaptor* pBus = busToAdaptor.value(hBus, 0);
    if(pBus == 0)
        return;

    ProtoMessage* pMsg = pBus->msgToProto.value(msgTypeId, 0);
    if(pMsg == 0)
        return;

    double* msgData = pMsg->asFloat;

    for(int k=0; k<numDataElems; ++k)
        if(pFlags[k])
            msgData[k] = pData[k];

    pBus->isValidData = true;
}

void BusAdaptor::timeHandler(BUS_HANDLE hBus, int step)
{
    BusAdaptor* pBus = busToAdaptor.value(hBus, 0);
    if(pBus == 0)
    {
        busAdvanceTime(1);
        return;
    }

    emit pBus->exchangeFinished();
}

BusAdaptor::BusAdaptor(QObject *parent):QObject(parent)
{
    hBus = 0;
    isValidData = false;
}

BusAdaptor::~BusAdaptor()
{
    if(pLib)
    {
        busDisconnect(hBus);
        busToAdaptor.remove(this);
    }
}

bool BusAdaptor::start(const QString nameFederate, const QString nameFederation, const QString urlFederation, const QString pathFdsc, QList<ProtoMessage *> messages)
{
    if(pLib == 0)
        return false;

    if(pLib == (QLibrary*)1)
    {
        pLib = new QLibrary("busmt-adapter");
        if(pLib->load())
        {
            busConnect = (BUS_CONNECT)pLib->resolve("busConnect");
            busDisconnect = (BUS_DISCONNECT)pLib->resolve("busDisconnect");
            busSetCurrent = (BUS_SETCURRENT)pLib->resolve("busSetCurrent");
            busGetCurrent = (BUS_GETCURRENT)pLib->resolve("busGetCurrent");
            busGetMsgTypeID = (BUS_GETMSGTYPEID)pLib->resolve("busGetMsgTypeID");
            busSetMsgTypeParamNames = (BUS_SETMSGTYPEPARAMNAMES)pLib->resolve("busSetMsgTypeParamNames");
            busGetMsgTypeParamNames = (BUS_GETMSGTYPEPARAMNAMES)pLib->resolve("busGetMsgTypeParamNames");
            busSendParamsI = (BUS_SENDPARAMSI)pLib->resolve("busSendParamsI");
            busSendParamsD = (BUS_SENDPARAMSD)pLib->resolve("busSendParamsD");
            busSetParamHandlerI = (BUS_SETPARAMHANLDERI)pLib->resolve("busSetParamHandlerI");
            busSetParamHandlerD = (BUS_SETPARAMHANLDERD)pLib->resolve("busSetParamHandlerD");
            busSetLogLevel = (BUS_SETLOGLEVEL)pLib->resolve("setLogLevel");
            busAdvanceTime = (BUS_ADVANCETIME)pLib->resolve("busAdvanceTime");
            busSetTimeHandler = (BUS_SETTIMEHANDLER)pLib->resolve("busSetTimeHandler");

            bool	bSomeZeroFunc = false;
            bSomeZeroFunc = (busConnect) ? bSomeZeroFunc : true;
            bSomeZeroFunc = (busDisconnect) ? bSomeZeroFunc : true;
            bSomeZeroFunc = (busSetCurrent) ? bSomeZeroFunc : true;
            bSomeZeroFunc = (busGetCurrent) ? bSomeZeroFunc : true;
            bSomeZeroFunc = (busGetMsgTypeID) ? bSomeZeroFunc : true;
            bSomeZeroFunc = (busSetMsgTypeParamNames) ? bSomeZeroFunc : true;
            bSomeZeroFunc = (busGetMsgTypeParamNames) ? bSomeZeroFunc : true;
            bSomeZeroFunc = (busSendParamsI) ? bSomeZeroFunc : true;
            bSomeZeroFunc = (busSendParamsD) ? bSomeZeroFunc : true;
            bSomeZeroFunc = (busSetParamHandlerI) ? bSomeZeroFunc : true;
            bSomeZeroFunc = (busSetParamHandlerD) ? bSomeZeroFunc : true;
            bSomeZeroFunc = (busAdvanceTime) ? bSomeZeroFunc : true;
            bSomeZeroFunc = (busSetTimeHandler) ? bSomeZeroFunc : true;

            if(bSomeZeroFunc)
            {
                qDebug() << "\nSome functions in busmt-adapter not found";
                delete pLib;
                pLib = 0;
                return false;
            }

        } else
        {
            qDebug() << pLib->errorString();
            delete pLib;
            pLib = 0;
            return false;
        }
    }

//    busSetLogLevel(3);

    QByteArray  arrFederate = nameFederate.toLatin1();
    QByteArray  arrFederation = nameFederation.toLatin1();
    QByteArray  arrFederationUrl = urlFederation.toLatin1();
    QByteArray  arrPath = pathFdsc.toAscii();

    hBus = busConnect(arrFederationUrl.constData(), arrFederate.constData(), arrFederation.constData(), arrPath.constData());
    if(hBus == 0)
        return false;

    busToAdaptor.insert(hBus, this);

    QByteArray  arrName, arrAllParams;
    foreach (ProtoMessage *pMsg, messages)
    {
        arrName = pMsg->name.toLatin1();
        BUS_MSG_TYPE_ID msgId = busGetMsgTypeID(arrName.constData());
        if(msgId == 0)
            continue;

        msgToProto.insert(msgId, pMsg);

        if(pMsg->isInteger)
        {
            pMsg->asInt = new int[pMsg->params.size()];
            memset(pMsg->asInt, 0, pMsg->params.size()*sizeof(int));
        }
        else
        {
            pMsg->asFloat = new double[pMsg->params.size()];
            memset(pMsg->asFloat, 0, pMsg->params.size()*sizeof(double));
        }


        QString all_params = pMsg->params.join(" ");
        char *params = new char[all_params.size() + 1];
        arrAllParams = all_params.toLatin1();
        strcpy(params, arrAllParams.constData());

        busSetMsgTypeParamNames(msgId, params);

        if(pMsg->isInteger)
            busSetParamHandlerI(msgId, paramHandlerI);
        else
            busSetParamHandlerD(msgId, paramHandlerD);
    }

    busSetTimeHandler(timeHandler);

    emit connected();

    return true;
}

void BusAdaptor::slotBeginExhange()
{
    if(hBus == 0)
        return;

    busSetCurrent(hBus);
    busAdvanceTime(1);
}
