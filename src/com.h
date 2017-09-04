#ifndef _COM_H_
#define _COM_H_

#include <QDialog>
#include <QTimer>
#include <QQueue>
#include <windows.h>
#include "ui_wirelessglove.h"

#define MAX_PORT_NUM	24
#define MAX_BAUD_NUM	13
#define MAX_PARITY_NUM	5
#define MAX_BYTESIZE_NUM 5
#define MAX_STOPBITS_NUM 3
#define MAX_BUFFER_SIZE	 1024
#define MAX_CURVE_NUM	 6
static char portNameArray[MAX_PORT_NUM][6]=
{
	"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9", "COM10", "COM11", "COM12","COM13", 
	"COM14", "COM15", "COM16", "COM17", "COM18", "COM19", "COM20", "COM21", "COM22", "COM23", "COM24"
};
static int baudRateArray[MAX_BAUD_NUM] =
{
	110, 300, 600, 1200, 2400, 4800, 9600, 
	14400, 19200, 38400, 56000, 57600, 115200
};
static char baudRateNameArray[MAX_BAUD_NUM][7] =
{
	"110", "300", "600", "1200", "2400", "4800", "9600", 
	"14400", "19200", "38400", "56000", "57600", "115200"
};
static char parityNameArray[MAX_PARITY_NUM][6]=
{
	"NONE","ODD","EVEN","MARK","SPACE"
};
static char byteSizeArray[MAX_BYTESIZE_NUM][6]=
{
	"4bits","5bits","6bits","7bits","8bits"
};
static char stopBitsArray[MAX_STOPBITS_NUM][8]=
{
	"1bit","1.5bits","2bits"
};
static char curveNameArray[MAX_CURVE_NUM][3]=
{
	"Ax","Ay","Az","Gx","Gy","Gz"
};


class ComPort:public QDialog,public Ui_curve
{
	Q_OBJECT
public:
	ComPort(QDialog *parent =0);

signals:
	void portNumChanged(int);
	void baudRateChanged(int);
	void parityChanged(int);
	void byteSizeChanged(int);
	void stopBitsChanged(int);
	void updatePlotWindow(int id, QMap<int, QQueue<QPointF>> *curveMap);

public slots:
	void setPortNum(int);
	void setBaudRate(int);
	void setParity(int);
	void setByteSize(int);
	void setStopBits(int);
	void changeFouceCurve(int);
	void toggleStartButton(void);
	void readComBuffer();

private:
	void initPortNum(void);
	void initBaudRate(void);
	void initParity(void);
	void initByteSize(void);
	void initStopBits(void);
	void initCurveName(void);
	void connectUIControl(void);
	void handleError(void);
	void startCom(void);
	void stopCom(void);
	unsigned short kalmanFilter(short index, unsigned short value);
	unsigned short Kalman(short index, unsigned short value);

private:
	QTimer *m_timer;
	HANDLE hCom;
	bool bstarted;
	unsigned char portNum;
	unsigned int  baudRate;
	unsigned char parity;
	unsigned char byteSize;
	unsigned char stopBits;
	unsigned int dataCnt;
	QQueue<QPointF> dataBuffer;
	QMap<int, QQueue<QPointF>> curveMap;
};

#endif