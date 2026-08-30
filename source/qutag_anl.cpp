#include <qutag_anl.h>
#include <iostream>

qutaganl::qutaganl(){



    previouskey = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    cumulative = vectorDouble(5,0);
    flagaux = QVector<int>(3);
    flag = QVector<QVector<int>>(6,flagaux);
    counterplot = QVector<int>(6,0);
    outputCounter = vectorDouble(MAX_LOGIC, 0);

    in_QKD_ph= QVector<int>(4,0);
    in_QKD_iw= QVector<int>(4,0);
    in_QKD_zero= QVector<int>(4,0);
    in_QKD_pxq= QVector<int>(4,0);

    LSource= QVector<int>(MAX_LOGIC,0);
    RSource= QVector<int>(MAX_LOGIC,0);
    LWin= QVector<int>(MAX_LOGIC,0);
    RWin= QVector<int>(MAX_LOGIC,0);
    logicOP= QVector<int>(MAX_LOGIC,0);

    qRegisterMetaType<vectorInt64>("vectorInt64");
    qRegisterMetaType<vectorInt32>("vectorInt32");
    qRegisterMetaType<vectorInt8>("vectorInt8");
    qRegisterMetaType<vectorDouble>("vectorDouble");
    qRegisterMetaType<vectorBool>("vectorBool");
    qRegisterMetaType<vectorInt>("vectorInt");


    timestampProcess *analisis = new timestampProcess;
    analisis->moveToThread(&anlWorker1);
    connect(&anlWorker1, &QThread::finished, analisis, &QObject::deleteLater);
    connect(this, SIGNAL(timestampANL(vectorInt64 , vectorInt , int ,
                                          int , int,
                                          int , double , double ,
                                          vectorInt , vectorInt ,
                                          vectorInt ,vectorInt ,
                                          vectorInt , vectorInt , vectorInt ,
                                          vectorInt, bool )),
            analisis, SLOT(timestampANL(vectorInt64 , vectorInt , int ,
                                           int , int,
                                           int , double , double ,
                                           vectorInt , vectorInt ,
                                           vectorInt ,vectorInt ,
                                           vectorInt , vectorInt , vectorInt ,
                                           vectorInt, bool)),
            Qt::QueuedConnection);

    connect(analisis, SIGNAL(TScumulator_fromThread(vectorDouble)),
            this, SLOT(TScumulator(vectorDouble)), Qt::QueuedConnection);
    connect(analisis, SIGNAL(saveTTondisk_(long, long)), this, SLOT(saveTTondisk(long, long)));
    anlWorker1.start();

}

qutaganl::~qutaganl(){
 if(rawTT->isOpen())rawTT->close();
 anlWorker1.quit();
 anlWorker1.wait();
}

void qutaganl::timestampREC(const vectorInt64 &inconimg_vectorTimetags, const vectorInt &inconimg_vectorChannels, int inconimg_tsvalid){
    emit Chang_anlAvilable(false);

   emit timestampANL(inconimg_vectorTimetags, inconimg_vectorChannels, inconimg_tsvalid,
                  numberOfLogicPlots, this->in_startChan,
                      in_QKD_numb, in_QKD_time,  clkdiffT,
                      LSource, RSource,
                      LWin,RWin,
                      in_QKD_ph,in_QKD_zero, in_QKD_iw,
                      logicOP, saveTSon);
}



void timestampProcess::timestampANL(const vectorInt64 &vectorTimetags, const vectorInt &vectorChannels, int tsvalid,
                                    int numberOfLogicPlots, int in_startChan,
                                    int in_QKD_numb, double in_QKD_time, double clkdiffT,
                                    const vectorInt &LSource, const vectorInt &RSource,
                                    const vectorInt &LWin,const vectorInt &RWin,
                                    const vectorInt &in_QKD_ph, const vectorInt &in_QKD_zero, const vectorInt &in_QKD_iw,
                                    const vectorInt &logicOP, bool saveTSon_){
    int j;  //current element of the vector index
    int qq;// current qubit within clk signals
    double diffh=0;
    double futurediffh=0;
    vectorDouble threadCounter(numberOfLogicPlots,0);
    int ChannelIndex;
    int StopIndex;
    QVector<bool> flagL(logicOP.size(),0);
    QVector<bool> flagR(logicOP.size(),0);
    ////two vectors with the same size (tsvalid) are comming, one with timestamps (vectorTimetags) and the other with the corresponding channel (vectorChannels)////
    //// the variable i point the current element on the vector
    /// the variable j point the current element within one clk cicle
    /// the variable qq point the current "qubit" (set of windows) within one ckl cycle
    /// the variable clkdiffT is the TS relative to the beggining of the current qubitTS
    /// LSource[] and RSource[], stores which source you use for logic, from 0 and ahead are the previous results, -1 to -4 correspond to the 4 histograms

    for ( int i=0 ; i<tsvalid; i++) {  // iterate across all the ts comming
        ChannelIndex = abs(int(vectorChannels[i])); // check the channel of the current i
        if(ChannelIndex == in_startChan){ //found the start channel (5)
            qq=0;//starts from the first qubit (of 10 in the current case)
            j=i+1; //next element on the vector after the start channel
            if(j>=tsvalid)break; //next element is actualy out of the array - STOP
            StopIndex= abs(int(vectorChannels[j])); // check the channel for j element, starts from i+1 and keep increasing iside the while
            // unless we found again the clk channel (5), calculate the relative TS (diffh) TS-TSclk and substract the corresponding offset from the lines
            for(;StopIndex!=in_startChan;j++){
                diffh = vectorTimetags[j]-vectorTimetags[i]-in_QKD_zero[StopIndex-1];
                if(diffh<=0){
                    if(j+1 >=tsvalid)break;
                    StopIndex = abs(int(vectorChannels[j]));
                    continue;
                }
                while(diffh>=(qq+1)*in_QKD_time && qq<in_QKD_numb)qq++;//increase the counter for the qubit counter as long the relativeTS is not bigger than the next-qubitTS...
                if(qq>=in_QKD_numb)break;
                clkdiffT=diffh - qq*in_QKD_time;//TS relative to the beggining of the current qubit is calculated (clkdiffT)
                if(clkdiffT<0)std::cout<<"internal TS is negative!!!!!!!!!!!!!       qq: "<<qq<<std::endl;
                for (int ii=0;ii<numberOfLogicPlots;ii++) {// go thru all the logic on second tab
                    for (int pp=0;pp<4;pp++) {//go thru the 4 histograms
                        if(LSource[ii]==-pp-1 && StopIndex == pp+1){// if the current channel on the vector correspond with the channel on the left logic
                            if((LWin[ii]*in_QKD_ph[pp])<clkdiffT && clkdiffT<(LWin[ii]*in_QKD_ph[pp]+in_QKD_iw[pp]))flagL[ii]=true;//is the TS inside the window? Left logic
                        }
                        if(RSource[ii]==-pp-1 && StopIndex == pp+1){// if the current channel on the vector correspond with the channel on the Right logic
                            if((RWin[ii]*in_QKD_ph[pp])<clkdiffT && clkdiffT<(RWin[ii]*in_QKD_ph[pp]+in_QKD_iw[pp]))flagR[ii]=true;//is the TS inside the window? Right logic
                        }

                    }//4 channels

                   if(LSource[ii]>=0){//if the left logic is calling a previous result
                        if(logicOP[LSource[ii]]==1) if( flagL[LSource[ii]] && flagR[LSource[ii]] ) flagL[ii]=true;//and
                        if(logicOP[LSource[ii]]==0) if( flagL[LSource[ii]] || flagR[LSource[ii]] ) flagL[ii]=true;//or
                    }
                    if(RSource[ii]>=0){//if the Right logic is calling a previous result
                        if(logicOP[RSource[ii]]==1) if( flagL[RSource[ii]] && flagR[RSource[ii]] ) flagR[ii]=true;//and
                        if(logicOP[RSource[ii]]==0) if( flagL[RSource[ii]] || flagR[RSource[ii]] ) flagR[ii]=true;//or
                    }
                }//all relations on tab2

                if(j+1 >= tsvalid)break;
                StopIndex= abs(int(vectorChannels[j+1]));

                futurediffh = vectorTimetags[j+1] - vectorTimetags[i]-in_QKD_zero[StopIndex-1];

                if( (futurediffh>=(qq+1)*in_QKD_time || StopIndex==in_startChan) && qq+1<=in_QKD_numb){
                    for (int k=0;k<numberOfLogicPlots;k++) {
                        if(logicOP[k] == 1 && (flagL[k] && flagR[k])){
                            if(saveTSon_)emit saveTTondisk_(vectorTimetags[i], vectorTimetags[j-1]);
                            threadCounter[k]++;
                        }
                        if(logicOP[k] == 0 && (flagL[k] || flagR[k])) threadCounter[k]++;
                        flagL[k]=false;
                        flagR[k]=false;
                    }
                }//qqchange
            }//While -- clk signal reappears
        }
    }

    emit TScumulator_fromThread(threadCounter);
    threadCounter.clear();

}

void qutaganl::TScumulator(const vectorDouble &counter){

    key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    for (int i=0;i<counter.size();i++) {
        this->outputCounter[i] += counter[i];
    }

    if(key-previouskey>adqtime_2){
        for(int i = 0 ; i<outputCounter.size(); i++){
            if(LSource[i]>=0 && RSource[i]>=0){
                if(logicOP[i] == 2)outputCounter[i]=outputCounter[LSource[i]]+outputCounter[RSource[i]];
                if(logicOP[i] == 3)outputCounter[i]=outputCounter[LSource[i]]-outputCounter[RSource[i]];
                if(logicOP[i] == 4)outputCounter[i]=outputCounter[LSource[i]]*outputCounter[RSource[i]];
                if(logicOP[i] == 5 && outputCounter[RSource[i]] != 0)outputCounter[i]=outputCounter[LSource[i]]/outputCounter[RSource[i]];
            }
        }
        emit rates_tab2(this->outputCounter, key);
        previouskey=key;
        this->outputCounter.fill(0);

    }
    emit Chang_anlAvilable(true);
}



void qutaganl::chang_LogicWinL(QString t, int i){
    QRegularExpression rx("w(\\d+)");
    QRegularExpressionMatch match = rx.match(t);
    LWin[i]=match.captured(1).toInt();
}
void qutaganl::chang_LogicWinR(QString t, int i){
    QRegularExpression rx("w(\\d+)");
    QRegularExpressionMatch match = rx.match(t);
    RWin[i]=match.captured(1).toInt();
}

void qutaganl::chang_LogicOP(QString t, int index){

    if(t.compare("OR")==0)logicOP[index]=0;
    if(t.compare("AND")==0)logicOP[index]=1;
    if(t.compare("+")==0)logicOP[index]=2;
    if(t.compare("-")==0)logicOP[index]=3;
    if(t.compare("*")==0)logicOP[index]=4;
    if(t.compare("/")==0)logicOP[index]=5;
}
timestampProcess::timestampProcess(){

}

timestampProcess::~timestampProcess(){

}
void qutaganl::saveTTondisk(long clk, long tt){

    if(outTSstream != NULL)*outTSstream<< clk << "\t"<< tt << "\n";
}
void qutaganl::saveRawTSon(int a){
    if(a){
        QDateTime date = QDateTime::currentDateTime();
        QString formattedTime = date.toString("dd_MM_yyyy_hh_mm_ss");
        QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
        QString filename = "RawTS_"+formattedTimeMsg;

        rawTT = new QFile(filename);
        if (!rawTT->open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        outTSstream = new QTextStream(rawTT);
        saveTSon=true;

    }
    else{
        if(rawTT->isOpen()){
            rawTT->close();
            delete rawTT;
            delete outTSstream;
        }
        saveTSon=false;
    }
}
