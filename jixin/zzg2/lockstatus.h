#ifndef LOCKSTATUS_H
#define LOCKSTATUS_H

#include <QObject>
#include <QModbusRtuSerialMaster>
#include <QFileInfo>
#include <QSettings>
#include <QSerialPort>

class lockStatus:public QObject
{
    Q_OBJECT
public:
    lockStatus();
    void onReadReady();
    unsigned short CRC16_Modbus ( unsigned char *pdata, int len);
public slots:
    void opencell(QString currentCabinetNum,QString currentCellNum);
private:
    QByteArray sendData;
    QString qConfigFile;
    QSettings *configFile;
    QSerialPort *m_serialPort;
    QModbusRtuSerialMaster *modbusDevice;
    QString currentCabinetNum;
    QModbusRequest request;
};

#endif // LOCKSTATUS_H
