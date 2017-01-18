#include <QApplication>
#include "mainwindow.h"
#include "qresource.h"

int main( int argc, char** argv )
{
  // QT Stuff
	//QResource::registerResource("resources/MR_DWI.qrc");
    QApplication app( argc, argv );

    MainWindow myUI;
	myUI.show();

    return app.exec();
}
