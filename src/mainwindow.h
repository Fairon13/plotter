#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>

namespace Ui {
class MainWindow;
}

class PlotViewClass;
class AssetClass;
class DataViewClass;
class DataCollectorClass;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    bool    _needUpdate;
    bool    _edit;
    bool    _autoLoadData;
    bool    _autoConnect;

    QList<AssetClass*>                  _assets;
    QHash<AssetClass*, DataViewClass*>  _assetToView;

public:
    QString tmpDir;
    QString dataDir;

    static  MainWindow*     pWin;

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void    exchangeComplete(AssetClass* pAsset);

    void    setSelected(DataCollectorClass *pData);
    void    loadDataToAssets(const QString path);
    void    saveAssetsData(const QString path);

protected:
    void    closeEvent(QCloseEvent *event);
    void    timerEvent(QTimerEvent *event);
    void    showEvent(QShowEvent *event);
    void    hideEvent(QHideEvent *event);

public slots:
    void    onAddPipeAsset();
    void    onAddBusAsset();
    void    onChangeAsset();

    void    onAbout();
    void    onSaveState();
    void    onLoadState();

    void    onConnect(bool isChecked);
    void    onDataSave();
    void    onDataLoad();

    void    onClosePlots();
    void    onGridPlots();

    void    onDataViewClose(int index);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
