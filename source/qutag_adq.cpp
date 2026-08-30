#include <qutag_adq.h>
#include <iostream>//entradas y salidas por consola
#include <fstream>//archivos.txt
#include <time.h>

qutagadq::qutagadq(){

    timetags.reserve(QUTAG_TIMESTAMP_COUNT+1);
    channelsTDC.reserve(QUTAG_TIMESTAMP_COUNT+1);
    break_=false;
    adqpause_=false;
    histodataA=nullptr;
    histodataB=nullptr;
    histodataC=nullptr;
    histodataD=nullptr;
    anlAvilable=true;
    HIST_BINWIDTH=10;
    HIST_BINCOUNT=100;

}

void qutagadq::run(){
lautrun();

}

/* Check return code and exit on error */
void qutagadq::checkRc( const char * fctname, int rc ){
  if ( rc ) {
    printf("error\n");
    printf( ">>> %s: %s\n", fctname, TDC_perror( rc ) );
    QString errormessage= "error >>>" + QString(fctname) + ": " + QString(TDC_perror( rc ));
    if(rc == TDC_NotConnected)emit TDCerror("TDC not initialized, is disconnected or another instance is running");
    else emit TDCerror(errormessage);
  }
}


 qutagadq::~qutagadq(){

    TDC_deInit();

 }

 void qutagadq::lautrun(){

     printf( ">>> tdcbase version: %f\n", TDC_getVersion() );
     rc = TDC_init( -1 );                                 /* Accept every device */
     checkRc( "TDC_init", rc );

     rc = TDC_getTimebase( &timeBase );
     checkRc( "TDC_getTimebase", rc );

     std::cout<<" >>> timebase: "<<timeBase*1.e12<<"ps"<<std::endl;

     rc = TDC_enableChannels( 0xff );                     /* Use all channels */
     checkRc( "TDC_enableChannels", rc );

     rc = TDC_setTimestampBufferSize( QUTAG_TIMESTAMP_COUNT );
     checkRc( "TDC_setTimestampBufferSize", rc );

     rc = TDC_setExposureTime( 1000 );
     checkRc( "TDC_setExposureTime", rc );

     ///////////////initial thresholds and edges/////////////
     for(int i = 0 ; i<NQUTAGCHANNELS; i++){
         rc = TDC_getSignalConditioning(i, RoF+i,thresholds+i );
         checkRc( "TDC_getSignalConditioning", rc );
         std::cout<<" ch "<<i<<" edge : "<<RoF[i]<<"\t thresh"<<thresholds[i] <<std::endl;
     }

     /////////////filters//////
     for(int i = 0 ; i<5; i++){
         rc = TDC_getFilter(i, filtertype+i, ch_filtermask+i);
     }
     loadtdcfiltertype();

     /////delays//////

     rc = TDC_getChannelDelays(delays);
     fflush(stdout);

   /////histograms//////

   rc = TDC_enableStartStop( 1 );
   checkRc( "TDC_enableStartStop", rc );
   setHistograms();
   emit initdone();
   initdone_bool =1;

  TDC_clearAllHistograms ();

  double previous_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
  double current_time;

    while(!break_ ){
         current_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
         if((current_time-previous_time) > 1000*in_adqtime){
             getHisto();
             previous_time = current_time;
         }
         if(in_TSON)getTimeStamps();
         QThread::msleep(1);
     }
    std::cout<<"adq thread broke"<<std::endl;
 }

 void qutagadq::getHisto(){

    int count1, count2, count3, count4;

    if(paramschange)setHistogramsParam();
    if(rc)return;

     if (histodataA != 0) {
         delete [] histodataA;
     }

     histodataA = new Int32 [in_binsinplot];

     if (histodataB != 0) {
         delete [] histodataB;
     }

     histodataB = new Int32 [in_binsinplot];

     if (histodataC != 0) {
         delete [] histodataC;
     }

     histodataC = new Int32 [in_binsinplot];

     if (histodataD != 0) {
         delete [] histodataD;
     }

     histodataD = new Int32 [in_binsinplot];

    rc = TDC_getHistogram(QUTAG_START_CHANNEL, 1, 1, histodataA, &count, &tooSmall, &tooBig, &eventsA, &eventsB, &expTime );
    checkRc( "TDC_getHistogram A", rc );
    count1=count;
    std::copy(histodataA,histodataA+in_binsinplot,std::back_inserter(dataA));

    rc = TDC_getHistogram(QUTAG_START_CHANNEL, 2, 1, histodataB, &count, &tooSmall, &tooBig, &eventsA, &eventsB, &expTime );
    checkRc( "TDC_getHistogram B", rc );
    count2=count;
    std::copy(histodataB,histodataB+in_binsinplot,std::back_inserter(dataB));

    rc = TDC_getHistogram(QUTAG_START_CHANNEL, 3, 1, histodataC, &count, &tooSmall, &tooBig, &eventsA, &eventsB, &expTime );
    checkRc( "TDC_getHistogram C", rc );
    count3=count;
    std::copy(histodataC,histodataC+in_binsinplot,std::back_inserter(dataC));

    rc = TDC_getHistogram(QUTAG_START_CHANNEL, 4, 1, histodataD, &count, &tooSmall, &tooBig, &eventsA, &eventsB, &expTime );
    checkRc( "TDC_getHistogram D", rc );
    count4=count;
    std::copy(histodataD,histodataD+in_binsinplot,std::back_inserter(dataD));

    if(count1 != 0 || count2 !=0 || count3 !=0|| count4 !=0) emit qutaghist(dataA, dataB, dataC, dataD,  count1, count2, count3, count4);
       dataA.clear();
       dataB.clear();
       dataC.clear();
       dataD.clear();
 }

 void qutagadq::getTimeStamps(){
     timetags.clear();channelsTDC.clear();
     rc = TDC_getLastTimestamps( 1, timestamps, channels, &tsValid );
     checkRc( "TDC_getLastTimestamps", rc );
     std::copy(timestamps, timestamps + int(tsValid*(float(TSpercentage)/100)), std::back_inserter(timetags));
     std::copy(channels, channels + int(tsValid*(float(TSpercentage)/100)), std::back_inserter(channelsTDC));
     if(tsValid>=QUTAG_TIMESTAMP_COUNT)std::cout<<"qutag buffer saturated!!        "<<tsValid<<std::endl;
     if(tsValid>0 && anlAvilable){
        emit dataready(timetags, channelsTDC, int(tsValid*(float(TSpercentage)/100)));
    }

 }

 void qutagadq::setHistograms(){

     /////////////create the histograms on the FPGA /////////////

     rc = TDC_addHistogram(QUTAG_START_CHANNEL, 1, 1 );
     checkRc( "TDC_addHistogram", rc );
     rc = TDC_addHistogram( QUTAG_START_CHANNEL, 2, 1 );
     checkRc( "TDC_addHistogram", rc );
     rc = TDC_addHistogram( QUTAG_START_CHANNEL, 3, 1 );
     checkRc( "TDC_addHistogram", rc );
     rc = TDC_addHistogram( QUTAG_START_CHANNEL, 4, 1 );
     checkRc( "TDC_addHistogram", rc );
 }

  void qutagadq::setHistogramsParam(){

      HIST_BINWIDTH=in_binWidth;
      HIST_BINCOUNT=in_binsinplot;

      if((HIST_BINWIDTH!= HIST_BINWIDTH_out || HIST_BINCOUNT!=HIST_BINCOUNT_out) && HIST_BINWIDTH>=1){
        HIST_BINWIDTH_out= HIST_BINWIDTH;
        HIST_BINCOUNT_out = HIST_BINCOUNT;
        std::cout<<"HIST_BINWIDTH  :  "<< HIST_BINWIDTH_out<<std::endl;
        std::cout<<"HIST_BINCOUNT  :  "<< HIST_BINCOUNT_out<<std::endl;
        rc = TDC_setHistogramParams( HIST_BINWIDTH_out, HIST_BINCOUNT_out );
        checkRc( "TDC_setHistogramParams", rc );

      }
       paramschange=false;
  }

 void qutagadq::changThreshold(int ch){

     rc = TDC_configureSignalConditioning(ch, SCOND_MISC, RoF[ch], thresholds[ch]);
     checkRc( "TDC_configureSignalConditioning", rc );

 }

 void qutagadq::set_delays(){
     rc = TDC_setChannelDelays(delays);
     checkRc( "TDC_setChannelDelays", rc );
 }

 void qutagadq::loadtdcfiltertype(){

     for (int i =0; i<5;i++) {
         if(filtertype[i]==FILTER_NONE)filtertypeSTR[i]="NONE";
         if(filtertype[i]==FILTER_MUTE)filtertypeSTR[i]="MUTE";
         if(filtertype[i]==FILTER_COINC)filtertypeSTR[i]="COINC";
         if(filtertype[i]==FILTER_SYNC)filtertypeSTR[i]="SYNC";
         if(filtertype[i]==FILTER_INVALID)filtertypeSTR[i]="INVALID";
         std::cout<<filtertypeSTR[i].toStdString()<<std::endl;
     }

 }
 void qutagadq::updatefiltertype(int ch){
    for (int i =0; i<5;i++) {
        if(filtertypeSTR[i]=="NONE")filtertype[i]=FILTER_NONE;
        if(filtertypeSTR[i]=="MUTE")filtertype[i]=FILTER_MUTE;
        if(filtertypeSTR[i]=="COINC")filtertype[i]=FILTER_COINC;
        if(filtertypeSTR[i]=="SYNC")filtertype[i]=FILTER_SYNC;
    }
    setfilter(ch);
 }

 void qutagadq::setfilter(int ch){
     if(ch==0)rc = TDC_configureFilter(5, filtertype[ch] , ch_filtermask[ch]);
     else rc = TDC_configureFilter(ch, filtertype[ch] , ch_filtermask[ch]);
     checkRc( "TDC_configureFilter", rc );
 }
