#include "lockstatus.h"
#include <QCoreApplication>
#include <QDebug>

lockStatus::lockStatus()
{
    qConfigFile = QCoreApplication::applicationDirPath() + "/config.ini";
    QFileInfo configFileStr(qConfigFile);
    if(!configFileStr.isFile())
    {
        qDebug() << "loading faided...";
    }
    configFile = new QSettings(qConfigFile, QSettings::IniFormat);

    QVariant qvar = configFile->value(QString("/%1/%2").arg("config").arg("serialnum"));
    QString modbuscom = qvar.toString();

    configFile->deleteLater();
//打开串口
    m_serialPort = new QSerialPort();
    if(m_serialPort->isOpen())
    {
        m_serialPort->clear();
        m_serialPort->close();
    }
    m_serialPort->setBaudRate(QSerialPort::Baud9600,QSerialPort::AllDirections);//设置波特率和读写方向
    m_serialPort->setDataBits(QSerialPort::Data8);		//数据位为8位
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);//无流控制
    m_serialPort->setParity(QSerialPort::NoParity);	//无校验位
    m_serialPort->setStopBits(QSerialPort::OneStop); //一位停止位
    m_serialPort->setPortName(modbuscom);

//    connect(m_serialPort,SIGNAL(readyRead()),this,SLOT(receiveRfidInfo()));

    if(m_serialPort->open(QIODevice::ReadWrite))//用ReadWrite 的模式尝试打开串口
    {
        qDebug()<<"串口成功打开QSerialPort";
    }
    else
    {
        qDebug()<<"无法打开串口QSerialPort";
    }
    connect(m_serialPort,&QSerialPort::readyRead,this,&lockStatus::onReadReady);
}
//打开抽屉
void lockStatus::opencell(QString currentCabinetNum,QString currentCellNum)
{
    qDebug() << "entry opencell";
    //抽屉号
    quint8 startaddr = currentCellNum.toInt()+8;
    QByteArray addr;
    addr.insert(0,startaddr);
    startaddr = currentCabinetNum.toInt();
    //柜号
    QByteArray addr2;
    addr2.insert(0,startaddr);
    //发送数据
    sendData.clear();
    sendData=addr2 + QByteArray::fromHex("1000")+addr+QByteArray::fromHex("0001020001");
    //加校验位
    unsigned short crc =  CRC16_Modbus((unsigned char *)sendData.data(),sendData.length());
//    qDebug() << "crc:" << crc;
    unsigned char *pCrc = (unsigned char *)&crc;
    QByteArray ba((char*)pCrc);
    sendData = sendData + ba;

    if(m_serialPort->isOpen()){
        m_serialPort->write(sendData);
//        qDebug()<<"已发送："<<QString::fromUtf8(sendData);
        for(int i = 0; i < sendData.length();i++){
            qDebug() << QString::number(sendData[i]);
        }
    }else{
        qDebug()<<"failed";
    }
}

void lockStatus::onReadReady()
{
    qDebug() << "onReadReady";

    if (m_serialPort->bytesAvailable()) {
       //串口收到的数据可能不是连续的，需要的话应该把数据缓存下来再进行协议解析，类似tcp数据处理
       const QByteArray recv_data=m_serialPort->readAll();
       qDebug() << recv_data << recv_data.length();
       //接收发送要一致，如果是处理字节数据，可以把QByteArray当数组一样取下标，或者用data()方法转为char*形式
       if(recv_data.length() != 8){
           qDebug() << "recv data length error...";
           return;
       }
       for(int i = 0;i < recv_data.length() - 2;i++){
           if(recv_data[i] != sendData[i]){
               qDebug() << "recv data msg error...";
               return;
           }
       }

       unsigned short crc =  CRC16_Modbus((unsigned char *)recv_data.data(),recv_data.length() - 2);
       char *pCrc = (char *)&crc;
       for(int i = 0; i < 2;i++){
           if(pCrc[i] != recv_data[6+i]){
               qDebug() << "recv data check error..." << QString::number(pCrc[i]) << " " << QString::number(recv_data[6+i]);
               return;
           }
       }
       qDebug() << "open door success";
   }
}
//计算校验位
unsigned short lockStatus::CRC16_Modbus(unsigned char *pdata,int len)
{
    unsigned short crc=0xFFFF;
    int i, j;
    for ( j=0; j<len;j++)
    {
        crc=crc^pdata[j];
        for ( i=0; i<8; i++)
        {
            if( ( crc&0x0001) >0)
        {
            crc=crc>>1;
            crc=crc^ 0xa001;
        }
        else
            crc=crc>>1;
        }
    }
    return crc;
}
