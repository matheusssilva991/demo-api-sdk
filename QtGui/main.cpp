#include "QtGui.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{

	qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
    QApplication a(argc, argv);
    
    QtGui w;
	w.setWindowTitle("Auto Detector DTi 1412i");
	w.setWindowIcon(QIcon(":/Assets/assets/x-ray.png"));
    w.show();
    return a.exec();
}
