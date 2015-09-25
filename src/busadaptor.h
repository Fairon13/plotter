#ifndef BUSADAPTOR_H
#define BUSADAPTOR_H

#include <QObject>
#include <QLibrary>
#include <QStringList>
#include <QHash>

//Работа с библиотекой
//Секция для busMT
typedef void* BUS_HANDLE;
typedef void* BUS_MSG_TYPE_ID;

#define		BUS_RES_SUCCESS				0
#define		BUS_RES_INVALID_PARAM		1
#define		BUS_RES_INVALID_HANDLE		2
#define		BUS_RES_DISCONNECTED		3
#define		BUS_NO_CURRENT_CONNECTION	4

#ifdef WIN32
typedef BUS_HANDLE		(__cdecl *BUS_CONNECT)   (const char* uri, const char* federateId, const char* federationId, const char* descSetFile);
typedef int				(__cdecl *BUS_DISCONNECT)(BUS_HANDLE hBus);
typedef int				(__cdecl *BUS_SETCURRENT)(BUS_HANDLE hBus);
typedef BUS_HANDLE		(__cdecl *BUS_GETCURRENT)(void);
typedef BUS_MSG_TYPE_ID	(__cdecl *BUS_GETMSGTYPEID)(const char* msgTypeName);

typedef int				(__cdecl *BUS_SETMSGTYPEPARAMNAMES)(BUS_MSG_TYPE_ID msgTypeId, const char* paramNames);
typedef const char*		(__cdecl *BUS_GETMSGTYPEPARAMNAMES)(BUS_MSG_TYPE_ID msgTypeId);

typedef int				(__cdecl *BUS_SENDPARAMSI)(BUS_MSG_TYPE_ID msgTypeId, const int* paramsValue, int paramCount);
typedef int				(__cdecl *BUS_SENDPARAMSD)(BUS_MSG_TYPE_ID msgTypeId, const double* paramsValue, int paramCount);

typedef void            (*busParamsHandlerI) (BUS_HANDLE hBus, BUS_MSG_TYPE_ID msgTypeId, const int* pData, const char* pFlags, int numDataElems);
typedef void            (*busParamsHandlerD) (BUS_HANDLE hBus, BUS_MSG_TYPE_ID msgTypeId, const double* pData, const char* pFlags, int numDataElems);

typedef int				(__cdecl *BUS_SETPARAMHANLDERI)(BUS_MSG_TYPE_ID msgTypeId, busParamsHandlerI handlerFunc);
typedef int				(__cdecl *BUS_SETPARAMHANLDERD)(BUS_MSG_TYPE_ID msgTypeId, busParamsHandlerD handlerFunc);

typedef void			(__cdecl *BUS_SETLOGLEVEL)(int logLevel);

typedef int				(__cdecl *BUS_ADVANCETIME)(int steps);
typedef void            (*busTimeHandler) (BUS_HANDLE, int);
typedef int				(__cdecl *BUS_SETTIMEHANDLER)(busTimeHandler* handlerFunc);
#else
typedef BUS_HANDLE		(*BUS_CONNECT)   (const char* uri, const char* federateId, const char* federationId, const char* descSetFile) __attribute__((cdecl));
typedef int				(*BUS_DISCONNECT)(BUS_HANDLE hBus)              __attribute__((cdecl));
typedef int				(*BUS_SETCURRENT)(BUS_HANDLE hBus)              __attribute__((cdecl));
typedef BUS_HANDLE		(*BUS_GETCURRENT)(void)                         __attribute__((cdecl));
typedef BUS_MSG_TYPE_ID	(*BUS_GETMSGTYPEID)(const char* msgTypeName)    __attribute__((cdecl));

typedef int				(*BUS_SETMSGTYPEPARAMNAMES)(BUS_MSG_TYPE_ID msgTypeId, const char* paramNames) __attribute__((cdecl));
typedef const char*		(*BUS_GETMSGTYPEPARAMNAMES)(BUS_MSG_TYPE_ID msgTypeId) __attribute__((cdecl));

typedef int				(*BUS_SENDPARAMSI)(BUS_MSG_TYPE_ID msgTypeId, const int* paramsValue, int paramCount) __attribute__((cdecl));
typedef int				(*BUS_SENDPARAMSD)(BUS_MSG_TYPE_ID msgTypeId, const double* paramsValue, int paramCount) __attribute__((cdecl));

typedef void            (*busParamsHandlerI) (BUS_HANDLE hBus, BUS_MSG_TYPE_ID msgTypeId, const int* pData, const char* pFlags, int numDataElems) __attribute__((cdecl));
typedef void            (*busParamsHandlerD) (BUS_HANDLE hBus, BUS_MSG_TYPE_ID msgTypeId, const double* pData, const char* pFlags, int numDataElems) __attribute__((cdecl));

typedef int				(*BUS_SETPARAMHANLDERI)(BUS_MSG_TYPE_ID msgTypeId, busParamsHandlerI handlerFunc) __attribute__((cdecl));
typedef int				(*BUS_SETPARAMHANLDERD)(BUS_MSG_TYPE_ID msgTypeId, busParamsHandlerD handlerFunc) __attribute__((cdecl));

typedef void			(*BUS_SETLOGLEVEL)(int logLevel) __attribute__((cdecl));

typedef int				(*BUS_ADVANCETIME)(int steps) __attribute__((cdecl));
typedef void            (*busTimeHandler) (BUS_HANDLE, int) __attribute__((cdecl));
typedef int				(*BUS_SETTIMEHANDLER)(busTimeHandler  handlerFunc) __attribute__((cdecl));
#endif

struct  ProtoMessage
{
    QString         name;
    QStringList     params;     //Разделённые пробелами имена параметров
    union
    {
        double      *asFloat;
        int         *asInt;
    };

    bool            isInteger;
};

class  BusAdaptor : public QObject
{
    Q_OBJECT

    static QLibrary    *pLib;
    static BUS_CONNECT					busConnect;
    static BUS_DISCONNECT				busDisconnect;
    static BUS_SETCURRENT				busSetCurrent;
    static BUS_GETCURRENT				busGetCurrent;
    static BUS_GETMSGTYPEID             busGetMsgTypeID;
    static BUS_SETMSGTYPEPARAMNAMES     busSetMsgTypeParamNames;
    static BUS_GETMSGTYPEPARAMNAMES     busGetMsgTypeParamNames;
    static BUS_SENDPARAMSI				busSendParamsI;
    static BUS_SENDPARAMSD				busSendParamsD;
    static BUS_SETPARAMHANLDERI         busSetParamHandlerI;
    static BUS_SETPARAMHANLDERD         busSetParamHandlerD;
    static BUS_SETLOGLEVEL				busSetLogLevel;
    static BUS_ADVANCETIME				busAdvanceTime;
    static BUS_SETTIMEHANDLER			busSetTimeHandler;

    static  QHash<BUS_HANDLE, BusAdaptor*>  busToAdaptor;

    QHash<BUS_MSG_TYPE_ID, ProtoMessage*>   msgToProto;
    BUS_HANDLE hBus;


#ifdef WIN32
    static __cdecl void paramHandlerI(BUS_HANDLE hBus, BUS_MSG_TYPE_ID msgTypeId, const int* pData, const char* pFlags, int numDataElems);
    static __cdecl void paramHandlerD(BUS_HANDLE hBus, BUS_MSG_TYPE_ID msgTypeId, const double* pData, const char* pFlags, int numDataElems);
    static __cdecl void timeHandler(BUS_HANDLE hBus, int step);
#else
    static  void paramHandlerI(BUS_HANDLE hBus, BUS_MSG_TYPE_ID msgTypeId, const int* pData, const char* pFlags, int numDataElems) __attribute__((cdecl));
    static  void paramHandlerD(BUS_HANDLE hBus, BUS_MSG_TYPE_ID msgTypeId, const double* pData, const char* pFlags, int numDataElems) __attribute__((cdecl));
    static  void timeHandler(BUS_HANDLE hBus, int step) __attribute__((cdecl));
#endif

public:
    bool    isValidData;

    BusAdaptor(QObject* parent = 0);
    ~BusAdaptor();

   bool start(const QString nameFederate, const QString nameFederation, const QString urlFederation, const QString pathFdsc, QList<ProtoMessage*>   messages);

public slots:
   void  slotBeginExhange(); //Begin exchange data

signals:
    void        exchangeFinished();
    void        connected();

};

#endif // BUSADAPTOR_H
