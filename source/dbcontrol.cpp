#include "dbcontrol.h"
#include <iostream>
#include <time.h>

DBControl::DBControl(){
    QString val;
    QFile file("databaseInfo.json");

    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        val = file.readAll();
        file.close();

        QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
        QJsonObject sett2 = d.object();
        QJsonValue subobj1 = d["server"];
        server = subobj1.toString();
        QJsonValue subobj2 = d["port"];
        port = subobj2.toInt();
        QJsonValue subobj3 = d["DBName"];
        database = subobj3.toString();
        QJsonValue subobj4 = d["user"];
        user = subobj4.toString();
        QJsonValue subobj5 = d["pass"];
        passwd = subobj5.toString();
    }
    else qDebug()<<"unable to read database json file";

}

DBControl::~DBControl(){
    if(db.isOpen())db.close();
    std::cout<<"log off from DB"<<std::endl;
}

void DBControl::run(){
    qDebug()<<"runnung db thread";
    this->DBConnect(server, port, database, user, passwd);

}

void DBControl::DBConnect(QString server, int port, QString database, QString login, QString password){

    std::cout<<"server:"<<server.toStdString()<<"  port:"<<port<<" user:"<<std::endl;


    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setDatabaseName(database);
    db.setHostName(server);
    db.setUserName(login);
    db.setPassword(password);
    db.setPort(port);

    connection_succesfull = db.open();
    qDebug() << db.lastError();
    if(connection_succesfull){
        qDebug()<<"connection DB success";
        createSchema();
    }
    else qDebug()<<"database connection failed ";

}

void DBControl::createSchema(){
    QSqlQuery query(db);

    query.exec(
        "create table if not exists runs ("
        "run_id int not null auto_increment primary key,"
        "started_at datetime not null"
        ") engine=InnoDB");

    // "channel" is 0..NUM_QKD_CHANNELS-1 (A/B/C/D), "window_index" is the
    // peak/window within that channel -- replaces the old wide table's
    // "A0","A1",...,"B0",... columns.
    query.exec(
        "create table if not exists tab1_readings ("
        "id bigint not null auto_increment primary key,"
        "run_id int not null,"
        "channel tinyint not null,"
        "window_index int not null,"
        "counts int not null,"
        "hist_adqtime float not null,"
        "recorded_at datetime not null,"
        "key run_channel_idx (run_id, channel, window_index),"
        "foreign key (run_id) references runs(run_id)"
        ") engine=InnoDB");

    // "logic_index" is the tab2 logic-combination plot index -- replaces
    // the old wide table's "ch<N>" columns.
    query.exec(
        "create table if not exists tab2_readings ("
        "id bigint not null auto_increment primary key,"
        "run_id int not null,"
        "logic_index int not null,"
        "value double not null,"
        "and_adqtime float not null,"
        "delayline double not null,"
        "recorded_at datetime not null,"
        "key run_logic_idx (run_id, logic_index),"
        "foreign key (run_id) references runs(run_id)"
        ") engine=InnoDB");

    query.exec(
        "create table if not exists tab2_filters ("
        "id bigint not null auto_increment primary key,"
        "run_id int not null,"
        "filter_index int not null,"
        "wavelength_nm double not null,"
        "bandwidth_pm int not null,"
        "recorded_at datetime not null,"
        "key run_id_idx (run_id),"
        "foreign key (run_id) references runs(run_id)"
        ") engine=InnoDB");
}

void DBControl::SaveTab2Values(vectorDouble datatab2, float andTime, double delayline){
    if(!connection_succesfull || current_run_id<0) return;

    QDateTime now = QDateTime::currentDateTime();

    QSqlQuery query(db);
    query.prepare(
        "insert into tab2_readings (run_id, logic_index, value, and_adqtime, delayline, recorded_at) "
        "values (:run_id, :logic_index, :value, :and_adqtime, :delayline, :recorded_at)");
    for (int i=0;i<_channels.size();i++) {
        query.bindValue(":run_id", current_run_id);
        query.bindValue(":logic_index", _channels[i]);
        query.bindValue(":value", datatab2[_channels[i]]);
        query.bindValue(":and_adqtime", andTime);
        query.bindValue(":delayline", delayline);
        query.bindValue(":recorded_at", now);
        if(!query.exec()) std::cout<<"tab2_readings insert failed: "<<query.lastError().text().toStdString()<<std::endl;
    }

    if(Noffilters>0){
        QSqlQuery filterQuery(db);
        filterQuery.prepare(
            "insert into tab2_filters (run_id, filter_index, wavelength_nm, bandwidth_pm, recorded_at) "
            "values (:run_id, :filter_index, :wavelength_nm, :bandwidth_pm, :recorded_at)");
        for(int i=0;i<Noffilters;i++){
            filterQuery.bindValue(":run_id", current_run_id);
            filterQuery.bindValue(":filter_index", i);
            filterQuery.bindValue(":wavelength_nm", filtersWLcurrentValue[i]);
            filterQuery.bindValue(":bandwidth_pm", filtersBWcurrentValue[i]);
            filterQuery.bindValue(":recorded_at", now);
            if(!filterQuery.exec()) std::cout<<"tab2_filters insert failed: "<<filterQuery.lastError().text().toStdString()<<std::endl;
        }
    }
}

void DBControl::SaveTab1Values(QVector<int> PlotA, QVector<int> PlotB, QVector<int> PlotC , QVector<int> PlotD , float hist_adqtime){
    if(!connection_succesfull || current_run_id<0) return;

    QDateTime now = QDateTime::currentDateTime();
    const QVector<int> *channels[4] = {&PlotA, &PlotB, &PlotC, &PlotD};

    QSqlQuery query(db);
    query.prepare(
        "insert into tab1_readings (run_id, channel, window_index, counts, hist_adqtime, recorded_at) "
        "values (:run_id, :channel, :window_index, :counts, :hist_adqtime, :recorded_at)");
    for (int ch=0; ch<4; ch++) {
        for (int i=0;i<channels[ch]->size();i++) {
            query.bindValue(":run_id", current_run_id);
            query.bindValue(":channel", ch);
            query.bindValue(":window_index", i);
            query.bindValue(":counts", channels[ch]->at(i));
            query.bindValue(":hist_adqtime", hist_adqtime);
            query.bindValue(":recorded_at", now);
            if(!query.exec()) std::cout<<"tab1_readings insert failed: "<<query.lastError().text().toStdString()<<std::endl;
        }
    }
}

void DBControl::CreateTableTab2(QVector<int> channels, QVector<int> logicL,QVector<int> logicR,QVector<int> WinL,QVector<int> WinR, QVector<bool> gate, int filters,  QLabel *lab){
   _channels = channels;
    Noffilters = filters;
    DB_numberOfLogicPlots = channels.size();

    // current_run_id was set by CreateTableTab1, which MainWindow always
    // emits (and this class always processes) first for a given run.
    lab->setText(QString("Run #%1").arg(current_run_id));

    QDateTime date = QDateTime::currentDateTime();
    QString formattedTime = date.toString("dd_MM_yyyy_hh_mm_ss");
    QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();

    QString filename = "databaseLOG_logic.txt";
    QFile dblog(filename);

    if(dblog.open(QIODevice::WriteOnly| QIODevice::Text | QIODevice::Append)){
        QTextStream out(&dblog);
        out<<formattedTimeMsg <<"\t Tab 2 \t";
        for (int i=0;i<DB_numberOfLogicPlots;i++) {
            out<<"channel"+QString::number(channels[i])<<"/Left_";
            if(logicL[i]==-1)out<<"PlotA/WinL"<<QString::number(WinL[i]);
            if(logicL[i]==-2)out<<"PlotB/WinL"<<QString::number(WinL[i]);
            if(logicL[i]==-3)out<<"PlotC/WinL"<<QString::number(WinL[i]);
            if(logicL[i]==-4)out<<"PlotD/WinL"<<QString::number(WinL[i]);
            if(logicL[i]>-1)out<<QString::number(logicL[i]);
            if(gate[i])out<<"/AND/Right_";
            if(!gate[i]) out<<"/OR/Right_";
            if(logicR[i]==-1)out<<"PlotA/WinR"<<QString::number(WinR[i]);
            if(logicR[i]==-2)out<<"PlotB/WinR"<<QString::number(WinR[i]);
            if(logicR[i]==-3)out<<"PlotC/WinR"<<QString::number(WinR[i]);
            if(logicR[i]==-4)out<<"PlotD/WinR"<<QString::number(WinR[i]);
            if(logicR[i]>-1)out<<QString::number(logicR[i]);
            out<<"\t";
        }
        out<<"\n";
        dblog.close();
    }
}


void DBControl::CreateTableTab1(int PlotA, int PlotB, int PlotC , int PlotD , QLabel *lab){
    // PlotA/B/C/D no longer size a per-run table -- tab1_readings is a
    // fixed schema and each SaveTab1Values call records exactly as many
    // rows as it's given. This slot's only remaining job is starting the
    // new run (it's always the first of CreateTableTab1/CreateTableTab2 to
    // run for a given run-start).
    Q_UNUSED(PlotA); Q_UNUSED(PlotB); Q_UNUSED(PlotC); Q_UNUSED(PlotD);

    QSqlQuery query(db);
    query.prepare("insert into runs (started_at) values (:started_at)");
    query.bindValue(":started_at", QDateTime::currentDateTime());
    if(!query.exec()){
        std::cout<<"failed to start new run: "<<query.lastError().text().toStdString()<<std::endl;
        current_run_id = -1;
        return;
    }
    current_run_id = query.lastInsertId().toInt();
    lab->setText(QString("Run #%1").arg(current_run_id));
}

void DBControl::disconnectFromServer()
{
    db.close();
    // Being connected and having an active run are two different things --
    // dropping the connection ends whatever run was in progress too, so a
    // later reconnect can't silently keep tagging new readings with a
    // run_id that started before this disconnect.
    connection_succesfull = false;
    current_run_id = -1;
}
