#ifndef TIMETAGGERULTRA_H
#define TIMETAGGERULTRA_H

#include <QObject>
#include <QtCore>

#include "CustomDelayedChannel.h"
#include "CustomStartStop.h"
#include "typedefs.h"

#include <Iterators.h>
#include <TimeTagger.h>

#define EVENT_BUFFER_SIZE 1000000
#define NTTUCHANNELS 5
#define TTUSTARTCHANNEL 5

enum TTdevice{TTU,TTX};

// Worker thread driving a Swabian Instruments Time Tagger (Ultra or X,
// selected via currentDevice) over its vendor SDK (TimeTagger.h/Iterators.h),
// using the SDK's built-in Histogram/Countrate/TimeTagStream measurement
// classes (CustomStartStop.h/CustomDelayedChannel.h are pulled in but
// currently unused -- they're the SDK's example custom-measurement classes,
// not something this file instantiates). Mirrors qutagadq's role for the
// qutools device: acquires timestamps, builds the tab1 histograms, and
// reports per-channel threshold/edge/delay/dead-time settings back to the
// hardware.
class timetaggerUltra : public QThread
{
    Q_OBJECT
public:

    void run();
    explicit timetaggerUltra();
    ~timetaggerUltra();
    int TTUChannelsinuse[NTTUCHANNELS];
    std::vector<int>TTUChannels;
    Resolution TTRes;
    bool anlAvilable = false;
    int in_TSON=0;
    bool break_;
    int in_binsinplot, in_histStart, in_binWidth;
    double in_adqtime;
    TTdevice currentDevice;
    int TSpercentage=10;
    int in_binwidth=1;
    bool GoUpdateStream = false;
    bool GoUpdateHisto =false;
    double thresholds[NTTUCHANNELS];
    double deadtimes[NTTUCHANNELS];
    int RoF[NTTUCHANNELS];
    int ttStartChanSelected=TTUSTARTCHANNEL;

public slots:

    void SetTTResStd(){this->TTRes = Resolution::Standard;}
    void SetTTResA(){this->TTRes = Resolution::HighResA;}
    void SetTTResB(){this->TTRes = Resolution::HighResB;}
    void SetTTResC(){this->TTRes = Resolution::HighResC;}
    void Chang_in_binsinplot(int val){this->in_binsinplot=val;this->GoUpdateHisto=true;}
    void Chang_in_binWidth(int val){this->in_binWidth=val;this->GoUpdateHisto=true;}
    void Chang_in_histStart(int val){this->in_histStart=val;}
    void Chang_in_adqtime(double val){this->in_adqtime=val;}
    void Chang_anlAvilable(bool val){this->anlAvilable =val;}

    void TSanl(int val){this->in_TSON=val;}
    void changTSper(int val){this->TSpercentage=val;}
    void Break(){break_= true;}

    void Chang_in_thch(double voltage, int channel){t->setTriggerLevel(TTUChannelsinuse[channel],voltage);thresholds[channel]=voltage;}
    void Chang_rof(QString text, int ch){if(text=="RISE")RoF[ch]=1;else RoF[ch]=-1;updateChannels();this->GoUpdateHisto=true;GoUpdateStream =true;}
    void Chang_delay(double d, int ch);
    void Chang_in_deadtime(double d, int ch);

private:
    TimeTagger *t;
    TimeTagStream *tts;
    SynchronizedMeasurements *tmg;
    Histogram *ttuhisto[NTTUCHANNELS];
    Countrate *ttuc;

    void getTimeStampsTTU();
    void getHisto();
    void setHistograms();
    void updateStream();
    void updateChannels();
signals:
    void dataready(const vectorInt64 &vectorTimetags, const vectorInt &vectorChannels, int tsvalid);
    void TTUhist(const vectorDouble &TTdata1, const vectorDouble &TTdata2, const vectorDouble &TTdata3, const vectorDouble &TTdata4, int count1, int count2, int count3, int count4);
    void TDCerror(QString);
    void ttuinitdone();
    void errortt(QString text);

};

#endif // TIMETAGGERULTRA_H
