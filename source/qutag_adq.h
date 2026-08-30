#ifndef ADQCLASS_H
#define ADQCLASS_H

#include <iostream>
#include <QObject>
#include <QtCore>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>              /* for exit() */
#include <math.h>
#include "typedefs.h"
#include "tdcstartstop.h"
#include <algorithm>

#define QUTAG_THRESHOLD_STEP 0.1
#define QUTAG_DELAY_STEP 100
#define QUTAG_START_CHANNEL 5

#define QUTAG_TIMESTAMP_COUNT   150000  /* Timestamp buffer size */

#define NQUTAGCHANNELS 5

// Worker thread driving the qutools qutag TDC over its vendor SDK
// (tdcstartstop.h/tdcbase.h): acquires timestamps, builds the tab1
// histograms (4 QKD channels' worth of data, computed here but reported
// through MainWindow's channel arrays), and reports per-channel filter/edge/
// delay settings back to the hardware.
class qutagadq : public QThread
{
    Q_OBJECT


public:

    void run();

    explicit qutagadq();
    ~qutagadq();
    void loadtdcfiltertype();
    bool anlAvilable =false;

public slots:

  void setHistograms();
  void setHistogramsParam();

  void adqpausechange(bool chang){adqpause_=chang;}
  void Break(){break_= true;}

  void updatefiltertype(int ch);
  void setfilter(int ch);

  void Chang_in_binsinplot(int val){this->in_binsinplot=val;paramschange=true;}
  void Chang_in_binWidth(int val){this->in_binWidth=val;paramschange=true;}
  void Chang_in_histStart(int val){in_histStart=val;}
  void Chang_in_adqtime(double val){this->in_adqtime=val;}
  void Chang_anlAvilable(bool val){this->anlAvilable =val;}



  void Chang_in_thch(double val, int ch){thresholds[ch]=val;changThreshold(ch);}

  void Chang_in_cw(int val){this->in_cw=val; TDC_setCoincidenceWindow(val);}



  void Chang_rof(QString text, int ch){if(text=="Rise")RoF[ch]=1;else RoF[ch]=0;changThreshold(ch);}


  void Chang_delay(double val, int ch){delays[ch]=int(val);set_delays();}



  void TSanl(int val){this->in_TSON=val;}
  void changTSper(int val){this->TSpercentage=val;}

  void Chang_qutag_filtertype(QString text, int row){filtertypeSTR[row]=text; updatefiltertype(row);}
  void Chang_qutag_filtermask(int status, int row, int column){if(status)ch_filtermask[row]|=0x1<<column;else ch_filtermask[row]&=~(0x1<<column);setfilter(0); }

signals:
    void dataready(const vectorInt64 &vectorTimetags, const vectorInt &vectorChannels, int tsvalid);
    void qutaghist(const vectorDouble &TTdata1, const vectorDouble &TTdata2, const vectorDouble &TTdata3, const vectorDouble &TTdata4, int count1, int count2, int count3, int count4);
    void TDCerror(QString text);
    void initdone();
private:
    void set_delays();
    QVector<double> dataA;
    QVector<double> dataB;
    QVector<double> dataC;
    QVector<double> dataD;
    bool paramschange = false;
    bool adqpause_;
    bool break_;
    void checkRc( const char * fctname, int rc );
    void getHisto();
    float rate(int ch_rate);
    int get_max_collection_time( float rate );
    int firstChanHist, secondChanHist;
    int ActHist[5][5]={{0}};

    QVector<int64_t> timetags;
    QVector<int> channelsTDC;

    bool _stop;
    int ret;
    Int32 rc, count, tooSmall, tooBig, tsValid, eventsA, eventsB, eventsC, eventsD, i, j, it, ch;
    Int64 expTime, lastTimestamp = 0;
    Int8  channels[QUTAG_TIMESTAMP_COUNT];
    int   coincCnt[TDC_COINC_CHANNELS];
    double timeBase = 0.;
    float TOTAL_RATE;

    void lautrun();
    void getTimeStamps();
    void changThreshold(int);
    int HIST_BINWIDTH;
    int HIST_BINCOUNT;
    int HIST_BINWIDTH_out, HIST_BINCOUNT_out;
    Int32 hist1[5000], hist2[5000];

    Int32 *histodataA;
    Int32 *histodataB;
    Int32 *histodataC;
    Int32 *histodataD;
    Int64 timestamps[QUTAG_TIMESTAMP_COUNT];


    /////////////////tab 1 variables///////////////

    double in_adqtime;

   int in_TSON=0;


public:
   int in_binsinplot, in_histStart, in_binWidth;
   int TSpercentage=10;
   clock_t begin, end;
   double cpu_time_used;
   bool initdone_bool = 0;
   double thresholds[NQUTAGCHANNELS];
   int in_cw;
   int RoF[NQUTAGCHANNELS];
   int ch_filtermask[NQUTAGCHANNELS] = {0};
   TDC_FilterType filtertype[NQUTAGCHANNELS];
   QString filtertypeSTR[NQUTAGCHANNELS] = {"NONE", "NONE", "NONE", "NONE", "NONE"};
   int delays[8] = {0};

};

#endif // ADQCLASS_H
