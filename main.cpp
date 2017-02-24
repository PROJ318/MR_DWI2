#include <QApplication>
#include "mainwindow.h"
#include "qresource.h"

int main( int argc, char** argv )
{
  // QT Stuff
	//Q_INIT_RESOURCE(style);
    QApplication app( argc, argv );

    MainWindow myUI;
	myUI.show();

    return app.exec();
}
