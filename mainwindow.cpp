#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //_client=new QModbusTcpClient(this);
    InitServer();
    InitButton();
    ui->Port->setText(QString("192.168.1.100:502"));
    ui->serverEdit->setMinimum(1);
    ui->serverEdit->setMaximum(247);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitServer()
{
    _server=new QModbusTcpServer(this);
    _reg.insert(QModbusDataUnit::Coils, { QModbusDataUnit::Coils, 0, 10 });
    _reg.insert(QModbusDataUnit::DiscreteInputs, { QModbusDataUnit::DiscreteInputs, 0, 10 });
    _reg.insert(QModbusDataUnit::InputRegisters, { QModbusDataUnit::InputRegisters, 0, 10 });
    _reg.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, 10 });

    _server->setMap(_reg);
    connect(_server, &QModbusServer::dataWritten,
            this, &MainWindow::RefreshRegister);
    connect(_server, &QModbusServer::stateChanged,
            this, &MainWindow::onStateChanged);
    connect(_server, &QModbusServer::errorOccurred,
            this, &MainWindow::handleDeviceError);
}
void MainWindow::InitButton()
{
    coilButtons.setExclusive(false);
    discreteButtons.setExclusive(false);

    QRegularExpression regexp(QStringLiteral("Coils_(?<ID>\\d+)"));
    const QList<QCheckBox *> coils = findChildren<QCheckBox *>(regexp);
    for (QCheckBox *cbx : coils)
        coilButtons.addButton(cbx, regexp.match(cbx->objectName()).captured("ID").toInt());
    connect(&coilButtons, SIGNAL(buttonClicked(int)), this, SLOT(coilChanged(int)));

    regexp.setPattern(QStringLiteral("DisInput_(?<ID>\\d+)"));
    const QList<QCheckBox *> discs = findChildren<QCheckBox *>(regexp);
    for (QCheckBox *cbx : discs)
        discreteButtons.addButton(cbx, regexp.match(cbx->objectName()).captured("ID").toInt());
    connect(&discreteButtons, SIGNAL(buttonClicked(int)), this, SLOT(discreteInputChanged(int)));

    regexp.setPattern(QLatin1String("(in|hold)Reg_(?<ID>\\d+)"));
    const QList<QLineEdit *> qle = findChildren<QLineEdit *>(regexp);
    for (QLineEdit *lineEdit : qle) {
        registers.insert(lineEdit->objectName(), lineEdit);
        lineEdit->setProperty("ID", regexp.match(lineEdit->objectName()).captured("ID").toInt());
        lineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[0-9a-f]{0,4}"),
            QRegularExpression::CaseInsensitiveOption), this));
        connect(lineEdit, &QLineEdit::textChanged, this, &MainWindow::setRegister);
    }
}

void MainWindow::onStateChanged(){}
void MainWindow::handleDeviceError(){}
void MainWindow::setRegister(const QString &value){}

void MainWindow::on_Connect_clicked()
{
    //get Url
    const QUrl url = QUrl::fromUserInput(ui->Port->text());
    _server->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
    _server->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());
    _server->setServerAddress(ui->serverEdit->text().toInt());

    if(_server->state() == QModbusDevice::UnconnectedState)
    {
        if (!_server->connectDevice())
        {
            QString hint="Connect failed: "+_server->errorString();
            msgbox.setText(hint);
            msgbox.exec();
            qDebug()<<hint;
            statusBar()->showMessage(tr("Connect failed: ") + _server->errorString(), 5000);
        }
        else
        {
            ui->Connect->setText("disconnect");
        }
    }
    else
    {
        _server->disconnectDevice();
        ui->Connect->setText("connect");
    }
}
void MainWindow::RefreshRegister(QModbusDataUnit::RegisterType table, int address, int size)
{
    for(int i=0;i<size;++i)
    {
        quint16 value;
        QString text;
        switch(table)
        {
        case QModbusDataUnit::Coils:
            _server->data(QModbusDataUnit::Coils,quint16(address+i),&value);
            coilButtons.button(address+i)->setChecked(value);
            break;

        }
    }



    for (int i = 0; i < size; ++i) {
        quint16 value;
        QString text;
        switch (table) {
        case QModbusDataUnit::Coils:
            modbusDevice->data(QModbusDataUnit::Coils, quint16(address + i), &value);
            coilButtons.button(address + i)->setChecked(value);
            break;
        case QModbusDataUnit::HoldingRegisters:
            modbusDevice->data(QModbusDataUnit::HoldingRegisters, quint16(address + i), &value);
            registers.value(QStringLiteral("holdReg_%1").arg(address + i))->setText(text
                .setNum(value, 16));
            break;
        default:
            break;
        }
    }
}
