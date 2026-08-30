#ifndef OVDL_H
#define OVDL_H

#include <QObject>
#include <QSerialPort>
#include <iostream>
#include <QFileDialog>

// Default serial device name, used only in log messages -- ovdlconnect()
// always opens whatever port name it's given.
#define OVDL_PORT "ttyUSB0"

// Driver for the optical variable delay line: a serial device controlled
// with a tiny ASCII protocol ("_ABS_<value>$" to set an absolute delay).
class OVDL : public QObject
{
    Q_OBJECT
public:
    explicit OVDL(QObject *parent = 0);

    QSerialPort *delaylineport;

public slots:
    void setDelay(float timeps);
    void ovdlconnect(QString s);
};

#endif // OVDL_H
