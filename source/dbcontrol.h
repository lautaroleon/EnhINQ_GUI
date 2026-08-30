#ifndef DBCONTROL_H
#define DBCONTROL_H

#include <QString>
#include <QtSql>
#include <QObject>
#include <QtCore>
#include <iostream>//entradas y salidas por consola
#include <fstream>//archivos.txt
#include <qlabel.h>
#include "typedefs.h"

// Runs the MySQL connection on its own thread (run()) and logs both tabs'
// acquisition data. Server credentials are loaded from databaseInfo.json.
//
// Schema: a fixed set of tables (created once, in createSchema()) rather
// than one CREATE TABLE per run -- runs holds one row per acquisition run
// (started_at), and tab1_readings/tab2_readings/tab2_filters store one row
// per (run_id, channel-or-logic-plot-index) reading in "long" format
// instead of one wide table with a column per channel/window. This avoids
// both the query/join pain of scattering data across hundreds of
// per-run-named tables, and the schema churn of needing a fixed channel/
// window count baked into column names -- run_id is a plain indexed
// ("MUL") foreign key, not a schema-defining part of a table name.
//
// NOTE: CreateTableTab1's 4 fixed int arguments are still coupled to
// MainWindow's NUM_QKD_CHANNELS==4 (that signal's arity), but no longer to
// the DB schema itself -- see MainWindow for the fuller picture on why the
// QKD channel count is a compile-time constant.
class DBControl : public QThread
{
    Q_OBJECT

public:
    void run();
    explicit DBControl();
    ~DBControl();
    void disconnectFromServer();
    void DBConnect(QString server, int port, QString database, QString login, QString password);

    bool connection_succesfull;
private:
    QSqlDatabase db;

    QString server, database, user, passwd;
    int port;

    // Creates runs/tab1_readings/tab2_readings/tab2_filters if they don't
    // already exist. Called once, right after a successful DBConnect().
    void createSchema();

    // Set by CreateTableTab1 when a new run starts (that signal always
    // fires before CreateTableTab2 -- both are emitted back-to-back from
    // MainWindow::createTablesDB(), and Qt queues same-thread-origin
    // signals in emission order), then used by CreateTableTab2 and every
    // subsequent SaveTab1Values/SaveTab2Values call as the run_id to
    // attach readings to.
    int current_run_id = -1;

    int DB_numberOfLogicPlots = 0;
    int Noffilters = 0;
    double filtersWLcurrentValue[MAX_N_FILTERS];
    int filtersBWcurrentValue[MAX_N_FILTERS];
    QVector<int> _channels;

public slots:
    void SaveTab2Values(vectorDouble datatab2, float andTime, double delayline);
    void SaveTab1Values(QVector<int> PlotA, QVector<int> PlotB, QVector<int> PlotC , QVector<int> PlotD , float hist_adqtime);
    void CreateTableTab2(QVector<int> channels, QVector<int> logicL,QVector<int> logicR,QVector<int> WinL,QVector<int> WinR, QVector<bool> gate,int filters, QLabel *lab);
    void CreateTableTab1(int PlotA, int PlotB, int PlotC , int PlotD, QLabel *lab );
    void setfiltersBW(int bw, int n){filtersBWcurrentValue[n]=bw;}
    void setfiltersWL(double wavel, int n){filtersWLcurrentValue[n]=wavel;}

};
#endif // DBCONTROL_H
