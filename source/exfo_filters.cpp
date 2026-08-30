#include "exfo_filters.h"
#include <iostream>

EXFO_Filters::EXFO_Filters()
{
    readlambda = new QRegularExpression("^LAMBDA=(?<value>([0-9]*[.])?[0-9]+)\r\n$");
    readfwhm = new QRegularExpression("^FWHM=(?<value>([0-9]*[.])?[0-9]+)\r\n$");


    QString val;
    QFile file("exfofilters.json");

    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        val = file.readAll();
        file.close();


        QJsonParseError jsonError;
        QJsonDocument jd = QJsonDocument::fromJson( val.toUtf8(), &jsonError );
        if( jsonError.error != QJsonParseError::NoError )
        {
            std::cout << "fromJson failed: " << jsonError.errorString().toStdString() << std::endl;
            return ;
        }
        if( jd.isObject() )
        {
            QJsonObject filterj = jd.object();
            QJsonValue filterjv[MAX_N_FILTERS];
            QJsonObject filterobj[MAX_N_FILTERS];
            QJsonValue  filteripv[MAX_N_FILTERS];
            QJsonValue  filterBWv[MAX_N_FILTERS];
            QJsonValue  filterWLv[MAX_N_FILTERS];
            QJsonValue  filterWLminv[MAX_N_FILTERS];
            QJsonValue  filterWLmaxv[MAX_N_FILTERS];
            QJsonValue  filterWLsdv[MAX_N_FILTERS];
            QJsonValue  filterWLssv[MAX_N_FILTERS];
            QJsonValue  filterBWminv[MAX_N_FILTERS];
            QJsonValue  filterBWmaxv[MAX_N_FILTERS];
            QJsonValue  filterBWsdv[MAX_N_FILTERS];
            QJsonValue  filterBWssv[MAX_N_FILTERS];


            for (int i = 0;  i < MAX_N_FILTERS; i++) {
                filterjv[i] = filterj.value("filter"+QString::number(i+1));
                filterobj[i] =  filterjv[i].toObject();

                filteripv[i] = filterobj[i].value("ip");
                filterips[i] = filteripv[i].toString();

                filterBWv[i] = filterobj[i].value("defaultBW");
                filterBWDef[i]= filterBWv[i].toDouble();

                filterWLv[i] = filterobj[i].value("defaultWL");
                filterWLDef[i]= filterWLv[i].toDouble();

                filterBWminv[i] = filterobj[i].value("defaultBWScanMin");
                filterBWScanMinDef[i] = filterBWminv[i].toInt();
                filterWLminv[i] = filterobj[i].value("defaultWLScanMin");
                filterWLScanMinDef[i] = filterWLminv[i].toDouble();

                filterBWmaxv[i] = filterobj[i].value("defaultBWScanMax");
                filterBWScanMaxDef[i] = filterBWmaxv[i].toInt();
                filterWLmaxv[i] = filterobj[i].value("defaultWLScanMax");
                filterWLScanMaxDef[i] = filterWLmaxv[i].toDouble();

                filterBWssv[i] = filterobj[i].value("defaultBWScanStepSize");
                filterBWScanStepSizeDef[i] = filterBWssv[i].toInt();
                filterWLssv[i] = filterobj[i].value("defaultWLScanStepSize");
                filterWLScanStepSizeDef[i] = filterWLssv[i].toDouble();
                filterBWsdv[i] = filterobj[i].value("defaultBWScanStepDur");
                filterBWScanStepDurDef[i] = filterBWsdv[i].toDouble();
                filterWLsdv[i] = filterobj[i].value("defaultWLScanStepDur");
                filterWLScanStepDurDef[i] = filterWLsdv[i].toDouble();
            }
        }
    }
    else qDebug()<<"unable to read database json file";


}

EXFO_Filters::~EXFO_Filters(){

    for(int i=0; i < NofFilters; i++){
        if(socket[i]->state() == QAbstractSocket::ConnectedState)socket[i]->disconnectFromHost();
    }
}

void EXFO_Filters::setBandwidth(int bw, int n){
    float fwhm = (float)bw/1000; // value comes in pm, command needs nm
    qDebug()<<fwhm;
    if(n<NofFilters){
        if(socket[n]->state() == QAbstractSocket::ConnectedState && bw>31 && bw<651){
            QString dataqs = "FWHM="+QString::number(fwhm)+"\r\n";
            qDebug()<<dataqs;
            socket[n]->write(dataqs.toUtf8());
        }
    }
}

void EXFO_Filters::setWavelength(double wavel, int n){
    qDebug()<<QString::number(wavel,'f',3);
    if(n<NofFilters){
        if(socket[n]->state() == QAbstractSocket::ConnectedState){
            QString dataqs = "LAMBDA=1"+QString::number(wavel,'f',3)+"\r\n";

            socket[n]->write(dataqs.toUtf8());
        }
    }
}

void EXFO_Filters::filterConnect(int n){

    NofFilters = n+1;

    qDebug()<<"ip :"<<filterips[n];

    if(socket[n] == NULL){
        qDebug() << "creating a new socket...."<<n;
        socket[n] = new QTcpSocket(this);
        connect(socket[n], &QTcpSocket::readyRead,[this, n]() { readTcpData(n); });
        connect(socket[n], &QTcpSocket::connected,[this,n](){ FilterConnected(n);});
        connect(socket[n], &QTcpSocket::disconnected, [this,n](){ FilterDisconnected(n);});
        connect(socket[n], &QTcpSocket::bytesWritten,[this,n](qint64 data){ FilterbytesWritten(data, n);});
    }

    socket[n]->connectToHost(filterips[n], 5025);
    if(!socket[n]->waitForConnected(10000))
    {
        qDebug() << "Error: " << socket[n]->errorString();
    }
}

void EXFO_Filters::setIP(QString ip, int n){
    filterips[n]=ip;
}

void EXFO_Filters::readTcpData(int n)
{
    QByteArray data = socket[n]->readAll();
    QString message = QString::fromUtf8(data);
    qDebug()<<message;

    QRegularExpressionMatch match1 = readlambda->match(message);
    if (match1.hasMatch()) {
        QString LAMBDA = match1.captured("value");
        if(!initdoneBW){
                emit DeviceWL(LAMBDA.toFloat(), n);
                initdoneWL=true;
        }
    }

    QRegularExpressionMatch match2 = readfwhm->match(message);
    if (match2.hasMatch()) {
        QString FWHM = match2.captured("value");
        if(!initdoneBW){
            emit DeviceBW((int)(FWHM.toFloat()*1000), n);
            initdoneBW=true;
        }
    }

}
void EXFO_Filters::FilterConnected(int n){
    QString dataqs = "*IDN?\r\n";
    socket[n]->write(dataqs.toUtf8()); //write the data itself
    dataqs = "LAMBDA?\r\n";
    socket[n]->write(dataqs.toUtf8()); //write the data itself
    dataqs = "FWHM?\r\n";
    socket[n]->write(dataqs.toUtf8()); //write the data itself
}

void EXFO_Filters::FilterDisconnected(int n){
    qDebug()<<"filter " <<n<< " disconected..";
}

void EXFO_Filters::FilterbytesWritten(qint64 data, int n){
    qDebug() << data << " bytes written...";
}
