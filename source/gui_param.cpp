#include "gui_param.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>

namespace {
    const char *kLabelStyleWhite = "color: rgb(255, 255, 255);";
    const char *kLabelStyleLight = "color: rgb(238, 238, 236);";

    QLabel *makeLabel(const QString &text, const char *style){
        QLabel *lab = new QLabel(text);
        lab->setStyleSheet(style);
        return lab;
    }
}

GUI_param::GUI_param(QWidget *parent) :
    QWidget(parent)
{
    setWindowTitle(tr("Windows Parameters"));
    setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(80, 80, 80, 255), stop:1 rgba(50, 50, 50, 255));");
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QHBoxLayout *outer = new QHBoxLayout(this);

    for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) buildChannelColumn(ch, outer);
    buildSharedColumn(outer);

    QObject::connect(QKD_DB, &QSlider::valueChanged, this, &GUI_param::sig_turnONDB);
    QObject::connect(hdf5button, &QPushButton::released, this, &GUI_param::savehdf5);

    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}

void GUI_param::buildChannelColumn(int ch, QHBoxLayout *outer){

    const QChar letter('A'+ch);
    QVBoxLayout *column = new QVBoxLayout();
    outer->addLayout(column);

    // Phase time
    {
        QHBoxLayout *row = new QHBoxLayout();
        row->addWidget(makeLabel(tr("Phase time ")+letter, kLabelStyleWhite));
        QKD_ph[ch] = new QSpinBox();
        QKD_ph[ch]->setStyleSheet(kLabelStyleWhite);
        QKD_ph[ch]->setSuffix(" [ps]");
        // Channel C (index 2) never had a minimum set in the original
        // gui_param.ui -- a pre-existing inconsistency (its phase-time
        // spinbox can go down to 0, unlike the others). Preserved as-is.
        if (ch != 2) QKD_ph[ch]->setMinimum(1);
        QKD_ph[ch]->setMaximum(MAX_PHASE);
        QKD_ph[ch]->setSingleStep(100);
        QObject::connect(QKD_ph[ch], QOverload<int>::of(&QSpinBox::valueChanged), this, [this, ch](int v){emit sig_QKD_ph(ch, v);});
        row->addWidget(QKD_ph[ch]);
        column->addLayout(row);
    }
    // Integration window
    {
        QHBoxLayout *row = new QHBoxLayout();
        // The original gui_param.ui mislabels channel C's row "Integration
        // Win B" (a copy-paste typo); preserved verbatim.
        row->addWidget(makeLabel(tr("Integration Win ")+letter, kLabelStyleWhite));
        QKD_iw[ch] = new QSpinBox();
        QKD_iw[ch]->setStyleSheet(kLabelStyleWhite);
        QKD_iw[ch]->setSuffix(" [ps]");
        QKD_iw[ch]->setMinimum(1);
        QKD_iw[ch]->setMaximum(MAX_INT_WIN);
        QKD_iw[ch]->setSingleStep(100);
        QObject::connect(QKD_iw[ch], QOverload<int>::of(&QSpinBox::valueChanged), this, [this, ch](int v){emit sig_QKD_iw(ch, v);});
        row->addWidget(QKD_iw[ch]);
        column->addLayout(row);
    }
    // Peaks per qubit
    {
        QHBoxLayout *row = new QHBoxLayout();
        row->addWidget(makeLabel(tr("Peaks x Qubit ")+letter, kLabelStyleWhite));
        QKD_pxq[ch] = new QSpinBox();
        QKD_pxq[ch]->setStyleSheet(kLabelStyleWhite);
        QKD_pxq[ch]->setMinimum(0);
        QKD_pxq[ch]->setMaximum(MAX_WIN);
        QObject::connect(QKD_pxq[ch], QOverload<int>::of(&QSpinBox::valueChanged), this, [this, ch](int v){emit sig_QKD_pxq(ch, v);});
        row->addWidget(QKD_pxq[ch]);
        column->addLayout(row);
    }
    // Offset
    {
        QHBoxLayout *row = new QHBoxLayout();
        row->addWidget(makeLabel(tr("Offset ")+letter, kLabelStyleWhite));
        QKD_zero[ch] = new QSpinBox();
        QKD_zero[ch]->setStyleSheet(kLabelStyleWhite);
        QKD_zero[ch]->setMaximum(MAX_LINES_OFFSET);
        QKD_zero[ch]->setSingleStep(10);
        QObject::connect(QKD_zero[ch], QOverload<int>::of(&QSpinBox::valueChanged), this, [this, ch](int v){emit sig_QKD_zero(ch, v);});
        row->addWidget(QKD_zero[ch]);
        column->addLayout(row);
    }
    // Log-scale checkbox
    {
        QHBoxLayout *row = new QHBoxLayout();
        Plot_log[ch] = new QCheckBox(tr("Plot ")+letter+tr(" log scale"));
        Plot_log[ch]->setStyleSheet(kLabelStyleLight);
        QObject::connect(Plot_log[ch], &QCheckBox::stateChanged, this, [this, ch](int v){emit sig_Plot_log(ch, v);});
        row->addWidget(Plot_log[ch]);
        column->addLayout(row);
    }
}

void GUI_param::buildSharedColumn(QHBoxLayout *outer){

    QVBoxLayout *column = new QVBoxLayout();
    outer->addLayout(column);

    QFormLayout *form = new QFormLayout();
    column->addLayout(form);

    QKD_time = new QDoubleSpinBox();
    QKD_time->setStyleSheet(kLabelStyleLight);
    QKD_time->setSuffix(" [ps]");
    QKD_time->setDecimals(3);
    QKD_time->setMaximum(MAX_QUBIT_TIME);
    QKD_time->setSingleStep(100);
    QObject::connect(QKD_time, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GUI_param::sig_QKD_time);
    form->addRow(makeLabel(tr("Qubit time"), kLabelStyleWhite), QKD_time);

    QKD_numb = new QSpinBox();
    QKD_numb->setStyleSheet(kLabelStyleWhite);
    QKD_numb->setMinimum(0);
    QKD_numb->setMaximum(MAX_QUBITS);
    QObject::connect(QKD_numb, QOverload<int>::of(&QSpinBox::valueChanged), this, &GUI_param::sig_QKD_numb);
    form->addRow(makeLabel(tr("# qubits "), kLabelStyleWhite), QKD_numb);

    QHBoxLayout *dbRow = new QHBoxLayout();
    dbRow->addWidget(makeLabel(tr("Save data"), kLabelStyleLight));
    QKD_DB = new QSlider(Qt::Horizontal);
    QKD_DB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QKD_DB->setMinimumSize(76, 0);
    QKD_DB->setMaximum(1);
    dbRow->addWidget(QKD_DB);
    column->addLayout(dbRow);

    hdf5button = new QPushButton(tr("HDF5 hist save"));
    hdf5button->setStyleSheet(kLabelStyleLight);
    column->addWidget(hdf5button);
}

void GUI_param::QKD_setDefault(){
    if(LoadPrevoiusSeason(1)){
        QKD_time->setValue(10000);
        QKD_numb->setValue(10);

        for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) {
            QKD_ph[ch]->setValue(2000);
            QKD_iw[ch]->setValue(1900);
            QKD_pxq[ch]->setValue(2);
        }
        QKD_zero[0]->setValue(80);
        QKD_zero[1]->setValue(0);
        QKD_zero[2]->setValue(0);
        QKD_zero[3]->setValue(0);
    }
}

bool GUI_param::LoadPrevoiusSeason(bool a){

    QString fileName = "LastSeasonVariables.conf";
    if (fileName.isEmpty())return 1;

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        std::cout<<"Unable to open file"<<std::endl;
        return 1;
    }
    std::cout<<"loading previous season parameters <<qubits>>"<<std::endl;
    QMap<QString, int> mapintout;
    QMap<QString, double> mapdoubleout;
    QDataStream in(&file);
    //in.setVersion(QDataStream::Qt_4_5);
    in>>mapintout;


    QMapIterator<QString,int>i(mapintout);
    while (i.hasNext()) {
        i.next();
       //std::cout<< i.key().toStdString() <<  ": " << i.value() << std::endl;
    }
    in>>mapdoubleout;
    QMapIterator<QString,double>j(mapdoubleout);
    while (j.hasNext()) {
        j.next();
     //   std::cout<< j.key().toStdString() <<  ": " << j.value() << std::endl;
    }

    if(mapdoubleout.contains("QKD_time"))QKD_time->setValue(mapdoubleout.value("QKD_time"));
    else QKD_time->setValue(10000);
    if(mapintout.contains("QKD_numb"))QKD_numb->setValue(mapintout.value("QKD_numb"));
    else QKD_numb->setValue(10);

    // Key names ("QKD_phA".."QKD_phD" etc) are kept exactly as before --
    // MainWindow::SaveSeason still writes this same on-disk format.
    static const int kDefaultPh[NUM_QKD_CHANNELS]   = {2000, 2000, 2000, 2000};
    static const int kDefaultIw[NUM_QKD_CHANNELS]   = {1900, 1900, 1900, 1900};
    static const int kDefaultPxq[NUM_QKD_CHANNELS]  = {2, 2, 2, 2};
    static const int kDefaultZero[NUM_QKD_CHANNELS] = {80, 0, 0, 0};

    for (int ch=0; ch<NUM_QKD_CHANNELS; ch++) {
        QString letter = QString(QChar('A'+ch));

        if(mapintout.contains("QKD_ph"+letter))QKD_ph[ch]->setValue(mapintout.value("QKD_ph"+letter));
        else QKD_ph[ch]->setValue(kDefaultPh[ch]);

        if(mapintout.contains("QKD_iw"+letter))QKD_iw[ch]->setValue(mapintout.value("QKD_iw"+letter));
        else QKD_iw[ch]->setValue(kDefaultIw[ch]);

        if(mapintout.contains("QKD_pxq"+letter))QKD_pxq[ch]->setValue(mapintout.value("QKD_pxq"+letter));
        else QKD_pxq[ch]->setValue(kDefaultPxq[ch]);

        if(mapintout.contains("QKD_zero"+letter))QKD_zero[ch]->setValue(mapintout.value("QKD_zero"+letter));
        else QKD_zero[ch]->setValue(kDefaultZero[ch]);
    }

    file.close();
    return 0;
}


GUI_param::~GUI_param()
{
}
