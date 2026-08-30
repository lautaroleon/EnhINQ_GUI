#ifndef GUI_PARAM_H
#define GUI_PARAM_H

#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <stdio.h>
#include <QtCore>
#include <iostream>//entradas y salidas por consola
#include <fstream>//archivos.txt
#include "typedefs.h"

class QHBoxLayout;

// This widget used to be laid out in Qt Designer (gui_param.ui): 4 visually
// identical columns of QKD windowing parameters, one per histogram channel
// (A/B/C/D), plus one shared column (qubit time/count, DB toggle, hdf5
// save). It's built in code instead so adding a channel is a matter of
// bumping NUM_QKD_CHANNELS, not adding a 5th Designer column by hand.
class GUI_param : public QWidget
{
    Q_OBJECT

public:
    static const int NUM_QKD_CHANNELS = 4;

    explicit GUI_param(QWidget *parent = nullptr);
    ~GUI_param();
    void QKD_setDefault();
    bool LoadPrevoiusSeason(bool a);
    void SaveSeason(bool a);

private:
    void buildChannelColumn(int channel, QHBoxLayout *outer);
    void buildSharedColumn(QHBoxLayout *outer);

    QSpinBox  *QKD_ph[NUM_QKD_CHANNELS];
    QSpinBox  *QKD_iw[NUM_QKD_CHANNELS];
    QSpinBox  *QKD_pxq[NUM_QKD_CHANNELS];
    QSpinBox  *QKD_zero[NUM_QKD_CHANNELS];
    QCheckBox *Plot_log[NUM_QKD_CHANNELS];

    QDoubleSpinBox *QKD_time;
    QSpinBox       *QKD_numb;
    QSlider        *QKD_DB;
    QPushButton    *hdf5button;

signals:
    void sig_QKD_time(double a);
    void sig_QKD_numb(int a);

    // One signal per QKD parameter, carrying the channel index -- replaces
    // what used to be 16 signals (sig_QKD_phA..D, sig_QKD_iwA..D, etc).
    void sig_QKD_ph(int channel, int a);
    void sig_QKD_iw(int channel, int a);
    void sig_QKD_pxq(int channel, int a);
    void sig_QKD_zero(int channel, int a);

    void savehdf5();
    void sig_turnONDB(int a);

    void sig_Plot_log(int channel, int a);
};

#endif // GUI_PARAM_H
