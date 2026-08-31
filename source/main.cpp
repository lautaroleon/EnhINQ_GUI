#include <QApplication>
#include <QFontDatabase>
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


QApplication a(argc, argv);

// The UI font is bundled as a Qt resource (see resources.qrc) rather than
// requested by name from the system. The previous code asked for the family
// "DejaVuSerif-Bold", which is not a family at all -- "DejaVu Serif" is the
// family and Bold is a weight -- so it matched nothing on either platform
// and only the Monospace style hint below took effect. Each platform then
// picked its own monospace default: DejaVu Sans Mono on Ubuntu, Courier New
// on Windows, which is why the two builds looked different.
//
// Registering the font here means both platforms get the same face whether
// or not it is installed system-wide. Must happen after the QApplication
// exists: QFontDatabase needs a live application instance.
for (const QString &path : { QStringLiteral(":/fonts/DejaVuSansMono.ttf"),
                             QStringLiteral(":/fonts/DejaVuSansMono-Bold.ttf") }) {
    if (QFontDatabase::addApplicationFont(path) == -1)
        fprintf(stderr, "warning: failed to load bundled font %s\n", qPrintable(path));
}

QFont font(QStringLiteral("DejaVu Sans Mono"));
font.setStyleHint(QFont::Monospace);   // fallback if the resource ever fails to load
// Pinned explicitly. The old code left the size unset, which happened to
// resolve to QFont's built-in default of 12pt because setFont() ran before
// the QApplication existed. Setting it after construction would otherwise
// let the platform default win (9pt on Windows) and shrink every widget.
font.setPointSize(12);
QApplication::setFont(font);


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
