#include "pipeasset.h"
#include "addingassetdialog.h"

#define fv_begin             0
#define fv_check_svc         1
#define fv_check_i           2
#define fv_check_e           3
#define fv_check_w           4
#define fv_check_points      5
#define fv_read_view_name    6
#define fv_skip_to_name      7
#define fv_read_param_name   8
#define fv_read_number       9
#define fv_check_g           10
#define fv_check_n           11
#define fv_check_o           12
#define fv_check_t           13
#define fv_check_r           14
#define fv_check_l           15

bool PipeAsset::readView(const char *&pData, const char *pEnd)
{
    char    state = fv_begin;
    char    nextState = fv_begin;
    char    prevState;

    _viewName.clear();
    _viewParam.clear();
    _viewChannel = 0;

    char        szTemp[32];
    char*       pTemp;

    forever
    {
        switch(state)
        {
        case    fv_begin:
            if(*pData == '/')
                state = fv_check_svc;
            break;
        case    fv_check_svc:
            if(*pData == 'v' || *pData == 'V')
            {
                _isSign = false;
                _isInteger = false;
                state = fv_check_i;
            }else if(*pData == 's' || *pData == 'S')
            {
                _isSign = true;
                _isInteger = false;
                state = fv_check_i;
            }else if(*pData == 'c' || *pData == 'C')
            {
                _isSign = false;
                _isInteger = true;
                prevState = fv_check_svc;
                state = fv_check_o;
            }else
                state = fv_begin;
            break;
        case    fv_check_i:
            if(*pData == 'i' || *pData == 'I')
            {
                if(_isSign)
                    state = fv_check_g;
                else
                    state = fv_check_e;
            }else
                state = fv_begin;
            break;
        case    fv_check_e:
            if(*pData == 'e' || *pData == 'E')
                state = fv_check_w;
            else
                state = fv_begin;
            break;
        case    fv_check_w:
            if(*pData == 'w' || *pData == 'W')
                state = fv_check_points;
            else
                state = fv_begin;
            break;
        case    fv_check_g:
            if(*pData == 'g' || *pData == 'G')
            {
                state = fv_check_n;
                prevState = fv_check_g;
            }else
                state = fv_begin;
            break;
        case    fv_check_n:
            if(*pData == 'n' || *pData == 'N')
            {
                if(prevState == fv_check_g)
                    state = fv_check_points;
                else if(prevState == fv_check_o)
                    state = fv_check_t;
            }else
                state = fv_begin;
            break;
        case    fv_check_o:
            if(*pData == 'o' || *pData == 'O')
            {
                if(prevState == fv_check_svc)
                {
                    state = fv_check_n;
                    prevState = fv_check_o;
                } else if(prevState == fv_check_r)
                    state = fv_check_l;
            }
            else
                state = fv_begin;
            break;
        case    fv_check_t:
            if(*pData == 't' || *pData == 'T')
                state = fv_check_r;
            else
                state = fv_begin;
            break;
        case    fv_check_r:
            if(*pData == 'r' || *pData == 'R')
            {
                state = fv_check_o;
                prevState = fv_check_r;
            }
            else
                state = fv_begin;
            break;
        case    fv_check_l:
            if(*pData == 'l' || *pData == 'L')
                state = fv_check_points;
            else
                state = fv_begin;
            break;
        case    fv_check_points:
            if(*pData == ':')
            {
                state = fv_read_view_name;
                pTemp = szTemp;
            }else
                state = fv_begin;
            break;
        case    fv_read_view_name:
            if(*pData == ' ' || *pData == '\t' || *pData == '\n' || *pData == '\r' || *pData == '\'')
            {
                *pTemp = 0;
                _viewName = QString::fromLatin1(szTemp);

                state = fv_skip_to_name;
                pTemp = szTemp;
                if(_isInteger)
                    nextState = fv_read_number;
                else
                    nextState = fv_read_param_name;
            } else
            {
                *pTemp = *pData;
                ++pTemp;
            }
            break;
        case    fv_skip_to_name:
            if(*pData == '\'')
                state = nextState;
            break;
        case    fv_read_param_name:
            if(*pData == ' ' || *pData == '\t' || *pData == '\n' || *pData == '\r' || *pData == '\'')
            {
                *pTemp = 0;
                _viewParam = QString::fromLatin1(szTemp);

                while(*pData != '\'')
                    ++pData;

                state = fv_skip_to_name;
                pTemp = szTemp;
                nextState = fv_read_number;
            } else
            {
                *pTemp = *pData;
                ++pTemp;
            }
            break;
        case    fv_read_number:
            if(*pData != '\'')
            {
                *pTemp = *pData;
                ++pTemp;
            } else
            {
                *pTemp = 0;
                bool    ok = false;
                _viewChannel = QString::fromLatin1(szTemp).toUInt(&ok);
                if(ok == false)
                    _viewChannel = 0;
                return true;
            }
            break;
        default:
            break;
        }

        ++pData;
        if(pData == pEnd)
            return false;
    }
    return true;
}

PipeAsset::PipeAsset(QObject *parent):AssetClass(parent)
{
    _type = ASSET_TYPE_PIPE;
    _pipeData = 0;
    _pipeSignals = 0;
}

PipeAsset::~PipeAsset()
{
    stop();
}

bool PipeAsset::load()
{
    AddingAssetDialog   pDialog;
    if(pDialog.exec() == QDialog::Accepted)
    {
        pDialog.fillAsset(this);
        readPitons();
        return true;
    }

    return false;
}

bool PipeAsset::edit()
{
    AddingAssetDialog   pDialog(this);
    if(pDialog.exec() == QDialog::Accepted){
        pDialog.fillAsset(this);

        return true;
    }

    return false;
}

void PipeAsset::start()
{
    AssetClass::start();

    _curTime = -1.;
    _noValue = true;

    _dataPipeReady = false;
    _signalsPipeReady = false;

    _pipeData = new PipeData(_pipeServer, _dataPort, 150000);
    connect(_pipeData, SIGNAL(exchangeFinished()), SLOT(exchangeCompleted()), Qt::QueuedConnection);
    connect(_pipeData, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);
    _pipeData->start();

    _pipeSignals = new PipeSignals(_pipeServer, _signalsPort, 150000);
    connect(_pipeSignals, SIGNAL(exchangeFinished()), SLOT(exchangeCompleted()), Qt::QueuedConnection);
    connect(_pipeSignals, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);
    _pipeSignals->start();
}

void PipeAsset::stop()
{
    if(_pipeData)
    {
        _pipeData->deletePipe();
        _pipeData = 0;
    }

    if(_pipeSignals)
    {
        _pipeSignals->deletePipe();
        _pipeSignals = 0;
    }
}

QString PipeAsset::description()
{
    QString txt = _name;

    txt.append('[');
    txt.append(QString::number(_collectors.count()));
    txt.append("] (");
    txt.append(_pipeServer);
    txt.append(", D:");
    txt.append(QString::number(_dataPort));
    txt.append(", S:");
    txt.append(QString::number(_signalsPort));
    txt.append(')');

    return txt;
}

void PipeAsset::readPitons()
{
    if(_path.isEmpty())
        return;

    QLatin1String   pitonTemplate("/piton.%1");

    for(int k=1; k<10; ++k)
    {
        QString strFile = _path;
        strFile.append(pitonTemplate);
        strFile = strFile.arg(k, 3, 10, QChar('0'));

        QFile   fileIn(strFile);
        if(!fileIn.open(QIODevice::ReadOnly))
            continue;

        if(fileIn.size() < 6)
            continue;

        QByteArray  arr = fileIn.readAll(); fileIn.close();
        const char* pData = arr.constData();
        const char* pEnd = pData + arr.size();
        while(readView(pData, pEnd))
        {
            if(_viewName.isEmpty() || _viewChannel == 0)
                continue;

            DataCollectorClass* pCollector = new DataCollectorClass(this, _viewName, _viewParam, _viewChannel, _isSign, _isInteger);

            if(_isInteger)
                _viewChannel |= 0x80000000u;

            _collectors.insert(_viewChannel, pCollector);
        }
    }
}

void PipeAsset::saveParams(QSettings &pSetup)
{
    pSetup.setValue("pipeServer",   _pipeServer);
    pSetup.setValue("pipePort",     _dataPort);
    pSetup.setValue("signalsPort",  _signalsPort);
}

void PipeAsset::loadParams(QSettings &pSetup)
{
    _pipeServer = pSetup.value("pipeServer", "localhost").toString();
    _dataPort = pSetup.value("pipePort").toUInt();
    _signalsPort = pSetup.value("signalsPort", _dataPort + 1).toUInt();
    _pipeSize = 4;
}

bool PipeAsset::pipeNotReady(QObject* pipe)
{
    if(pipe == _pipeData)
    {
        if(!_signalsPipeReady)
        {
            _dataPipeReady = true;
            return true;
        }
    } else if(pipe == _pipeSignals)
    {
        if(!_dataPipeReady)
        {
            _signalsPipeReady = true;
            return true;
        }
    }

    return false;
}

void PipeAsset::processData()
{
    if(_pipeSignals->_wasLoadState)
    {
        _pipeSignals->_wasLoadState = false;
        reset();
    } else if(_collectors.count() != 0)
    {
        if(_pipeData->viewsValue != 0)
        {
            if(_curTime != _pipeData->viewsValue[0])
            {
                if(_noValue)
                {
                    _startTime = _pipeData->viewsValue[0];
                    _noValue = false;
                } else
                {
                    //views
                    for(unsigned k=1; k < _pipeData->nEx; ++k)
                        addValue(_pipeData->viewsIdx[k], _pipeData->viewsValue[k]);

                    //controls
                    for(int n = 0; n < _pipeSignals->numData(); ++n)
                        addValue(_pipeSignals->channel(n), _pipeSignals->value(n));

                    addTime(_pipeData->viewsValue[0]);

                    upExchangeComplete();
                }
            }
        }
    }
    invokeExchange();
}

void PipeAsset::invokeExchange()
{
    _dataPipeReady = false;
    _signalsPipeReady = false;

    QMetaObject::invokeMethod(_pipeData, "slotBeginExhange", Qt::QueuedConnection);
    QMetaObject::invokeMethod(_pipeSignals, "slotBeginExhange", Qt::QueuedConnection);
}

void PipeAsset::exchangeCompleted()
{
    if(pipeNotReady(sender()))
        return;

    processData();
}

void PipeAsset::connected()
{
    if(pipeNotReady(sender()))
        return;

    reset();
    invokeExchange();
}
