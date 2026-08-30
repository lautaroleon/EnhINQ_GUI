#ifndef EXFO_FILTERS_H
#define EXFO_FILTERS_H

#include <QtCore>
#include <QtNetwork>
#include "typedefs.h"

// Default IPs, unused -- filter IPs are actually loaded from
// exfofilters.json (see the constructor) or set at runtime via setIP().
#define EXFO_IP1    "192.168.0.105"
#define EXFO_IP2    "192.168.0.156"

// Controls up to MAX_N_FILTERS EXFO tunable optical bandpass filters over
// TCP (one raw ASCII socket per filter, e.g. "LAMBDA=1550.000\r\n" /
// "FWHM=0.100\r\n"). Per-filter defaults and scan ranges are loaded from
// exfofilters.json on construction.
class EXFO_Filters : public QObject
{
    Q_OBJECT
public:
    EXFO_Filters();
    ~EXFO_Filters();

    // Per-filter configuration, loaded from exfofilters.json and indexable
    // by the same filter index "n" used throughout the slots/signals below.
    int NofFilters = 0;
    QString filterips[MAX_N_FILTERS] = {""};
    int filterBWDef[MAX_N_FILTERS] = {0};
    double filterWLDef[MAX_N_FILTERS] = {0};
    int filterBWScanMinDef[MAX_N_FILTERS] = {0};
    double filterWLScanMinDef[MAX_N_FILTERS] = {0};
    int filterBWScanMaxDef[MAX_N_FILTERS] = {0};
    double filterWLScanMaxDef[MAX_N_FILTERS] = {0};
    int filterBWScanStepSizeDef[MAX_N_FILTERS] = {0};
    double filterWLScanStepSizeDef[MAX_N_FILTERS] = {0};
    double filterBWScanStepDurDef[MAX_N_FILTERS] = {0};
    double filterWLScanStepDurDef[MAX_N_FILTERS] = {0};

private:

    QTcpSocket *socket[MAX_N_FILTERS] = {NULL};
    QRegularExpression *readlambda;
    QRegularExpression *readfwhm;
    bool initdoneBW = false;
    bool initdoneWL = false;

public slots:

    // Open (or reuse) the socket for filter n and connect to filterips[n].
    void filterConnect(int n);
    void setIP(QString ip, int n);

    // Push a new bandwidth (pm) / wavelength (nm) setpoint to filter n.
    void setBandwidth(int bw, int n);
    void setWavelength(double wavel, int n);

    void readTcpData(int n);
    void FilterConnected(int n);
    void FilterDisconnected(int n);
    void FilterbytesWritten(qint64 data, int n);

signals:
    // Emitted once per filter after connecting, echoing back the device's
    // reported wavelength/bandwidth so the GUI can sync its spinboxes.
    void DeviceWL(float a, int dev);
    void DeviceBW(int a, int dev);
};

#endif // EXFO_FILTERS_H
