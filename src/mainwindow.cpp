#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QCloseEvent>
#include <plotviewclass.h>
#include <QDesktopWidget>
#include <QMessageBox>

#include "AboutDialog.h"

#include "assetclass.h"
#include "pipeasset.h"
#include "busasset.h"

#include "dataviewclass.h"

MainWindow*     MainWindow::pWin = 0;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    pWin = this;

    _autoLoadData = false;
    _autoConnect = false;
    _needUpdate = false;
    startTimer(40);
    QMetaObject::invokeMethod(this, "onLoadState", Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::exchangeComplete(AssetClass *pAsset)
{
    DataViewClass* pView = _assetToView.value(pAsset);
    pView->updateView();

    PlotViewClass::updatePlotsData(pAsset);
    _needUpdate = true;
}

void MainWindow::setSelected(DataCollectorClass *pData)
{
    for(int idx = 0; idx < ui->tabWidget->count(); ++idx){
        DataViewClass* pView = static_cast<DataViewClass*>(ui->tabWidget->widget(idx));
        if(pView->setSelected(pData)){
            ui->tabWidget->setCurrentIndex(idx);
            return;
        }
    }
}

void MainWindow::loadDataToAssets(const QString path)
{
    QString pathName;
    foreach(AssetClass* pAsset, _assets)
    {
        pathName = path;
        pathName.append('/');
        pathName.append(pAsset->_name);

        if(QDir(pathName).exists())
        {
            pAsset->loadData(pathName);
            _assetToView.value(pAsset)->updateView();
        }
    }
}

void MainWindow::saveAssetsData(const QString path)
{
    QString pathName;
    foreach(AssetClass* pAsset, _assets)
    {
        pathName = path;
        pathName.append('/');
        pathName.append(pAsset->_name);
        pAsset->saveData(pathName);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    onSaveState();
    event->accept();
    exit(0);
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if(_needUpdate)
    {
        PlotViewClass::updatePlots();
        _needUpdate = false;
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    PlotViewClass::showPlots();
}

void MainWindow::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    PlotViewClass::hidePlots();
}

void MainWindow::onAddBusAsset()
{
    BusAsset*   pAsset = new BusAsset();
    if(pAsset->load() == false)
    {
        delete pAsset;
        return;
    }

    _assets.append(pAsset);

    DataViewClass*  pView = new DataViewClass(pAsset);
    ui->tabWidget->addTab(pView, pAsset->name());

    _assetToView.insert(pAsset, pView);
}

void MainWindow::onAddPipeAsset()
{
    PipeAsset*   pAsset = new PipeAsset();
    if(pAsset->load() == false)
    {
        delete pAsset;
        return;
    }

    _assets.append(pAsset);

    DataViewClass*  pView = new DataViewClass(pAsset);
    ui->tabWidget->addTab(pView, pAsset->name());

    _assetToView.insert(pAsset, pView);
}

void MainWindow::onChangeAsset()
{
    DataViewClass*  pView = static_cast<DataViewClass*>(ui->tabWidget->currentWidget());
    if(pView == 0)
        return;

    AssetClass*     pAsset = pView->asset();

    //disconnect
    if(ui->actionConnect->isChecked())
        ui->actionConnect->trigger();

    if(pAsset->edit()){

    }
}

void MainWindow::onAbout()
{
    AboutDialogClass*   pAbout = new AboutDialogClass(this);
    pAbout->exec();
    delete pAbout;
}

void MainWindow::onSaveState()
{
    if(!_edit)
        return;

    QSettings   pSetup("state.cfg", QSettings::IniFormat);
    pSetup.clear();

    pSetup.beginGroup("main");
    pSetup.setValue("pos", pos());
    pSetup.setValue("size", size());
    pSetup.setValue("tmp", tmpDir);

    pSetup.setValue("data", QDir::current().relativeFilePath(dataDir));
    pSetup.setValue("autoLoadData", _autoLoadData);
    pSetup.setValue("autoConnect", _autoConnect);
    pSetup.endGroup();

    QHash<DataCollectorClass*, int>     collectorToIdx;
    QHash<QCPGraph*, int>               graphToIdx;

    foreach(AssetClass* pAsset, _assets) {
        pAsset->prepareToSave(collectorToIdx);
    }
    ui->plotsManager->prepareToSave(graphToIdx);

    pSetup.beginGroup("assets");
    pSetup.beginWriteArray("asset");
    int idx = 0;
    foreach(AssetClass* pAsset, _assets)
    {
        pSetup.setArrayIndex(idx);
        pSetup.setValue("type", pAsset->_type);
        pAsset->saveState(pSetup, collectorToIdx, graphToIdx);
        ++idx;
    }
    pSetup.endArray();
    pSetup.endGroup();

    pSetup.beginGroup("plots");
    ui->plotsManager->saveState(pSetup, collectorToIdx, graphToIdx);
    pSetup.endGroup();
}

void MainWindow::onLoadState()
{
    QSettings   pSetup("state.cfg", QSettings::IniFormat);

    pSetup.beginGroup("main");
    move(pSetup.value("pos").toPoint());
    resize(pSetup.value("size").toSize());
    _edit = pSetup.value("edit", true).toBool();
    tmpDir = pSetup.value("tmp", QString(".")).toString();
    dataDir = pSetup.value("data", QString(".")).toString();
    _autoLoadData = pSetup.value("autoLoadData", false).toBool();
    _autoConnect = pSetup.value("autoConnect", false).toBool();
    pSetup.endGroup();

    pSetup.beginGroup("plots");
    ui->plotsManager->loadState(pSetup);
    pSetup.endGroup();

    AssetClass* pAsset;
    pSetup.beginGroup("assets");
    int num = pSetup.beginReadArray("asset");
    for(int k=0; k<num; ++k)
    {
        pSetup.setArrayIndex(k);

        quint8  asset_type = pSetup.value("type", ASSET_TYPE_PIPE).toUInt();
        if(asset_type == ASSET_TYPE_PIPE)
            pAsset = static_cast<AssetClass*>(new PipeAsset(this));
        else if(asset_type == ASSET_TYPE_BUS)
            pAsset = static_cast<AssetClass*>(new BusAsset(this));
        else
            continue;

        pAsset->loadState(pSetup);

        _assets.append(pAsset);

        DataViewClass*  pView = new DataViewClass(pAsset);
        ui->tabWidget->addTab(pView, pAsset->name());
        _assetToView.insert(pAsset, pView);
    }
    pSetup.endArray();
    pSetup.endGroup();

    ui->plotsManager->linkStage();
    foreach(AssetClass* pAsset, _assets)
        pAsset->linkStage();

    if(!_edit)
    {
//        Qt::WindowFlags WinFlags = windowFlags();

//        WinFlags &= ~(Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
//        WinFlags |= Qt::CustomizeWindowHint | Qt::FramelessWindowHint;

        //            if(bStayBackground)
        //                WinFlags |= Qt::WindowStaysOnBottomHint;

//        setWindowFlags(WinFlags);

        ui->tabWidget->hide();
        ui->mainToolBar->hide();
        ui->menuBar->hide();
    }

    if(_autoLoadData){
        loadDataToAssets(dataDir);
        PlotViewClass::reloadPlotsData();
        PlotViewClass::updatePlots();
    }

    if(_autoConnect){
        ui->actionConnect->setChecked(true);
    }
}

void MainWindow::onDataSave()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose folder to save data"), dataDir, QFileDialog::DontUseNativeDialog);
    if(path.isEmpty())
        return;

    dataDir = path;
    saveAssetsData(path);
}

void MainWindow::onDataLoad()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose folder to load data"), dataDir, QFileDialog::DontUseNativeDialog);
    if(path.isEmpty())
        return;

    dataDir = path;

    loadDataToAssets(path);

    PlotViewClass::reloadPlotsData();
    PlotViewClass::updatePlots();
}

void MainWindow::onClosePlots()
{
    PlotViewClass::closePlots();
}

void MainWindow::onGridPlots()
{
    QDesktopWidget* pDesktop = QApplication::desktop();

    QList<PlotViewClass*>   plots = PlotViewClass::visiblePlots();
    QList<PlotViewClass*>   screenToPlot[8];

    foreach(PlotViewClass* pPlot,  plots)
    {
        int idx = pDesktop->screenNumber(pPlot);
        if(idx != -1)
            screenToPlot[idx].append(pPlot);
    }

    for(int k = 0; k<8; ++k)
    {
        if(screenToPlot[k].size() == 0)
            continue;

        QRect   screenSize = pDesktop->screenGeometry(k);

        int plotsPerX = 1;
        int plotsPerY = 1;

        while(plotsPerX*plotsPerY < screenToPlot[k].size())
        {
            if( screenSize.width() / plotsPerX > screenSize.height() / plotsPerY )
                ++plotsPerX;
            else
                ++plotsPerY;
        }

        int size_x = screenSize.width()  / plotsPerX;
        int size_y = screenSize.height() / plotsPerY;

        int idx_x = 0, idx_y = 0;
        foreach(PlotViewClass* pPlot, screenToPlot[k])
        {
            pPlot->move(idx_x*size_x + screenSize.x(), idx_y*size_y + screenSize.y());
            pPlot->resize(size_x, size_y);

            ++idx_x;
            if(idx_x >= plotsPerX)
            {
                ++idx_y;
                idx_x = 0;
            }
        }


    }
}

void MainWindow::onDataViewClose(int index)
{
    DataViewClass*  pView = qobject_cast<DataViewClass*>(ui->tabWidget->widget(index));
    AssetClass* pAsset = pView->asset();

    QString     msg = tr("Asset %1 will be removed. All assets params will be removed from plots too.\nDo you want to proceed?").arg(pAsset->name());

    if(QMessageBox::question(this, tr("Removing Asset"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {
        //disconnect
        if(ui->actionConnect->isChecked())
            ui->actionConnect->trigger();

        //remove from data list
        ui->tabWidget->removeTab(index);
        delete pView;

        //remove from plots
        QList<QCPGraph*>  listGraphs = pAsset->graphs();
        ui->plotsManager->removeGraphs(listGraphs);

        //delete asset
        _assets.removeOne(pAsset);
        _assetToView.remove(pAsset);
        delete pAsset;
    }
}

void MainWindow::onConnect(bool isChecked)
{
    if(isChecked) {

        foreach(AssetClass* pAsset, _assets)
            pAsset->start();
    }else {

        foreach(AssetClass* pAsset, _assets)
            pAsset->stop();
    }
}
