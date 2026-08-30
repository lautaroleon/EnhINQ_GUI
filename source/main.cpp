#include <QApplication>
#include "mainwindow.h"

#include <signal.h>//ctl+c
#include <stdlib.h>
#include <stdio.h>//printf




/*void my_handler(int s){
           printf("\n ADIOS :D \n");
           exit(1); 
}*/



int main(int argc, char *argv[])
{


QFont font("DejaVuSerif-Bold");
font.setStyleHint(QFont::Monospace);
QApplication::setFont(font);
QApplication a(argc, argv);


///////////ctl+c/////////////////////////////

 /*struct sigaction sigIntHandler;

   sigIntHandler.sa_handler = my_handler;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;

   sigaction(SIGINT, &sigIntHandler, NULL);
*/
////////////////////////////////////////////


MainWindow w;



w.show();


return a.exec();
;

}
