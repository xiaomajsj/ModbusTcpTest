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

    ui->Port->setText(QString("192.168.1.100:502"));
    ui->serverEdit->setMinimum(1);
    ui->serverEdit->setMaximum(247);

}
void MainWindow::InitButton()
{
    coilButtons.setExclusive(false);
    discreteButtons.setExclusive(false);

    //Find all checkbox with objectname Coil**, and pack them all into coilButtons with specific id. RegularExpression is used
    //to place checkbox in order according to their object name. e.g. Coil0 is numbered as id=0

    QRegularExpression regexp(QStringLiteral("Coil(?<ID>\\d+)"));
    const QList<QCheckBox *> coils = findChildren<QCheckBox *>(regexp);
    for (QCheckBox *cbx : coils)
        coilButtons.addButton(cbx, regexp.match(cbx->objectName()).captured("ID").toInt());
    //set the server register if coil is changed
    connect(&coilButtons, SIGNAL(buttonClicked(int)), this, SLOT(coilChanged(int)));

    //Same, pack all DisInput checkbox
    regexp.setPattern(QStringLiteral("DisInput(?<ID>\\d+)"));
    const QList<QCheckBox *> discs = findChildren<QCheckBox *>(regexp);
    for (QCheckBox *cbx : discs)
        discreteButtons.addButton(cbx, regexp.match(cbx->objectName()).captured("ID").toInt());
    //set the server register if discret input is changed
    connect(&discreteButtons, SIGNAL(buttonClicked(int)), this, SLOT(discreteInputChanged(int)));


    //pack all lineEdit into registers with a QHash, each lineEdit is identified through QString in QHash<QString, QLineEdit *>
    //Also numbered them with Property "ID".

    regexp.setPattern(QLatin1String("(Input|Holding)Register(?<ID>\\d+)"));
    const QList<QLineEdit *> qle = findChildren<QLineEdit *>(regexp);
    for (QLineEdit *lineEdit : qle) {
        registers.insert(lineEdit->objectName(), lineEdit);
        lineEdit->setProperty("ID", regexp.match(lineEdit->objectName()).captured("ID").toInt());
        lineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[0-9a-f]{0,4}"),
            QRegularExpression::CaseInsensitiveOption), this));
        lineEdit->setPlaceholderText("Hexadecimal A-F, a-f, 0-9.");
        connect(lineEdit, &QLineEdit::textChanged, this, &MainWindow::setRegister);
    }

    //When the coil and diskret input is clicked, slots are triggered to set the _server register data
    //  When the text in lineEdit(InputRegister and HoldingRegister) is changed, slots are triggered.
    //  Also, when the coil and holding register is written by client, it will trigger RefreshRegister. Then, if the text is changed, it will trigger
    //  setRegisters to set the _server register data.
}

void MainWindow::coilChanged(int id)
{
    if(!_server)return;
    if(!_server->setData(QModbusDataUnit::Coils,quint16(id),coilButtons.button(id)->isChecked()))
        statusBar()->showMessage("Failed to set Coil registers");
}
void MainWindow::discreteInputChanged(int id)
{
    if(!_server)return;
    if(!_server->setData(QModbusDataUnit::InputRegisters,quint16(id),discreteButtons.button(id)->isChecked()))
        statusBar()->showMessage("Failed to set discret Input registers");
}

void MainWindow::onStateChanged()
{
    if(_server->state() == QModbusDevice::UnconnectedState)
    {
        ui->Connect->setText("connect");
    }
    else
    {
        ui->Connect->setText("disconnect");
    }
}

void MainWindow::handleDeviceError(QModbusDevice::Error newError)
{
    if (newError == QModbusDevice::NoError || !_server)
        return;

    statusBar()->showMessage(_server->errorString(), 5000);
}

void MainWindow::setRegister(const QString &value)
{
    if(!_server)return;
    const QString objectName=QObject::sender()->objectName();
    if(registers.contains(objectName))
    {
        bool ok=true;
        if(objectName.contains("InputReg"))
        {
            const quint16 id=quint16(QObject::sender()->property("ID").toUInt());
            ok=_server->setData(QModbusDataUnit::InputRegisters,id,value.toUShort(&ok,16));
        }
        else if(objectName.contains("HoldingReg"))
        {
            const quint16 id=quint16(QObject::sender()->property("ID").toUInt());
            ok=_server->setData(QModbusDataUnit::HoldingRegisters,id,value.toUShort(&ok,16));
        }
        if(!ok)
        {
            QString hint="Failed to set Register";
            qDebug()<<hint;
            statusBar()->showMessage(hint);
            msgbox.setText(hint);
            msgbox.exec();
        }
    }

}

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
    //Only the coils and HoldingRegisters can be both written and read. Input Registers and Discret input can only be read.
    //Therefore, only coils and holding Register is refreshed when receiving QModbusServer::dataWritten signal

    for(int i=0;i<size;++i)
    {
        quint16 value;
        QString text;
        //pay attention to value. data() function will set &value as the current register data.
        switch(table)
        {
        case QModbusDataUnit::Coils:
            _server->data(QModbusDataUnit::Coils,quint16(address+i),&value);
            coilButtons.button(address+i)->setChecked(value);
            break;
        case QModbusDataUnit::HoldingRegisters:
            _server->data(QModbusDataUnit::HoldingRegisters,quint16(address+i),&value);
            registers.value(QStringLiteral("HoldingReg%1").arg(address + i))->setText(text
                .setNum(value, 16));
            break;
        default:
            break;
        }
    }
}
