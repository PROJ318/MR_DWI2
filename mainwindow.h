#ifndef MAINWINDOW_H
#define MAINWINDOW_H


//QT lib
//#include <QHash>
//#include <QString>
//#include <QStringList>
//#include <QProgressDialog>
//#include <QVariant>
//#include <QWidget>
//#include <QMessageBox>
//#include <qdebug.h>
#include <QMainWindow>

//VTK lib
#include <vtkSmartPointer.h>
#include <vtkObject.h>

namespace Ui {
class MainWindow;
}

class Ui_MainWindow;
class DicomModule;
class vtkImageData;
class QVTKWidget;
class DisplayPort;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	/**
	* \brief CreateQtPartControl(QWidget *parent) sets the view objects from ui_QmitkDicomExternalDataWidgetControls.h.
	*
	* \param parent is a pointer to the parent widget
	*/
	//virtual void CreateQtPartControl(QWidget *parent);

	/**
	* \brief Initializes the widget. This method has to be called before widget can start.
	*/
	void Initialize();

signals:


	public slots :


	protected slots :
	void onStartdicom();
	void onDicomLoaded(bool);
	void onProcButtonClicked(bool, vtkSmartPointer <vtkImageData>, const QString, const float, const float);

protected:
	void ExtenImageViewer2D(vtkSmartPointer <vtkImageData> imagedata, QVTKWidget *qvtkWidget, std::string imageLabel);

private:

	QHash < const QString, float >  ScalingParameters;
	Ui::MainWindow *ui;
	DicomModule * DicomUI;
	DisplayPort* displayLayout;
};

#endif // MAINWINDOW_H
