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
    void handleDeviceError();
    void RefreshRegister(QModbusDataUnit::RegisterType table, int address, int size);
    void setRegister(const QString &value);

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
