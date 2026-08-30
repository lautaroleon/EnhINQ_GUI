#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>
#include <iostream>//entradas y salidas por consola
#include <fstream>//archivos.txt
#include <vector>//min_element//max_exelement//HD
#include <QApplication>
#include <QtCore>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow){

  ui->setupUi(this);
  setGeometry(200, 200, 1500, 800);

  ui->label_8->setVisible(SHOW_LOGO);

  // The "Parameters" tab (per-channel thresholds/delays/filters/edges) only
  // gets populated once a device is initialized -- it starts empty. Rather
  // than leaving it as a normally-empty tab in the middle of the tab bar,
  // pop it out into its own floating window (same pattern as qkdparam's
  // Lines window), shown on demand from Edit > Device Parameters.
  int confTabIndex = ui->horizontalTabWidget->indexOf(ui->conf);
  if (confTabIndex != -1) ui->horizontalTabWidget->removeTab(confTabIndex);
  ui->conf->setParent(nullptr);
  ui->conf->setWindowFlags(Qt::Window);
  ui->conf->setWindowTitle(tr("Device Parameters"));
  ui->conf->resize(600, 500);

  // Build the 4 QKD channel panels (plot + counts/track form + total LCD)
  // that used to be 4 near-identical Designer widgets (plotAwid/B/C/D).
  // This is the one place that still knows there are 4 of them -- adding a
  // channel is bumping NUM_QKD_CHANNELS and this loop still runs once per
  // channel.
  for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) buildQKDChannelPanel(ch);

setupsignalslot();

setWindowTitle(QString("INQNET TDC"));

for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) setupHistoPlot(m_qkdPlot[ch]);

plotslist<<QString("")<<QString("PlotA")<<QString("PlotB")<< QString("PlotC")<< QString("PlotD");
logicrelations<<QString("")<<QString("OR")<<QString("AND")<<QString("+")<<QString("-")<<QString("*")<<QString("/");

setupratePlot(ui->PlotTrack);
setupratePlot_tab2(ui->PlotTab2);

setup_histolines_QKD();


if(LoadPrevoiusSeason(1)){
    std::cout<<"loading hard coded initial values"<<std::endl;

    ui->histStart->setValue(0);

    ui->adqtime->setValue(1);//update rate Adq time

    ui->adqtime_2->setValue(1);


    ui->Max_delayd->setValue(500);
    ui->homscan_timed->setValue(10);
    ui->stepduration->setValue(30);

}

lastPointKey_tab1 = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
lastPointKey_tab2 = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;


tab2buttongroup.setExclusive(0);

qkdparam.LoadPrevoiusSeason(1);

setupsignalslot2();

initR=true;
for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) createQKDLines(ch);


t2lableftp=new QLabel("LeftP");
t2lableftw=new QLabel("LeftW");
t2lablogic=new QLabel("logic");
t2labrightp=new QLabel("RightP");
t2labrightw=new QLabel("RightW");
t2labcur=new QLabel("Value");
t2labmin=new QLabel("Min");
t2labmax=new QLabel("Max");
t2labavr=new QLabel("Avr");

t2labcur->setVisible(false);
t2labmin->setVisible(false);
t2labmax->setVisible(false);
t2labavr->setVisible(false);

t2lableftp->setAlignment(Qt::AlignHCenter);
t2lableftw->setAlignment(Qt::AlignHCenter);
t2lablogic->setAlignment(Qt::AlignHCenter);
t2labrightp->setAlignment(Qt::AlignHCenter);
t2labrightw->setAlignment(Qt::AlignHCenter);
t2labcur->setAlignment(Qt::AlignHCenter);
t2labmin->setAlignment(Qt::AlignHCenter);
t2labmax->setAlignment(Qt::AlignHCenter);
t2labavr->setAlignment(Qt::AlignHCenter);

t2lableftp->setStyleSheet("color: rgb(238, 238, 236)");
t2lableftw->setStyleSheet("color: rgb(238, 238, 236)");
t2lablogic->setStyleSheet("color: rgb(238, 238, 236)");
t2labrightp->setStyleSheet("color: rgb(238, 238, 236)");
t2labrightw->setStyleSheet("color: rgb(238, 238, 236)");
t2labcur->setStyleSheet("color: rgb(238, 238, 236)");
t2labmin->setStyleSheet("color: rgb(238, 238, 236)");
t2labmax->setStyleSheet("color: rgb(238, 238, 236)");
t2labavr->setStyleSheet("color: rgb(238, 238, 236)");

ui->logicgrid->setHorizontalSpacing(5);

ui->logicgrid->addWidget(t2lableftp,0,1);
ui->logicgrid->addWidget(t2lableftw,0,2);
ui->logicgrid->addWidget(t2lablogic,0,3);
ui->logicgrid->addWidget(t2labrightp,0,4);
ui->logicgrid->addWidget(t2labrightw,0,5);
ui->logicgrid->addWidget(t2labcur,0,6);
ui->logicgrid->addWidget(t2labmin,0,7);
ui->logicgrid->addWidget(t2labmax,0,8);
ui->logicgrid->addWidget(t2labavr,0,9);

ui->dbcronometeroff->setDateTime(QDateTime::currentDateTime());

AddLogicSelectorElements();

}


//////////////////////////////////////////////////////////
///////////////////setups///////////////////////////
///////////////////////////////////////////////////////////

void MainWindow::QUTAG_initdone(){


    qutag_paremetes_setup();

    for (int i = 0;i<NQUTAGCHANNELS ;i++ ) {
        thch[i]->setValue(qutag.thresholds[i]);
        qutagFilterType[i]->setCurrentText(qutag.filtertypeSTR[i]);
        delaych[i]->setValue(double(qutag.delays[i]));
        if(qutag.RoF[i])qutagEdge[i]->setCurrentText("Rise");else qutagEdge[i]->setCurrentText("Fall");
    }
    anl.Chang_in_startChan(QUTAG_START_CHANNEL);

}

void MainWindow::TTUinitdone(){

    TTU_paremetes_setup();

    for (int i = 0;i<NTTUCHANNELS ;i++ ) {
        threshTTU[i]->setValue(TTUThresh[i]);
        delayTTU[i]->setValue(TTUdelays[i]);
        deadtimeTTU[i]->setValue(TTUdeadtimes[i]);
        TTUEdge[i]->setCurrentText(rofMW[i]);
    }
    anl.Chang_in_startChan(TTUSTARTCHANNEL);
}

void MainWindow::setup_histolines_QKD(){

    for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) {
        for (int i = 0 ; i<MAX_QUBITS*2 ; i++) {
            for (int j=0; j<MAX_WIN; j++) {
                qkdLines[ch][j][i] = new QCPItemStraightLine(m_qkdPlot[ch]);
                qkdLines[ch][j][i]->setPen(QPen(QColor::fromHsv( j*(360/MAX_WIN), 255, 255, 255  )));
                qkdLines[ch][j][i]->setVisible(0);
            }
        }
    }
}

void MainWindow::setupratePlot(QCustomPlot *scope){


scope->plotLayout()->clear();

QCPAxisRect *wideAxisRect = new QCPAxisRect(scope);


  wideAxisRect->setupFullAxesBox(true);
  wideAxisRect->axis(QCPAxis::atRight, 0)->setTickLabels(true);

  wideAxisRect->axis(QCPAxis::atRight, 0)->setTickLabelColor(Qt::white);
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setTickLabelColor(Qt::white);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setTickLabelColor(Qt::white);

  wideAxisRect->axis(QCPAxis::atRight, 0)->setBasePen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atTop, 0)->setBasePen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setBasePen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setBasePen(QPen(Qt::white, 1));

  wideAxisRect->axis(QCPAxis::atRight, 0)->setTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atTop, 0)->setTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setTickPen(QPen(Qt::white, 1));

  wideAxisRect->axis(QCPAxis::atRight, 0)->setSubTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atTop, 0)->setSubTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setSubTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setSubTickPen(QPen(Qt::white, 1));

  wideAxisRect->axis(QCPAxis::atLeft, 0)->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->grid()->setSubGridVisible(true);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->grid()->setSubGridVisible(true);
  wideAxisRect->axis(QCPAxis::atLeft, 0)->grid()->setZeroLinePen(Qt::NoPen);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->grid()->setZeroLinePen(Qt::NoPen);
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setUpperEnding(QCPLineEnding::esSpikeArrow);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setUpperEnding(QCPLineEnding::esSpikeArrow);


  wideAxisRect->axis(QCPAxis::atLeft, 0)->setLabelColor(Qt::white);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setLabelColor(Qt::white);

  wideAxisRect->setRangeZoom(Qt::Vertical);


 scope->plotLayout()->addElement(0, 0, wideAxisRect);

  QCPGraph *graph1 = scope->addGraph(wideAxisRect->axis(QCPAxis::atBottom), wideAxisRect->axis(QCPAxis::atLeft));
  graph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1), QBrush(Qt::white),4));
  graph1->setPen(QPen(QColor(200, 0, 0), 2));

  QCPGraph *graph2 = scope->addGraph(wideAxisRect->axis(QCPAxis::atBottom), wideAxisRect->axis(QCPAxis::atLeft));
  graph2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1), QBrush(Qt::red),4));
  graph2->setPen(QPen(QColor(0, 200, 0), 2));

  QCPGraph *graph3 = scope->addGraph(wideAxisRect->axis(QCPAxis::atBottom), wideAxisRect->axis(QCPAxis::atLeft));
  graph3->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1), QBrush(Qt::blue),4));
  graph3->setPen(QPen(QColor(200, 200, 0), 2));

  QCPGraph *graph4 = scope->addGraph(wideAxisRect->axis(QCPAxis::atBottom), wideAxisRect->axis(QCPAxis::atLeft));
  graph4->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1), QBrush(Qt::green),4));
  graph4->setPen(QPen(QColor(200, 200, 200), 2));



  QLinearGradient plotGradient;
  plotGradient.setStart(0, 0);
  plotGradient.setFinalStop(0, 350);
  plotGradient.setColorAt(0, QColor(80, 80, 80));
  plotGradient.setColorAt(1, QColor(50, 50, 50));
  scope->setBackground(plotGradient);
  QLinearGradient axisRectGradient;
  axisRectGradient.setStart(0, 0);
  axisRectGradient.setFinalStop(0, 350);
  axisRectGradient.setColorAt(0, QColor(80, 80, 80));
  axisRectGradient.setColorAt(1, QColor(30, 30, 30));
  scope->axisRect()->setBackground(axisRectGradient);



  QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
  timeTicker->setTimeFormat("%h:%m:%s");
  scope->xAxis->setTicker(timeTicker);
  scope->rescaleAxes();
  scope->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

void MainWindow::setupratePlot_tab2(QCustomPlot *scope){


scope->plotLayout()->clear();

QCPAxisRect *wideAxisRect = new QCPAxisRect(scope);


  wideAxisRect->setupFullAxesBox(true);
  wideAxisRect->axis(QCPAxis::atRight, 0)->setTickLabels(true);

  wideAxisRect->axis(QCPAxis::atRight, 0)->setTickLabelColor(Qt::white);
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setTickLabelColor(Qt::white);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setTickLabelColor(Qt::white);

  wideAxisRect->axis(QCPAxis::atRight, 0)->setBasePen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atTop, 0)->setBasePen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setBasePen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setBasePen(QPen(Qt::white, 1));

  wideAxisRect->axis(QCPAxis::atRight, 0)->setTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atTop, 0)->setTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setTickPen(QPen(Qt::white, 1));

  wideAxisRect->axis(QCPAxis::atRight, 0)->setSubTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atTop, 0)->setSubTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setSubTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setSubTickPen(QPen(Qt::white, 1));

  wideAxisRect->axis(QCPAxis::atLeft, 0)->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->grid()->setSubGridVisible(true);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->grid()->setSubGridVisible(true);
  wideAxisRect->axis(QCPAxis::atLeft, 0)->grid()->setZeroLinePen(Qt::NoPen);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->grid()->setZeroLinePen(Qt::NoPen);
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setUpperEnding(QCPLineEnding::esSpikeArrow);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setUpperEnding(QCPLineEnding::esSpikeArrow);


  wideAxisRect->axis(QCPAxis::atLeft, 0)->setLabelColor(Qt::white);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setLabelColor(Qt::white);

  wideAxisRect->setRangeZoom(Qt::Vertical);


 scope->plotLayout()->addElement(0, 0, wideAxisRect);

 for (int i=0;i<MAX_LOGIC;i++) {
     graphtab2[i] = new QCPGraph(wideAxisRect->axis(QCPAxis::atBottom), wideAxisRect->axis(QCPAxis::atLeft));
     graphtab2[i]->setPen(QPen(QColor::fromHsv( i*(360/MAX_LOGIC), 255, 255, 255  )));
 }



  QLinearGradient plotGradient;
  plotGradient.setStart(0, 0);
  plotGradient.setFinalStop(0, 350);
  plotGradient.setColorAt(0, QColor(80, 80, 80));
  plotGradient.setColorAt(1, QColor(50, 50, 50));
  scope->setBackground(plotGradient);
  QLinearGradient axisRectGradient;
  axisRectGradient.setStart(0, 0);
  axisRectGradient.setFinalStop(0, 350);
  axisRectGradient.setColorAt(0, QColor(80, 80, 80));
  axisRectGradient.setColorAt(1, QColor(30, 30, 30));
  scope->axisRect()->setBackground(axisRectGradient);

  scope->rescaleAxes();

  scope->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);


}

void MainWindow::setupHistoPlot(QCustomPlot *histograma){

  histograma->plotLayout()->clear();

  QCPAxisRect *wideAxisRect = new QCPAxisRect(histograma);


  wideAxisRect->setupFullAxesBox(true);
  wideAxisRect->axis(QCPAxis::atRight, 0)->setTickLabels(true);
  wideAxisRect->axis(QCPAxis::atTop, 0)->setTickLabels(true);

  wideAxisRect->axis(QCPAxis::atRight, 0)->setTickLabelColor(Qt::white);
  wideAxisRect->axis(QCPAxis::atTop, 0)->setTickLabelColor(Qt::white);
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setTickLabelColor(Qt::white);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setTickLabelColor(Qt::white);

  wideAxisRect->axis(QCPAxis::atRight, 0)->setBasePen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atTop, 0)->setBasePen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setBasePen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setBasePen(QPen(Qt::white, 1));

  wideAxisRect->axis(QCPAxis::atRight, 0)->setTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atTop, 0)->setTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setTickPen(QPen(Qt::white, 1));

  wideAxisRect->axis(QCPAxis::atRight, 0)->setSubTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atTop, 0)->setSubTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setSubTickPen(QPen(Qt::white, 1));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setSubTickPen(QPen(Qt::white, 1));

  wideAxisRect->axis(QCPAxis::atLeft, 0)->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  wideAxisRect->axis(QCPAxis::atBottom, 0)->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
  wideAxisRect->axis(QCPAxis::atLeft, 0)->grid()->setSubGridVisible(true);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->grid()->setSubGridVisible(true);
  wideAxisRect->axis(QCPAxis::atLeft, 0)->grid()->setZeroLinePen(Qt::NoPen);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->grid()->setZeroLinePen(Qt::NoPen);
  wideAxisRect->axis(QCPAxis::atLeft, 0)->setUpperEnding(QCPLineEnding::esSpikeArrow);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setUpperEnding(QCPLineEnding::esSpikeArrow);


  wideAxisRect->axis(QCPAxis::atLeft, 0)->setLabelColor(Qt::white);
  wideAxisRect->axis(QCPAxis::atBottom, 0)->setLabelColor(Qt::white);

  wideAxisRect->setRangeZoom(Qt::Horizontal);


  histograma->plotLayout()->addElement(0, 0, wideAxisRect);


  QCPGraph *graph1 = histograma->addGraph(wideAxisRect->axis(QCPAxis::atBottom), wideAxisRect->axis(QCPAxis::atLeft));

  graph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1), QBrush(Qt::white),4));
  graph1->setPen(QPen(QColor(100, 100, 100), 2));
  graph1->setLineStyle((QCPGraph::LineStyle)4);

  histograma->xAxis->setRange(0, 10000);

  QLinearGradient plotGradient;
  plotGradient.setStart(0, 0);
  plotGradient.setFinalStop(0, 350);
  plotGradient.setColorAt(0, QColor(80, 80, 80));
  plotGradient.setColorAt(1, QColor(50, 50, 50));
  histograma->setBackground(plotGradient);
  QLinearGradient axisRectGradient;
  axisRectGradient.setStart(0, 0);
  axisRectGradient.setFinalStop(0, 350);
  axisRectGradient.setColorAt(0, QColor(80, 80, 80));
  axisRectGradient.setColorAt(1, QColor(30, 30, 30));
  histograma->axisRect()->setBackground(axisRectGradient);



 // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
  histograma->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);




}

void MainWindow::buildQKDChannelPanel(int ch){
    // This used to be one Designer widget per channel (plotAwid, plotBwid,
    // plotCwid_2, plotDwid) -- same tree, built once per channel here
    // instead. Ported property-for-property from the removed .ui, including
    // two pre-existing inconsistencies: only channels A and B (0,1) ever had
    // the gradient background on the panel itself, and PlotA's QCustomPlot
    // had a (no-op) empty styleSheet override that the others didn't.
    static const char *kChannelLetter[NUM_QKD_CHANNELS] = {"A","B","C","D"};
    static const bool  kHasGradientBg[NUM_QKD_CHANNELS] = {true, true, false, false};
    const char *kStyleLight = "color: rgb(238, 238, 236);";
    const char *kStyleWhite = "color: rgb(255, 255, 255);";
    const char *kGradientBg = "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255));";

    QWidget *panel = new QWidget();
    if (kHasGradientBg[ch]) panel->setStyleSheet(kGradientBg);

    QHBoxLayout *hbox = new QHBoxLayout(panel);
    hbox->setContentsMargins(0,0,0,0);

    QCustomPlot *plot = new QCustomPlot();
    plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    hbox->addWidget(plot);

    QVBoxLayout *side = new QVBoxLayout();
    hbox->addLayout(side);

    QLabel *title = new QLabel(QString("Plot ")+kChannelLetter[ch]);
    title->setStyleSheet(kStyleLight);
    side->addWidget(title);

    // Static 2-row header: "Counts"/"Track" labels, then "Total"/tsv-LCD.
    QFormLayout *header = new QFormLayout();
    side->addLayout(header);
    QLabel *countsLab = new QLabel(tr("Counts")); countsLab->setStyleSheet(kStyleLight);
    QLabel *trackLab  = new QLabel(tr("Track"));  trackLab->setStyleSheet(kStyleLight);
    header->addRow(countsLab, trackLab);
    QLabel *totalLab = new QLabel(tr("Total")); totalLab->setStyleSheet(kStyleWhite);
    m_qkdTsv[ch] = new QLCDNumber();
    header->addRow(totalLab, m_qkdTsv[ch]);

    // Populated later, per window, by setupsignalslot2().
    m_qkdTrackFL[ch] = new QFormLayout();
    side->addLayout(m_qkdTrackFL[ch]);

    side->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    m_qkdPlot[ch] = plot;
    ui->verticalLayout_18->addWidget(panel);
}

void MainWindow::setupsignalslot(){

    qRegisterMetaType<vectorInt64>("vectorInt64");
    qRegisterMetaType<vectorInt32>("vectorInt32");
    qRegisterMetaType<vectorInt8>("vectorInt8");
    qRegisterMetaType<vectorDouble>("vectorDouble");
    qRegisterMetaType<vectorBool>("vectorBool");
    qRegisterMetaType<vectorInt>("vectorInt");

    QObject::connect(ui->adqtime, SIGNAL(valueChanged(double)), this, SLOT(Chang_in_adqtime(double)));
    QObject::connect(ui->adqtime, SIGNAL(valueChanged(double)), &qutag, SLOT(Chang_in_adqtime(double)));
    QObject::connect(ui->adqtime, SIGNAL(valueChanged(double)), &TTU1, SLOT(Chang_in_adqtime(double)));

    QObject::connect(ui->histStart, SIGNAL(valueChanged(int)), &anl, SLOT(Chang_in_histStart(int)));
    QObject::connect(ui->histStart, SIGNAL(valueChanged(int)), this, SLOT(Chang_in_histStart(int)));
    QObject::connect(ui->histStart, SIGNAL(valueChanged(int)), &qutag, SLOT(Chang_in_histStart(int)));
    QObject::connect(ui->histStart, SIGNAL(valueChanged(int)), &TTU1, SLOT(Chang_in_histStart(int)));

    QObject::connect(ui->binWidth, SIGNAL(valueChanged(int)), &anl, SLOT(Chang_in_binWidth(int)));
    QObject::connect(ui->binWidth, SIGNAL(valueChanged(int)), this, SLOT(Chang_in_binWidth(int)));
    QObject::connect(ui->binWidth, SIGNAL(valueChanged(int)), &qutag, SLOT(Chang_in_binWidth(int)));
    QObject::connect(ui->binWidth, SIGNAL(valueChanged(int)), &TTU1, SLOT(Chang_in_binWidth(int)));

    QObject::connect(ui->binsinplot, SIGNAL(valueChanged(int)), &anl, SLOT(Chang_in_binsinplot(int)));
    QObject::connect(ui->binsinplot, SIGNAL(valueChanged(int)), this, SLOT(Chang_in_binsinplot(int)));
    QObject::connect(ui->binsinplot, SIGNAL(valueChanged(int)), &qutag, SLOT(Chang_in_binsinplot(int)));
    QObject::connect(ui->binsinplot, SIGNAL(valueChanged(int)), &TTU1, SLOT(Chang_in_binsinplot(int)));

    QObject::connect(ui->adqtime_2, SIGNAL(valueChanged(double)), &anl, SLOT(Chang_adqtime_2(double)));
    QObject::connect(ui->adqtime_2, SIGNAL(valueChanged(double)), this, SLOT(Chang_adqtime_2(double)));

    QObject::connect(&anl, SIGNAL(Chang_anlAvilable(bool)), &qutag, SLOT(Chang_anlAvilable(bool)));

    QObject::connect(&anl, SIGNAL(CombinationChange(bool)), this, SLOT(CombinationChange(bool)));

    QObject::connect(&qutag, SIGNAL(dataready(vectorInt64, vectorInt, int)), &anl, SLOT(timestampREC(vectorInt64, vectorInt, int)),Qt::QueuedConnection);
    QObject::connect(&TTU1, SIGNAL(dataready(vectorInt64, vectorInt, int)), &anl, SLOT(timestampREC(vectorInt64, vectorInt, int)),Qt::QueuedConnection);

    QObject::connect(&anl, SIGNAL(anlongoing(bool)), &qutag, SLOT(adqpausechange(bool)));

    QObject::connect(&qutag, SIGNAL(qutaghist(vectorDouble, vectorDouble, vectorDouble, vectorDouble, int, int , int ,int )), this, SLOT(histoplot(vectorDouble, vectorDouble, vectorDouble, vectorDouble, int, int , int ,int )),Qt::QueuedConnection);
    QObject::connect(&TTU1, SIGNAL(TTUhist(vectorDouble, vectorDouble, vectorDouble, vectorDouble, int, int , int ,int )), this, SLOT(histoplot(vectorDouble, vectorDouble, vectorDouble, vectorDouble, int, int , int ,int )),Qt::QueuedConnection);


    QObject::connect(&anl, SIGNAL(rates_tab2(vectorDouble, double)), this, SLOT(plotRates_tab2(vectorDouble, double)));


    QObject::connect(ui->DBON, SIGNAL(valueChanged(int)), this, SLOT(turnONDB(int)));
    QObject::connect(ui->CreateTables, SIGNAL(released()), this, SLOT(createTablesDB()));


    QObject::connect(ui->actionSave_state, SIGNAL(triggered(bool)), this, SLOT(SaveState(bool)));
    QObject::connect(ui->actionLoad_state, SIGNAL(triggered(bool)), this, SLOT(LoadState(bool)));


    QObject::connect(ui->loadLogic, SIGNAL(released()), this, SLOT(loadLogicS()));
    QObject::connect(ui->saveLogic, SIGNAL(released()), this, SLOT(saveLogicS()));

    QObject::connect(this, SIGNAL(main_CreateTableTab1(int ,int ,int ,int , QLabel *)), &dbc, SLOT(CreateTableTab1(int ,int ,int ,int, QLabel *)));
    QObject::connect(this, SIGNAL( main_CreateTableTab2(vectorInt32,vectorInt32,vectorInt32,vectorInt32,vectorInt32, vectorBool, int, QLabel*)), &dbc, SLOT(CreateTableTab2(vectorInt32,vectorInt32,vectorInt32,vectorInt32,vectorInt32, vectorBool, int, QLabel*)),Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(main_SaveTab2Values(vectorDouble, float, double)), &dbc, SLOT(SaveTab2Values(vectorDouble, float, double)),Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(main_SaveTab1Values(vectorInt32,vectorInt32,vectorInt32,vectorInt32,float)), &dbc, SLOT(SaveTab1Values(vectorInt32,vectorInt32,vectorInt32,vectorInt32,float)),Qt::QueuedConnection);


    QObject::connect(ui->homscan_timed, SIGNAL(valueChanged(double)), this, SLOT(Chang_homscan_time(double)));
    QObject::connect(ui->homscan, SIGNAL(valueChanged(int)), this, SLOT(Chang_homscan(int)));

    QObject::connect(ui->tab2_xrange, SIGNAL(valueChanged(int)), this, SLOT(chang_tab2range(int)));


    QObject::connect(ui->reset_delay, SIGNAL(released()), this, SLOT(resetdelay()));

    QObject::connect(ui->Max_delayd, SIGNAL(valueChanged(double)), this, SLOT(chang_in_max_del(double)));

    QObject::connect(ui->stepduration, SIGNAL(valueChanged(int)), this, SLOT(chang_in_stepduration(int)));

    QObject::connect(ui->clean_tab2, SIGNAL(released()), this, SLOT(clean_tab2()));


    QObject::connect(&qutag, SIGNAL(TDCerror(QString)), this, SLOT(error1(QString)) );

    QObject::connect(&qkdparam, SIGNAL(sig_QKD_time(double)), this, SLOT(chang_QKD_time(double)));

    QObject::connect(&qkdparam, SIGNAL(sig_QKD_numb(int)), this, SLOT(chang_QKD_numb(int)));


    // qkdparam now emits one signal per QKD parameter (carrying the channel
    // index), matching the signature of the parameterized chang_QKD_* slots
    // here and in anl -- no per-channel fan-out needed any more.
    QObject::connect(&qkdparam, &GUI_param::sig_QKD_ph,   this, &MainWindow::chang_QKD_ph);
    QObject::connect(&qkdparam, &GUI_param::sig_QKD_iw,   this, &MainWindow::chang_QKD_iw);
    QObject::connect(&qkdparam, &GUI_param::sig_QKD_pxq,  this, &MainWindow::chang_QKD_pxq);
    QObject::connect(&qkdparam, &GUI_param::sig_QKD_zero, this, &MainWindow::chang_QKD_zero);

    QObject::connect(&qkdparam, SIGNAL(sig_QKD_time(double)), &anl, SLOT(chang_QKD_time(double)));

    QObject::connect(&qkdparam, SIGNAL(sig_QKD_numb(int)), &anl, SLOT(chang_QKD_numb(int)));

    QObject::connect(&qkdparam, &GUI_param::sig_QKD_ph,   &anl, &qutaganl::chang_QKD_ph);
    QObject::connect(&qkdparam, &GUI_param::sig_QKD_iw,   &anl, &qutaganl::chang_QKD_iw);
    QObject::connect(&qkdparam, &GUI_param::sig_QKD_pxq,  &anl, &qutaganl::chang_QKD_pxq);
    QObject::connect(&qkdparam, &GUI_param::sig_QKD_zero, &anl, &qutaganl::chang_QKD_zero);

    QObject::connect(ui->actionQKD, SIGNAL(triggered()), &qkdparam, SLOT(show()));
    QObject::connect(ui->actionDeviceParams, SIGNAL(triggered()), ui->conf, SLOT(show()));

    QObject::connect(ui->AddLogic, SIGNAL(released()), this, SLOT(AddLogicSelectorElement()));

    QObject::connect(ui->TSON, SIGNAL(valueChanged(int)), &qutag, SLOT(TSanl(int)));
    QObject::connect(ui->TSper, SIGNAL(valueChanged(int)), &qutag, SLOT(changTSper(int)));

    QObject::connect(ui->TSON, SIGNAL(valueChanged(int)), &TTU1, SLOT(TSanl(int)));
    QObject::connect(ui->TSper, SIGNAL(valueChanged(int)), &TTU1, SLOT(changTSper(int)));

    QObject::connect(this, SIGNAL(setOVDL(float)), &ovdl_1, SLOT(setDelay(float)));

    QObject::connect(ui->VDL_start, SIGNAL(valueChanged(double)), this, SLOT(chang_VDL_start(double)));

    QObject::connect(&qutag, SIGNAL(initdone()), this , SLOT(QUTAG_initdone()));
    QObject::connect(&TTU1, SIGNAL(ttuinitdone()), this , SLOT(TTUinitdone()));

    QObject::connect(ui->actioninit_Qutag, SIGNAL(triggered(bool)), this, SLOT(runQutag(bool)));

    QObject::connect(this, SIGNAL(MWChang_qutag_filtertype(QString, int)), &qutag, SLOT(Chang_qutag_filtertype(QString, int)));
    QObject::connect(this, SIGNAL(MWChang_qutag_filtermask(int, int, int)), &qutag, SLOT(Chang_qutag_filtermask(int, int, int)));

    QObject::connect(this, SIGNAL( MWChang_qutagThresh(double, int)), &qutag, SLOT(Chang_in_thch(double, int)));
    QObject::connect(this, SIGNAL( MWChang_TTUThresh(double, int)), &TTU1, SLOT(Chang_in_thch(double, int)));
    QObject::connect(this, SIGNAL( MWChang_TTUThresh(double, int)), this, SLOT(Chang_MW_TTUThresh(double, int)));

    QObject::connect(this, SIGNAL(MWChang_qutag_edge(QString, int)), &qutag, SLOT(Chang_rof(QString, int)));
    QObject::connect(this, SIGNAL(MWChang_TTU_edge(QString, int)), &TTU1, SLOT(Chang_rof(QString, int)));
    QObject::connect(this, SIGNAL(MWChang_TTU_edge(QString, int)), this, SLOT(Chang_MWTTU_rof(QString, int)));

    QObject::connect(this, SIGNAL( MWChang_qutag_delay(double, int)), &qutag, SLOT(Chang_delay(double, int)));
    QObject::connect(this, SIGNAL( MWChang_TTU_delay(double, int)), &TTU1, SLOT(Chang_delay(double, int)));
    QObject::connect(this, SIGNAL( MWChang_TTU_delay(double, int)), this, SLOT(Chang_delayTTUMain(double, int)));

    QObject::connect(this, SIGNAL( MWChang_TTU_deadtime(double, int)), &TTU1, SLOT(Chang_in_deadtime(double, int)));
    QObject::connect(this, SIGNAL( MWChang_TTU_deadtime(double, int)), this, SLOT(Chang_MW_TTUdeadtime(double, int)));

    QObject::connect(ui->actionTTURes_Std, SIGNAL(triggered(bool)), this, SLOT(TTURes_Std(bool)));
    QObject::connect(ui->actionTTURes_A, SIGNAL(triggered(bool)), this, SLOT(TTURes_A(bool)));
    QObject::connect(ui->actionTTURes_B, SIGNAL(triggered(bool)), this, SLOT(TTURes_B(bool)));
    QObject::connect(ui->actionTTURes_C, SIGNAL(triggered(bool)), this, SLOT(TTURes_C(bool)));

    QObject::connect(ui->actionTTXRes_Std, SIGNAL(triggered(bool)), this, SLOT(TTXRes_Std(bool)));
    QObject::connect(ui->actionTTXRes_B, SIGNAL(triggered(bool)), this, SLOT(TTXRes_B(bool)));

    QObject::connect(ui->connect_OVDL, SIGNAL(released()), this, SLOT(connectOVDLmw()));

    QObject::connect(ui->addfilter, SIGNAL(released()), this, SLOT(addfilterMW()));

    QObject::connect(this, SIGNAL( MWfilterConnect(int) ), &EXFOfilters, SLOT( filterConnect(int) ) );
    QObject::connect(this, SIGNAL( MWfilteripRet(QString , int) ), &EXFOfilters, SLOT( setIP(QString, int) ) );

    QObject::connect(this, SIGNAL( MWFilterWLChange(double, int) ), &EXFOfilters, SLOT( setWavelength(double, int) ) );
    QObject::connect(this, SIGNAL( MWFilterBWChange(int, int) ), &EXFOfilters, SLOT( setBandwidth(int, int) ) );

    QObject::connect(this, SIGNAL( MWFilterWLChange(double, int) ), &dbc, SLOT( setfiltersWL(double, int) ) );
    QObject::connect(this, SIGNAL( MWFilterBWChange(int, int) ), &dbc, SLOT( setfiltersBW(int, int) ) );

    QObject::connect(&EXFOfilters, SIGNAL(DeviceWL(float, int)), this, SLOT(loadFilterWL(float, int)));
    QObject::connect(&EXFOfilters, SIGNAL(DeviceBW(int, int)), this, SLOT(loadFilterBW(int, int)));
    QObject::connect(ui->rawtssave, SIGNAL(valueChanged(int)), &anl, SLOT(saveRawTSon(int)));

    QObject::connect(this, SIGNAL( BWscanONsignal(int, int) ), this, SLOT( BWfilterscanslot(int, int) ) );
    QObject::connect(this, SIGNAL( WLscanONsignal(int, int) ), this, SLOT( WLfilterscanslot(int, int) ) );

    QObject::connect(ui->tab2showcurrent, SIGNAL(stateChanged(int)), this, SLOT(t2showcurrent(int)));
    QObject::connect(ui->tab2showmin, SIGNAL(stateChanged(int)), this, SLOT(t2showmin(int)));
    QObject::connect(ui->tab2showmax, SIGNAL(stateChanged(int)), this, SLOT(t2showmax(int)));
    QObject::connect(ui->tab2showavr, SIGNAL(stateChanged(int)), this, SLOT(t2showavr(int)));

    QObject::connect(ui->dbofftimer, SIGNAL(released()), this, SLOT(programDBoff()));

    QObject::connect(&TTU1, SIGNAL(errortt(QString)), this, SLOT(error1(QString)));

    QObject::connect(ui->RemoveLogic,  SIGNAL(released()), this, SLOT(RemoveLogic()));

    QObject::connect(ui->loopfilterscan,  SIGNAL(stateChanged(int)), this, SLOT(loopfilterscanch(int)));

}

void MainWindow::setupsignalslot2(){

    for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) {
        for (int i=0;i<MAX_WIN;i++) {
            qkdTrack[ch][i] = new QRadioButton(QString(QString::number(i)));
            QObject::connect(qkdTrack[ch][i], &QRadioButton::toggled, [this, ch](bool val){Chang_track(ch, val);});

            qkdTrackGroup[ch].addButton(qkdTrack[ch][i]);

            qkdTrack[ch][i]->setStyleSheet("background-color: rgb(238, 238, 236)");

            qkdCounter[ch][i] = new QLCDNumber(7);

            m_qkdTrackFL[ch]->addRow(qkdCounter[ch][i], qkdTrack[ch][i]);
            m_qkdTrackFL[ch]->setRowVisible(i,false);
        }
    }
}

//////////////////////////////////////////////////////////
///////////////////plotting///////////////////////////
///////////////////////////////////////////////////////////

void MainWindow::plotRates(int plot, int event, double key){


    double value1 = event; 

   if(plot==0){
        ui->PlotTrack->graph(0)->addData(key-lastPointKey_tab1, value1);
    }
   if(plot==1){
        ui->PlotTrack->graph(1)->addData(key-lastPointKey_tab1, value1);
    }
   if(plot==2){
        ui->PlotTrack->graph(2)->addData(key-lastPointKey_tab1, value1);
    }
   if(plot==3){
        ui->PlotTrack->graph(3)->addData(key-lastPointKey_tab1, value1);
    }


    ui->PlotTrack->xAxis->setRange(key-lastPointKey_tab1, 120, Qt::AlignRight);

    ui->PlotTrack->replot();

    if(trackRateChang && plot== 0){
        ui->PlotTrack->graph(0)->data()->clear();
        trackRateChang=false;
    }
    if(trackRateChang && plot==1){
        ui->PlotTrack->graph(1)->data()->clear();
        trackRateChang=false;
    }
    if(trackRateChang && plot==2){
        ui->PlotTrack->graph(2)->data()->clear();
        trackRateChang=false;
    }
    if(trackRateChang && plot==3){
        ui->PlotTrack->graph(3)->data()->clear();
        trackRateChang=false;
    }

}

void MainWindow::plotRates_tab2(const vectorDouble &counters, double key){

    for (int i=0;i<numberOfLogicPlots;i++) {
        if(cleanTab2){
                tab2data[i].clear();
                ui->PlotTab2->graph(i)->data()->clear();
        }

        LogicCurrent[i]->display(counters[i]);
        tab2data[i].prepend(counters[i]);

        if(trackTab2[i]){
            ui->PlotTab2->graph(i)->addData(key-lastPointKey_tab2, counters[i]);
            if(CombiChang ){
                ui->PlotTab2->graph(i)->data()->clear();
                CombiChang=false;
            }

        }
        if(tab2data[i].size()>xrange)tab2data[i].resize(xrange);

        double t2min = *std::min_element(tab2data[i].begin(),tab2data[i].end());
        double t2max = *std::max_element(tab2data[i].begin(), tab2data[i].end());
        double t2avr = std::accumulate(tab2data[i].begin(), tab2data[i].end(), 0.0) / tab2data[i].size();

        LogicMin[i]->display(t2min);
        LogicMax[i]->display(t2max);
        LogicAvr[i]->display(t2avr);

    }
    if(cleanTab2)cleanTab2=false;


    if(dbrunning){
        if(in_homscan && prev_homscan<=in_Max_delay){
            del_key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

            if(prev_homscan<in_VDL_start)prev_homscan=in_VDL_start;
            if(del_key-del_previouskey>in_stepduration){
                prev_homscan+=in_homscan_time;
                del_previouskey=del_key;
                setOVDL(float(prev_homscan));
                ui->current_delay_pos->display(prev_homscan);
            }

        }

        emit main_SaveTab2Values(counters, float(in_adqtime_2), prev_homscan);
    }
   ui->PlotTab2->xAxis->setRange(key-lastPointKey_tab2, double(xrange), Qt::AlignRight);
   ui->PlotTab2->replot();


}

//////////////////////////////////////////////////////////
///////////////////histograms///////////////////////////
///////////////////////////////////////////////////////////


void MainWindow::histoplot(const vectorDouble &datA, const vectorDouble &datB, const vectorDouble &datC, const vectorDouble &datD, int count1,int count2 ,int count3 ,int count4 ){
    double binwidth=double(in_binWidth);
    int histStart_bins = int(in_histStart/binwidth);
    int maxsize = qMax((datA.size()-histStart_bins),qMax((datB.size()-histStart_bins),qMax((datC.size()-histStart_bins),(datD.size()-histStart_bins))));
    QVector<double> x(maxsize);

    m_qkdTsv[0]->display(count1);
    m_qkdTsv[1]->display(count2);
    m_qkdTsv[2]->display(count3);
    m_qkdTsv[3]->display(count4);

    for (int i=0; i<x.size(); ++i){
        x[i] = binwidth*i+in_histStart;
    }

    const vectorDouble *datIn[NUM_QKD_CHANNELS] = {&datA, &datB, &datC, &datD};
    QVector<double> datOut[NUM_QKD_CHANNELS];

    for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) {
        datOut[ch] = datIn[ch]->mid(histStart_bins,-1);

        m_qkdPlot[ch]->graph(0)->data()->clear();
        m_qkdPlot[ch]->graph(0)->setData(x, datOut[ch]);
        m_qkdPlot[ch]->graph(0)->rescaleAxes();
        m_qkdPlot[ch]->replot();
    }

    for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) {
        for (int D=0; D<datOut[ch].size(); D++) {
            for (int i=0; i<in_QKD_pxq[ch]; i++) {
                for(int j=0; j<in_QKD_numb;j++){
                    if(x[D]>j*in_QKD_time+in_QKD_zero[ch]+i*in_QKD_ph[ch] && x[D]<j*in_QKD_time+in_QKD_zero[ch]+i*in_QKD_ph[ch]+in_QKD_iw[ch])P_R[ch][i]+=datOut[ch][D];
                }
            }
        }
    }
  double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

  for (int i=0;i<4;i++) {
      for (int j=0;j<4;j++) {
          if(P_T[i][j])plotRates(i, P_R[i][j],key);
      }
  }
    QVector<int> chanData[NUM_QKD_CHANNELS];
    for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) {
        for(int i=0;i<in_QKD_pxq[ch];i++) {
            qkdCounter[ch][i]->display(P_R[ch][i]);
            chanData[ch].append(P_R[ch][i]);
            P_R[ch][i]=0;
        }
    }

    if(dbrunning)emit main_SaveTab1Values(chanData[0], chanData[1], chanData[2], chanData[3], in_adqtime);
  x.clear();
}

void MainWindow::turnONDB(int val){
    if(!dbc.isRunning()  && val==1){
        dbc.run();
        for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) {
            for (int i=0;i<in_QKD_pxq[ch];i++) {
              qkdTrack[ch][i]->setEnabled(true);
            }
        }
    }
    if(val==0){
        dbc.disconnectFromServer();
        // Disconnecting ends whatever run was in progress -- dbrunning must
        // not stay true across a disconnect, or a later reconnect (without
        // an explicit "Create Tables" click to start a new run) would look
        // like data is still being recorded when it isn't.
        dbrunning=false;
    }

}

void MainWindow::createTablesDB(){
    if(dbc.connection_succesfull){
        QVector<int> ActiveChan;
        QVector<int> logicL;
        QVector<int> logicR;
        QVector<int> WinL;
        QVector<int> WinR;
        QVector<bool> gate;

        for (int i=0;i<MAX_LOGIC;i++) {
          if(trackTab2[i]){
              ActiveChan.append(i);
              logicL.append(LSource[i]);
              logicR.append(RSource[i]);
              WinL.append(LWin[i]);
              WinR.append(RWin[i]);
              if(logicOP[i])gate.append(1);
              else gate.append(0);
          }
        }

        for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) {
            for (int i=0;i<in_QKD_pxq[ch];i++) {
              qkdTrack[ch][i]->setEnabled(false);
            }
        }

        emit main_CreateTableTab1(in_QKD_pxq[0],in_QKD_pxq[1],in_QKD_pxq[2],in_QKD_pxq[3], ui->tabledisplay1);
        emit main_CreateTableTab2(ActiveChan, logicL, logicR, WinL, WinR, gate, numberOfFilters, ui->tabledisplay2);
        dbrunning=true;
    }else error1("database not open yet");
}
void MainWindow::SaveState(bool a){
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save Current Configuration"), "",
            tr("Configuration (*.conf);;All Files (*)"));

    if (fileName.isEmpty())
            return;
        else {
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::information(this, tr("Unable to open file"),
                    file.errorString());
                return;
            }

            QDataStream out(&file);

                    out.setVersion(QDataStream::Qt_4_5);
                    QMap<QString, int> mapint;
                    QMap<QString, double> mapdouble;
                    QString localstring;


                    for ( int i=0;i<3;i++) {
                           for (int j =0;j<3;j++) {
                               for (int k = 0;k<2;k++) {
                                   localstring = QString("Plot_Win_BoE[%1][%2][%3]").arg(i).arg(j).arg(k);
                                    if(!mapint.contains(localstring))mapint.insert(localstring,Plot_Win_BoE[i][j][k]);
                                    else mapint.value(localstring, Plot_Win_BoE[i][j][k]);
                               }

                           }

                     }

                    for ( int i=0;i<3;i++) {
                           for (int j =0;j<2;j++) {
                                   localstring = QString("tab2_plot[%1][%2]").arg(i).arg(j);
                                    if(!mapint.contains(localstring))mapint.insert(localstring,tab2_plot[i][j]);
                                    else mapint.value(localstring, tab2_plot[i][j]);
                               }
                     }

                    for ( int i=0;i<3;i++) {
                           for (int j =0;j<2;j++) {
                                   localstring = QString("tab2_win[%1][%2]").arg(i).arg(j);
                                    if(!mapint.contains(localstring))mapint.insert(localstring,tab2_win[i][j]);
                                    else mapint.value(localstring, tab2_win[i][j]);
                               }
                     }

                    mapint.insert("in_startChan",in_startChan);

                    mapint.insert("in_histStart",in_histStart);
                    mapint.insert("in_binWidth",in_binWidth);
                    mapint.insert("in_binsinplot",in_binsinplot);

                    mapdouble.insert("in_adqtime", in_adqtime);
                    mapdouble.insert("in_adqtime_2", double(in_adqtime_2));

                    out<<mapint;
                    out<<mapdouble;
            }

}

bool MainWindow::LoadPrevoiusSeason(bool a){


    QString fileName = "LastSeasonVariables.conf";
    if (fileName.isEmpty())return 1;

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("Unable to open file for the paremeters"),
        file.errorString());
        return 1;
    }
    else  std::cout<<"loading previous season parameters"<<std::endl;


    QMap<QString, int> mapintout;
    QMap<QString, double> mapdoubleout;
    QMap<QString, QString> mapstringout;

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_4_5);
    in>>mapintout;
    in>>mapdoubleout;
    in>>mapstringout;


    QMapIterator<QString,int>i(mapintout);
    while (i.hasNext()) {
        i.next();
    }
    QMapIterator<QString,double>j(mapdoubleout);
    while (j.hasNext()) {
        j.next();
    }

    if(mapintout.contains("in_histStart"))ui->histStart->setValue(mapintout.value("in_histStart"));
    else ui->histStart->setValue(0);
    if(mapintout.contains("in_binWidth"))ui->binWidth->setValue(mapintout.value("in_binWidth"));
    else ui->binWidth->setValue(1);
    if(mapintout.contains("in_binsinplot"))ui->binsinplot->setValue(mapintout.value("in_binsinplot"));
    else ui->binsinplot->setValue(1000);
    if(mapdoubleout.contains("in_adqtime"))ui->adqtime->setValue(mapdoubleout.value("in_adqtime"));
    else ui->adqtime->setValue(1);

    ui->adqtime_2->setValue(1);
    if(mapdoubleout.contains("Max_delay"))ui->Max_delayd->setValue(mapdoubleout.value("Max_delay"));
    else ui->Max_delayd->setValue(500);
    if(mapintout.contains("stepduration"))ui->stepduration->setValue(mapintout.value("stepduration"));
    else ui->stepduration->setValue(30);
    if(mapintout.contains("homscan_time"))ui->homscan_timed->setValue(mapintout.value("homscan_time"));
    else ui->homscan_timed->setValue(1);

    if(mapdoubleout.contains("QKD_time"))ui->Max_delayd->setValue(mapdoubleout.value("Max_delay"));
    else ui->Max_delayd->setValue(500);


    if(mapintout.contains("TSper"))ui->TSper->setValue(mapintout.value("TSper"));
    else ui->TSper->setValue(10);

    if(mapdoubleout.contains("QKD_time"))ui->Max_delayd->setValue(mapdoubleout.value("Max_delay"));
    else ui->Max_delayd->setValue(500);


    for(int i = 0; i<NTTUCHANNELS; i++){
        QString wordTTUThresh = "TTUThresh" + QString::number(i);
        if(mapdoubleout.contains(wordTTUThresh))TTUThresh[i]=mapdoubleout.value(wordTTUThresh);

        QString wordTTUdelays = "TTUdelays" + QString::number(i);
        if(mapdoubleout.contains(wordTTUdelays))TTUdelays[i]=mapdoubleout.value(wordTTUdelays);

        QString wordTTUdeadtime = "TTUdeadtime" + QString::number(i);
        if(mapdoubleout.contains(wordTTUdeadtime))TTUdeadtimes[i]=mapdoubleout.value(wordTTUdeadtime);

        QString wordrofMW = "rofMW"+ QString::number(i);
        if(mapstringout.contains(wordrofMW))rofMW[i]=mapstringout.value(wordrofMW);
    }

    file.close();

    return 0;
}

 void MainWindow::SaveSeason(bool a){

    QString CurrentSeason = "LastSeasonVariables.conf";

    QFile file(CurrentSeason);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("Unable to open file"),
        file.errorString());
        return;
    }

    QDataStream out(&file);

    QMap<QString, int> mapint;
    QMap<QString, double> mapdouble;
    QMap<QString, QString> mapstring;


    mapint.insert("in_startChan",in_startChan);

    mapint.insert("in_histStart",in_histStart);
    mapint.insert("in_binsinplot",in_binsinplot);
    mapint.insert("in_binWidth",in_binWidth);

    mapdouble.insert("in_adqtime", in_adqtime);
    mapdouble.insert("in_adqtime_2", double(in_adqtime_2));

    mapdouble.insert("QKD_time",in_QKD_time);
    mapint.insert("QKD_numb",in_QKD_numb);

    // Key names ("QKD_phA".."QKD_phD" etc) are kept exactly as before --
    // GUI_param::LoadPrevoiusSeason still reads this same on-disk format.
    for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) {
        QString letter = QString(QChar('A'+ch));
        mapint.insert("QKD_ph"+letter,   in_QKD_ph[ch]);
        mapint.insert("QKD_iw"+letter,   in_QKD_iw[ch]);
        mapint.insert("QKD_pxq"+letter,  in_QKD_pxq[ch]);
        mapint.insert("QKD_zero"+letter, in_QKD_zero[ch]);
    }

    mapint.insert("TSper",qutag.TSpercentage);



    mapdouble.insert("Max_delay",in_Max_delay);
    mapdouble.insert("homscan_time",in_homscan_time);
    mapint.insert("stepduration",in_stepduration);
   for(int i = 0; i<NTTUCHANNELS; i++){
        QString wordTTUThresh = "TTUThresh" + QString::number(i);
        mapdouble.insert(wordTTUThresh,TTUThresh[i]);
        QString wordTTUdelays = "TTUdelays" + QString::number(i);
        mapdouble.insert(wordTTUdelays,TTUdelays[i]);
        QString wordTTUdeadtime = "TTUdeadtime" + QString::number(i);
        mapdouble.insert(wordTTUdeadtime,TTUdeadtimes[i]);
        QString wordrofMW = "rofMW"+ QString::number(i);
        mapstring.insert(wordrofMW, rofMW[i]);
   }

    out<<mapint;
    out<<mapdouble;
    out<<mapstring;

    file.close();
}

void MainWindow::LoadState(bool a){
    QString fileName = QFileDialog::getOpenFileName(this,
                tr("Load Configuration"), "",
                tr("Configuration (*.conf);;All Files (*)"));
        if (fileName.isEmpty())
                return;
    else {

            QFile file(fileName);

            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::information(this, tr("Unable to open file"),
                    file.errorString());
                return;
            }
            QMap<QString, int> mapintout;
            QMap<QString, double> mapdoubleout;
            QDataStream in(&file);
            in.setVersion(QDataStream::Qt_4_5);
            in>>mapintout;





            if(mapintout.contains("in_histStart"))ui->histStart->setValue(mapintout.value("in_histStart"));
            if(mapintout.contains("in_binsinplot"))ui->binsinplot->setValue(mapintout.value("in_binsinplot"));

            if(mapdoubleout.contains("in_adqtime"))ui->adqtime->setValue(mapdoubleout.value("in_adqtime"));
            if(mapdoubleout.contains("in_adqtime_2"))ui->adqtime_2->setValue(mapdoubleout.value("in_adqtime_2"));


               QMapIterator<QString,int>i(mapintout);
               while (i.hasNext()) {
                   i.next();
                   std::cout<< i.key().toStdString() <<  ": " << i.value() << std::endl;
               }
               in>>mapdoubleout;
               QMapIterator<QString,double>j(mapdoubleout);
               while (j.hasNext()) {
                   j.next();
                   std::cout<< j.key().toStdString() <<  ": " << j.value() << std::endl;
               }

    }

}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::loadLogicS(){

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                  "./",
                                                  tr("Images (LogicState*)"));

    if (fileName.isEmpty())return;
    else {

        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return;
        }

        QMap<QString, int> mapintout;
        QMap<QString, QString> mapstringout;

        QDataStream in(&file);
        in>>mapintout;
        in>>mapstringout;

        if(mapintout.contains("numberOfLogicPlots")){
            int logicnumberloaded = mapintout.value("numberOfLogicPlots");
            for(int i = 0; i<logicnumberloaded; i++)if(numberOfLogicPlots <=logicnumberloaded)this->AddLogicSelectorElement();
            for(int i = 0; i<logicnumberloaded; i++){
                QString LeftSourceMap = QString("SourceLeft%1").arg(i);
                if(mapintout.contains(LeftSourceMap)){
                    int LeftSourceMapValue = mapintout.value(LeftSourceMap);
                    if(LeftSourceMapValue<0){
                        LogicL[i]->setCurrentIndex(-1*LeftSourceMapValue);
                        QString WinLeftMap = QString("WindowLeft%1").arg(i);
                        if(mapintout.contains(WinLeftMap)){
                            int WinLeftMapValue = mapintout.value(WinLeftMap);
                            LogicWinL[i]->setCurrentIndex(WinLeftMapValue+1);

                        }
                    }
                    if(LeftSourceMapValue>=0)LogicL[i]->setCurrentText(QString::number(LeftSourceMapValue));
                }
                QString RightSourceMap = QString("SourceRight%1").arg(i);
                if(mapintout.contains(RightSourceMap)){
                    int RightSourceMapValue = mapintout.value(RightSourceMap);
                    if(RightSourceMapValue<0){
                        LogicR[i]->setCurrentIndex(-1*RightSourceMapValue);
                        QString WinRightMap = QString("WindowRight%1").arg(i);
                        if(mapintout.contains(WinRightMap)){
                            int WinLeftMapValue = mapintout.value(WinRightMap);
                            LogicWinR[i]->setCurrentIndex(WinLeftMapValue+1);
                        }
                    }
                    if(RightSourceMapValue>=0)LogicR[i]->setCurrentText(QString::number(RightSourceMapValue));
                }
                QString LogicOperatorMap = QString("LogicOperator%1").arg(i);
                if(mapstringout.contains(LogicOperatorMap) ){
                    LogicG[i]->setCurrentText(mapstringout.value(LogicOperatorMap));
                }
            }

        }
    }
}

void MainWindow::saveLogicS(){


    bool ok;
    QString com = QInputDialog::getText(this, tr("Record DATA"),tr("Insert a name for the file"), QLineEdit::Normal,QDir::home().dirName(), &ok);

        if (ok && !com.isEmpty()){
            com.prepend("LogicState_");
            com.append(".txt");
            QFile LogicFile(com);
            if (!LogicFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::information(this, tr("Unable to open file"),
                LogicFile.errorString());
                return;
            }
            QDataStream out(&LogicFile);

            QMap<QString, int> mapint;
            QMap<QString, QString> mapstring;

            mapint.insert("numberOfLogicPlots", anl.numberOfLogicPlots);

            for(int i=0;i<anl.numberOfLogicPlots;i++){
                mapint.insert(QString("SourceLeft%1").arg(i), anl.LSource[i]);
                mapint.insert(QString("SourceRight%1").arg(i), anl.RSource[i]);
                mapint.insert(QString("WindowLeft%1").arg(i), anl.LWin[i]);
                mapint.insert(QString("WindowRight%1").arg(i), anl.RWin[i]);
                mapstring.insert(QString("LogicOperator%1").arg(i), LogicG[i]->currentText());
            }
            out<<mapint;
            out<<mapstring;
       }

}

void MainWindow::clean_tab2(){
       cleanTab2=true;
}

void MainWindow::Chang_homscan(int val){
    in_homscan=bool(val);
    if(in_homscan && !firstscan){
        firstscan=true;
        del_previouskey = 0;
    }
}

void MainWindow::error1(QString text){
	QMessageBox msgBox;
    msgBox.setText(text);
	msgBox.exec();
}

void MainWindow::closeEvent(QCloseEvent *event){
   SaveSeason(true);
   qDebug() << "closeEvent: signalling worker threads to stop...";

   // Ask each worker to stop. Note: qutag/TTU1 stop via their own break_ flag
   // checked inside run()'s loop -- QThread::quit() only affects a thread's
   // own exec() event loop, which none of these run(), so the previous
   // qutag.quit()/dbc.quit() calls here were no-ops and have been dropped.
   qutag.Break();
   TTU1.Break();
   anl.Break();

   qApp->setOverrideCursor(Qt::WaitCursor);

   // Give each thread a real chance to exit via a blocking wait() instead of
   // polling isRunning() with sleep(1) -- that busy-wait could add up to a
   // full extra second of delay per thread and gave no way to notice a
   // thread that's genuinely stuck (e.g. blocked inside a vendor SDK call
   // or a hung DB connection attempt). We still ultimately wait for every
   // thread to finish -- destroying a QThread that's still running is
   // undefined behavior -- but we log if one is taking unusually long
   // instead of freezing silently forever.
   struct Worker { QThread *thread; const char *name; };
   const Worker workers[] = { {&qutag, "qutag"}, {&TTU1, "TTU1"}, {&dbc, "dbc"} };
   const unsigned long perAttemptTimeoutMs = 5000;

   for (const Worker &w : workers) {
       while (!w.thread->wait(perAttemptTimeoutMs)) {
           qWarning() << w.name << "hasn't stopped yet after" << perAttemptTimeoutMs
                      << "ms -- still waiting for it to finish before closing.";
       }
   }

   qApp->restoreOverrideCursor();
   qDebug() << "closeEvent: all worker threads stopped, closing.";

   event->accept();
}

void MainWindow::Chang_in_binsinplot(int val){
    this->in_binsinplot=val;
    ui->HistoEndDisplay->display(in_binWidth*in_binsinplot);
}

void MainWindow::Chang_in_histStart(int val){
    this->in_histStart=val;
    ui->HistoEndDisplay->display(in_binWidth*in_binsinplot);
}

void MainWindow::Chang_in_binWidth(int val){
    this->in_binWidth=val;
    ui->HistoEndDisplay->display(in_binWidth*in_binsinplot);
}

void MainWindow::Chang_log1(int val){
    if(val){
        setup_log_plot(m_qkdPlot[0]);
    }else{
        setupHistoPlot(m_qkdPlot[0]);
    }
}
void MainWindow::Chang_log2(int val){
    if(val){
        setup_log_plot(m_qkdPlot[1]);
    }else{
        setupHistoPlot(m_qkdPlot[1]);
    }
}
void MainWindow::Chang_log3(int val){
    if(val){
        setup_log_plot(m_qkdPlot[2]);
    }else{
        setupHistoPlot(m_qkdPlot[2]);
    }
}
void MainWindow::Chang_log4(int val){
    if(val){
        setup_log_plot(m_qkdPlot[3]);
    }else{
        setupHistoPlot(m_qkdPlot[3]);
    }
}
void MainWindow::setup_log_plot(QCustomPlot *histo){

    histo->yAxis->grid()->setSubGridVisible(true);
    histo->xAxis->grid()->setSubGridVisible(true);
    histo->yAxis->setScaleType(QCPAxis::stLogarithmic);
    histo->yAxis2->setScaleType(QCPAxis::stLogarithmic);
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    histo->yAxis->setTicker(logTicker);
    histo->yAxis2->setTicker(logTicker);
    histo->yAxis->setNumberFormat("eb"); // e = exponential, b = beautiful decimal powers
    histo->yAxis->setNumberPrecision(0); // makes sure "1*10^4" is displayed only as "10^4"

}



void MainWindow::createQKDLines(int ch){

    for(int i=0; i<MAX_QUBITS*2; i++){
        for (int j=0;j<MAX_WIN;j++) {
            if((i+1)%2){
                qkdLines[ch][j][i]->point1->setCoords(i/2*in_QKD_time+in_QKD_zero[ch]+j*in_QKD_ph[ch],0);
                qkdLines[ch][j][i]->point2->setCoords(i/2*in_QKD_time+in_QKD_zero[ch]+j*in_QKD_ph[ch],1);
            }
            else{
                qkdLines[ch][j][i]->point1->setCoords((i-1)/2*in_QKD_time+in_QKD_zero[ch]+in_QKD_iw[ch]+j*in_QKD_ph[ch],0);
                qkdLines[ch][j][i]->point2->setCoords((i-1)/2*in_QKD_time+in_QKD_zero[ch]+in_QKD_iw[ch]+j*in_QKD_ph[ch],1);
            }
            if(i<in_QKD_numb*2 && j<in_QKD_pxq[ch])qkdLines[ch][j][i]->setVisible(1);
        }
    }
    for (int j=0;j<MAX_WIN;j++)if(j<in_QKD_pxq[ch])m_qkdTrackFL[ch]->setRowVisible(j,true);
    m_qkdPlot[ch]->replot();
}

void MainWindow::hidelinesQ(int ch, int qubits){

    for (int i= 2*qubits;i<2*in_QKD_numb;i++) {
        for (int j=0;j<MAX_WIN;j++) {
            qkdLines[ch][j][i]->setVisible(0);
        }
    }
}
void MainWindow::hidelinesW(int ch, int win){
    for (int j=win;j<in_QKD_pxq[ch];j++)m_qkdTrackFL[ch]->setRowVisible(j, false);
    for (int i=0;i<2*in_QKD_numb;i++) {
        for (int j=win;j<in_QKD_pxq[ch];j++) {
            qkdLines[ch][j][i]->setVisible(0);
        }
    }
}

void MainWindow::AddLogicSelectorElements(){
    for(int i=0; i< MAX_LOGIC; i++){
        LogicTrack[i] = new QRadioButton(QString(QString::number(i)));
        LogicTrack[i]->setStyleSheet("background-color: rgb(238, 238, 236)");
        tab2buttongroup.addButton(LogicTrack[i]);
        QObject::connect(LogicTrack[i], &QRadioButton::toggled, [this, i](bool tog){tracktab2_change(tog, i);});


        LogicL[i] = new QComboBox();
        LogicL[i]->setStyleSheet("QComboBox { background-color: darkGray }" "QListView { color: white; }");
        LogicL[i]->addItems(plotslist);
        QObject::connect(LogicL[i], &QComboBox::currentTextChanged, [this, i](QString logicl){AddLogicSelectorWindowsL(logicl, i);});

        if(i>0){
            for (int j=i-1;j>=0;j--) {
               LogicL[i]->addItem(QString::number(j));
            }
        }

        LogicG[i] = new QComboBox();
        LogicG[i]->setStyleSheet("QComboBox { background-color: darkGray }" "QListView { color: white; }");
        LogicG[i]->addItems(logicrelations);
        QObject::connect(LogicG[i], &QComboBox::currentTextChanged, [this, i](QString logicg){this->chang_LogicOP(logicg, i); anl.chang_LogicOP(logicg, i);});


        LogicR[i] = new QComboBox();
        LogicR[i]->setStyleSheet("QComboBox { background-color: darkGray }" "QListView { color: white; }");
        LogicR[i]->addItems(plotslist);

        QObject::connect(LogicR[i], &QComboBox::currentTextChanged, [this, i](QString logicr){AddLogicSelectorWindowsR(logicr, i);});

        if(i>0){
            for (int j=i-1;j>=0;j--) {
               LogicR[i]->addItem(QString::number(j));
            }
        }
        LogicCurrent[i]= new QLCDNumber();
        LogicCurrent[i]->setDigitCount(8);
        LogicCurrent[i]->setVisible(false);
        LogicMin[i] = new QLCDNumber();
        LogicMin[i]->setDigitCount(8);
        LogicMin[i]->setVisible(false);
        LogicMax[i] = new QLCDNumber();
        LogicMax[i]->setDigitCount(8);
        LogicMax[i]->setVisible(false);
        LogicAvr[i] = new QLCDNumber();
        LogicAvr[i]->setDigitCount(8);
        LogicAvr[i]->setVisible(false);

        LogicTrack[i]->setVisible(false);
        LogicL[i]->setVisible(false);
        LogicG[i]->setVisible(false);
        LogicR[i]->setVisible(false);

        ui->logicgrid->addWidget(LogicTrack[i],i+1,0);
        ui->logicgrid->addWidget(LogicL[i], i+1, 1);
        ui->logicgrid->addWidget(LogicG[i], i+1, 3);
        ui->logicgrid->addWidget(LogicR[i],i+1,4);
        ui->logicgrid->addWidget(LogicCurrent[i],i+1,6);
        ui->logicgrid->addWidget(LogicMin[i],i+1,7);
        ui->logicgrid->addWidget(LogicMax[i],i+1,8);
        ui->logicgrid->addWidget(LogicAvr[i],i+1,9);
    }

}

void MainWindow::AddLogicSelectorElement(){

    int i = numberOfLogicPlots;

    if(i>MAX_LOGIC){
        qDebug()<<"reached the maximum number of logic plots: "<<MAX_LOGIC;
        return;
   }
    LogicTrack[i]->setVisible(true);
    LogicL[i]->setVisible(true);
    LogicG[i]->setVisible(true);
    LogicR[i]->setVisible(true);

    if(ui->tab2showcurrent->isChecked() )LogicCurrent[i]->setVisible(true);
    if(ui->tab2showmin->isChecked() )LogicMin[i]->setVisible(true);
    if(ui->tab2showmax->isChecked() )LogicMax[i]->setVisible(true);
    if(ui->tab2showavr->isChecked() )LogicAvr[i]->setVisible(true);

    numberOfLogicPlots++;
    qDebug()<<"number of logics plots: "<<numberOfLogicPlots;
    anl.numberOfLogicPlots=this->numberOfLogicPlots;

}

void MainWindow::RemoveLogic(){

    int i = numberOfLogicPlots;

    LogicTrack[i]->setVisible(false);
    LogicL[i]->setVisible(false);
    LogicG[i]->setVisible(false);
    LogicR[i]->setVisible(false);

    LogicCurrent[i]->setVisible(false);
    LogicMin[i]->setVisible(false);
    LogicMax[i]->setVisible(false);
    LogicAvr[i]->setVisible(false);
    if(LogicTrack[i]->isChecked())LogicTrack[i]->toggle();
    if(i<1){
        qDebug()<<"nothing to delete here";
        return;
    }else numberOfLogicPlots--;
    qDebug()<<"number of logics plots: "<<numberOfLogicPlots;
    anl.numberOfLogicPlots=this->numberOfLogicPlots;
}
void MainWindow::AddLogicSelectorWindowsL(QString t, int i){

    if((t.compare("PlotA")==0 || t.compare("PlotB")==0 || t.compare("PlotC")==0 || t.compare("PlotD")==0) && LogicWinL[i]==nullptr){
        LogicWinL[i] = new QComboBox();
        LogicWinL[i]->setStyleSheet("QComboBox { background-color: darkGray }" "QListView { color: white; }");
        ui->logicgrid->addWidget(LogicWinL[i],i+1,2);
        QObject::connect(LogicWinL[i], &QComboBox::currentTextChanged, [this, i](QString logicwinl){this->chang_LogicWinL(logicwinl, i); anl.chang_LogicWinL(logicwinl, i);});
        leftexpanded[i]=true;
    }
    else {
        anl.LSource[i]=t.toInt();
        this->LSource[i]=t.toInt();
    }

    updateTab2Windows(t,i);

}

void MainWindow::AddLogicSelectorWindowsR(QString t, int i){
   if((t.compare("PlotA")==0 || t.compare("PlotB")==0 || t.compare("PlotC")==0 || t.compare("PlotD")==0) && LogicWinR[i]==nullptr){
        LogicWinR[i] = new QComboBox();
        LogicWinR[i]->setStyleSheet("QComboBox { background-color: darkGray }" "QListView { color: white; }");
        ui->logicgrid->addWidget(LogicWinR[i],i+1,5);
        QObject::connect(LogicWinR[i], &QComboBox::currentTextChanged, [this, i](QString logicwinr){this->chang_LogicWinR(logicwinr, i); anl.chang_LogicWinR(logicwinr, i);});
    }
   else {
       anl.RSource[i]=t.toInt();
       this->RSource[i]=t.toInt();
   }
   updateTab2Windows(t,i);
}

void MainWindow::updateTab2Windows(QString t, int index){

    // Source code convention (unchanged, shared with qutaganl): PlotA..D map
    // to LSource/RSource == -1..-NUM_QKD_CHANNELS.
    static const char *kChannelName[NUM_QKD_CHANNELS] = {"PlotA","PlotB","PlotC","PlotD"};

    for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) {
        if(t.compare(kChannelName[ch])!=0) continue;

        int source = -(ch+1);
        if(LogicWinL[index]!=nullptr && LogicL[index]->currentText().compare(kChannelName[ch])==0){
            LogicWinL[index]->clear();
            LogicWinL[index]->addItems(qkdWindowList[ch]);
            anl.LSource[index]=source;
            this->LSource[index]=source;
        }
        if(LogicWinR[index]!=nullptr && LogicR[index]->currentText().compare(kChannelName[ch])==0){
            LogicWinR[index]->clear();
            LogicWinR[index]->addItems(qkdWindowList[ch]);
            anl.RSource[index]=source;
            this->RSource[index]=source;
        }
    }
}

void MainWindow::chang_QKD_time(double val){
    in_QKD_time=val;
    if(initR){
        for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) createQKDLines(ch);
    }
}
void MainWindow::chang_QKD_numb(int val){

    if(in_QKD_numb>val){
        for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) hidelinesQ(ch, val);
    }
    in_QKD_numb=val;

    for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) createQKDLines(ch);
}

void MainWindow::chang_QKD_ph(int channel, int val){
    in_QKD_ph[channel]=val;
    if(initR)createQKDLines(channel);
}

void MainWindow::chang_QKD_pxq(int channel, int val){
    if(in_QKD_pxq[channel]>val)hidelinesW(channel, val);
    in_QKD_pxq[channel]=val;
    if(initR)createQKDLines(channel);

    QStringList &windowList = qkdWindowList[channel];
    windowList.clear();
    windowList<<"";
    for (int j=0;j<in_QKD_pxq[channel];j++) {
        windowList<<(QString("w")+QString::number(j));
    }
    static const char *kChannelName[NUM_QKD_CHANNELS] = {"PlotA","PlotB","PlotC","PlotD"};
    for (int index=0;index<numberOfLogicPlots;index++) {
        updateTab2Windows(kChannelName[channel],index);
    }
}
void MainWindow::chang_QKD_iw(int channel, int val){
    in_QKD_iw[channel]=val;
    if(initR)createQKDLines(channel);
}
void MainWindow::chang_QKD_zero(int channel, int val){
    in_QKD_zero[channel] =val;
    if(initR)createQKDLines(channel);
}

void MainWindow::chang_LogicWinL(QString t, int i){
    QRegularExpression rx("w(\\d+)");
    QRegularExpressionMatch match = rx.match(t);
    LWin[i]=match.captured(1).toInt();
}
void MainWindow::chang_LogicWinR(QString t, int i){
    QRegularExpression rx("w(\\d+)");
    QRegularExpressionMatch match = rx.match(t);
    RWin[i]=match.captured(1).toInt();
}

void MainWindow::chang_LogicOP(QString t, int index){
    if(t.compare("AND")==0)logicOP[index]=true;
    if(t.compare("OR")==0)logicOP[index]=false;
}

void MainWindow::Chang_track(int channel, bool val){
    for(int i=0; i<in_QKD_pxq[channel]; i++){
        P_T[channel][i]=qkdTrack[channel][i]->isChecked();
    }
    if(val)ui->PlotTrack->yAxis->rescale();
}

void MainWindow::tracktab2_change(bool c, int i){
    trackTab2[i]=c;
    if(c)ui->PlotTab2->graph(i)->addToLegend();
    else ui->PlotTab2->graph(i)->removeFromLegend();
}

void MainWindow::qutag_paremetes_setup(){

    /*************Thresholds*********/

    QLabel *thchLab[NQUTAGCHANNELS];


    for (int i=0;i<NQUTAGCHANNELS ;i++) {
        thchLab[i] = new QLabel(tr("Threshold Channel ")+QString::number(i));
        thchLab[i]->setStyleSheet("color: rgb(238, 238, 236)");
        thch[i] = new QDoubleSpinBox();
        thch[i]->setMaximum(3);
        thch[i]->setMinimum(-3);
        thch[i]->setDecimals(3);
        thch[i]->setSuffix(" [V]");
        thch[i]->setSingleStep(QUTAG_THRESHOLD_STEP);
        thch[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
        ui->Parameters_left->addRow(thchLab[i], thch[i]);
        QObject::connect(thch[i], &QDoubleSpinBox::valueChanged,[this, i](double thresh) {emit MWChang_qutagThresh(thresh, i);});
    }

      /*************Filters*********/


    QHBoxLayout *FiltersHLayout[NQUTAGCHANNELS];
    QHBoxLayout *FiltersboxesHLayout[NQUTAGCHANNELS];
    QLabel *filterLab[NQUTAGCHANNELS];

    for (int i=0;i<NQUTAGCHANNELS ;i++) {
        filterLab[i] = new QLabel(tr("Filter Ch ")+QString::number(i));
        filterLab[i]->setStyleSheet("color: rgb(238, 238, 236)");

        FiltersHLayout[i] = new QHBoxLayout();
        FiltersboxesHLayout[i] = new QHBoxLayout();

        ui->Parameters_left->addRow(filterLab[i], FiltersHLayout[i]);

        qutagFilterType[i] = new QComboBox();

        qutagFilterType[i]->setStyleSheet("QComboBox { background-color: darkGray }" "QListView { color: white; }");
        qutagFilterType[i] ->addItem(tr("NONE"));
        qutagFilterType[i] ->addItem(tr("MUTE"));
        qutagFilterType[i] ->addItem(tr("COINC"));
        qutagFilterType[i] ->addItem(tr("SYNC"));

        QObject::connect(qutagFilterType[i], &QComboBox::currentTextChanged,[this, i](const QString text) {emit MWChang_qutag_filtertype(text, i);});

        for (int j=0;j<NQUTAGCHANNELS ;j++) {
            qutagfilter[i][j] = new QCheckBox(QString::number(j)+tr(" "));
            qutagfilter[i][j]->setStyleSheet("color: rgb(238, 238, 236);");
            FiltersboxesHLayout[i]->addWidget(qutagfilter[i][j]);
            QObject::connect(qutagfilter[i][j], &QCheckBox::stateChanged,[this, i, j](int state) {emit MWChang_qutag_filtermask(state, i, j);});
        }

        FiltersHLayout[i]->addWidget(qutagFilterType[i]);
        FiltersHLayout[i]->addLayout(FiltersboxesHLayout[i]);

    }

    /*********Delays**********/


    QLabel *delLab[NQUTAGCHANNELS];
    for (int i=0;i<NQUTAGCHANNELS ;i++) {
        delLab[i] = new QLabel(tr("Delay ch ")+QString::number(i));
        delLab[i]->setStyleSheet("color: rgb(238, 238, 236)");
        delaych[i] = new QDoubleSpinBox();
        delaych[i]->setMaximum(100000);
        delaych[i]->setMinimum(-100000);
        delaych[i]->setDecimals(0);
        delaych[i]->setSuffix(" [ps]");
        delaych[i]->setSingleStep(QUTAG_DELAY_STEP);
        delaych[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
        ui->Parameters_left->addRow(delLab[i], delaych[i]);
        QObject::connect(delaych[i], &QDoubleSpinBox::valueChanged,[this, i](double delay) {emit MWChang_qutag_delay(delay, i);});
    }

    /********** trigger polarity***********/

    QLabel *edgeLab[NQUTAGCHANNELS];

    for (int i=0;i<NQUTAGCHANNELS ;i++) {
        edgeLab[i] = new QLabel(tr("Trigger polarity ch ")+QString::number(i));
        edgeLab[i]->setStyleSheet("color: rgb(238, 238, 236)");
        qutagEdge[i] = new QComboBox();
        qutagEdge[i]->setStyleSheet("QComboBox { background-color: darkGray }" "QListView { color: white; }");
        qutagEdge[i] ->addItem(tr("RISE"));
        qutagEdge[i] ->addItem(tr("FALL"));

        QObject::connect(qutagEdge[i], &QComboBox::currentTextChanged,[this, i](const QString text) {emit MWChang_qutag_edge(text, i);});

        ui->parameters_R->addRow(edgeLab[i], qutagEdge[i]);
    }
}

void MainWindow::runQutag(bool a){
    if(!qutag.isRunning()){
        qutag.start();
    }
}

void MainWindow::TTU_paremetes_setup(){

    ///thresholds/////

    QLabel *thchLab[NTTUCHANNELS];

    for (int i=0;i<NTTUCHANNELS ;i++) {
        thchLab[i] = new QLabel(tr("Threshold Channel ")+QString::number(i+1));
        thchLab[i]->setStyleSheet("color: rgb(238, 238, 236)");
        threshTTU[i] = new QDoubleSpinBox();
        threshTTU[i]->setMaximum(2.5);
        threshTTU[i]->setMinimum(-2.5);
        threshTTU[i]->setDecimals(3);
        threshTTU[i]->setSuffix(" [V]");
        threshTTU[i]->setSingleStep(QUTAG_THRESHOLD_STEP);
        threshTTU[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
        ui->Parameters_left->addRow(thchLab[i], threshTTU[i]);
        QObject::connect(threshTTU[i], &QDoubleSpinBox::valueChanged,[this, i](double thresh) {emit MWChang_TTUThresh(thresh, i);});
    }

    /////rise or fall////
    QLabel *edgeLabTTU[NTTUCHANNELS];
    for (int i=0;i<NTTUCHANNELS ;i++) {
        edgeLabTTU[i] = new QLabel(tr("Trigger polarity ch ")+QString::number(i));
        edgeLabTTU[i]->setStyleSheet("color: rgb(238, 238, 236)");
        TTUEdge[i] = new QComboBox();
        TTUEdge[i]->setStyleSheet("QComboBox { background-color: darkGray }" "QListView { color: white; }");
        TTUEdge[i] ->addItem(tr("RISE"));
        TTUEdge[i] ->addItem(tr("FALL"));

        QObject::connect(TTUEdge[i], &QComboBox::currentTextChanged,[this, i](const QString text) {emit MWChang_TTU_edge(text, i);});

        ui->parameters_R->addRow(edgeLabTTU[i], TTUEdge[i]);
    }
    /*********Delays**********/


    QLabel *delLabTTU[NTTUCHANNELS];
    for (int i=0;i<NTTUCHANNELS ;i++) {
        delLabTTU[i] = new QLabel(tr("Delay ch ")+QString::number(i));
        delLabTTU[i]->setStyleSheet("color: rgb(238, 238, 236)");
        delayTTU[i] = new QDoubleSpinBox();
        delayTTU[i]->setMaximum(100000000);
        delayTTU[i]->setMinimum(-1000000);
        delayTTU[i]->setDecimals(0);
        delayTTU[i]->setSuffix(" [ps]");
        delayTTU[i]->setSingleStep(QUTAG_DELAY_STEP);
        delayTTU[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
        ui->Parameters_left->addRow(delLabTTU[i], delayTTU[i]);
        QObject::connect(delayTTU[i], &QDoubleSpinBox::valueChanged,[this, i](double delay) {emit MWChang_TTU_delay(delay, i);});
    }

    /*********Dead time**********/
    // Hardware clamps to its own valid range per Swabian Instruments' Time
    // Tagger API docs: Ultra ~2 ns-2147 us, X ~1.333 ns-716 us. The wider
    // Ultra max is used here for the spinbox; the device silently clamps
    // it down further on Time Tagger X.
    QLabel *deadtimeLabTTU[NTTUCHANNELS];
    for (int i=0;i<NTTUCHANNELS ;i++) {
        deadtimeLabTTU[i] = new QLabel(tr("Dead time ch ")+QString::number(i));
        deadtimeLabTTU[i]->setStyleSheet("color: rgb(238, 238, 236)");
        deadtimeTTU[i] = new QDoubleSpinBox();
        deadtimeTTU[i]->setMaximum(2147000000);
        deadtimeTTU[i]->setMinimum(0);
        deadtimeTTU[i]->setDecimals(0);
        deadtimeTTU[i]->setSuffix(" [ps]");
        deadtimeTTU[i]->setSingleStep(QUTAG_DELAY_STEP);
        deadtimeTTU[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
        ui->Parameters_left->addRow(deadtimeLabTTU[i], deadtimeTTU[i]);
        QObject::connect(deadtimeTTU[i], &QDoubleSpinBox::valueChanged,[this, i](double dt) {emit MWChang_TTU_deadtime(dt, i);});
    }
}


void MainWindow::TTURes_Std(bool a){
    TTU1.SetTTResStd();
    TTU1.currentDevice = TTU;
    if(!TTU1.isRunning()){
        TTU1.start();
    }
}
void MainWindow::TTURes_A(bool a){
    TTU1.SetTTResA();
    TTU1.currentDevice = TTU;
    if(!TTU1.isRunning()){
        TTU1.start();
    }
}
void MainWindow::TTURes_B(bool a){
    TTU1.SetTTResB();
    TTU1.currentDevice = TTU;
    if(!TTU1.isRunning()){
        TTU1.start();
    }
}
void MainWindow::TTURes_C(bool a){
    TTU1.SetTTResC();
    TTU1.currentDevice = TTU;
    if(!TTU1.isRunning()){
        TTU1.start();
    }
}

void MainWindow::TTXRes_Std(bool a){
    TTU1.SetTTResStd();
    TTU1.currentDevice = TTX;
    if(!TTU1.isRunning()){
        TTU1.start();
    }
}
void MainWindow::TTXRes_B(bool a){
    TTU1.SetTTResB();
    TTU1.currentDevice = TTX;
    if(!TTU1.isRunning()){
        TTU1.start();
    }
}


void MainWindow::connectOVDLmw(){

    QString ovdlport = QFileDialog::getOpenFileName(this, tr("Open File"), "/dev/", tr("Images (ttyUSB*)"));

    if (ovdlport.isEmpty())return;
    else {
        ovdl_1.ovdlconnect(ovdlport);
    }
}


void MainWindow::addfilterMW(){


    if(numberOfFilters<MAX_N_FILTERS)numberOfFilters++;
    else return;
    int i = numberOfFilters-1;
    ////main horizontal layouts for each filter (3 rows)

    QHBoxLayout *filterParamsLayout = new QHBoxLayout();
    ui->filtersLayout->addLayout(filterParamsLayout);
    QHBoxLayout *filterWLscanLayout = new QHBoxLayout();
    ui->filtersLayout->addLayout(filterWLscanLayout);
    QHBoxLayout *filterBWscanLayout = new QHBoxLayout();
    ui->filtersLayout->addLayout(filterBWscanLayout);
    filterBWscanLayout->setContentsMargins(0,0,0,20);

    ////tiles for the main horizontal layouts, these goes on the left of the screen

    QLabel *filterTitle = new QLabel(tr("Filter ")+QString::number(numberOfFilters));// left of filterParamsLayout
    filterTitle->setStyleSheet("color: rgb(238, 238, 236)");
    filterParamsLayout->addWidget(filterTitle);
    QLabel *filterWLtitle = new QLabel(tr("Scan WL ")+QString::number(numberOfFilters));
    filterWLtitle->setStyleSheet("color: rgb(238, 238, 236)");
    filterWLscanLayout->addWidget(filterWLtitle);
    QLabel *filterBWtitle = new QLabel(tr("Scan BW ")+QString::number(numberOfFilters));
    filterBWtitle->setStyleSheet("color: rgb(238, 238, 236)");
    filterBWscanLayout->addWidget(filterBWtitle);

    ////for the first row add 3 pairs, connectButton-ip, WLlabel-WLset, BWLabel-BWset
    ///
    /// connectButton-ip
    ///
    QGridLayout *FilterFormLayout = new QGridLayout();
    filterconnect[i] = new QPushButton("connect");
    filterconnect[i]->setStyleSheet("color: rgb(238, 238, 236)");
    QObject::connect(filterconnect[i], &QPushButton::released,[this, i]() {emit MWfilterConnect(i);});
    FilterFormLayout->addWidget(filterconnect[i],0,0);
    filterip[i] = new QLineEdit(EXFOfilters.filterips[i]);
    QObject::connect(filterip[i], &QLineEdit::returnPressed,[this, i]() {emit MWfilteripRet(filterip[i]->text(),i);});
    filterip[i]->setStyleSheet("color: rgb(238, 238, 236)");
    filterip[i]->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    FilterFormLayout->addWidget(filterip[i],1,0);
    filterParamsLayout->addLayout(FilterFormLayout);

    ///WL label - WL set

    QLabel *filterWLlabel = new QLabel("Walvelength");
    filterWLlabel->setStyleSheet("color: rgb(238, 238, 236)");
    FilterFormLayout->addWidget(filterWLlabel,0,1);
    filterWavel[i] = new QDoubleSpinBox();
    filterWavel[i]->setMaximum(620);
    filterWavel[i]->setMinimum(480);
    filterWavel[i]->setDecimals(4);
    filterWavel[i]->setPrefix("1");
    filterWavel[i]->setSuffix(" [nm]");
    filterWavel[i]->setSingleStep(0.5);
    filterWavel[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
    QObject::connect(filterWavel[i], &QDoubleSpinBox::valueChanged,[this, i](double wavel) {emit MWFilterWLChange(wavel, i);});
    filterWavel[i]->setValue(EXFOfilters.filterWLDef[i]);
    FilterFormLayout->addWidget(filterWavel[i],1,1);

    ///BW label - BW set

    QLabel *filterBWlabel = new QLabel("Bandwidth");
    filterBWlabel->setStyleSheet("color: rgb(238, 238, 236)");
    FilterFormLayout->addWidget(filterBWlabel,0,2);
    filterBandw[i] = new QSpinBox();
    filterBandw[i]->setMaximum(650);
    filterBandw[i]->setMinimum(32);
    filterBandw[i]->setSuffix(" [pm]");
    filterBandw[i]->setSingleStep(1);
    filterBandw[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
    QObject::connect(filterBandw[i], &QSpinBox::valueChanged,[this, i](int bandw) {emit MWFilterBWChange(bandw, i);});
    filterBandw[i]->setValue(EXFOfilters.filterBWDef[i]);
    FilterFormLayout->addWidget(filterBandw[i],1,2);

    /////second row, WL scan with 5 elements (+labels): scanSlider, min, max,step, step duration
    ///
    ////labels:
    QGridLayout *FilterWLGridLayout = new QGridLayout();
    QLabel *WLscanLab = new QLabel("Scan");
    WLscanLab->setStyleSheet("color: rgb(238, 238, 236)");
    QLabel *WLscanMinLab = new QLabel("Min");
    WLscanMinLab->setStyleSheet("color: rgb(238, 238, 236)");
    QLabel *WLscanMaxLab = new QLabel("Max");
    WLscanMaxLab->setStyleSheet("color: rgb(238, 238, 236)");
    QLabel *WLscanStepSizeLab = new QLabel("Step Size");
    WLscanStepSizeLab->setStyleSheet("color: rgb(238, 238, 236)");
    QLabel *WLscanStepDurationLab = new QLabel("Step Duration");
    WLscanStepDurationLab->setStyleSheet("color: rgb(238, 238, 236)");
    FilterWLGridLayout->addWidget(WLscanLab, 0,0 );
    FilterWLGridLayout->addWidget(WLscanMinLab, 0,1 );
    FilterWLGridLayout->addWidget(WLscanMaxLab, 0,2 );
    FilterWLGridLayout->addWidget(WLscanStepSizeLab, 0,3 );
    FilterWLGridLayout->addWidget(WLscanStepDurationLab, 0,4 );

    ////WL scan slider
    WLscanON[i] = new QSlider(Qt::Orientation::Horizontal);
    WLscanON[i]->setMaximum(1);
    QObject::connect(WLscanON[i], &QSlider::valueChanged,[this, i](int s) {emit WLscanONsignal(s, i);});
    FilterWLGridLayout->addWidget(WLscanON[i],1,0);

    ///WL scan min
    WLscanMin[i] = new QDoubleSpinBox();
    WLscanMin[i]->setStyleSheet("color: rgb(238, 238, 236)");
    WLscanMin[i]->setMaximum(1620);
    WLscanMin[i]->setMinimum(1480);
    WLscanMin[i]->setSuffix(" [nm]");
    WLscanMin[i]->setSingleStep(1);
    WLscanMin[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
    FilterWLGridLayout->addWidget(WLscanMin[i],1,1);
    WLscanMin[i]->setValue(EXFOfilters.filterWLScanMinDef[i]);
    /////WL scan max

    WLscanMax[i] = new QDoubleSpinBox();
    WLscanMax[i]->setStyleSheet("color: rgb(238, 238, 236)");
    WLscanMax[i]->setMaximum(1620);
    WLscanMax[i]->setMinimum(1480);
    WLscanMax[i]->setSuffix(" [nm]");
    WLscanMax[i]->setSingleStep(1);
    WLscanMax[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
    FilterWLGridLayout->addWidget(WLscanMax[i],1,2);
    WLscanMax[i]->setValue(EXFOfilters.filterWLScanMaxDef[i]);
    ////WL scan step size

    WLscanstepsize[i] = new QDoubleSpinBox();
    WLscanstepsize[i]->setStyleSheet("color: rgb(238, 238, 236)");
    WLscanstepsize[i]->setMaximum(140);
    WLscanstepsize[i]->setMinimum(0.001);
    WLscanstepsize[i]->setSuffix(" [nm]");
    WLscanstepsize[i]->setSingleStep(1);
    WLscanstepsize[i]->setDecimals(3);
    WLscanstepsize[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
    FilterWLGridLayout->addWidget(WLscanstepsize[i],1,3);
    WLscanstepsize[i]->setValue(EXFOfilters.filterWLScanStepSizeDef[i]);

    ///WL scan step duration

    WLscanstepduration[i] = new QDoubleSpinBox();
    WLscanstepduration[i]->setStyleSheet("color: rgb(238, 238, 236)");
    WLscanstepduration[i]->setMaximum(86400);//one day
    WLscanstepduration[i]->setMinimum(1);
    WLscanstepduration[i]->setSuffix(" [s]");
    WLscanstepduration[i]->setSingleStep(1);
    WLscanstepduration[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
    FilterWLGridLayout->addWidget(WLscanstepduration[i],1,4);
    WLscanstepduration[i]->setValue(EXFOfilters.filterWLScanStepDurDef[i]);

    ///add the grid layout with the scan elements to the secon row layout
    filterWLscanLayout->addLayout(FilterWLGridLayout);

    ///--------------------///


    ///Thid row with the elements for BW scan: scanSlider, min, max,step, step duration
    QGridLayout *FilterBWGridLayout = new QGridLayout();

    ///labels:

    QLabel *BWscanLab = new QLabel("Scan");
    BWscanLab->setStyleSheet("color: rgb(238, 238, 236)");
    QLabel *BWscanMinLab = new QLabel("Min");
    BWscanMinLab->setStyleSheet("color: rgb(238, 238, 236)");
    QLabel *BWscanMaxLab = new QLabel("Max");
    BWscanMaxLab->setStyleSheet("color: rgb(238, 238, 236)");
    QLabel *BWscanStepSizeLab = new QLabel("Step Size");
    BWscanStepSizeLab->setStyleSheet("color: rgb(238, 238, 236)");
    QLabel *BWscanStepDurationLab = new QLabel("Step Duration");
    BWscanStepDurationLab->setStyleSheet("color: rgb(238, 238, 236)");
    FilterBWGridLayout->addWidget(BWscanLab, 0,0 );
    FilterBWGridLayout->addWidget(BWscanMinLab, 0,1 );
    FilterBWGridLayout->addWidget(BWscanMaxLab, 0,2 );
    FilterBWGridLayout->addWidget(BWscanStepSizeLab, 0,3 );
    FilterBWGridLayout->addWidget(BWscanStepDurationLab, 0,4 );

    ////BW scan slider
    BWscanON[i] = new QSlider(Qt::Orientation::Horizontal);
    BWscanON[i]->setMaximum(1);
    QObject::connect(BWscanON[i], &QSlider::valueChanged,[this, i](int s) {emit BWscanONsignal(s, i);});
    FilterBWGridLayout->addWidget(BWscanON[i],1,0);

    ///BW scan min

    BWscanMin[i] = new QSpinBox();
    BWscanMin[i]->setStyleSheet("color: rgb(238, 238, 236)");
    BWscanMin[i]->setMaximum(650);
    BWscanMin[i]->setMinimum(32);
    BWscanMin[i]->setSuffix(" [pm]");
    BWscanMin[i]->setSingleStep(1);
    BWscanMin[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
    FilterBWGridLayout->addWidget(BWscanMin[i],1,1);
    BWscanMin[i]->setValue(EXFOfilters.filterBWScanMinDef[i]);

    /////BW scan max

    BWscanMax[i] = new QSpinBox();
    BWscanMax[i]->setStyleSheet("color: rgb(238, 238, 236)");
    BWscanMax[i]->setMaximum(650);
    BWscanMax[i]->setMinimum(32);
    BWscanMax[i]->setSuffix(" [pm]");
    BWscanMax[i]->setSingleStep(1);
    BWscanMax[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
    FilterBWGridLayout->addWidget(BWscanMax[i],1,2);
    BWscanMax[i]->setValue(EXFOfilters.filterBWScanMaxDef[i]);

    ////BW scan step size

    BWscanstepsize[i] = new QSpinBox();
    BWscanstepsize[i]->setStyleSheet("color: rgb(238, 238, 236)");
    BWscanstepsize[i]->setMaximum(618);
    BWscanstepsize[i]->setMinimum(1);
    BWscanstepsize[i]->setSuffix(" [pm]");
    BWscanstepsize[i]->setSingleStep(1);
    BWscanstepsize[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
    FilterBWGridLayout->addWidget(BWscanstepsize[i],1,3);
    BWscanstepsize[i]->setValue(EXFOfilters.filterBWScanStepSizeDef[i]);


    ///BW scan step duration

    BWscanstepduration[i] = new QDoubleSpinBox();
    BWscanstepduration[i]->setStyleSheet("color: rgb(238, 238, 236)");
    BWscanstepduration[i]->setMaximum(86400);//one day
    BWscanstepduration[i]->setMinimum(1);
    BWscanstepduration[i]->setSuffix(" [s]");
    BWscanstepduration[i]->setSingleStep(1);
    BWscanstepduration[i]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255)); color: rgb(238, 238, 236)");
    FilterBWGridLayout->addWidget(BWscanstepduration[i],1,4);
    BWscanstepduration[i]->setValue(EXFOfilters.filterBWScanStepDurDef[i]);

    ///add the grid layout with the scan elements to the secon row layout
    filterBWscanLayout->addLayout(FilterBWGridLayout);



}

void MainWindow::loadFilterWL(float val, int dev){

    if(dev<numberOfFilters){
        QSignalBlocker blocker(filterWavel[dev]);
        filterWavel[dev]->setValue((double)val);
        blocker.unblock();
    }
}

void MainWindow::loadFilterBW(int val, int dev){
    if(dev<numberOfFilters){
        QSignalBlocker blocker(filterBandw[dev]);
        filterBandw[dev]->setValue(val);
        blocker.unblock();
    }
}

void MainWindow::BWfilterscanslot(int signal, int i){
    filterBandw[i]->setValue(BWscanMin[i]->value());
    if(BWscantimer[i]!=nullptr && signal==0){
        BWscantimer[i]->stop();
    }
    if(BWscantimer[i]==nullptr && signal){
        BWscantimer[i] = new QTimer(this);
        connect(BWscantimer[i], &QTimer::timeout, [this,i]() {BWscanstep(i);});
    }

    if(BWscantimer[i]!=nullptr && signal )BWscantimer[i]->start(BWscanstepduration[i]->value()*1000);


}

void MainWindow::WLfilterscanslot(int signal, int i){

    filterWavel[i]->setValue(WLscanMin[i]->value()-1000);
    if(WLscantimer[i]!=nullptr && signal==0){
        WLscantimer[i]->stop();
    }
    if(WLscantimer[i]==nullptr && signal){
        WLscantimer[i] = new QTimer(this);
        connect(WLscantimer[i], &QTimer::timeout, [this,i]() {WLscanstep(i);});
    }

    if(WLscantimer[i]!=nullptr && signal )WLscantimer[i]->start(WLscanstepduration[i]->value()*1000);
}


void MainWindow::BWscanstep(int i){
    if(filterBandw[i]->value() < BWscanMax[i]->value()){

        QString current = QString::number(filterBandw[i]->value()) ;
        int newval = current.toInt() + BWscanstepsize[i]->value();
        filterBandw[i]->setValue(newval);

    }else{
        if(loopfilterscanvar)filterBandw[i]->setValue(BWscanMin[i]->value());
        else BWscantimer[i]->stop();
    }
}

void MainWindow::WLscanstep(int i){
    if(filterWavel[i]->value() < WLscanMax[i]->value()-1000){

        QString current = QString::number(filterWavel[i]->value(),'f',filterWavel[i]->decimals()) ;    
        double newval = current.toDouble() + WLscanstepsize[i]->value();
        filterWavel[i]->setValue(newval);

    }else{
        if(loopfilterscanvar)filterWavel[i]->setValue(WLscanMin[i]->value()-1000);
        else WLscantimer[i]->stop();
    }
}

void MainWindow::t2showcurrent(int a){
    if(a){
        for (int i = 0; i < numberOfLogicPlots; i++) {
            LogicCurrent[i]->setVisible(true);
            t2labcur->setVisible(true);
        }
    }
    else{
        for (int i = 0; i < numberOfLogicPlots; i++) {
            LogicCurrent[i]->setVisible(false);
            t2labcur->setVisible(false);

        }
    }
}

void MainWindow::t2showmin(int a){
    if(a){
        for (int i = 0; i < numberOfLogicPlots; i++) {
            LogicMin[i]->setVisible(true);
            t2labmin->setVisible(true);
        }
    }
    else{
        for (int i = 0; i < numberOfLogicPlots; i++) {
            LogicMin[i]->setVisible(false);
            t2labmin->setVisible(false);

        }
    }
}
void MainWindow::t2showmax(int a){
    if(a){
        for (int i = 0; i < numberOfLogicPlots; i++) {
            LogicMax[i]->setVisible(true);
            t2labmax->setVisible(true);
        }
    }
    else{
        for (int i = 0; i < numberOfLogicPlots; i++) {
            LogicMax[i]->setVisible(false);
            t2labmax->setVisible(false);

        }
    }
}
void MainWindow::t2showavr(int a){
    if(a){
        for (int i = 0; i < numberOfLogicPlots; i++) {
            LogicAvr[i]->setVisible(true);
            t2labavr->setVisible(true);
        }
    }
    else{
        for (int i = 0; i < numberOfLogicPlots; i++) {
            LogicAvr[i]->setVisible(false);
            t2labavr->setVisible(false);

        }
    }
}


void MainWindow::programDBoff(){

    int dboffinms = QDateTime::currentDateTime().msecsTo(ui->dbcronometeroff->dateTime());
    std::cout<<"database will be off in "<<dboffinms/60000<<" minutes, miliseconds: "<<dboffinms<<std::endl;
    ui->dbcronometeroff->setStyleSheet("color: rgb(238, 0, 0)");
    QTimer::singleShot(dboffinms, this, SLOT(slideDBoff()));
}
void MainWindow::slideDBoff(){ui->DBON->setValue(0); ui->dbcronometeroff->setStyleSheet("color: rgb(238, 238, 236)");}


