#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModbusDevice>
#include <QModbusTcpClient>
#include <QModbusDataUnit>
#include <QModbusTcpServer>
#include <QDebug>
#include <QUrl>
#include <QMessageBox>
#include <QButtonGroup>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

static const int InputRegisterAddress[]=
{
    1000,1001,1002,1100,1200,1300,1400,1500,1700,1800,1900,2000,1111
};
static const int HoldingRegisterAddress[]=
{
    1600,1601,1602,1603
};

enum InputRegisterType
{
    State=0,
    ChargeState,
    StationState
};
enum HoldingRegisterType
{
    InputPower=0,
    Suspend,
    ChargeControl,
    CommunicationTimeOut
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void InitServer();

    void InitButton();


public slots:
    void on_Connect_clicked();

    void onStateChanged();
    void handleDeviceError(QModbusDevice::Error newError);
    void RefreshRegister(QModbusDataUnit::RegisterType table, int address, int size);
    void setRegister(const QString &value);

    void coilChanged(int id);
    void discreteInputChanged(int id);

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QModbusTcpClient *_client;
    QModbusTcpServer *_server;

    QModbusDataUnitMap _reg;

    QButtonGroup coilButtons;
    QButtonGroup discreteButtons;
    QHash<QString, QLineEdit *> registers;


    QMessageBox msgbox;

};
#endif // MAINWINDOW_H
