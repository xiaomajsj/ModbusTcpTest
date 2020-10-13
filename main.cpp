#include "chargemodbus.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChargeModbus w;
    w.show();
    return a.exec();
}
