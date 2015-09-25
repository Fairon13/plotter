#include "busasset.h"
#include "addingbusassetdialog.h"

#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/descriptor.h>

#include <QFileInfo>
#include <QProcess>
#include <QMessageBox>

#include "mainwindow.h"

bool BusAsset::readProto()
{
    using namespace google::protobuf;
    using namespace google::protobuf::compiler;

    QFileInfo   fileInfo(_protoPath);
    if(!fileInfo.exists())
        return false;

    class ErrorCollector: public MultiFileErrorCollector
    {
        public:
        ErrorCollector() : MultiFileErrorCollector() {}
        void AddError(const string & filename, int line, int column, const string & message) {}
    } pErr;

    DiskSourceTree           pSrc;
    pSrc.MapPath("", fileInfo.path().toStdString());

    QByteArray arrData = fileInfo.fileName().toLatin1();
    Importer        pImport(&pSrc, &pErr);
    const FileDescriptor  *pFileDesc = pImport.Import(arrData.constData());

    if(pFileDesc == 0)
        return false;

    ProtoMessage    *pMsg;
    QStringList     paramsFloat;
    QStringList     paramsInteger;

    for(int k = 0; k<pFileDesc->message_type_count(); ++k)
    {
        paramsFloat.clear();
        paramsInteger.clear();

        const Descriptor    *pDesc = pFileDesc->message_type(k);
        for(int n=0; n<pDesc->field_count(); ++n)
        {
            const FieldDescriptor   *pField = pDesc->field(n);
            if(pField->type() == FieldDescriptor::TYPE_DOUBLE)
                paramsFloat.append(QString::fromStdString(pField->name()));
            else if(pField->type() == FieldDescriptor::TYPE_INT32)
                paramsInteger.append(QString::fromStdString(pField->name()));
        }

        if(!paramsFloat.isEmpty())
        {
            pMsg = new ProtoMessage;
            pMsg->name = QString::fromStdString(pDesc->name());
            pMsg->params = paramsFloat;
            pMsg->isInteger = false;
            _messages.append(pMsg);
        }

        if(!paramsInteger.isEmpty())
        {
            pMsg = new ProtoMessage;
            pMsg->name = QString::fromStdString(pDesc->name());
            pMsg->params = paramsInteger;
            pMsg->isInteger = true;
            _messages.append(pMsg);
        }
    }

    return true;
}

void BusAsset::processData()
{
    if(_busAdaptor == 0)
        return;

    if(_busAdaptor->isValidData)
    {
        if(_collectors.size() != 0)
        {
            if(_noValue)
            {
                _startTime = 0.;
                _curTime = 0.;
                _noValue = false;
            } else
            {
                _curTime += _timeStepSize;
                addTime(_curTime);

                unsigned    paramIdx;
                unsigned    messageIdx = 0;
                foreach(ProtoMessage*  pMsg, _messages)
                {
                    paramIdx = 0;
                    for(int k = 0; k < pMsg->params.size(); ++k)
                    {
                        unsigned    idx = (messageIdx << 16) | paramIdx;
                        if(pMsg->isInteger)
                            addValue(idx, pMsg->asInt[k]);
                        else
                            addValue(idx, pMsg->asFloat[k]);
                        ++paramIdx;
                    }
                    ++messageIdx;
                }

                upExchangeComplete();
            }
        }
        _busAdaptor->isValidData = false;
    }

    invokeExchange();
}

void BusAsset::invokeExchange()
{
    if(_busAdaptor)
        QMetaObject::invokeMethod(_busAdaptor, "slotBeginExhange", Qt::QueuedConnection);
}

void BusAsset::saveParams(QSettings &pSetup)
{
    pSetup.setValue("federationName", _nameFederation);
    pSetup.setValue("federationUrl",  _urlFederate);
    pSetup.setValue("federateName",   _nameFederate);
    pSetup.setValue("protoFile",      _protoPath);
    pSetup.setValue("timeStepSize", _timeStepSize);

    pSetup.beginWriteArray("message", _messages.count());
    int idx = 0;
    foreach(ProtoMessage* msg, _messages)
    {
        pSetup.setArrayIndex(idx);
        pSetup.setValue("name", msg->name);
        pSetup.setValue("params", msg->params);
        pSetup.setValue("is_integer", msg->isInteger);
        ++idx;
    }
    pSetup.endArray();
}

void BusAsset::loadParams(QSettings &pSetup)
{
    _nameFederation = pSetup.value("federationName", "Federation").toString();
    _urlFederate = pSetup.value("federationUrl", "tcp://localhost:61616").toString();
    _nameFederate = pSetup.value("federateName", "Federate").toString();
    _protoPath = pSetup.value("protoFile", "federate.proto").toString();
    _timeStepSize = pSetup.value("timeStepSize", 0.1).toDouble();

    ProtoMessage* msg = 0;
    int idx = pSetup.beginReadArray("message");
    for(int k=0; k<idx; ++k)
    {
        pSetup.setArrayIndex(k);
        QStringList params = pSetup.value("params").toStringList();
        if(params.count())
        {
            msg = new ProtoMessage;
            msg->name = pSetup.value("name", "no_name").toString();
            msg->params = params;
            msg->isInteger = pSetup.value("is_integer", false).toBool();
            if(msg->isInteger)
            {
                msg->asInt = new int[params.count()];
                memset(msg->asInt, 0, params.count()*sizeof(int));
            } else
            {
                msg->asFloat = new double[params.count()];
                memset(msg->asFloat, 0, params.count()*sizeof(double));
            }

            _messages.append(msg);
        }
    }

    pSetup.endArray();
}

BusAsset::BusAsset(QObject *parent):AssetClass(parent)
{
    _type = ASSET_TYPE_BUS;
    _timeStepSize = 0.1;
    _busAdaptor = 0;
}

BusAsset::~BusAsset()
{
    foreach (ProtoMessage* pProto, _messages)
        delete pProto;

    stop();
}

bool BusAsset::load()
{
    //Ask user input
    AddingBusAssetDialog   pDialog;
    if(pDialog.exec() != QDialog::Accepted)
        return false;

    pDialog.fillAsset(this);

    //Load data
    if(!readProto())
        return false;

    unsigned    paramIdx;
    unsigned    messageIdx = 0;
    foreach(ProtoMessage*  pMsg, _messages)
    {
        paramIdx = 0;
        foreach (QString name, pMsg->params) {

            unsigned    idx = (messageIdx << 16) | paramIdx;
            DataCollectorClass* pCollector = new DataCollectorClass(this, name, pMsg->name, idx, false, pMsg->isInteger);
            if(pMsg->isInteger)
                _collectors.insert(idx | 0x80000000, pCollector);
            else
                _collectors.insert(idx, pCollector);
            ++paramIdx;
        }
        ++messageIdx;
    }

    return true;
}

bool BusAsset::edit()
{
    //Ask user input
    AddingBusAssetDialog   pDialog(this);
    if(pDialog.exec() != QDialog::Accepted)
        return false;

    pDialog.fillAsset(this);

    return true;
}

void BusAsset::start()
{
    AssetClass::start();

    _curTime = -1.;
    _noValue = true;

    QFileInfo   fileInfo(_protoPath);
    fileInfo.fileName();

    _pathFdsc = _protoPath;
    _pathFdsc.truncate(_protoPath.size() - 5);
    _pathFdsc.append("fdsc");

    QStringList	listString;
    QString param = "--proto_path=";
    param.append(fileInfo.path());
    listString.append(param);

    param = "--descriptor_set_out=";
    param.append(_pathFdsc);
    listString.append(param);

    listString.append(_protoPath);

    int res = QProcess::execute("protoc", listString);
    if(res == -2)
    {
        QMessageBox::critical(MainWindow::pWin, tr("Running protoc"), tr("protoc cannot be started"));
        return;
    } else if(res == -1)
    {
        QMessageBox::critical(MainWindow::pWin, tr("Running protoc"), tr("protoc has crushed"));
        return;
    }

    _busAdaptor = new BusAdaptor;
    connect(_busAdaptor, SIGNAL(exchangeFinished()), SLOT(exchangeCompleted()), Qt::QueuedConnection);
    connect(_busAdaptor, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);

    _busAdaptor->start(_nameFederate, _nameFederation, _urlFederate, _pathFdsc, _messages);
}

void BusAsset::stop()
{
    if(_busAdaptor)
    {
        delete _busAdaptor;
        _busAdaptor = 0;
    }
}

QString BusAsset::description()
{
    QString txt;

    txt.append(_nameFederate);
    txt.append('[');
    txt.append(QString::number(_collectors.size()));
    txt.append(']');
    txt.append('(');
    txt.append(_urlFederate);
    txt.append("):");
    txt.append(_nameFederation);

    return txt;
}

void BusAsset::exchangeCompleted()
{
    processData();
}

void BusAsset::connected()
{
    reset();
    invokeExchange();
}

