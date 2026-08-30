#ifndef ANLCLASS_H
#define ANLCLASS_H

#include <iostream>
#include <QObject>
#include <QtCore>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>              /* for exit() */
#include <math.h>
#include <qutag_adq.h>
#include <iostream>//entradas y salidas por consola
#include <fstream>//archivos.txt
#include <QtConcurrent/QtConcurrent>
#include <QFuture>

typedef QVector<Int32> vectorInt32;
typedef QVector<int> vectorInt;

#define MAX_LOGIC 25

// Timestamp analysis pipeline: receives raw (timestamp, channel) tags from
// whichever acquisition device is running (qutagadq or timetaggerUltra) via
// timestampREC(), hands them off to timestampProcess on anlWorker1 for the
// actual logic-combination math (tab2's AND/OR trees over the 4 QKD
// histogram windows), and reports the resulting rates back up.
class qutaganl : public QObject
{
    Q_OBJECT


public:

   bool anlAvilable = false;
  explicit qutaganl();

  QThread anlWorker1;
    ~qutaganl();

    // tab2 logic-combination state, one entry per logic plot (MAX_LOGIC of
    // them). LSource/RSource select each side's input: 0..MAX_LOGIC-1 means
    // "output of logic plot N", while -1/-2/-3/-4 mean "QKD histogram
    // channel A/B/C/D" (LWin/RWin then pick which window of that channel).
   vectorInt RSource;
   vectorInt LSource;
   vectorInt LWin;
   vectorInt RWin;
    int numberOfLogicPlots=0;
    QVector<int> logicOP;

    double clkdiffT;
    ////first plot////
    int in_binsinplot, in_startChan=QUTAG_START_CHANNEL, in_histStart, in_binWidth;
    double in_adqtime;
    int in_PlotACh1, in_PlotACh2, in_PlotBCh1, in_PlotBCh2, in_PlotCCh1, in_PlotCCh2, in_PlotDCh1, in_PlotDCh2;

   //////tab 2 param/////////
    int xtime;
    float adqtime_tab2;

    // QKD windowing parameters, one entry per histogram channel (see
    // MainWindow::in_QKD_ph/iw/pxq/zero for the matching GUI-side state).
    vectorInt in_QKD_ph;
    vectorInt in_QKD_iw;
    vectorInt in_QKD_zero;
    vectorInt in_QKD_pxq;
    double in_QKD_time=200;
    int in_QKD_numb=30;

public slots:



  void timestampREC(const vectorInt64 &inconimg_vectorTimetags, const vectorInt &inconimg_vectorChannels, int inconimg_tsvalid);

  void Chang_in_startChan(int starchan){this->in_startChan=starchan;}
  void Chang_in_PlotAChn1(int val){this->in_PlotACh1=val;}
  void Chang_in_PlotAChn2(int val){this->in_PlotACh2=val;}
  void Chang_in_PlotBChn1(int val){this->in_PlotBCh1=val;}
  void Chang_in_PlotBChn2(int val){this->in_PlotBCh2=val;}
  void Chang_in_PlotCChn1(int val){this->in_PlotCCh1=val;}
  void Chang_in_PlotCChn2(int val){this->in_PlotCCh2=val;}

  void Chang_in_binsinplot(int val){this->in_binsinplot=val;}
  void Chang_in_histStart(int val){this->in_histStart=val;}
  void Chang_in_binWidth(int val){this->in_binWidth=val;}

  void Chang_adqtime_2(double val){this->adqtime_2=val;}

  void chang_QKD_time(double val){in_QKD_time=val;}
  void chang_QKD_numb(int val){in_QKD_numb=val;}

  // One parameterized slot per QKD parameter, replacing what used to be 4
  // near-identical overloads per parameter (chang_QKD_phA..D etc), each
  // just assigning into a different index of the same vector.
  void chang_QKD_ph(int channel, int val){in_QKD_ph[channel]=val;}
  void chang_QKD_pxq(int channel, int val){in_QKD_pxq[channel]=val;}
  void chang_QKD_iw(int channel, int val){in_QKD_iw[channel]=val;}
  void chang_QKD_zero(int channel, int val){in_QKD_zero[channel]=val;}

  void chang_LogicWinL(QString t, int i);
  void chang_LogicWinR(QString t, int i);
  void chang_LogicOP(QString t, int index);

    void Break(){break_= true;}

    void TScumulator(const vectorDouble &counter);
    void saveRawTSon(int a);

    void saveTTondisk(long clk, long tt);

signals:

    void histo1signal(const vectorDouble &TTdata);      //histogram 1 data ready
    void anlongoing(bool ong);                          //analysis program still working
    void Chang_anlAvilable(bool val);
    void CombinationChange(bool val);
    void rates_tab2(const vectorDouble &counters ,double);
    void timestampANL(const vectorInt64 &vectorTimetags, const vectorInt &vectorChannels, int tsvalid,
                                        int numberOfLogicPlots, int in_startChan,
                                        int in_QKD_numb, double in_QKD_time, double clkdiffT,
                                        const vectorInt &LSource, const vectorInt &RSource,
                                        const vectorInt &LWin,const vectorInt &RWin,
                                        const vectorInt &in_QKD_ph, const vectorInt &in_QKD_zero, const vectorInt &in_QKD_iw,
                                        const vectorInt &logicOP, bool saveTSon);
private:
 QFile *rawTT;
 QTextStream *outTSstream;
 bool break_=false;
  QVector<double> histo1data;
  std::ofstream file;
  bool saveTSon =false;
  int ChannelIndex=0, StopIndex=0;
  QVector<int> counterplot;
  QVector<int> flagaux;
  QVector<QVector<int>> flag;
  double adqtime_2=0;
  double previouskey;
  double key;
  bool anlbusy =false;
  vectorDouble outputCounter;
  vectorDouble cumulative;
  int aux1=0, aux2=0;

  vectorInt64 vectorTimetags;
  vectorInt vectorChannels;
  int tsvalid;
};

// Runs on qutaganl::anlWorker1: the actual per-timestamp logic-combination
// math, moved off the acquisition thread so it doesn't stall data capture.
class timestampProcess : public QObject
{
    Q_OBJECT
public:
    timestampProcess();
    ~timestampProcess();

public slots:
    void timestampANL(const vectorInt64 &vectorTimetags, const vectorInt &vectorChannels, int tsvalid,
                                        int numberOfLogicPlots, int in_startChan,
                                        int in_QKD_numb, double in_QKD_time, double clkdiffT,
                                        const vectorInt &LSource, const vectorInt &RSource,
                                        const vectorInt &LWin,const vectorInt &RWin,
                                        const vectorInt &in_QKD_ph, const vectorInt &in_QKD_zero, const vectorInt &in_QKD_iw,
                                        const vectorInt &logicOP, bool saveTSon);

signals:
    void TScumulator_fromThread(const vectorDouble &counter);
    void saveTTondisk_(long clk, long tt);
};

#endif
