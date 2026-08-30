#include "ovdl.h"

OVDL::OVDL(QObject *parent) : QObject(parent)
{

}

void OVDL::setDelay(float timeps){
    std::cout<<"writing to vdl: "<<timeps<<std::endl;
    QString scommand = "_ABS_";
    scommand+=QString::number(float(timeps))+"$";
    delaylineport->write(scommand.toUtf8());
    QByteArray answ = delaylineport->readAll();
    QString ans_out = QString::fromUtf8(answ);
    std::cout<<ans_out.toStdString()<<std::endl;

}

void OVDL::ovdlconnect(QString s){

    delaylineport = new QSerialPort(s);
    delaylineport->setBaudRate(QSerialPort::Baud9600);
    delaylineport->setDataBits(QSerialPort::Data8);
    delaylineport->setStopBits(QSerialPort::OneStop);
    delaylineport->setParity(QSerialPort::NoParity);
    delaylineport->setFlowControl(QSerialPort::NoFlowControl);

    if (delaylineport->open(QSerialPort::ReadWrite)) {
        std::cout<< "SERIAL open  "<< OVDL_PORT <<std::endl;
    }
    else std::cout<< "FAIL to open  "<< OVDL_PORT <<std::endl;
}
