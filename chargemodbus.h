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
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QVector>



QT_BEGIN_NAMESPACE
namespace Ui { class ChargeModbus; }
QT_END_NAMESPACE

static const QList<int> InputRegisterAddress=
{
    100,101,102,110,120,130,140,150,170,180,190,200,210,111
};
static const QList<int> HoldingRegisterAddress=
{
    160,161,162,163,164,165
};

enum InputRegisterType
{
    State=0,
    ChargingVoltage=3,
    ChargeCurrent,
    ChargePower,
    ChargedTime,
    ChargedEnergy,
    SOC,
    ConnectorType,
    MaxChargePower,
    MinChargePOwer,
    PaymentConfirm,
    ErrorCode
};
enum HoldingRegisterType
{
    InputPower=0,
    ChargeControl,
    ChargerID,
    SessionID,
    MoneyBooked,
    CommunicationTimeOut
};

class ChargeModbus : public QMainWindow
{
    Q_OBJECT

public:
    ChargeModbus(QWidget *parent = nullptr);
    ~ChargeModbus();

    void Connect();
    void Disconnect();

    //Example:set State in inputregister as 1
    //data.insert(InputRegisterAddress[InputRegisterType::State],1);
    //SetInRegData(data);

    void SetInRegData(QHash<int,quint16> data);
    void SetHoldRegData(QHash<int,quint16> data);
    QHash<int,quint16> GetInRegData();
    QHash<int,quint16> GetHoldRegData();


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

    void on_pushButton_2_clicked();


private:
    Ui::ChargeModbus *ui;
    QModbusTcpClient *_client;
    QModbusTcpServer *_server;

    QModbusDataUnitMap _reg;
    bool readOnlyLock=false;
    bool changeDataInWidget=true;

    QButtonGroup coilButtons;
    QButtonGroup discreteButtons;
    QHash<QString, QLineEdit *> registers;


    QMessageBox msgbox;

    QMutex _setDataMutex;
    QMutex _setInputDataMutex;
    QMutex _setHoldingDataMutex;
    QMutex _setDataFromWidgetMutex;

    bool CheckInputHoldingValid(QModbusDataUnit::RegisterType table, int address);

    void InitServer();
    void Debugging();
    void InitButton();

};
#endif // MAINWINDOW_H
