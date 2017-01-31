
//Internal include
#include <vtkRoiInteractor.h>
#include <vtkUDInteractorStyleImage.h>
#include <DicomHelper.h>
#include "displayport.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "DicomModule.h"
#include "displayport.h"

//QT include
#include <QMessageBox>
#include <QStyleFactory>
#include <qfile.h>
#include <qdebug.h>
#include <qstring.h>


//VTK include
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

#define MAXTHRESHHOLD 3
#define MAXBVALUE 2000

bool cmp(std::pair<float, int> p1, std::pair<float, int> p2)
{
	if (p1.first < p2.first) return 1;
	return 0;

}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
	//Initialize();
	//CreateQtPartControl(this);

	this->m_DicomHelper = NULL;
	this->sourceImage = vtkSmartPointer<vtkImageData>::New();//VTK image pointer

	m_SourceImageCurrentSlice = 0;
	//m_QuantitativeImageCurrentSlice = 0;
	this->m_MaskThreshold = MAXTHRESHHOLD;
	this->m_ComputedBValue = MAXBVALUE;

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
	//std::cout << "Style Set OK" << std::endl;

	connect(ui->FileButton, SIGNAL(clicked()), this, SLOT(onStartdicom()));
	connect(DicomUI, SIGNAL(SignalDicomRead(QStringList)), this, SLOT(OnImageFilesLoaded(QStringList)));
	connect(ui->diffusionModule, SIGNAL(SignalTestButtonFired(bool , vtkSmartPointer <vtkImageData>, QString,  float,  float)), 
		this, SLOT(onProcButtonClicked(bool, vtkSmartPointer <vtkImageData>, const QString, const float, const float)));
	connect(this, SIGNAL(SignalSetSourceImage(DicomHelper*,int)), ui->diffusionModule, SLOT(onSetSourceImage(DicomHelper*,int)));
	connect(this, SIGNAL(SignalRecalcAll(int)), ui->diffusionModule, SLOT(onRecalcAll(int)));
	//connect(ui->diffusionModule, SIGNAL(SignalDicomLoaded(bool)), this, SLOT(onDicomLoaded(bool)));
	//connect(DicomUI, SIGNAL(SignalDicomToDataManager(QStringList)), ui->diffusionModule, SLOT(OnImageFilesLoaded(QStringList)));
	//connect(ui->DicomUI, SIGNAL(SignalStartDicomImport(QStringList)), ui->internalDataWidget, SLOT(OnStartDicomImport(QStringList)));

	////Connect Segment buttons
	////connect(m_Controls->pointer1, SIGNAL(toggled(bool)), this, SLOT(onRoiPointer(bool)));
	//connect(m_Controls->pointer1, SIGNAL(pressed()), this, SLOT(addROI()));
	//connect(m_Controls->pointer2, SIGNAL(toggled(bool)), this, SLOT(onCursorPickValue(bool)));
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
		ImageViewer2D(data, vtkWindow, imageName.toStdString());
	}
	else
	{
		this->displayLayout->removeWindow(imageName);
	}
}

MainWindow::~MainWindow()
{
	this->sourceImage->Delete();
	this->sourceImage = NULL;

	this->m_DicomHelper = NULL;

    delete ui;
}

void MainWindow::OnImageFilesLoaded(const QStringList& fileLists)
{
	//load data
	vtkStringArray* loadingFiles = vtkStringArray::New();
	loadingFiles->SetNumberOfValues(fileLists.size());
	for (int i = 0; i < fileLists.size(); i++)
	{
		loadingFiles->SetValue(i, fileLists.at(i).toStdString().c_str());
	}
	this->m_DicomHelper = new DicomHelper(loadingFiles);



	if (m_DicomHelper->numberOfBValue < 2)
		//for none diffusion images
		this->sourceImage = this->m_DicomHelper->DicomReader->GetOutput();
	else
	{
		//for Diffusion images.
		this->SortingSourceImage();
		//this->UpdateMaskVectorImage();
	}

	const QString orgLabel("Source"); //TO_DO: using serie name/protocol name instead

	QWidget *orgItem(this->displayLayout->getWindow(orgLabel));
	QVTKWidget *vtkWindow;
	if (orgItem != NULL)
	{
		std::cout << "Original Image existed " << std::endl;
		vtkWindow = static_cast <QVTKWidget*>(orgItem);



		m_SourceImageCurrentSlice = 0;
		this->m_MaskThreshold = MAXTHRESHHOLD;
		this->m_ComputedBValue = MAXBVALUE;

		//m_QuantitativeImageCurrentSlice = 0;
		//this->m_DicomHelper = nullptr;
	}
	else{
		vtkWindow = new QVTKWidget;
		this->displayLayout->insertWindow(vtkWindow, orgLabel);
	}

	//emit SignalDicomLoaded(true);
	DicomUI->hide();
	//std::cout << "srcimage viewer" << std::endl;
	DisplayDicomInfo(this->sourceImage);
	ImageViewer2D(this->sourceImage, vtkWindow, orgLabel.toStdString());

	emit SignalSetSourceImage(m_DicomHelper, m_SourceImageCurrentSlice);
}

void MainWindow::SortingSourceImage()
{
	//sorting b value list from small to larger using vector Pair sort.
	//which can get the sorted index array. it can be used for the 
	//source image sorting.
	std::vector< std::pair<float, int> > vectorPair;
	std::vector<int> bValueOrderIndex(m_DicomHelper->BvalueList.size());
	cout << "b values lise:" << m_DicomHelper->BvalueList.size() << endl;
	for (int i = 0; i < m_DicomHelper->BvalueList.size(); i++)
	{
		bValueOrderIndex[i] = i;
		vectorPair.push_back(std::make_pair(m_DicomHelper->BvalueList[i], bValueOrderIndex[i]));
	}
	std::stable_sort(vectorPair.begin(), vectorPair.end(), cmp);
	for (int i = 0; i < m_DicomHelper->BvalueList.size(); i++)
	{
		cout << "b value:" << vectorPair[i].first << "index:" << vectorPair[i].second << endl;
		m_DicomHelper->BvalueList[i] = vectorPair[i].first;
		bValueOrderIndex[i] = vectorPair[i].second;
	}


	//allocate memory for sourceImage based on the dicom output data.
	this->sourceImage->SetDimensions(this->m_DicomHelper->imageDimensions);
	this->sourceImage->SetSpacing(this->m_DicomHelper->DicomReader->GetOutput()->GetSpacing());
	this->sourceImage->SetOrigin(this->m_DicomHelper->DicomReader->GetOutput()->GetOrigin());
	this->sourceImage->AllocateScalars(VTK_UNSIGNED_SHORT, this->m_DicomHelper->numberOfComponents);

	// Sorting the output data based on the b Value order
	int* dims = this->m_DicomHelper->imageDimensions;
	for (int z = 0; z<dims[2]; z++)
	{
		for (int y = 0; y<dims[1]; y++)
		{
			for (int x = 0; x<dims[0]; x++)
			{
				int numOfGradDir = this->m_DicomHelper->numberOfGradDirection;
				if (this->m_DicomHelper->IsoImageLabel > -1) numOfGradDir += 1;
				SourceImagePixelType *dicomPtr = static_cast<SourceImagePixelType *>(this->m_DicomHelper->DicomReader->GetOutput()->GetScalarPointer(x, y, z));
				SourceImagePixelType *sourcePtr = static_cast<SourceImagePixelType *>(this->sourceImage->GetScalarPointer(x, y, z));
				sourcePtr[0] = dicomPtr[bValueOrderIndex[0] * numOfGradDir];

				//sorting based on the b value order
				int sourceCmpIndex = 1;
				for (int cmp = 1; cmp < this->m_DicomHelper->DicomReader->GetNumberOfScalarComponents(); cmp++)
				{
					//get the b value index for current component
					int bValueIndex = (cmp - 1) / numOfGradDir + 1;
					//get the grad direction indec for current component
					int gradDirIndex = cmp - 1 - numOfGradDir*(bValueIndex - 1);
					//calculate the corrsponding component index in the dicom output data
					int dicomCmpIndex;
					if (bValueOrderIndex[bValueIndex] < bValueOrderIndex[0])
						dicomCmpIndex = (bValueOrderIndex[bValueIndex]) * numOfGradDir + gradDirIndex;
					else
						dicomCmpIndex = (bValueOrderIndex[bValueIndex] - 1) * numOfGradDir + gradDirIndex + 1;
					// remove the Isotropic direction for DTI
					if (this->m_DicomHelper->IsoImageLabel != dicomCmpIndex)
						sourcePtr[sourceCmpIndex++] = dicomPtr[dicomCmpIndex];
				}
			}
		}
	}
}

void MainWindow::DisplayDicomInfo(vtkSmartPointer <vtkImageData> imageData)
{

	const int dataDim = imageData->GetDataDimension();

	int dims[3];
	double origins[3];
	double spacing[3];
	int extent[6];

	double range[2];
	imageData->GetDimensions(dims);
	qDebug() << "image dims: " << dims[0] << "x" << dims[1] << "x" << dims[2] << endl;
	imageData->GetOrigin(origins);
	qDebug() << "image origins: " << origins[0] << " " << origins[1] << " " << origins[2] << endl;
	imageData->GetSpacing(spacing);
	qDebug() << "image spacing: " << spacing[0] << "x" << spacing[1] << "x" << spacing[2] << endl;
	imageData->GetExtent(extent);
	qDebug() << "extent: " << extent[0] << "x" << extent[1] << "x" << extent[2] << "x" << extent[3] << "x" << extent[4] << "x" << extent[5] << endl;
	imageData->GetScalarRange(range);
	qDebug() << "range: " << range[0] << "x" << range[1] << endl;

	vtkMedicalImageProperties* properties = m_DicomHelper->DicomReader->GetMedicalImageProperties();
	QString imageInfo(tr("Patient Name : "));
	imageInfo.append(QLatin1String(properties->GetPatientName()));
	imageInfo.append("\n");
	imageInfo.append(tr("Study Name: "));
	imageInfo.append(QLatin1String(properties->GetStudyDescription()));
	imageInfo.append("\n");
	imageInfo.append(tr("Scan Name: "));
	imageInfo.append(QLatin1String(properties->GetSeriesDescription()));
	imageInfo.append("\n");
	imageInfo.append(tr("Scan Date: "));
	imageInfo.append(QLatin1String(properties->GetAcquisitionDate()));
	imageInfo.append("\n");

	imageInfo.append(tr("Image Dimension: [ %1 : %2 : %3 ]\n").arg(dims[0]).arg(dims[1]).arg(dims[2]));
	imageInfo.append(tr("Number Of b Value : %1 \n").arg(m_DicomHelper->numberOfBValue));
	imageInfo.append(tr("Number Of Gradient Direction : %1 \n").arg(m_DicomHelper->numberOfGradDirection));
	ui->infoBrowser->setText(imageInfo);
}

void MainWindow::ImageViewer2D(vtkSmartPointer <vtkImageData> imageData, QVTKWidget *qvtkWidget, std::string imageLabel)
{
	//Is this necessary?
	/*if (qvtkWidget->GetRenderWindow()->GetInteractor())
	{
	qvtkWidget->GetRenderWindow()->Finalize();
	qvtkWidget->GetRenderWindow()->GetInteractor()->ExitCallback();
	}*/

	//Done inside interacterstyle rather than here, so as to stay the same as in Source image viewer
	//double *imageDataRange = new double[2];
	//imageDataRange = imageData->GetScalarRange();//Replace with to be displayed
	//
	//double colorWindow, colorLevel;
	//if (imageData->GetNumberOfScalarComponents() == 3)
	//{
	//	//color map 
	//	colorWindow = 255.0;
	//	colorLevel = 127.5;

	//}
	//else
	//{
	//	double *imageDataRange = new double[2];
	//	imageDataRange = imageData->GetScalarRange();//Replace with to be displayed
	//	colorWindow = imageDataRange[1] - imageDataRange[2];
	//	colorLevel = 0.5* (imageDataRange[1] + imageDataRange[0]);
	//}

	vtkSmartPointer<vtkImageViewer2> imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
	imageViewer->SetInputData(imageData);
	imageViewer->SetSliceOrientationToXY();
	//imageViewer->SetColorWindow(colorWindow);
	//imageViewer->SetColorLevel(colorLevel);

	vtkSmartPointer<vtkTextProperty> sliceTextProp = vtkSmartPointer<vtkTextProperty>::New();
	sliceTextProp->SetFontFamilyToCourier();
	sliceTextProp->SetFontSize(18);
	sliceTextProp->SetVerticalJustificationToBottom();
	sliceTextProp->SetJustificationToLeft();

	vtkSmartPointer<vtkTextMapper> sliceTextMapper = vtkSmartPointer<vtkTextMapper>::New();
	std::string msg = imageLabel.compare("Source") == 0 ? StatusMessage::Format(imageViewer->GetSliceMin(), imageViewer->GetSliceMax()) : imageLabel;
	sliceTextMapper->SetInput(msg.c_str());
	sliceTextMapper->SetTextProperty(sliceTextProp);

	vtkSmartPointer<vtkActor2D> sliceTextActor = vtkSmartPointer<vtkActor2D>::New();
	sliceTextActor->SetMapper(sliceTextMapper);
	sliceTextActor->SetPosition(15, 10);

	/*vtkSmartPointer<vtkTextMapper> imageLabelMapper = vtkSmartPointer<vtkTextMapper>::New();
	std::string msg = imageLabel.compare("Source") == 0 ? StatusMessage::Format(imageViewer->GetSliceMin(), imageViewer->GetSliceMax()) : imageLabel;
	imageLabelMapper->SetInput(msg.c_str());
	imageLabelMapper->SetTextProperty(sliceTextProp);

	vtkSmartPointer<vtkActor2D>  imageLabelActor = vtkSmartPointer<vtkActor2D>::New();
	imageLabelActor->SetMapper(imageLabelMapper);
	imageLabelActor->SetPosition(0, 10);*/

	imageViewer->GetRenderer()->AddActor2D(sliceTextActor);

	//vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//Use QVTKInteractor rather than vtkRenderWindowInteractor!!! So that interactor start and end events are handled by qApp->exec() and qApp->Exit();
	vtkSmartPointer<QVTKInteractor> renderWindowInteractor = vtkSmartPointer<QVTKInteractor>::New();
	vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle = vtkSmartPointer<myVtkInteractorStyleImage>::New();

	myInteractorStyle->SetImageViewer(imageViewer);
	myInteractorStyle->SetStatusMapper(sliceTextMapper);
	myInteractorStyle->GetCurrentSliceNumber(m_SourceImageCurrentSlice);

	renderWindowInteractor->SetInteractorStyle(myInteractorStyle);
	renderWindowInteractor->AddObserver(vtkCommand::MouseWheelForwardEvent, this, &MainWindow::ShareWindowEvent);
	renderWindowInteractor->AddObserver(vtkCommand::MouseWheelBackwardEvent, this, &MainWindow::ShareWindowEvent);
	//imageViewer->GetRenderWindow()->SetSize(qvtkWidget->width(),qvtkWidget->height());
	//imageViewer->GetRenderer()->SetBackground(0.2, 0.3, 0.4);
	qvtkWidget->SetRenderWindow(imageViewer->GetRenderWindow());
	//qvtkWidget->GetRenderWindow()->vtkRenderWindow::SetSize(qvtkWidget->width(), qvtkWidget->height());
	//std::cout << "QVTKWIDEGT SIZE = " << * (qvtkWidget->GetRenderWindow()->GetSize()) << std::endl;
	//std::cout << "ImageViewer SIZE = " << *(imageViewer->GetSize()) << std::endl;
	//std::cout << "QVTKWIDEGT SIZE = " << qvtkWidget->width()<< "-" << qvtkWidget->height() << std::endl;
	//qvtkWidget->GetRenderWindow()->vtkRenderWindow::SetSize(800, 800);
	//qvtkWidget->GetRenderWindow()->vtkRenderWindow::SetPosition(qvtkWidget->x(), qvtkWidget->y());
	qvtkWidget->GetRenderWindow()->SetInteractor(renderWindowInteractor);//crutial to let qvtkWidget share the same interactor with imageViewer
	//std::cout << "QVTKWIDEGT SIZE after interactor= " << qvtkWidget->width() << "-" << qvtkWidget->height() << std::endl;
	//imageViewer->SetupInteractor(renderWindowInteractor);

	imageViewer->GetRenderer()->ResetCamera(); //Reset camera and then render is better
	vtkSmartPointer<vtkCamera> camera = imageViewer->GetRenderer()->GetActiveCamera();
	this->SetImageFillWindow(camera, imageData, qvtkWidget->width(), qvtkWidget->height());
	imageViewer->GetRenderer()->SetActiveCamera(camera);
	//imageViewer->GetRenderer()->SetBackground(0.2, 0.3, 0.4);
	qvtkWidget->GetRenderWindow()->Render();
	renderWindowInteractor->Initialize();
	qvtkWidget->show();//qvtkWidget->show() changes window size!!!!
	//std::cout << "QVTKWIDEGT SIZE after show= " << qvtkWidget->width() << "-" << qvtkWidget->height() << std::endl;
}

void MainWindow::ShareWindowEvent()
{
	std::cout << "[ShareWindowEvent] Sending recalculate signal" << std::endl;
	emit SignalRecalcAll(m_SourceImageCurrentSlice);
}

void MainWindow::IVIMImageViewer(vtkSmartPointer <vtkImageData> imageData, QVTKWidget *qvtkWidget, int imageIdx)
{
	vtkSmartPointer <vtkLookupTable> lookupTable = vtkSmartPointer <vtkLookupTable>::New();
	lookupTable->SetNumberOfTableValues(256);//try below color range
	lookupTable->SetTableRange(0.1, 255.1);//try below color range
	lookupTable->SetHueRange(0.66667, 0.0);//rainbow color map: from blue to red
	lookupTable->UseBelowRangeColorOn();
	lookupTable->SetBelowRangeColor(0.0, 0.0, 0.0, 1.0);
	lookupTable->Build();

	//vtkSmartPointer<vtkColorTransferFunction> lookupTable = vtkSmartPointer<vtkColorTransferFunction>::New();
	//lookupTable->SetColorSpaceToRGB();
	//lookupTable->AddRGBPoint(0,0.0,0.0,1.0);
	//lookupTable->AddRGBPoint(255, 1.0, 0.0, 0.0);
	//lookupTable->SetScaleToLinear();
	vtkSmartPointer <vtkImageExtractComponents> scalarComponent = vtkSmartPointer <vtkImageExtractComponents>::New();
	scalarComponent->SetInputData(imageData);
	scalarComponent->SetComponents(imageIdx);
	scalarComponent->Update();

	vtkSmartPointer <vtkImageMapToColors> scalarValueToColors = vtkSmartPointer <vtkImageMapToColors>::New();
	scalarValueToColors->SetLookupTable(lookupTable);
	scalarValueToColors->PassAlphaToOutputOn();
	scalarValueToColors->SetInputData(scalarComponent->GetOutput());

	vtkSmartPointer <vtkImageActor> imageActor = vtkSmartPointer <vtkImageActor>::New();
	imageActor->GetMapper()->SetInputConnection(scalarValueToColors->GetOutputPort());
	imageActor->GetProperty()->SetInterpolationTypeToNearest();

	//imageActor->GetProperty()->SetAmbient(1);
	//imageActor->GetProperty()->SetDiffuse(0);

	vtkSmartPointer <vtkScalarBarActor> scalarBar = vtkSmartPointer <vtkScalarBarActor>::New();
	scalarBar->SetLookupTable(lookupTable);
	//scalarBar->SetTitle("Title");
	//scalarBar->SetNumberOfLabels(4);//Default is 5
	scalarBar->SetDrawTickLabels(0);//Disable labels

	//Adjust scalarBar positon according to image Actor
	//double imageActorPos[3];
	//imageActor->GetPosition(imageActorPos);
	//double scalarBarPos[1];
	//scalarBar->GetPosition();
	//imageActor->GetMaxXBound();
	//std::cout << "imageActor pos = " << imageActorPos[0] << " " << imageActorPos[1] << " " << imageActorPos[2] << std::endl;
	//std::cout << "imageActor xBound = " << imageActor->GetMaxXBound() << " " << imageActor->GetMinXBound() << std::endl;
	//std::cout << "imageActor yBound = " << imageActor->GetMaxYBound() << " " << imageActor->GetMinYBound() << std::endl;

	//std::cout << "scalarBar default pos = " << scalarBar->GetPosition()[0] << " " << scalarBar->GetPosition()[1] << std::endl;
	//std::cout << "scalarBar default height width = " << scalarBar->GetHeight() << " " << scalarBar->GetWidth() << std::endl;
	//std::cout << "QVTKWIDEGT SIZE BEFORE show= " << qvtkWidget->width() << "-" << qvtkWidget->height() << std::endl;

	//std::cout << "imageActor xBound = " << imageActor->GetMaxXBound() << " " << imageActor->GetMinXBound() << std::endl;
	//imageActor->get//GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
	//scalarBar->GetPositionCoordinate()->SetCoordinateSystemToWorld();
	//scalarBar->GetPositionCoordinate()->SetValue(0.9 * imageActor->GetMaxXBound(), 0.8 * imageActor->GetMaxXBound());


	vtkSmartPointer <vtkRenderer> renderer = vtkSmartPointer <vtkRenderer>::New();
	renderer->AddActor2D(scalarBar);
	renderer->AddActor(imageActor);
	renderer->ResetCamera();


	//renderer->SetErase(1);
	//renderer->GradientBackgroundOn();
	//renderer->SetBackground2();
	renderer->SetBackground(0.0, 0.0, 0.0);

	vtkSmartPointer <vtkRenderWindow> renderWindow = vtkSmartPointer <vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	//renderWindow->SetBorders(1);
	//std::cout << "renderer size = " << renderer->GetSize()[0] << " " << renderer->GetSize()[1] << std::endl;
	vtkSmartPointer <QVTKInteractor> interactor = vtkSmartPointer <QVTKInteractor>::New();

	vtkSmartPointer <vtkInteractorStyleImage> interactorStyle = vtkSmartPointer <vtkInteractorStyleImage>::New();

	interactor->SetInteractorStyle(interactorStyle);

	qvtkWidget->SetRenderWindow(renderWindow);
	qvtkWidget->GetRenderWindow()->SetInteractor(interactor);


	renderer->ResetCamera();//Reset camera and then render is better
	vtkSmartPointer<vtkCamera> camera = renderer->GetActiveCamera();
	this->SetImageFillWindow(camera, scalarComponent->GetOutput(), qvtkWidget->width(), qvtkWidget->height());
	//imageViewer->GetRenderer()->SetActiveCamera(camera);
	//imageViewer->GetRenderer()->SetBackground(0.2, 0.3, 0.4);
	qvtkWidget->GetRenderWindow()->Render();
	interactor->Initialize();
	qvtkWidget->show();
	//std::cout << "QVTKWIDEGT SIZE AFTER show= " << qvtkWidget->width() << "-" << qvtkWidget->height() << std::endl;

}

void MainWindow::SetImageFillWindow(vtkSmartPointer <vtkCamera> & camera, vtkSmartPointer <vtkImageData> imageData, double width, double height){

	if (!(camera || imageData)) return;

	//int dims[3];
	double origins[3];
	double spacing[3];
	int extent[6];
	//	double range[2];
	double center[3];
	//imageData->GetDimensions(dims);
	imageData->GetOrigin(origins);
	imageData->GetSpacing(spacing);
	imageData->GetExtent(extent);



	camera->ParallelProjectionOn();

	double xFov = (-extent[0] + extent[1] + 1)*spacing[0];
	double yFov = (-extent[2] + extent[3] + 1)*spacing[1];

	double screenRatio = height / width;
	double imageRatio = yFov / xFov;

	double parallelScale = imageRatio > screenRatio ? yFov : screenRatio / imageRatio*yFov;

	double focalDepth = camera->GetDistance();

	center[0] = origins[0] + spacing[0] * .5*(extent[0] + extent[1]);
	center[1] = origins[1] + spacing[1] * .5*(extent[2] + extent[3]);
	center[2] = origins[2] + spacing[2] * .5*(extent[4] + extent[5]);

	camera->SetParallelScale(0.5*parallelScale);
	//std::cout << "parallel scale minHalfFov " << minHalfFov << std::endl;
	//std::cout << "focal depth " << focalDepth << std::endl;
	//std::cout << "view angle " << camera->GetViewAngle() << std::endl;
	//std::cout << "Quantitative imageviewer, input width, input height =  " << width << " " << height << std::endl;
	//std::cout << "center " << center[0] << " " << center[1] << " " << center[2] << std::endl;
	camera->SetFocalPoint(center[0], center[1], center[2]);
	camera->SetPosition(center[0], center[1], focalDepth);
	//camera->SetViewAngle(80);
}



