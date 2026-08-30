#include "timetaggerultra.h"

timetaggerUltra::timetaggerUltra()
{

    t = nullptr; // createTimeTagger() may never succeed (e.g. no hardware
                 // attached); freeTimeTagger() must not be called on a
                 // garbage pointer if the app is closed before that happens.
    TTRes = Resolution::Standard;
    break_=false;
    for(int i = 0; i<NTTUCHANNELS; i++)RoF[i] = 1;
}


timetaggerUltra::~timetaggerUltra()
{
    qDebug()<<"TTU destructor";
    if(t) freeTimeTagger(t);
}



void timetaggerUltra::run(){
    std::vector<std::string> taggers = scanTimeTagger();
    if (taggers.empty()) {
        std::cout << std::endl << "No time tagger found." << std::endl << "Please attach a Time Tagger." << std::endl;
        emit errortt("No time tagger found. Please attach a Time Tagger.");
        return;
    }


    // connect to a time tagger

/*
if any high resolution mode is selected, the hardware will combine 2, 4 or even 8 input channels and average
their timestamps. This results in a discretization jitter improvement of factor sqrt(N) for N combined channels. The
averaging is implemented before any filter, buffer or USB transmission. So all of those features are available with
the averaged timestamps. Because of hardware limitations, only fixed combinations of channels are supported:
• HighResA: 1 : [1,2], 3 : [3,4], 5 : [5,6], 7 : [7,8], 10 : [10,11], 12 : [12,13], 14 : [14,15], 16 : [16,17], 9, 18
• HighResB: 1 : [1,2,3,4], 5 : [5,6,7,8], 10 : [10,11,12,13], 14 : [14,15,16,17], 9, 18
• HighResC: 5 : [1,2,3,4,5,6,7,8], 14 : [10,11,12,13,14,15,16,17], 9, 18 The inputs 9 and 18 are always available
without averaging. The number of channels available will be limited to the number of channels licensed.
*/


    try{
        t = createTimeTagger("", TTRes);
    }
    catch (std::invalid_argument const& ex){
        qDebug()<<"can't create TT";
        std::cout << "#1: " << ex.what() << '\n';
        emit errortt(QString(ex.what()));
        return;
    }

    updateChannels();
    updateStream();

    emit ttuinitdone();
    setHistograms();

    double previous_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
    double current_time;

   while(!break_ && tts->isRunning()){
        current_time = QDateTime::currentDateTime().toMSecsSinceEpoch();

        if((current_time-previous_time) > 1000*in_adqtime){
            getHisto();//TDC_clearAllHistograms ();
            previous_time = current_time;
        }

        if(in_TSON)getTimeStampsTTU();
        QThread::msleep(1);
    }
}

void timetaggerUltra::updateStream(){

    long buffersize = EVENT_BUFFER_SIZE;


    delete tts;
    try{
        tts = new TimeTagStream(t,buffersize , TTUChannels );
    }
    catch (std::invalid_argument const& ex){
        std::cout << "#1: " << ex.what() << '\n';
    }

    tts->start();
    GoUpdateStream=false;
}

void timetaggerUltra::getTimeStampsTTU(){

    if(GoUpdateStream)updateStream();

    TimeTagStreamBuffer ttsb = tts->getData();

    std::vector<int>channels;
    std::vector<long long>timestamps;

    ttsb.getChannels([&channels](size_t size){
        channels.resize(size);
        return channels.data();
    });

    ttsb.getTimestamps([&timestamps](size_t size){
        timestamps.resize(size);
        return timestamps.data();
    });

    QVector<int64_t> timetags = QVector<int64_t>(timestamps.begin(),timestamps.end());

    QVector<int> channelsTDC = QVector<int>(channels.begin(),channels.end());

    if(ttsb.size>=EVENT_BUFFER_SIZE)std::cout<<"timetaggerultra buffer saturated!!        "<<ttsb.size<<std::endl;
    if(ttsb.size>0 ){
        emit dataready(timetags, channelsTDC, int(ttsb.size*(float(TSpercentage)/100)));
    }
}

void timetaggerUltra::getHisto(){
    if(GoUpdateHisto)setHistograms();
    int count[NTTUCHANNELS];
    QVector<double> dataA, dataB, dataC, dataD;
    std::vector<double> datac;
    ttuc->getData([&datac](size_t size) {
        datac.resize(size);
        return datac.data();
    });
    for(int i = 0; i<NTTUCHANNELS; i++){
        count[i]=(int)datac[i];
        std::vector<int32_t> histodata;
        ttuhisto[i]->getData([&histodata](size_t size1) {
            histodata.resize(size1);
            return histodata.data();
            });
        if(i==0){
            dataA = QVector<double>(histodata.begin(), histodata.end());
            ttuhisto[i]->clear();
        }
        if(i==1){
            dataB = QVector<double>(histodata.begin(), histodata.end());
            ttuhisto[i]->clear();
        }
        if(i==2){
            dataC = QVector<double>(histodata.begin(), histodata.end());
            ttuhisto[i]->clear();
        }
        if(i==3){
            dataD = QVector<double>(histodata.begin(), histodata.end());
            ttuhisto[i]->clear();
        }
    }

    datac.clear();
    if(!dataA.isEmpty() || !dataB.isEmpty() || !dataC.isEmpty() || !dataD.isEmpty() ) emit TTUhist(dataA, dataB, dataC, dataD,  count[0], count[1], count[2], count[3]);
    else qDebug()<<"TTU histograms empty";

}

void timetaggerUltra::setHistograms(){
    for(int i = 0; i<NTTUCHANNELS; i++){
        if(ttuhisto[i] != NULL)delete ttuhisto[i];
        try{
            ttuhisto[i] = new Histogram(t, TTUChannelsinuse[i] , ttStartChanSelected, this->in_binWidth, this->in_binsinplot);
            qDebug()<<"create histogram "<<i<< " channel: "<<TTUChannelsinuse[i]<<" width: "<<in_binWidth<<" nbins: "<<this->in_binsinplot;
        }
        catch (std::invalid_argument const& ex){
            std::cout << "#1: " << ex.what() << '\n';
        }
        if(ttuc  == NULL){
            try{
                ttuc = new Countrate(t, TTUChannels);
                qDebug()<<"create rate counters";
            }
            catch (std::invalid_argument const& ex){
                std::cout << "#1: " << ex.what() << '\n';
            }
        }
    }
    GoUpdateHisto=false;
}


void timetaggerUltra::updateChannels(){

    if(TTRes == Resolution::HighResA){
        for(int i = 0; i< NTTUCHANNELS; i++){
            if(i==0)TTUChannelsinuse[i]=RoF[i]*1;
            if(i==1)TTUChannelsinuse[i]=RoF[i]*3;
            if(i==2)TTUChannelsinuse[i]=RoF[i]*7;
            if(i==3)TTUChannelsinuse[i]=RoF[i]*10;
            if(i==4)TTUChannelsinuse[i]=RoF[i]*5;//clock
            else TTUChannelsinuse[i]=1;
        }
    }
    if(TTRes == Resolution::HighResB && currentDevice==TTU){
        for(int i = 0; i< NTTUCHANNELS; i++){
            if(i==0)TTUChannelsinuse[i]=RoF[i]*1;
            if(i==1)TTUChannelsinuse[i]=RoF[i]*10;
            if(i==2)TTUChannelsinuse[i]=RoF[i]*14;
            if(i==3)TTUChannelsinuse[i]=RoF[i]*9;// std res, high res ch5 reserved for clock
            if(i==4)TTUChannelsinuse[i]=RoF[i]*5;//clock
            else TTUChannelsinuse[i]=1;
        }
    }
    if(TTRes == Resolution::HighResB && currentDevice==TTX){
        for(int i = 0; i< NTTUCHANNELS; i++){
            if(i==0)TTUChannelsinuse[i]=RoF[i]*1;
            if(i==1)TTUChannelsinuse[i]=RoF[i]*9;
            if(i==2)TTUChannelsinuse[i]=RoF[i]*13;
            if(i==3)TTUChannelsinuse[i]=RoF[i]*14;// std res, high res ch5 reserved for clock
            if(i==4)TTUChannelsinuse[i]=RoF[i]*5;//clock
            else TTUChannelsinuse[i]=1;
        }

    }
    if(TTRes == Resolution::HighResC){
        for(int i = 0; i< NTTUCHANNELS; i++){
            if(i==0)TTUChannelsinuse[i]=RoF[i]*14;
            if(i==1)TTUChannelsinuse[i]=RoF[i]*9;
            if(i==2)TTUChannelsinuse[i]=RoF[i]*18;
            if(i==3)TTUChannelsinuse[i]=RoF[i]*1;//non avilable, likely will trigger an error
            if(i==4)TTUChannelsinuse[i]=RoF[i]*5;//clock
            else TTUChannelsinuse[i]=1;
        }

    }
    if(TTRes == Resolution::Standard){
        for(int i = 0; i< NTTUCHANNELS; i++){
            if(i==0)TTUChannelsinuse[i]=RoF[i]*1;
            else if(i==1)TTUChannelsinuse[i]=RoF[i]*2;
            else if(i==2)TTUChannelsinuse[i]=RoF[i]*3;
            else if(i==3)TTUChannelsinuse[i]=RoF[i]*4;
            else if(i==4)TTUChannelsinuse[i]=RoF[i]*5;//clock
            else TTUChannelsinuse[i]=1;
        }
    }

    //copy the channels on a std vector
    int n = sizeof(TTUChannelsinuse) / sizeof(TTUChannelsinuse[0]);
    TTUChannels = std::vector<int>(TTUChannelsinuse, TTUChannelsinuse+n);
}

void timetaggerUltra::Chang_delay(double d, int ch){
    t->setInputDelay(TTUChannelsinuse[ch], d);
}

void timetaggerUltra::Chang_in_deadtime(double d, int ch){
    // Time Tagger hardware clamps to its own device-specific min/max
    // (Ultra: ~2 ns - 2147 us, X: ~1.333 ns - 716 us), so we don't need
    // to duplicate that range-checking here.
    t->setDeadtime(TTUChannelsinuse[ch], d);
    deadtimes[ch] = d;
}
