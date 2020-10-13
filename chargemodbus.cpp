#include "chargemodbus.h"
#include "ui_chargemodbus.h"

ChargeModbus::ChargeModbus(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChargeModbus)
{
    ui->setupUi(this);
    //_client=new QModbusTcpClient(this);
    changeDataInWidget=true;
    InitServer();
    InitButton();

}

ChargeModbus::~ChargeModbus()
{
    delete ui;
}
void ChargeModbus::InitServer()
{
    _server=new QModbusTcpServer(this);
    _reg.insert(QModbusDataUnit::Coils, { QModbusDataUnit::Coils, 0, quint16(10) });
    _reg.insert(QModbusDataUnit::DiscreteInputs, { QModbusDataUnit::DiscreteInputs, 0, quint16(10) });
    _reg.insert(QModbusDataUnit::InputRegisters, { QModbusDataUnit::InputRegisters, 0, quint16(300) });
    _reg.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, quint16(300) });

    QMapIterator<QModbusDataUnit::RegisterType, QModbusDataUnit> i(_reg);
    qDebug()<<"The modbus server map is:";
    while (i.hasNext()) {
        i.next();
        QVector<quint16> test;
        test=i.value().values();
        qDebug() <<i.key()<< ": " <<test << "Start address is: "<<i.value().startAddress();
    }
    qDebug()<<"The holding register map is:"<<_reg.value(QModbusDataUnit::HoldingRegisters).values();

    qDebug()<<"Setting _reg as data map result: "<<_server->setMap(_reg);

    //Debugging();

    //Signal and slot information exchange: dataWritten(DataUnit,address,size) has three inputs, so they can directly pass to RefreshRegister(DataUnit,address,size)
    connect(_server, &QModbusServer::dataWritten,
            this, &ChargeModbus::RefreshRegister);
    connect(_server, &QModbusServer::stateChanged,
            this, &ChargeModbus::onStateChanged);
    connect(_server, &QModbusServer::errorOccurred,
            this, &ChargeModbus::handleDeviceError);

    ui->Port->setText(QString("192.168.1.100:502"));
    ui->serverEdit->setMinimum(1);
    ui->serverEdit->setMaximum(247);
}

void ChargeModbus::Debugging()
{
    QModbusDataUnit testUnit(QModbusDataUnit::HoldingRegisters, 0, quint16(200));

    QModbusDataUnit testUnit2(QModbusDataUnit::InputRegisters, 0, quint16(200));
    QModbusDataUnit testUnit3(QModbusDataUnit::HoldingRegisters, -1, quint16(200));
    qDebug()<<"Before setting the data map, holding register map is:"<<testUnit.values();
    qDebug()<<"Before setting the data map, input register map is:"<<testUnit2.values();
    QVector<quint16> test;
    test.append(quint16(70));
    test.append(quint16(50));
    test.append(quint16(30));
    test.append(quint16(10));
    testUnit.setValues(test.mid(0,testUnit.valueCount()));
    qDebug()<<"Test is dataunit setting is functioning"<<testUnit.values();

    qDebug()<<"Set holding using setdata(QModbusdataunit) as 70,50,30,10 status:"<<_server->setData(testUnit);
    _server->data(&testUnit3);
    qDebug()<<"After setting the holding register as 70,50,30,10, holding register map is:"<<testUnit3.values()<<"start address is:"<<testUnit3.startAddress()<<"has:"<<testUnit3.valueCount();

    testUnit3.setStartAddress(-1);
    qDebug()<<"Set holding using setdata(QModbusdataunit::registertype,address,size) as 80 status:"<<_server->setData(QModbusDataUnit::HoldingRegisters,160,quint16(80));
    _server->data(&testUnit3);
    qDebug()<<"After setting the holding register as 80 holding register map is:"<<testUnit3.values()<<"start address is:"<<testUnit3.startAddress()<<"has:"<<testUnit3.valueCount();

    qDebug()<<"Getting Holding register status:"<<_server->data(&testUnit);
    qDebug()<<"Getting Input register status:"<<_server->data(&testUnit2);
    qDebug()<<"Getting Holding register status(get all):"<<_server->data(&testUnit3);
    qDebug()<<"After setting the data map, holding register map is:"<<testUnit.values()<<"start address is:"<<testUnit.startAddress()<<"has:"<<testUnit.valueCount();
    qDebug()<<"After setting the data map, input register map is:"<<testUnit2.values()<<"start address is:"<<testUnit2.startAddress()<<"has:"<<testUnit2.valueCount();
    qDebug()<<"After setting the data map, holding register map is(get all):"<<testUnit3.values()<<"start address is:"<<testUnit3.startAddress();
}

void ChargeModbus::on_pushButton_clicked()
{
    //test data set
    QString value="ffff";
    bool ok;

    int test=1605;
    _server->setData(QModbusDataUnit::HoldingRegisters,quint16(test),value.toUShort(&ok,16));
    int test2=8;
    _server->setData(QModbusDataUnit::Coils,quint16(test2),true);

    //test setter and getter
    QHash<int,quint16> testData=GetInRegData();
    testData.insert(InputRegisterAddress[InputRegisterType::State],0xffff);
    testData.insert(140,0xffff);
    testData.insert(140,0xfffa);
    SetInRegData(testData);
    qDebug()<<testData;
    quint16 newValue;
    _server->data(QModbusDataUnit::InputRegisters,quint16(InputRegisterAddress[InputRegisterType::State]),&newValue);
    qDebug()<<"Setting Input register"<<InputRegisterAddress[InputRegisterType::State]<<"as"<<value<<". Result is"<<newValue;

    QModbusDataUnit testUnit(QModbusDataUnit::InputRegisters,200,10);
    _server->data(&testUnit);
    qDebug()<<"After the setting, Input registers map is:"<<testUnit.values();


}

void ChargeModbus::on_pushButton_2_clicked()
{
    bool ok=true;
    int id=HoldingRegisterAddress[HoldingRegisterType::InputPower];
    QString value="ffff";
    qDebug()<<"Setting holding register test result is:"<<_server->setData(QModbusDataUnit::HoldingRegisters,quint16(id),value.toUShort(&ok,16));
    qDebug()<<QModbusDataUnit::HoldingRegisters;
    quint16 newValue;
    qDebug()<<"Getting holding register test result is:"<<_server->data(QModbusDataUnit::HoldingRegisters,quint16(id),&newValue);
    ui->TestLineEdit2->setText(value.setNum(newValue,16));
    qDebug()<<"Setting Holding register"<<id<<"as"<<value<<". Result is"<<newValue;

    QModbusDataUnit testUnit(QModbusDataUnit::HoldingRegisters,0,10);
    qDebug()<<"Before the setting, holding registers map is:"<<testUnit.values();
    _server->data(&testUnit);
    qDebug()<<"After the setting, holding registers map is:"<<testUnit.values();

    QHash<int,quint16> testData;
    testData.insert(HoldingRegisterAddress.at(HoldingRegisterType::InputPower),0xfffa);
    testData.insert(161,0xfff1);
    qDebug()<<"Testing setHoldreg function:";
    SetHoldRegData(testData);

}

void ChargeModbus::InitButton()
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
    if(changeDataInWidget)connect(&coilButtons, SIGNAL(buttonClicked(int)), this, SLOT(coilChanged(int)));

    //Same, pack all DisInput checkbox
    regexp.setPattern(QStringLiteral("DisInput(?<ID>\\d+)"));
    const QList<QCheckBox *> discs = findChildren<QCheckBox *>(regexp);
    for (QCheckBox *cbx : discs)
        discreteButtons.addButton(cbx, regexp.match(cbx->objectName()).captured("ID").toInt());
    //set the server register if discret input is changed
    if(changeDataInWidget)connect(&discreteButtons, SIGNAL(buttonClicked(int)), this, SLOT(discreteInputChanged(int)));


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

        //Signal and slot information exchange: QLineEdit::textChanged(const QString &) has a qstring input, so it can directly pass to setRegister(const QString)
        if(changeDataInWidget)connect(lineEdit, &QLineEdit::textChanged, this, &ChargeModbus::setRegister);
    }

    //When the coil and diskret input is clicked, slots are triggered to set the _server register data
    //  When the text in lineEdit(InputRegister and HoldingRegister) is changed, slots are triggered.
    //  Also, when the coil and holding register is written by client, it will trigger RefreshRegister. Then, if the text is changed, it will trigger
    //  setRegisters to set the _server register data.

    //Set input register labels as address
    regexp.setPattern(QLatin1String("InputRegisterLabel(?<ID>\\d+)"));
    const QList<QLabel *> labelInputReg = findChildren<QLabel *>(regexp);
    for(QLabel *label : labelInputReg)
    {
        label->setProperty("ID", regexp.match(label->objectName()).captured("ID").toInt());
    }

    for(int i=0;i<InputRegisterAddress.size();i++)
    {
        QString value;
        for(QLabel *label : labelInputReg)
        {
            if(label->property("ID")==i){label->setText(value.number(InputRegisterAddress.at(i)));}
        }
    }

    //Set input register labels as address
    regexp.setPattern(QLatin1String("HoldingRegisterLabel(?<ID>\\d+)"));
    const QList<QLabel *> labelHoldReg = findChildren<QLabel *>(regexp);
    for(QLabel *label : labelHoldReg)
    {
        label->setProperty("ID", regexp.match(label->objectName()).captured("ID").toInt());
    }


    for(int i=0;i<HoldingRegisterAddress.size();i++)
    {
        QString value;
        for(QLabel *label : labelHoldReg)
        {
            if(label->property("ID")==i){label->setText(value.number(HoldingRegisterAddress.at(i)));}
        }
    }

}

void ChargeModbus::coilChanged(int id)
{
    if(!_server)return;
    if(!_server->setData(QModbusDataUnit::Coils,quint16(id),coilButtons.button(id)->isChecked()))
        statusBar()->showMessage("Failed to set Coil registers");
}
void ChargeModbus::discreteInputChanged(int id)
{
    if(!_server)return;
    if(!_server->setData(QModbusDataUnit::InputRegisters,quint16(id),discreteButtons.button(id)->isChecked()))
        statusBar()->showMessage("Failed to set discret Input registers");
}

void ChargeModbus::onStateChanged()
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

void ChargeModbus::handleDeviceError(QModbusDevice::Error newError)
{
    if (newError == QModbusDevice::NoError || !_server)
        return;

    statusBar()->showMessage(_server->errorString(), 5000);
}


//**************still need to add register check****************//
void ChargeModbus::SetInRegData(QHash<int,quint16> data)
{
    QMutexLocker locker(&_setInputDataMutex);
    if(!_server){return;}
    bool _dataExist=false;
    QList<int>::const_iterator i_list;
    for (i_list = InputRegisterAddress.begin(); i_list != InputRegisterAddress.end(); ++i_list)
    {
        if(!data.contains(*i_list)){continue;}
        readOnlyLock=true;
        QHash<int,quint16>::const_iterator i_data = data.find(*i_list);

        //protect in case the address has multiple match
//        while (i_data!= data.end() && i_data.key() == InputRegisterAddress[i]) {
//            qDebug() << i_data.value();
//            ++i_data;
//        }
        quint16 newData=i_data.value();
        _server->setData(QModbusDataUnit::InputRegisters,quint16(*i_list),newData);
        _dataExist=true;
    }
    if(!_dataExist)
    {
        QString hint="Failed to set any Input registers! No data match.";
        statusBar()->showMessage(hint);
        qDebug()<<hint;
    }
}

void ChargeModbus::SetHoldRegData(QHash<int,quint16> data)
{
    QMutexLocker locker(&_setHoldingDataMutex);
    if(!_server){return;}
    bool _dataExist=false;
    QList<int>::const_iterator i_list;
    for (i_list = HoldingRegisterAddress.begin(); i_list != HoldingRegisterAddress.end(); ++i_list)
    {
        if(!data.contains(*i_list)){continue;}
        readOnlyLock=true;
        QHash<int,quint16>::const_iterator i_data = data.find(*i_list);

        //protect in case the address has multiple match
//        while (i_data!= data.end() && i_data.key() == HoldingRegisterAddress[i]) {
//            qDebug() << i_data.value() << Qt::endl;
//            ++i;
//        }
        quint16 newData=i_data.value();
        _server->setData(QModbusDataUnit::HoldingRegisters,quint16(*i_list),newData);
        _dataExist=true;
    }
    if(!_dataExist)
    {
        QString hint="Failed to set any Holding registers! No data match.";
        statusBar()->showMessage(hint);
        qDebug()<<hint;
    }
}



void ChargeModbus::setRegister(const QString &value)
{
    QMutexLocker locker(&_setDataFromWidgetMutex);
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

void ChargeModbus::on_Connect_clicked()
{
    Connect();
}

void ChargeModbus::Connect()
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
    }
    else
    {
        _server->disconnectDevice();
    }
}

void ChargeModbus::Disconnect()
{
    if(_server->state()==QModbusDevice::ConnectedState){_server->disconnectDevice();}
    if(_server->state() == QModbusDevice::UnconnectedState){ui->Connect->setText("connect");}
}

QHash<int,quint16> ChargeModbus::GetInRegData()
{
    QHash<int,quint16> data;
    data.clear();
    if(!_server){return data;}

    quint16 value;
    int registerID;
    QList<int>::const_iterator i_list;
    for(i_list=InputRegisterAddress.begin();i_list!=InputRegisterAddress.end();++i_list)
    {
        registerID=*i_list;
        _server->data(QModbusDataUnit::InputRegisters,*i_list,&value);
        data.insert(registerID,value);
    }

    //Code to iterate through hash
//    QHash<int, quint16>::const_iterator i = data.constBegin();
//    while (i != data.constEnd()) {
//        qDebug() << i.key() << ": " << i.value() << Qt::endl;
//        ++i;
//    }
    return data;
}

QHash<int,quint16> ChargeModbus::GetHoldRegData()
{
    QHash<int,quint16> data;
    data.clear();
    if(!_server){return data;}

    quint16 value;
    int registerID;
    QList<int>::const_iterator i_list;
    for(i_list=HoldingRegisterAddress.begin();i_list!=HoldingRegisterAddress.end();++i_list)
    {
        registerID=*i_list;
        _server->data(QModbusDataUnit::HoldingRegisters,*i_list,&value);
        data.insert(registerID,value);
    }
    return data;
}


bool ChargeModbus::CheckInputHoldingValid(QModbusDataUnit::RegisterType table, int address)
{
    bool _valid=false;
    QString _registerType;
    switch(table)
    {
    default:break;
    case QModbusDataUnit::InputRegisters:
        _registerType="#Input Register#";
        _valid=InputRegisterAddress.contains(address)?true:false;
        break;
    case QModbusDataUnit::HoldingRegisters:
        _registerType="#Holding Register#";
        _valid=HoldingRegisterAddress.contains(address)?true:false;
        break;
    }
    if(!_valid)
    {
        QString hint="Cant find matching address " + QString("").setNum(address)+" of register type " + _registerType + " in current server";
        qDebug()<<hint;
        statusBar()->showMessage(hint);
        return _valid;
    }
    return _valid;
}

void ChargeModbus::RefreshRegister(QModbusDataUnit::RegisterType table, int address, int size)
{
    //Only the coils and HoldingRegisters can be both written and read. Input Registers and Discret input can only be read.
    //Therefore, only coils and holding Register is refreshed when receiving QModbusServer::dataWritten signal

    //in order to create API, changings of input register and discret input are allowed

    //if(_server->state()==QModbusDevice::UnconnectedState)return;

    for(int i=0;i<size;++i)
    {
        quint16 value;
        QString text;

        bool CheckValid;

        switch(table)
        {
        default:
            return;
        case QModbusDataUnit::InputRegisters:
            //Check if there is match in Holding address
            if(!CheckInputHoldingValid(table,address+i)){return;}
            break;
        case QModbusDataUnit::HoldingRegisters:
            if(!CheckInputHoldingValid(table,address+i)){return;}
            break;
        case QModbusDataUnit::Coils:
            //Check if there is match in coils
            QRegularExpression regexp(QStringLiteral("Coil(?<ID>\\d+)"));
            for (QAbstractButton *cbx : coilButtons.buttons())
            {
                CheckValid=regexp.match(cbx->objectName()).captured("ID").toInt()==address+i ? true : false;
                if(CheckValid){break;}
            }
            if(!CheckValid)
            {
                QString hint="Cant find matching address " + QString("").setNum(address+i)+" of register type #Coils# in current server";
                qDebug()<<hint;
                statusBar()->showMessage(hint);
                return;
            }
            break;
        }

        //pay attention to value. data() function will set &value as the current register data.
        QMutexLocker locker(&_setDataMutex);
        switch(table)
        {
        default:
            break;
        case QModbusDataUnit::Coils:
            _server->data(QModbusDataUnit::Coils,quint16(address+i),&value);
            coilButtons.button(address+i)->setChecked(value);
            break;
            break;
        case QModbusDataUnit::HoldingRegisters:
            _server->data(QModbusDataUnit::HoldingRegisters,quint16(address+i),&value);
            registers.value(QStringLiteral("HoldingRegister%1").arg(address + i))->setText(text.setNum(value, 16));
            break;
        case QModbusDataUnit::InputRegisters:
            if(readOnlyLock)
            {
                _server->data(QModbusDataUnit::InputRegisters,quint16(address+i),&value);
                registers.value(QStringLiteral("InputRegister%1").arg(address + i))->setText(text.setNum(value, 16));
            }
            readOnlyLock=false;
            break;
        }
    }
}




