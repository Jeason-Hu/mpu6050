#include <QApplication>
#include "com.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	ComPort *myCom = new ComPort;
	myCom->show();

	return app.exec();
}