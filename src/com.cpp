#include "com.h"

ComPort::ComPort(QDialog *parent)
	:QDialog(parent)
{
	setupUi(this);

	initPortNum();
	initBaudRate();
	initParity();
	initByteSize();
	initStopBits();
	initCurveName();
	connectUIControl();

	setPortNum(0);
	setBaudRate(6);
	setParity(0);
	setByteSize(4);
	setStopBits(0);

	bstarted = false;
	dataCnt = 0;
	m_timer = new QTimer(this);
	connect(m_timer,SIGNAL(timeout()),this,SLOT(readComBuffer()));
	//memset(dataBuffer,0,sizeof(dataBuffer));
	//dataBuffer.reserve(MAX_BUFFER_SIZE);
	for (quint32 index=0; index<MAX_CURVE_NUM; ++index)
	{
		curveMap[index].reserve(MAX_BUFFER_SIZE);
	}
}

void ComPort::initPortNum(void)
{
	for(quint32 index=0; index<MAX_PORT_NUM; ++index)
	{
		portComboBox->addItem(portNameArray[index]);
	}
}

void ComPort::initBaudRate(void)
{
	for (quint32 index=0; index<MAX_BAUD_NUM; ++index)
	{
		baudComboBox->addItem(baudRateNameArray[index]);
	}
}

void ComPort::initParity(void)
{
	for (quint32 index=0; index<MAX_PARITY_NUM; ++index)
	{
		parityComboBox->addItem(parityNameArray[index]);
	}
}

void ComPort::initByteSize(void)
{
	for (quint32 index=0; index<MAX_BYTESIZE_NUM; ++index)
	{
		databitsComboBox->addItem(byteSizeArray[index]);
	}
}

void ComPort::initStopBits(void)
{
	for (quint32 index=0; index<MAX_STOPBITS_NUM; ++index)
	{
		stopbitsComboBox->addItem(stopBitsArray[index]);
	}
}

void ComPort::initCurveName(void)
{
	for (quint32 index=0; index<MAX_CURVE_NUM; ++index)
	{
		curveNameComboBox->addItem(curveNameArray[index]);
	}
}

void ComPort::setPortNum(int _value)
{
	if(_value != portNum)
	{
		portNum = _value;
		emit portNumChanged(portNum);
	}
}

void ComPort::setBaudRate(int _value)
{
	if(_value != baudRate)
	{
		baudRate = _value;
		emit baudRateChanged(baudRate);
	}
}

void ComPort::setParity(int _value)
{
	if(_value != parity)
	{
		parity = _value;
		emit parityChanged(parity);
	}
}

void ComPort::setByteSize(int _value)
{
	if(_value != byteSize-4)
	{
		byteSize = _value+4;
		emit byteSizeChanged(byteSize-4);
	}
}

void ComPort::setStopBits(int _value)
{
	if(_value != stopBits)
	{
		stopBits = _value;
		emit stopBitsChanged(stopBits);
	}
}

void ComPort::changeFouceCurve(int _value)
{
	emit updatePlotWindow(_value, &curveMap);
}

void ComPort::toggleStartButton(void)
{
	if(bstarted)
	{
		bstarted = false;
		stopCom();
	}
	else
	{
		bstarted = true;
		startCom();
	}
}

void ComPort::connectUIControl(void)
{
	connect(portComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(setPortNum(int)));
	connect(this,SIGNAL(portNumChanged(int)),portComboBox,SLOT(setCurrentIndex(int)));
	connect(baudComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(setBaudRate(int)));
	connect(this,SIGNAL(baudRateChanged(int)),baudComboBox,SLOT(setCurrentIndex(int)));
	connect(parityComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(setParity(int)));
	connect(this,SIGNAL(parityChanged(int)),parityComboBox,SLOT(setCurrentIndex(int)));
	connect(databitsComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(setByteSize(int)));
	connect(this,SIGNAL(byteSizeChanged(int)),databitsComboBox,SLOT(setCurrentIndex(int)));
	connect(stopbitsComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(setStopBits(int)));
	connect(this,SIGNAL(stopBitsChanged(int)),stopbitsComboBox,SLOT(setCurrentIndex(int)));
	connect(startButton,SIGNAL(clicked()),this,SLOT(toggleStartButton()));
	connect(this,SIGNAL(updatePlotWindow(int, QMap<int, QQueue<QPointF>> *)),plotwindow,SLOT(setCurveData(int, QMap<int, QQueue<QPointF>> *)));
	connect(curveNameComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(changeFouceCurve(int)));
}

void ComPort::handleError(void)
{
	CloseHandle(hCom);
	hCom = NULL;
	startButton->setEnabled(false);
}

void ComPort::startCom(void)
{
	DCB comdcb;
	hCom=CreateFileA(portNameArray[portNum],GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING,0,NULL);
	if(hCom==(HANDLE)-1) 
	{ 
		printf("open %s error!\n",portNameArray[portNum]);
		return ; 
	}
	if(!GetCommState(hCom,&comdcb))
	{
		printf("get %s state error!\n",portNameArray[portNum]);
		handleError();
		return;
	}
	comdcb.BaudRate = baudRateArray[baudRate];
	comdcb.ByteSize = byteSize;
	comdcb.fParity  = parity;
	comdcb.StopBits = stopBits;
	if(!SetCommState(hCom,&comdcb))
	{
		printf("set %s state error!\n",portNameArray[portNum]);
		handleError();
		return;
	}
	if(!SetupComm(hCom,1024*4,1024*4))
	{
		printf("set %s buffer error!\n",portNameArray[portNum]);
		handleError();
		return;
	}

	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout=1;
	TimeOuts.ReadTotalTimeoutMultiplier=1;
	TimeOuts.ReadTotalTimeoutConstant=1;
	TimeOuts.WriteTotalTimeoutMultiplier=500;
	TimeOuts.WriteTotalTimeoutConstant=2000;
	if(!SetCommTimeouts(hCom,&TimeOuts))
	{
		printf("set %s timeout error!\n",portNameArray[portNum]);
		handleError();
		return;
	}
	if(!PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR))
	{
		printf("clear %s buffer error!\n",portNameArray[portNum]);
		handleError();
		return;
	}

	m_timer->start(100);
	startButton->setText(tr("&stop"));
}

void ComPort::stopCom(void)
{
	m_timer->stop();
	startButton->setText(tr("&start"));
	CloseHandle(hCom);
	hCom = NULL;
}

void ComPort::readComBuffer()
{
	DWORD readCnt=0;
	static unsigned char data[1024]={0};

	if(ReadFile(hCom,data,sizeof(data),&readCnt,NULL))
	{
		int index=0;
		while(index<2*MAX_CURVE_NUM+1 && (data[index]!='S'||data[index+2*MAX_CURVE_NUM+1]!='S'))	index++;
		readCnt -= (readCnt-index)%(2*MAX_CURVE_NUM+1);
		for (; index<readCnt&&index++; )
		{
			for (int curNum=0; curNum<MAX_CURVE_NUM; ++curNum)
			{
				static unsigned short valuePredict, prevalue,preprevalue;
				if(curveMap[curNum].size()==MAX_BUFFER_SIZE)
					curveMap[curNum].dequeue();

				if (curveMap[curNum].size()==0)
				{
					prevalue=0;
					preprevalue=0;
				}
				else if (curveMap[curNum].size()==1)
				{
					prevalue=curveMap[curNum][0].ry();
					preprevalue=0;
				}
				else
				{
					prevalue=curveMap[curNum][curveMap[curNum].size()-1].ry();
					preprevalue=curveMap[curNum][curveMap[curNum].size()-2].ry();
				}
				valuePredict = 2*prevalue-preprevalue;
				unsigned short temp = data[index++];
				temp |= ((unsigned short)data[index++]<<8);
				//temp = (temp+valuePredict)/2;
				curveMap[curNum].enqueue(QPointF(0,(signed short)kalmanFilter(curNum,temp)));
				//curveMap[curNum].enqueue(QPointF(0,(signed short)Kalman(curNum,temp)));
				//curveMap[curNum].enqueue(QPointF(0,(signed short)temp));
			}
			#if 0
			if(dataBuffer.size()==MAX_BUFFER_SIZE)
				dataBuffer.dequeue();
			dataBuffer.enqueue(QPointF(0,(signed short)(data[index++]|data[index]<<8)));
			#endif
		}
		for (index=0; index<curveMap[0].size(); ++index)
		{
			for (quint32 curNum=0; curNum<MAX_CURVE_NUM; ++curNum)
			{
				curveMap[curNum][index].setX(index);
			}
			//dataBuffer[index].setX(index);
		}
	}

	emit updatePlotWindow(curveNameComboBox->currentIndex(), &curveMap);
}

#define MAX_SIGNAL_NUM  6
#define MEASURE_DELTA2  100.0f
#define GAMA_SHRESHOLD  3.0f
unsigned short X[MAX_SIGNAL_NUM]={0};
unsigned short Y[MAX_SIGNAL_NUM]={0};
float signalDelta2[MAX_SIGNAL_NUM]={MEASURE_DELTA2};
float measureDelta2[MAX_SIGNAL_NUM]={MEASURE_DELTA2};
float Delta2[MAX_SIGNAL_NUM]={MEASURE_DELTA2};
float K[MAX_SIGNAL_NUM]={2.0f/3.0f};

unsigned short ComPort::kalmanFilter(short index, unsigned short value)
{
	unsigned short temp=0;
	unsigned short A;
	K[index] = (Delta2[index]+signalDelta2[index])/(Delta2[index]+signalDelta2[index]+MEASURE_DELTA2);

	temp = Y[index]+K[index]*(value-Y[index]);

	if((value-Y[index])/sqrt(measureDelta2[index]) > GAMA_SHRESHOLD)
	{
		signalDelta2[index] = measureDelta2[index];
		Delta2[index] = measureDelta2[index];
	}
	else
	{
		signalDelta2[index] = K[index]*measureDelta2[index];
		Delta2[index] = (1-K[index])*measureDelta2[index]+signalDelta2[index];
	}

	if(value || X[index])
	{
		A = 2*value*(X[index])/(value*value+X[index]*X[index]);
		X[index] = value;
	}
	//measureDelta2[index] = (value-A*Y[index])*(value-A*Y[index]);

	return Y[index]=temp;
}

#define Q 10
#define R 10
float P[MAX_CURVE_NUM]={10};
unsigned short ComPort::Kalman(short index, unsigned short value)
{
	float K=0;
	P[index] += Q;
	K = P[index]/(P[index]+R);
	P[index] = (1-K)*P[index];

	return Y[index]=Y[index]+K*(value-Y[index]);
}