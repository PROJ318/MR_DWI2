#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "DicomModule.h"
#include "displayport.h"

#include <QMessageBox>
#include <QStyleFactory>
#include <qfile.h>
#include <qdebug.h>
#include <qstring.h>


//VTK lib
#include <vtkDataObjectToTable.h>
#include <vtkElevationFilter.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkImageActor.h>
#include <vtkCamera.h>
#include <vtkCornerAnnotation.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkImageMapper3D.h>
#include <vtkImageProperty.h>
#include <vtkImageMapToColors.h>
#include <vtkLookupTable.h>
#include <vtkImageData.h>
#include <vtkImageIterator.h>
#include <vtkImageViewer2.h>
#include <vtkPicker.h>
#include <vtkAssemblyPath.h>
#include <vtkContourWidget.h>
#include <vtkMedicalImageProperties.h>
#include <vtkWindowToImageFilter.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkVectorText.h>
#include <vtkStringArray.h>
#include <vtkRendererCollection.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkPNGWriter.h>
#include <vtkExtractVOI.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageAppendComponents.h>
#include <vtkScalarBarActor.h>
#include <vtkQtTableView.h>
#include <QVTKWidget.h>
#include <QVTKInteractor.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
	//Initialize();
	//CreateQtPartControl(this);
	DicomUI = new DicomModule(this);
	DicomUI->hide();

	//Set Window Style

	//QFile file(":/dark.qss");
	//file.open(QFile::ReadOnly);
	//QString styleSheet = QLatin1String(file.readAll());
	//setStyleSheet(styleSheet);
	this->ui = new Ui::MainWindow;
	this->ui->setupUi(this);
	showMaximized();

	displayLayout = new DisplayPort;
	this->ui->ViewArea->setLayout(displayLayout);
	
	//ui->toolLine->setPalette(framePalette);
	//ui->toolLine->setAutoFillBackground(true); // set dockwidget as trasparent floating
	//ui->dockWidget->setWindowFlags(Qt::FramelessWindowHint);
	//ui->dockWidget->setFloating(true);
	//QApplication::setStyle(QStyleFactory::create("fusion"));

	std::cout << "Style Set OK" << std::endl;
	connect(ui->FileButton, SIGNAL(clicked()), this, SLOT(onStartdicom()));
	connect(DicomUI, SIGNAL(SignalDicomRead(QStringList)), ui->diffusionModule, SLOT(OnImageFilesLoaded(QStringList)));
	connect(ui->diffusionModule, SIGNAL(SignalDicomLoaded(bool)), this, SLOT(onDicomLoaded(bool)));

	connect(ui->diffusionModule, SIGNAL(SignalTestButtonFired(bool , vtkSmartPointer <vtkImageData>, QString,  float,  float)), 
		this, SLOT(onProcButtonClicked(bool, vtkSmartPointer <vtkImageData>, const QString, const float, const float)));
	//connect(DicomUI, SIGNAL(SignalDicomToDataManager(QStringList)), ui->diffusionModule, SLOT(OnImageFilesLoaded(QStringList)));
	//connect(ui->DicomUI, SIGNAL(SignalStartDicomImport(QStringList)), ui->internalDataWidget, SLOT(OnStartDicomImport(QStringList)));
}

void MainWindow::onStartdicom()
{
	//std::cout << "button clicked" << std::endl;
	DicomUI->setWindowFlags(Qt::Window);
	DicomUI->setWindowTitle(tr("DICOM BROWSER"));
	DicomUI->resize(1600, 1000);
	DicomUI->show();
	
    //connect();
}

void MainWindow::onDicomLoaded(bool isLoaded)
{
	//std::cout << "received SignalDicomLoaded" << std::endl;
	if (isLoaded)
	{
		DicomUI->hide();
	}
	
}

void MainWindow::onProcButtonClicked(bool addOrRemove, vtkSmartPointer <vtkImageData> data, const QString imageName, const float scale, const float slope)
{	
	if (addOrRemove) //true:add, false:remove
	{
		QWidget *wdwItem(this->displayLayout->getWindow(imageName));
		QVTKWidget* vtkWindow;
		if (wdwItem != NULL)
		{
			qDebug() << imageName << " window exists";
			vtkWindow = static_cast <QVTKWidget*> (wdwItem);
		}
		else{
			vtkWindow = new QVTKWidget;
			qDebug() << imageName << " window not exists, creating window";
			this->displayLayout->insertWindow(vtkWindow, imageName);
			//std::cout << "... window created" << std::endl;
		}		

		if (ScalingParameters.contains(imageName + '_s'))
		{
			ScalingParameters.remove(imageName + '_s');
			ScalingParameters.remove(imageName + '_k');
		}
		ScalingParameters.insert(imageName + '_s', scale);
		ScalingParameters.insert(imageName + '_k', slope);
		ExtenImageViewer2D(data, vtkWindow, imageName.toStdString());
	}
	else
	{
		this->displayLayout->removeWindow(imageName);
	}
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ExtenImageViewer2D(vtkSmartPointer <vtkImageData> imageData, QVTKWidget *qvtkWidget, std::string imageLabel)
{
	cout << "quantitative image value: " << imageData->GetScalarComponentAsFloat(60, 60, 0, 0) << endl;
	cout << "quantitative: " << imageData->GetOrigin()[0] << imageData->GetOrigin()[1] << endl;
	if (qvtkWidget->GetRenderWindow()->GetInteractor())
	{
		qvtkWidget->GetRenderWindow()->Finalize();
		qvtkWidget->GetRenderWindow()->GetInteractor()->ExitCallback();
	}
	double *imageDataRange = new double[2];
	imageDataRange = imageData->GetScalarRange();//Replace with to be displayed

	double colorWindow, colorLevel;
	if (imageData->GetNumberOfScalarComponents() == 3)
	{
		//color map 
		colorWindow = 255.0;
		colorLevel = 127.5;

	}
	else
	{
		double *imageDataRange = new double[2];
		imageDataRange = imageData->GetScalarRange();//Replace with to be displayed
		colorWindow = imageDataRange[1] - imageDataRange[2];
		colorLevel = 0.5* (imageDataRange[1] + imageDataRange[0]);
	}

	/*vtkSmartPointer<vtkResliceImageViewer> imageViewer = vtkSmartPointer<vtkResliceImageViewer>::New();*/
	vtkSmartPointer<vtkImageViewer2> imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
	imageViewer->SetInputData(imageData);
	imageViewer->SetSliceOrientationToXY();
	imageViewer->SetColorWindow(colorWindow);
	imageViewer->SetColorLevel(colorLevel);

	vtkSmartPointer<vtkTextProperty> sliceTextProp = vtkSmartPointer<vtkTextProperty>::New();
	sliceTextProp->SetFontFamilyToCourier();
	sliceTextProp->SetFontSize(18);
	sliceTextProp->SetVerticalJustificationToBottom();
	sliceTextProp->SetJustificationToLeft();

	vtkSmartPointer<vtkTextMapper> imageLabelMapper = vtkSmartPointer<vtkTextMapper>::New();
	//std::string msg = StatusMessage::Format(imageViewer->GetSliceMin(), imageViewer->GetSliceMax());
	imageLabelMapper->SetInput(imageLabel.c_str());
	imageLabelMapper->SetTextProperty(sliceTextProp);

	vtkSmartPointer<vtkActor2D>  imageLabelActor = vtkSmartPointer<vtkActor2D>::New();
	imageLabelActor->SetMapper(imageLabelMapper);
	imageLabelActor->SetPosition(0, 10);

	imageViewer->GetRenderer()->AddActor2D(imageLabelActor);



	qvtkWidget->SetRenderWindow(imageViewer->GetRenderWindow());



	imageViewer->GetRenderer()->ResetCamera(); //Reset camera and then render is better
	//vtkSmartPointer<vtkCamera> camera = imageViewer->GetRenderer()->GetActiveCamera();
	//this->SetImageFillWindow(camera, imageData, qvtkWidget->width(), qvtkWidget->height());

	qvtkWidget->GetRenderWindow()->Render();
	qvtkWidget->show();//qvtkWidget->show() changes window size!!!!
	//std::cout << "QVTKWIDEGT SIZE after show= " << qvtkWidget->width() << "-" << qvtkWidget->height() << std::endl;
}

