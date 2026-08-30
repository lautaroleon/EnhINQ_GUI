/////////////////////////////////////////////
/// by Lautaro Narvaez
/// For quantum communication aplications
///
/// lautaro@caltech.edu
/// lautaroleon@gmail.com
///
/// third party code from:
/// QT
/// MYSQL
/// Qutools
/// Swabian Instruments
/// qcustomplot
/// /////////////////////////////////////////


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h"
#include <QtCore>
#include <ctime>
#include <stdio.h>
#include "qutag_adq.h"
#include "qutag_anl.h"
#include "dbcontrol.h"
#include "ovdl.h"
#include <math.h>
#include "gui_param.h"
#include "timetaggerultra.h"
#include "exfo_filters.h"

// Set to 1 to show the logo in tab1's top-left corner (ui->label_8), 0 to
// hide it. Compile-time only -- no runtime UI toggle.
#define SHOW_LOGO 0

namespace Ui {
class MainWindow;
}

// Top-level window and owner of every worker/device object: qutagadq (the
// qutools qutag) and timetaggerUltra (Swabian Time Tagger Ultra/X) each
// acquire timestamps and feed qutaganl, which does the tab2 logic-
// combination math; DBControl logs both tabs' results; OVDL and
// EXFO_Filters drive the delay line and tunable filters. Tab1's 4 QKD
// histogram channels (A/B/C/D) are addressed by index via
// NUM_QKD_CHANNELS/qkdLines/m_qkdPlot etc, not one member per channel --
// see the comment above NUM_QKD_CHANNELS below for why.
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:

  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  void closeEvent(QCloseEvent *event);

  void setupratePlot(QCustomPlot *customPlot);
  void setupratePlot_tab2(QCustomPlot *customPlot);

  void setupsignalslot();
  void setupsignalslot2();
  void setupHistoPlot(QCustomPlot *customPlot);
  void buildQKDChannelPanel(int channel);
  void QUTAG_setup_comboboxes();
  void qutag_paremetes_setup();
  void TTU_paremetes_setup();


private slots:
    void QUTAG_initdone();
    void TTUinitdone();

    void plotRates(int AoB, int event, double key);
    void plotRates_tab2(const vectorDouble &dat, double key);
    void changeStartchan(int starchan){this->in_startChan=starchan;}

    void histoplot(const vectorDouble &dat1, const vectorDouble &dat2, const vectorDouble &dat3, const vectorDouble &dat4, int count1,int count2 ,int count3 ,int count4 );

    void Chang_in_binsinplot(int val);
    void Chang_in_histStart(int val);
    void Chang_in_binWidth(int val);

    void Chang_in_adqtime(double val){this->in_adqtime=val;}


  void CombinationChange(bool val){CombiChang =val;}

  void Chang_adqtime_2(double val){in_adqtime_2=val;}

  void turnONDB(int val);
  void slideDBoff();

  void createTablesDB();
  void SaveState(bool a);
  void LoadState(bool a);
  void SaveSeason(bool a);
  bool LoadPrevoiusSeason(bool a);

  void tab2_plot1_activate(bool val){in_tab2_plot1=val;}
  void tab2_plot2_activate(bool val){in_tab2_plot2=val;}
  void tab2_plot3_activate(bool val){in_tab2_plot3=val;}
  void tab2_plot4_activate(bool val){in_tab2_plot4=val;}
  void tab2_plot5_activate(bool val){in_tab2_plot5=val;}
  void tab2_plot6_activate(bool val){in_tab2_plot6=val;}

  void loadLogicS();
  void saveLogicS();
  void clean_tab2();

  void Chang_delayline(int val){in_delayline=val;}

  void Chang_homscan_time(double val){in_homscan_time=val;}
  void Chang_homscan(int val);

  void chang_tab2range(int val){if(val<MAX_T2LOGIC_PLOT)xrange=val;}


   void resetdelay(){in_delayline=0;prev_homscan=0;}
   void chang_in_max_del(double val){in_Max_delay=val;}

   void chang_in_stepduration(int val){in_stepduration=val;}

   void Chang_in_thch1(double val){this->in_thch1=val;}
   void Chang_in_thch2(double val){this->in_thch2=val;}
   void Chang_in_thch3(double val){this->in_thch3=val;}
   void Chang_in_thch4(double val){this->in_thch4=val;}
   void Chang_in_cw(int val){this->in_cw=val; }

   void setup_log_plot(QCustomPlot *histo);
   void Chang_log1(int val);
   void Chang_log2(int val);
   void Chang_log3(int val);
   void Chang_log4(int val);

   void error1(QString text);

    void chang_QKD_time(double val);
    void chang_QKD_numb(int val);

   // Per-channel QKD parameter slots. One channel (0..NUM_QKD_CHANNELS-1) per
   // A/B/C/D histogram; replaces what used to be 4 near-identical overloads
   // per parameter (chang_QKD_phA..D, chang_QKD_pxqA..D, etc).
   void chang_QKD_ph(int channel, int val);
   void chang_QKD_pxq(int channel, int val);
   void chang_QKD_iw(int channel, int val);
   void chang_QKD_zero(int channel, int val);

   void hidelinesQ(int channel, int qubits);
   void hidelinesW(int channel, int windows);

   void createQKDLines(int channel);

   void setup_histolines_QKD();

   void AddLogicSelectorElement();
   void AddLogicSelectorElements();
   void AddLogicSelectorWindowsL(QString t, int index);
   void AddLogicSelectorWindowsR(QString t, int index);

   void updateTab2Windows(QString t, int index);

   void tracktab2_change(bool c, int i);

   void chang_LogicWinL(QString t, int i);
   void chang_LogicWinR(QString t, int i);
   void chang_LogicOP(QString s, int i);

   void chang_VDL_start(double v){in_VDL_start=v;}

   // Replaces Chang_trackA..D(bool) -- one radio-button-toggled handler
   // parameterized by channel index instead of 4 copies.
   void Chang_track(int channel, bool val);

   void runQutag(bool a);

   void TTURes_Std(bool a);
   void TTURes_A(bool a);
   void TTURes_B(bool a);
   void TTURes_C(bool a);

   void TTXRes_Std(bool a);
   void TTXRes_B(bool a);


   void connectOVDLmw();

   void addfilterMW();

   void loadFilterWL(float val, int dev);

   void loadFilterBW(int val, int dev);

   void Chang_delayTTUMain(double delay, int channel){TTUdelays[channel]=delay;}
   void Chang_MW_TTUdeadtime(double deadtime, int channel){TTUdeadtimes[channel]=deadtime;}
   void Chang_MW_TTUThresh(double threshold, int channel){TTUThresh[channel]=threshold;}
   void Chang_MWTTU_rof(QString rof, int channel){rofMW[channel] = rof;}

   void BWfilterscanslot(int, int);
   void WLfilterscanslot(int, int);

   void WLscanstep(int);
   void BWscanstep(int);

   void t2showcurrent(int a);
   void t2showmin(int a);
   void t2showmax(int a);
   void t2showavr(int a);

   void programDBoff();

   void RemoveLogic();

   void loopfilterscanch(int a){loopfilterscanvar= a;}

private:

  Ui::MainWindow *ui;
  timetaggerUltra TTU1;
  qutagadq qutag;
  qutaganl anl;
  DBControl dbc;
  OVDL ovdl_1;
  QString demoName;
  QTimer dataTimer;
  GUI_param qkdparam;
  EXFO_Filters EXFOfilters;

  QCPItemTracer *itemDemoPhaseTracer;
  int currentDemoIndex;
  double prom;
  QButtonGroup *buttonGroup1 ;
  QButtonGroup *buttonGroup2 ;
  bool trackRateChang =false, CombiChang =false;
  double in_adqtime_2;
  QVector<int> datach1;
  QVector<int> datacali;
  bool dbrunning=false;

  double lastPointKey_tab1;
  double lastPointKey_tab2;

  ////first tab//////
  QMap<QString, int>windows;

  ///general Configs////
  int in_binsinplot, in_startChan, in_binWidth, in_histStart;
  double in_adqtime;

  /////first plot////
  int P_R[4][MAX_WIN]={{0}};
  bool P_T[4][MAX_WIN]={{0}};
  int Plot_Win_BoE[3][3][2]={{{0}}};
  double qkd_prevKey;
  bool initR=false;

  ////SECOND tab////



  int tab2_plot[6][2]={{0}};
  int tab2_win[4][2]={{0}};

  bool in_tab2_plot1=false;
  bool in_tab2_plot2=false;
  bool in_tab2_plot3=false;
  bool in_tab2_plot4=false;
  bool in_tab2_plot5=false;
  bool in_tab2_plot6=false;

  int in_delayline=0;

  bool in_homscan=false;
 double in_homscan_time=1;
 double prev_homscan=0;
 int xrange = 120;
 double in_Max_delay=500;
 bool firstscan=false;
 int in_stepduration;
 double del_key, del_previouskey;

 double in_thch1, in_thch2, in_thch3, in_thch4;
 int in_cw;
 bool RoF[5];
 int logar[4];

 bool cleanTab2 = false;

 QList<int> tab2data[MAX_LOGIC];

 //////lines///////////////

   // Number of QKD histogram channels (formerly the hardcoded A/B/C/D plots).
   // Adding a channel is now a matter of bumping this constant and wiring one
   // more {plot, trackFL, tsv-LCD} triple in the constructor -- see
   // m_qkdPlot/m_qkdTrackFL/m_qkdTsv below -- instead of touching every
   // A/B/C/D-suffixed member and function that used to exist here.
   static const int NUM_QKD_CHANNELS = 4;

   QCPItemStraightLine *qkdLines[NUM_QKD_CHANNELS][MAX_WIN][MAX_QUBITS*2];

   int    in_QKD_ph[NUM_QKD_CHANNELS]   = {50, 50, 50, 50};
   int    in_QKD_iw[NUM_QKD_CHANNELS]   = {45, 45, 45, 45};
   int    in_QKD_pxq[NUM_QKD_CHANNELS]  = {0, 0, 0, 0};
   int    in_QKD_zero[NUM_QKD_CHANNELS] = {1, 1, 1, 1};
   double in_QKD_time=200;
   int  in_QKD_numb=30;

   QLCDNumber   *qkdCounter[NUM_QKD_CHANNELS][MAX_WIN];
   QRadioButton *qkdTrack[NUM_QKD_CHANNELS][MAX_WIN];
   QButtonGroup  qkdTrackGroup[NUM_QKD_CHANNELS];

   // One {plot, trackFL, tsv-LCD} triple per channel, built by
   // buildQKDChannelPanel() -- these used to be ui->PlotA/B/C/D,
   // ui->trackFL_A/B/C/D and ui->plotAtsv/B/C/D, i.e. 4 Designer-built
   // panels. The rest of the class addresses a channel by index instead of
   // by A/B/C/D suffix.
   QCustomPlot  *m_qkdPlot[NUM_QKD_CHANNELS]    = {nullptr, nullptr, nullptr, nullptr};
   QFormLayout  *m_qkdTrackFL[NUM_QKD_CHANNELS] = {nullptr, nullptr, nullptr, nullptr};
   QLCDNumber   *m_qkdTsv[NUM_QKD_CHANNELS]     = {nullptr, nullptr, nullptr, nullptr};

   ////tab2: logic-combination plots, one entry per plot (MAX_LOGIC of them)////
   int numberOfLogicPlots=0;
   QRadioButton *LogicTrack[MAX_LOGIC];
   QComboBox *LogicL[MAX_LOGIC];
   bool leftexpanded[MAX_LOGIC] = {false};
   QComboBox *LogicR[MAX_LOGIC];
   QComboBox *LogicG[MAX_LOGIC];
   QHBoxLayout *LogicBox[MAX_LOGIC];
   QButtonGroup tab2buttongroup;
   QComboBox *LogicWinR[MAX_LOGIC]={nullptr};
   QComboBox *LogicWinL[MAX_LOGIC]={nullptr};
   QLCDNumber *LogicCurrent[MAX_LOGIC];
   QLCDNumber *LogicMin[MAX_LOGIC];
   QLCDNumber *LogicMax[MAX_LOGIC];
   QLCDNumber *LogicAvr[MAX_LOGIC];

   QStringList plotslist, logicrelations;
   QStringList qkdWindowList[NUM_QKD_CHANNELS];

   bool trackTab2[MAX_LOGIC]={false};

   QCPGraph *graphtab2[MAX_LOGIC];

   int LSource[MAX_LOGIC]={0};
   int RSource[MAX_LOGIC]={0};
   int LWin[MAX_LOGIC]={0};
   int RWin[MAX_LOGIC]={0};
   int logicOP[MAX_LOGIC]={0};
   double in_VDL_start = 0;

   ////per-hardware-channel config widgets (conf tab), built in code by
   ////qutag_paremetes_setup()/TTU_paremetes_setup() -- one column per
   ////device-reported channel (NQUTAGCHANNELS/NTTUCHANNELS), independent of
   ////the tab1 QKD channel count above////
   QDoubleSpinBox *thch[NQUTAGCHANNELS];
   QDoubleSpinBox *delaych[NQUTAGCHANNELS];
   QCheckBox *qutagfilter[NQUTAGCHANNELS][NQUTAGCHANNELS];
   QComboBox *qutagFilterType[NQUTAGCHANNELS];
   QComboBox *qutagEdge[NQUTAGCHANNELS];

   QDoubleSpinBox *threshTTU[NTTUCHANNELS];
   double TTUThresh[NTTUCHANNELS];
   QDoubleSpinBox *delayTTU[NTTUCHANNELS];
   double TTUdelays[NTTUCHANNELS];
   QDoubleSpinBox *deadtimeTTU[NTTUCHANNELS];
   double TTUdeadtimes[NTTUCHANNELS];
   QCheckBox *TTUtriggerfilter[NTTUCHANNELS];
   QCheckBox *TTUfilter[NTTUCHANNELS];
   QComboBox *TTUEdge[NTTUCHANNELS];
   QString rofMW[NTTUCHANNELS];

   /////////////Filters Tab//////////////
   QScrollArea *filtersTabScroll;
   int numberOfFilters = 0;
   QDoubleSpinBox *filterWavel[MAX_N_FILTERS];
   QSpinBox *filterBandw[MAX_N_FILTERS];
   QPushButton *filterconnect[MAX_N_FILTERS];
   QLineEdit *filterip[MAX_N_FILTERS];
   QSlider *WLscanON[MAX_N_FILTERS];
   QDoubleSpinBox *WLscanMin[MAX_N_FILTERS];
   QDoubleSpinBox *WLscanMax[MAX_N_FILTERS];
   QDoubleSpinBox *WLscanstepsize[MAX_N_FILTERS];
   QDoubleSpinBox *WLscanstepduration[MAX_N_FILTERS];

   QSlider *BWscanON[MAX_N_FILTERS];
   QSpinBox *BWscanMin[MAX_N_FILTERS];
   QSpinBox *BWscanMax[MAX_N_FILTERS];
   QSpinBox *BWscanstepsize[MAX_N_FILTERS];
   QDoubleSpinBox *BWscanstepduration[MAX_N_FILTERS];

   QTimer *BWscantimer[MAX_N_FILTERS];
   QTimer *WLscantimer[MAX_N_FILTERS];

   QLabel* t2lableftp;
   QLabel *t2lableftw;
   QLabel *t2lablogic;
   QLabel *t2labrightp;
   QLabel *t2labrightw;
   QLabel *t2labcur;
   QLabel *t2labmin;
   QLabel *t2labmax;
   QLabel *t2labavr;

   int loopfilterscanvar= 0;



signals:

   void main_CreateTableTab1(int PlotA, int PlotB, int PlotC , int PlotD , QLabel *lab );
    void main_CreateTableTab2(QVector<int> channels, QVector<int> logicL,QVector<int> logicR,QVector<int> WinL,QVector<int> WinR, QVector<bool> gate, int filters, QLabel *lab2);
    void main_SaveTab2Values(vectorDouble datatab2, float andTime, double delayline);
    void main_SaveTab1Values(QVector<int> PlotA, QVector<int> PlotB, QVector<int> PlotC , QVector<int> PlotD, float hist_adqtime);
    void setOVDL(float timeps);
    void MWChang_qutag_filtertype(QString, int);
    void MWChang_qutag_filtermask(int, int, int);
    void MWChang_qutagThresh(double, int);
    void MWChang_qutag_edge(QString, int);
    void MWChang_qutag_delay(double, int);

    void MWChang_TTUThresh(double, int);
    void MWChang_TTU_edge(QString, int);
    void MWChang_TTU_delay(double, int);
    void MWChang_TTU_deadtime(double, int);

    void MWfilterConnect(int);
    void MWFilterWLChange(double, int);
    void MWFilterBWChange(int, int);
    void MWfilteripRet(QString, int);

    void WLscanONsignal(int, int);
    void BWscanONsignal(int, int);

};

#endif // MAINWINDOW_H
