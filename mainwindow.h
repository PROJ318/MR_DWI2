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
class vtkCamera;
class DicomHelper;

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

	void SignalSetSourceImage(DicomHelper*, int);
	void SignalRecalcAll(int);

	public slots :


	protected slots :
	void onStartdicom();
	//void onDicomLoaded(bool);
	void onProcButtonClicked(bool, vtkSmartPointer <vtkImageData>, const QString, const float, const float);
	void OnImageFilesLoaded(const QStringList& fileLists);

	/////// @brief
	/////// In this slot, change the interactor of qvtkwindows to ROI drawing.
	/////// 
	//void addROI();//bool toggle

	/////// @brief
	/////// In this slot, change the interactor of qvtkwindows to pixel picker.
	/////// 
	//void onCursorPickValue(bool _istoggled);

	/////// @brief
	/////// In this slot, change the interactor of qvtkwindows to pixel picker.
	/////// 
	//void onDisplayPickValue(vtkObject* obj, unsigned long,
	//	void* client_data, void*,
	//	vtkCommand * command);

protected:

	void DisplayDicomInfo(vtkSmartPointer <vtkImageData> imageData);
	void SortingSourceImage(); //This should be moved to DicomHelper class
	void ImageViewer2D(vtkSmartPointer <vtkImageData> imageData, QVTKWidget *qvtkWidget, std::string imageLabel);

	void IVIMImageViewer(vtkSmartPointer <vtkImageData>, QVTKWidget *qvtkWidget, int imageIdx);
	void SetImageFillWindow(vtkSmartPointer <vtkCamera> &camera, vtkSmartPointer <vtkImageData> imageData, double width, double height);
	
	void ShareWindowEvent();	

private:
	QHash < const QString, float >  ScalingParameters;
	Ui::MainWindow *ui;
	DicomModule* DicomUI;
	DicomHelper* m_DicomHelper;//initialization? 
	DisplayPort* displayLayout;

	vtkSmartPointer < vtkImageData > sourceImage;

	int sourceScalarType = 0;
	int m_SourceImageCurrentSlice;
	//int m_QuantitativeImageCurrentSlice;

	double m_MaskThreshold;
	double m_ComputedBValue;
};

#endif // MAINWINDOW_H
