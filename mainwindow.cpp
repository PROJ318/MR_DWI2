
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
#include <QStandardItemModel>
#include <QStandardItem>
#include <qfile.h>
#include <qdebug.h>
#include <qstring.h>
#include <QPainter>
#include "qevent.h"
#include <QtTest/QtTest>
#include <QInputDialog>

// CTK
#include <ctkFileDialog.h>

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
#include <vtkCornerAnnotation.h>
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
#include <vtkImageProperty.h>
#include <vtkCollection.h>
#include <vtkOrientedGlyphContourRepresentation.h>


bool cmp(std::pair<float, int> p1, std::pair<float, int> p2)
{
	if (p1.first < p2.first) return 1;
	return 0;

}
//vtkImageActor* getImageActorFromRender(vtkRenderer* inputRenderer)
//{
//	if (!inputRenderer)
//	{
//		qDebug() << "inputRenderer is NULL" << endl;
//	}
//	vtkPropCollection* actorCollection = inputRenderer->GetActors();
//	qDebug() << actorCollection->GetNumberOfItems() << "actors are found" << endl;
//	actorCollection->InitTraversal();
//	for (vtkIdType i = 0; i < actorCollection->GetNumberOfItems(); i++)
//	{
//		vtkProp* nextActor = actorCollection->GetNextProp();
//		qDebug() << "nextActor " << i << " : " << nextActor->GetClassName() << endl;
//		std::string className = nextActor->GetClassName();
//		std::string wantedClass = "vtkImageActor";
//		if (className == wantedClass)
//		{
//			qDebug() << "nextActor " << i << " is a vtkImageActor!" << endl;
//			vtkImageActor* wantActor = dynamic_cast<vtkImageActor*> (nextActor);
//			return wantActor;
//		}
//		else
//		{
//			qDebug() << "nextActor " << i << " : " << nextActor->GetClassName() << endl;
//		}
//	}
//	return NULL;
//}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
	this->m_DicomHelper = NULL;
	this->sourceImage = vtkSmartPointer<vtkImageData>::New();//VTK image pointer

	m_SourceImageCurrentSlice = 0;
	m_SourceImageMaxSlice = 0;
	m_SourceImageMinSlice = 0;

	DicomUI = new DicomModule(this);
	DicomUI->hide();

	//Set Window Style
	//QFile file(":/qdarkstyle/style.qss");
	//file.open(QFile::ReadOnly);
	//QString styleSheet = QLatin1String(file.readAll());
	//setStyleSheet(styleSheet);

	this->ui = new Ui::MainWindow;
	this->ui->setupUi(this);
	showMaximized();

	roiInfoModel = new QStandardItemModel;
	this->ui->statisBrowser->setModel(roiInfoModel);

	connect(ui->FileButton, SIGNAL(clicked()), this, SLOT(onStartdicom()));
	connect(DicomUI, SIGNAL(SignalDicomRead(QStringList)), this, SLOT(OnImageFilesLoaded(QStringList)));

	connect(ui->SaveButton, SIGNAL(clicked()), this, SLOT(onExportImage()));

	//Diffusion Module Connections
	connect(ui->diffusionModule, SIGNAL(SignalTestButtonFired(bool , vtkSmartPointer <vtkImageData>, QString,  float,  float)), 
		this, SLOT(onProcButtonClicked(bool, vtkSmartPointer <vtkImageData>, const QString, const float, const float)));
	connect(this, SIGNAL(SignalSetSourceImage(DicomHelper*,int)), ui->diffusionModule, SLOT(onSetSourceImage(DicomHelper*,int)));
	connect(this, SIGNAL(SignalRecalcAll(int)), ui->diffusionModule, SLOT(onRecalcAll(int)));

	//ImageInfo Module Connections
	connect(ui->pickingBtn, SIGNAL(toggled(bool)), this, SLOT(onCursorPickValue(bool)));

	//ROI Module Connections	
	connect(ui->newROI, SIGNAL(clicked()), this, SLOT(addROI()));
	connect(ui->statisBrowser, SIGNAL(clicked(QModelIndex )), this, SLOT(onClickTreeView(QModelIndex )));

	//View Frame Connections
	connect(ui->ViewFrame, SIGNAL(signalWheel(const QString, int, Qt::Orientation)), this, SLOT(onWheelWdw(const QString, int, Qt::Orientation)));
	connect(ui->ViewFrame, SIGNAL(signalFocusIn(const QString)), this, SLOT(onFocusWdw(const QString)));
	connect(ui->ViewFrame, SIGNAL(signalFocusIn(const QString)), ui->diffusionModule, SLOT(onSelectImage(const QString)));
	connect(ui->ViewFrame, SIGNAL(signalMouseEvent(QMouseEvent *, const QString)), this, SLOT(onBroadcastEvent(QMouseEvent *, const QString)));
	connect(ui->ViewFrame, SIGNAL(signalKeyEvent(QKeyEvent *)), this, SLOT(onKeyEvent(QKeyEvent *)));
	connect(ui->ViewFrame, SIGNAL(signalResizeEvent(const QString, const QSize, const QSize)), this, SLOT(onWdwResizeEvent(const QString, const QSize, const QSize)));

	//connect(ui->diffusionModule, SIGNAL(SignalDicomLoaded(bool)), this, SLOT(onDicomLoaded(bool)));
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

void MainWindow::onProcButtonClicked(bool addOrRemove, vtkSmartPointer <vtkImageData> data, const QString imageName, const float scale, const float slope)
{	
	if (addOrRemove) //true:add, false:remove
	{
		QWidget *wdwItem(this->ui->ViewFrame->getWindow(imageName));
		QVTKWidget* vtkWindow;
		if (wdwItem != NULL)
		{
			qDebug() << imageName << " window exists";
			vtkWindow = static_cast <QVTKWidget*> (wdwItem);
		}
		else{
			vtkWindow = new QVTKWidget;
			qDebug() << imageName << " window not exists, creating window";
			this->ui->ViewFrame->insertWindow(vtkWindow, imageName);
			//std::cout << "... window created" << std::endl;
		}		

		if (ScalingParameters.contains(imageName + QString("_s")))
		{
			ScalingParameters.remove(imageName + QString("_s"));
			ScalingParameters.remove(imageName + QString("_k"));
		}
		ScalingParameters.insert(imageName + QString("_s"), scale);
		ScalingParameters.insert(imageName + QString("_k"), slope);
		ImageViewer2D(data, vtkWindow, imageName.toStdString());
	}
	else
	{
		/*if (ActiveWdw.contains(imageName))
		{
			QWidget* focusWindow = ui->ViewFrame->getWindow(imageName);
			focusWindow->setStyleSheet("border: none;");
			ActiveWdw.removeOne(imageName);
		}*/
		ActiveWdw.removeOne(imageName);
		this->ui->ViewFrame->removeWindow(imageName);
	}
}

void MainWindow::onCalc3DButtonClicked(QString imageName)
{
	qDebug() << imageName << "3D data received" << endl;
	//image3Dstorage.insert(imageName, imageData3D);
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

	//TO_DO: delete image3Dstorage, refresh viewport. 

	m_SourceImageMaxSlice = this->sourceImage->GetDimensions()[2] - 1;
	
	const QString orgLabel("Source"); //TO_DO: using serie name/protocol name instead
	QWidget *orgItem(this->ui->ViewFrame->getWindow(orgLabel));
	QVTKWidget *vtkWindow;
	if (orgItem != NULL)
	{
		std::cout << "Original Image existed " << std::endl;
		vtkWindow = static_cast <QVTKWidget*>(orgItem);
		m_SourceImageCurrentSlice = 0;
		//m_QuantitativeImageCurrentSlice = 0;
		//this->m_DicomHelper = nullptr;
	}
	else{
		vtkWindow = new QVTKWidget;
		this->ui->ViewFrame->insertWindow(vtkWindow, orgLabel);
	}

	//emit SignalDicomLoaded(true);
	DicomUI->hide();
	//std::cout << "srcimage viewer" << std::endl;
	DisplayDicomInfo(this->sourceImage);

	//vtkSmartPointer<vtkImageData> currentSourceImage = vtkSmartPointer<vtkImageData>::New();
	//this->ComputeCurrentSourceImage(m_SourceImageCurrentSlice, currentSourceImage);
	//ImageViewer2D(currentSourceImage, vtkWindow, orgLabel.toStdString());

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
	//if (qvtkWidget->GetRenderWindow()->GetInteractor())
	//{
	//qvtkWidget->GetRenderWindow()->Finalize();
	//qvtkWidget->GetRenderWindow()->GetInteractor()->ExitCallback();
	//}
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
	std::string msg = imageLabel.compare("Source") == 0 ? StatusMessage::Format(m_SourceImageCurrentSlice, m_SourceImageMaxSlice) : imageLabel;
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

	vtkSmartPointer<vtkCornerAnnotation> cornerAnnotation =
		vtkSmartPointer<vtkCornerAnnotation>::New();
	cornerAnnotation->SetLinearFontScaleFactor(2);
	cornerAnnotation->SetNonlinearFontScaleFactor(1);
	cornerAnnotation->SetMaximumFontSize(20);
	cornerAnnotation->SetText(3, "<window>\n<level>");
	cornerAnnotation->GetTextProperty()->SetColor(0.0, 1.0, 0.0);
	imageViewer->GetRenderer()->AddViewProp(cornerAnnotation);

	//vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//Use QVTKInteractor rather than vtkRenderWindowInteractor!!! So that interactor start and end events are handled by qApp->exec() and qApp->Exit();
	vtkSmartPointer<QVTKInteractor> renderWindowInteractor = vtkSmartPointer<QVTKInteractor>::New();
	vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle = vtkSmartPointer<myVtkInteractorStyleImage>::New();

	myInteractorStyle->SetImageViewer(imageViewer);
	//myInteractorStyle->SetStatusMapper(sliceTextMapper);
	//myInteractorStyle->GetCurrentSliceNumber(m_SourceImageCurrentSlice);

	//float scalingPara[2];
	//scalingPara[0] = 1;
	//scalingPara[1] = 0;
	std::cout << "After interactor slice number is " << m_SourceImageCurrentSlice << std::endl;
	//myInteractorStyle->GetRoiInteraction()->SetInteractor(renderWindowInteractor);
	//myInteractorStyle->GetRoiInteraction()->SetImageActor();
	//myInteractorStyle->GetRoiInteraction()->initialize(imageViewer->GetImageActor(), ui->statisBrowser, renderWindowInteractor, scalingPara);
	
	renderWindowInteractor->SetInteractorStyle(myInteractorStyle);
	//renderWindowInteractor->AddObserver(vtkCommand::MouseWheelForwardEvent, this, &MainWindow::ShareWindowEvent);
	//renderWindowInteractor->AddObserver(vtkCommand::MouseWheelBackwardEvent, this, &MainWindow::ShareWindowEvent);
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
	qvtkWidget->show();

	imageViewer->GetRenderer()->ResetCamera();//Reset camera and then render is better
	vtkSmartPointer<vtkCamera> camera = imageViewer->GetRenderer()->GetActiveCamera();
	double *bounds = imageViewer->GetRenderer()->ComputeVisiblePropBounds();
	int *size = qvtkWidget->GetRenderWindow()->GetSize();
	this->ImageAutoFillWindow(camera, bounds, size);
	qvtkWidget->GetRenderWindow()->Render();
	renderWindowInteractor->Initialize();
	//qvtkWidget->show() changes window size!!!!
	//std::cout << "QVTKWIDEGT SIZE after show= " << qvtkWidget->geometry() << "-" << qvtkWidget->height() << std::endl;
}

void MainWindow::ShareWindowEvent()
{
	std::cout << "[ShareWindowEvent] Sending recalculate signal" << std::endl;
	
	//vtkSmartPointer<vtkImageData> currentSourceImage = vtkSmartPointer<vtkImageData>::New();
	//this->ComputeCurrentSourceImage(m_SourceImageCurrentSlice, currentSourceImage);
	//QWidget *wdwItem(this->ui->ViewFrame->getWindow(QString("Source")));
	//ImageViewer2D(currentSourceImage, static_cast <QVTKWidget*>(wdwItem), "Source");

	emit SignalRecalcAll(m_SourceImageCurrentSlice);

	if (RoiCollection.contains(m_SourceImageCurrentSlice))
	{
		qDebug() << RoiCollection.value(m_SourceImageCurrentSlice)->GetNumberOfItems() << "is stored in slice: " << m_SourceImageCurrentSlice << endl;

		QHash < const QString, QWidget * > currentWindows = ui->ViewFrame->getAllWindow();
		QHashIterator<const QString, QWidget * > wdwIter(currentWindows);
		while (wdwIter.hasNext()) {
			wdwIter.next();
			QVTKWidget *thisWindow = static_cast <QVTKWidget*> (wdwIter.value());
			qDebug() << "Retrieve ROI on window " << wdwIter.key() << "at slice " << m_SourceImageCurrentSlice << endl;
			RoiCollection.value(m_SourceImageCurrentSlice)->InitTraversal();
			for (vtkIdType i = 0; i < RoiCollection.value(m_SourceImageCurrentSlice)->GetNumberOfItems(); i++)
			{
				vtkSmartPointer<vtkPolyData> plydata = vtkPolyData::SafeDownCast(RoiCollection.value(m_SourceImageCurrentSlice)->GetItemAsObject(i));
				vtkRoiInteractor* RoiInterObs = new vtkRoiInteractor;
				RoiInterObs->usePolydata(thisWindow->GetRenderWindow()->GetInteractor(), plydata);
			}
		}
	}
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
	double *bounds = renderer->ComputeVisiblePropBounds();
	int *size = qvtkWidget->GetRenderWindow()->GetSize();
	this->ImageAutoFillWindow(camera, bounds, size);
	//imageViewer->GetRenderer()->SetActiveCamera(camera);
	//imageViewer->GetRenderer()->SetBackground(0.2, 0.3, 0.4);
	qvtkWidget->GetRenderWindow()->Render();
	interactor->Initialize();
	qvtkWidget->show();
	//std::cout << "QVTKWIDEGT SIZE AFTER show= " << qvtkWidget->width() << "-" << qvtkWidget->height() << std::endl;

}

void MainWindow::onCursorPickValue(bool _istoggled)
{


	//qDebug() << "it's OK, areaInROI" << roiInfoModel->item(0, 0)->text() << endl;
	if (_istoggled)
	{
		QHash < const QString, QWidget * > currentWindows = ui->ViewFrame->getAllWindow();
		QHashIterator<const QString, QWidget * > wdwIter(currentWindows);
		while (wdwIter.hasNext())
		{
			wdwIter.next();
			QVTKWidget *thisWindow = static_cast <QVTKWidget*> (wdwIter.value());
			vtkRenderWindowInteractor* thisInteractor = thisWindow->GetRenderWindow()->GetInteractor();
			if (!thisInteractor)
			{
				std::cout << "thisWindow is empty";
				return;
			}
			//qDebug() << "Connecting" << qimageLabel << ScalingParameters.value(QString("ADC_s"));;
			thisWindow->setAutomaticImageCacheEnabled(true);
			float curScalPara[2];
			curScalPara[0] = ScalingParameters.value(wdwIter.key() + QString("_s"));
			curScalPara[1] = ScalingParameters.value(wdwIter.key() + QString("_k"));
			vtkSmartPointer<vtkEventQtSlotConnect> Connections = vtkEventQtSlotConnect::New();
			//std::cout << curScalPara[0] << "-" << curScalPara[1] << " ?= ";
			Connections->Connect(thisInteractor, vtkCommand::MouseMoveEvent,
				this, SLOT(onDisplayPickValue(vtkObject*, unsigned long, void*, void*, vtkCommand*)), curScalPara);

		}
	}else{
		ShareWindowEvent();
	}

}

void MainWindow::onDisplayPickValue(vtkObject* obj, unsigned long, void* client_data, void*, vtkCommand*)
{
	//vtkRenderWindow* _renderWindow = static_cast<vtkRenderWindow*>(renderWindow);
	// get interactor
	//std::cout << client_data <<std::endl;
	vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::SafeDownCast(obj);
	if (!rwi)
	{
		qDebug() << "rwi is empty";
		return;
	}
	//std::string* _imageLabel = static_cast<std::string*>(client_data);

	//float scale[2];
	//memcpy(scale,client_data,2*sizeof(float));
	float* scale = static_cast<float*>(client_data);
	
	//vtkRenderWindowInteractor *rwi = _renderWindow->GetInteractor();

	vtkRenderer* renderer = rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
	vtkImageActor* _Actor = static_cast<myVtkInteractorStyleImage*>(rwi->GetInteractorStyle())->GetImageActor();	
	vtkPropCollection* actorCollection = renderer->GetViewProps();
	vtkCornerAnnotation* _Annotation = (vtkCornerAnnotation*)(actorCollection->GetItemAsObject(2));

	if (!_Annotation)
	{
		qDebug() << "Cannot Get Annotation" << endl;
	}

	_Actor->InterpolateOff();
	

	vtkImageData* _image = _Actor->GetMapper()->GetInput();
	//static_cast<myVtkInteractorStyleImage*>(rwi->GetInteractorStyle())->GetInputImage();

	if (!_image)
	{
		qDebug() << "image data is empty";
		return;
	}

	rwi->GetPicker()->PickFromListOn();
	rwi->GetPicker()->AddPickList(_Actor);
	rwi->GetPicker()->Pick(rwi->GetEventPosition()[0], rwi->GetEventPosition()[1],
		0.0, renderer);



	// There could be other props assigned to this picker, so 
	// make sure we picked the image actor
	vtkAbstractPropPicker *picker;
	picker = vtkAbstractPropPicker::SafeDownCast(rwi->GetPicker());
	vtkAssemblyPath* path = picker->GetPath();
	bool validPick = false;
	if (path)
	{
		vtkCollectionSimpleIterator sit;
		path->InitTraversal(sit);
		vtkAssemblyNode *node;
		for (int i = 0; i < path->GetNumberOfItems() && !validPick; ++i)
		{
			node = path->GetNextNode(sit);
			if (_Actor == vtkImageActor::SafeDownCast(node->GetViewProp()))
			{
				validPick = true;
			}
		}
	}

	//qDebug() << rwi->GetEventPosition()[0] <<"-"<< rwi->GetEventPosition()[1] << std::endl;

	if (!validPick)
	{		
		_Annotation->SetText(1, "Off Image");
		rwi->Render();
		return;
	}

	
	// Get the world coordinates of the pick
	double pos[3];
	rwi->GetPicker()->GetPickPosition(pos);

	//Debug() << pos[0] << pos[1] << std::endl;
	
	int image_coordinate[3];
	double spacing[3];
	_image->GetSpacing(spacing);

	// vtkImageViewer2::SLICE_ORIENTATION_XY
	image_coordinate[0] = vtkMath::Round(pos[0] / spacing[0]);
	image_coordinate[1] = vtkMath::Round(pos[1] / spacing[1]);
	image_coordinate[2] = 0; // it's 2D image, the third dimension was set to zero.
	
	std::string message = "Location: [";
	message += vtkVariant(image_coordinate[0]).ToString();
	message += ", ";
	message += vtkVariant(image_coordinate[1]).ToString();
	//message += ", ";
	//message += vtkVariant(image_coordinate[2]).ToString();
	message += "]\nValue: ";

	//std::cout << message;

	float tuple_float = _image->GetScalarComponentAsFloat(image_coordinate[0], image_coordinate[1], image_coordinate[2], 0);
	
	//std::cout << scale[0] << "><" << scale[1] << std::endl;
	tuple_float = tuple_float * scale[0] + scale[1];
	//std::cout << tuple_float;
	tuple_float *= 1000;  // for 10^-3 display
	// mentain the two bits after the decimal point
	tuple_float = int(tuple_float * 100);
	tuple_float = tuple_float / 100;	
	message += vtkVariant(tuple_float).ToString();
	//message += "*10^(-3)mm2/s";

	_Annotation->SetText(1, message.c_str());
	renderer->AddActor(_Annotation);
	rwi->Render();

	//ui->infoBrowser->setText(QLatin1String(message.c_str()));
}

void MainWindow::onClickTreeView(const QModelIndex &index)
{
	QStandardItem *item = roiInfoModel->itemFromIndex(index);
	qDebug() << "User clicked item at row: " << index.row() << " col: " << index.column() << "is " << item->text() << "it has" << item->rowCount() << endl;
	
	if (RoiCollection.contains(m_SourceImageCurrentSlice))
	{		
		qDebug() << "ROI number: " << RoiCollection.value(m_SourceImageCurrentSlice)->GetNumberOfItems() << "at slice" << m_SourceImageCurrentSlice <<endl;
	}
	else
	{
		qDebug() << "No ROIs at slice " << m_SourceImageCurrentSlice << endl;
	}
}

void MainWindow::onFocusWdw(const QString widgetName)
{
	qDebug() << "mouse is at" << widgetName;

	if (ActiveWdw.contains(widgetName))
	{
		ActiveWdw.removeOne(widgetName);
		ui->ViewFrame->onRemoveLabelWdw(widgetName);
//		QWidget* focusWindow = ui->ViewFrame->getWindow(widgetName);
		qDebug() << "Changing " << widgetName << " from Active to Inactive";
	}
	else{
		ActiveWdw << widgetName;
		ui->ViewFrame->onLabelWdw(widgetName);
		//QWidget* focusWindow = ui->ViewFrame->getWindow(widgetName);
		//QVTKWidget *vtkWindow = static_cast <QVTKWidget*> (focusWindow);
		qDebug() << "Changing " << widgetName << " from Inactive to Active";		
	}

	QString debugOut; debugOut += QString("Active Windows are: ");
	for (int i = 0; i < ActiveWdw.size(); ++i) 
	{
		debugOut += ActiveWdw.at(i);
		debugOut += QString("_");
	}
	qDebug() << debugOut;
}

void MainWindow::onWheelWdw(const QString widgetName, int sliceSign, Qt::Orientation orient)
{
	QString result;
	if (sliceSign > 0) {
		if (orient == Qt::Vertical) {
			if (m_SourceImageCurrentSlice < m_SourceImageMaxSlice)
			{
				result = widgetName + "Mouse Wheel Event: UP";
				m_SourceImageCurrentSlice++;
				ShareWindowEvent();
			}
		}
		else {
			result = widgetName + "Mouse Wheel Event: LEFT";
		}
	}
	else if (sliceSign < 0) {
		if (orient == Qt::Vertical) {
			if (m_SourceImageCurrentSlice > m_SourceImageMinSlice)
			{
				result = widgetName + "Mouse Wheel Event: DOWN";
				m_SourceImageCurrentSlice--;
				ShareWindowEvent();
			}
		}
		else {
			result = widgetName + "Mouse Wheel Event: RIGHT";
		}
	}
	qDebug() << result <<"cur Slice is "<< m_SourceImageCurrentSlice;
}

void MainWindow::onBroadcastEvent(QMouseEvent* e, const QString ImageName)
{
	if (ActiveWdw.size() > 1)
	{
		const QEvent::Type t = e->type();
		//qDebug() << "BroadcastEvent event " << e->source() << " accepted? " << e->isAccepted()<< endl;
		QMouseEvent* newEvent;
		newEvent = new QMouseEvent(e->type(), e->localPos(), e->windowPos(),
			e->screenPos(), e->button(), e->buttons(),
			e->modifiers(), Qt::MouseEventSynthesizedByQt);

		for (int i = 0; i < ActiveWdw.size(); ++i) {
			if (ActiveWdw.at(i) != ImageName)
			{
				QVTKWidget *thisWindow = static_cast <QVTKWidget*> (ui->ViewFrame->getWindow(ActiveWdw.at(i)));
				QCoreApplication::sendEvent(thisWindow, newEvent);
			}
		}

		//QHash < const QString, QWidget * > currentWindows = ui->ViewFrame->getAllWindow();
		//QHashIterator<const QString, QWidget * > wdwIter(currentWindows);
		//while (wdwIter.hasNext()) {
		//	wdwIter.next();
		//	if (wdwIter.key() != ImageName)
		//	{
		//		QVTKWidget *thisWindow = static_cast <QVTKWidget*> (wdwIter.value());
		//		//thisWindow->setAutomaticImageCacheEnabled(true);			
		//		QCoreApplication::sendEvent(thisWindow, newEvent);
		//	}
		//}
	}
}

void MainWindow::onKeyEvent(QKeyEvent * e)
{
	QWidget *wdwItem(this->ui->ViewFrame->getWindow(QString("ADC")));
	QVTKWidget*	vtkWindow = static_cast <QVTKWidget*> (wdwItem);
	QCoreApplication::postEvent(vtkWindow, e);
}

void MainWindow::debug(QMouseEvent* e)
{
	const QEvent::Type t = e->type();
	qDebug() << "vtkWidget has recieved" << t <<"Accepted? "<<e->isAccepted();

}

void MainWindow::addROI() //bool _istoggled
{
	bool okFlag,dupFlag;
	QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
		tr("ROI name:"), QLineEdit::Normal, tr("ROI1"), &okFlag);
	QStandardItem *root = roiInfoModel->invisibleRootItem();
	if (okFlag && !text.isEmpty())
	{
		//const QString RoiName = text;		
		for (int i = 0; i < root->rowCount(); i++)
		{
			if (text == root->child(i)->text())
			{
				qDebug() << "Duplicate ROI name" << endl;
				dupFlag = false;
			}
		}
	}

	if (dupFlag && !text.isEmpty())
	{
		qDebug() << "Adding new ROI " << endl;
		QList<QStandardItem *> newROIRow;
		newROIRow << new QStandardItem(text) << new QStandardItem("Area") << new QStandardItem("Value") << new QStandardItem("Range");
		root->appendRow(newROIRow);

		//1. Overide all windows to be active
		QList < QString >  temp;
		foreach(QString s, ActiveWdw){ temp << s; }
		QHash < const QString, QWidget * > currentWindows = ui->ViewFrame->getAllWindow();
		QHashIterator<const QString, QWidget * > wdwIter(currentWindows);
		while (wdwIter.hasNext())
		{
			wdwIter.next();
			if (!ActiveWdw.contains(wdwIter.key()))
			{
				ActiveWdw << wdwIter.key();
			}
		}
		qDebug() << ActiveWdw << endl;

		//2. Add a new Polydata into vtkcollection of this slice
		vtkCollection* thisRoiCllct;
		if (RoiCollection.contains(m_SourceImageCurrentSlice))
		{

			thisRoiCllct = RoiCollection.value(m_SourceImageCurrentSlice);
			qDebug() << "Add ROI to existing ROI collections at slice " << m_SourceImageCurrentSlice
				<< "ROI number = " << thisRoiCllct->GetNumberOfItems() << endl;
		}
		else
		{
			thisRoiCllct = vtkCollection::New();
			qDebug() << "Creating ROIs at slice " << m_SourceImageCurrentSlice << endl;

		}
		RoiCollection.insert(m_SourceImageCurrentSlice, thisRoiCllct);
		//roiCurrentSlc.reserve(currentWindows.capacity);
		//QHashIterator<const QString, vtkCollection* > roiIter(roiCurrentSlc);
		foreach(QString wdwName, ActiveWdw)
		{
			QVTKWidget *thisWindow = static_cast <QVTKWidget*> (ui->ViewFrame->getWindow(wdwName));
			qDebug() << "add ROI on window " << wdwName;
			float scalingPara[2];
			scalingPara[0] = ScalingParameters.value(wdwName + QString("_s"));
			scalingPara[1] = ScalingParameters.value(wdwName + QString("_k"));
			qDebug() << "slope intercept = " << scalingPara[0] << scalingPara[1] << wdwName;
			vtkSmartPointer<vtkRenderWindowInteractor> renInter = static_cast<vtkRenderWindowInteractor*>(thisWindow->GetRenderWindow()->GetInteractor());
			vtkRoiInteractor* RoiInterObs = new vtkRoiInteractor;
			RoiInterObs->initialize(renInter, newROIRow.first(), scalingPara, wdwName, thisRoiCllct);
		}

		//qDebug() << "after roi drawing, rois in this slice = " << thisRoiCllct->GetNumberOfItems();
		//if (thisRoiCllct->GetNumberOfItems() > 0)
		//{
		//	RoiCollection.insert(m_SourceImageCurrentSlice, thisRoiCllct);
		//}
		//else
		//{
		//	thisRoiCllct->Delete();
		//}

		//3. Overide all windows to be active
		//foreach(QString s, ActiveWdw)
		//{
		//	if (!temp.contains(s)){
		//		ActiveWdw.removeAll(s);
		//	}
		//}
		//qDebug() << ActiveWdw;
	}
}

void MainWindow::onWdwResizeEvent(const QString widgetName, const QSize oldsize, const QSize cursize)
{
	qDebug() << widgetName << "changes size from " << oldsize << "to " << cursize << endl;
	QVTKWidget *qvtkWidget = static_cast <QVTKWidget*>(ui->ViewFrame->getWindow(widgetName));
	if (qvtkWidget)
	{
		vtkSmartPointer < vtkRendererCollection> renderers = qvtkWidget->GetRenderWindow()->GetRenderers();
		if (renderers)
		{
			renderers->GetNumberOfItems();
			renderers->InitTraversal();
			while (vtkSmartPointer < vtkRenderer> renderer = renderers->GetNextItem())
			{
				renderer->ResetCamera();//Reset window before ImageAutoFillWindow
				double *bounds = renderer->ComputeVisiblePropBounds();
				//int *size = qvtkWidget->GetRenderWindow()->GetSize();
				//if (index == numberOfWindows)
				//{
				int windowSize[2]; windowSize[0] = cursize.width(); windowSize[1] = cursize.height();
				//}

				//std::cout <<"image x bounds = " << bounds[1] << std::endl;
				//std::cout << "window width = " << windowSize[0] << " window height = " << windowSize[1] << std::endl;
				//qDebug() << qvtkWidget->geometry() << endl;
				this->ImageAutoFillWindow(renderer->GetActiveCamera(), bounds, windowSize);
				//might need to break out if number of renderers > 1
				//break;
				//renderer->GetActiveCamera()->ParallelProjectionOn();
				//double xFov = bounds[1] - bounds[0];
				//double yFov = bounds[3] - bounds[2];
				//double screenRatio = double(cursize.height()) / cursize.width();//height / width;
				//double imageRatio = yFov / xFov;
				//double parallelScale = imageRatio > screenRatio ? yFov : screenRatio / imageRatio*yFov;
				//renderer->GetActiveCamera()->SetParallelScale(0.5*parallelScale);
			}
		}
		qvtkWidget->GetRenderWindow()->Render();
	}
}

void MainWindow::ImageAutoFillWindow(vtkSmartPointer <vtkCamera> camera, double * imageBounds, int *windowSize)
{
	camera->ParallelProjectionOn();

	double xFov = imageBounds[1] - imageBounds[0];
	double yFov = imageBounds[3] - imageBounds[2];

	double screenRatio = double(windowSize[1]) / windowSize[0];//height / width;
	double imageRatio = yFov / xFov;

	double parallelScale = imageRatio > screenRatio ? yFov : screenRatio / imageRatio*yFov;
	camera->SetParallelScale(0.5*parallelScale);
	//camera->SetFocalPoint(center[0], center[1], center[2]);
	//camera->SetPosition(center[0], center[1], focalDepth);
}

void MainWindow::onExportImage()
{
	if (ActiveWdw.size() > 0)
	{
		QString debugOut; debugOut += QString("Exporting Dicom Files of:");
		for (int i = 0; i < ActiveWdw.size(); ++i)
		{
			debugOut += QString(" ");
			debugOut += QString("[") + ActiveWdw.at(i) + QString("]");
		}
		qDebug() << debugOut;

		QMessageBox info;
		info.setWindowTitle(tr("Export Dicom"));
		info.setText(debugOut);
		info.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		int ret = info.exec();

		if (ret == QMessageBox::Ok)
		{
			QString directory =
				QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Save Files to..."), QDir::currentPath()));
			qDebug() << "dicom export to" << directory << endl;
			if (directory.size() > 1)
			{
				ui->diffusionModule->onCalc3D(directory);
			}
		}
		else{
			return;
		}

	}
	else{
		QMessageBox info;
		info.setText("You have to select an window for export.");
		info.exec();
	}
}


