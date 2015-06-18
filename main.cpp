#include <QApplication>
#include "mainwindow.h"

/*Главный класс*/

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    QApplication::setApplicationName("3D_Viewer");


	MainWindow w;
	w.show();


	return a.exec();
}
