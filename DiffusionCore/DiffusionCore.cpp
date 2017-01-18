#include "DiffusionCore.h"
#include "ui_DiffusionCore.h"

// VTK
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

// ITK
#include <itkCastImageFilter.h>
#include <itkComposeImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkVTKImageToImageFilter.h>

// Qt
#include <QCheckBox>
#include <QMessageBox>
#include <QDialog>
#include <QMouseEvent>
#include <QGroupBox>
//#include <qbuttongroup.h>

//Customized Header
//#include <itkVtkAllHeaders.h>
//#include <VTKtoITK.h> // not needed now
#include <itkAdcMapFilter.h>
#include <itkComputedDwiFilter.h>
#include <itkComputedEadcFilter.h>
#include <itkMaskVectorImageFilter.h>
#include <itkDisplayOptimizer.h>
#include <itkDwiIVIMFilter2.h>
#include <itkGetDiffusionImageFilter.h>
#include <itkTensor.h>
#include <vtkRoiInteractor.h>
#include <vtkUDInteractorStyleImage.h>
#include <DicomHelper.h>
#include "displayport.h"

#include <utility>

#include <vtkOrientedGlyphContourRepresentation.h>
#include <vtkImageActorPointPlacer.h>
//TRY SetNumberOfThreads(1) to solve the multi-thread ranmdom results
//Notify Wenxing

bool cmp(std::pair<float, int> p1, std::pair<float, int> p2)
{
	if (p1.first < p2.first) return 1;
	return 0;

}

DiffusionCore::DiffusionCore(QWidget *parent)
	:QWidget(parent)
	//, dicomHelp(nullptr)
{	
	this->m_Controls = nullptr;	
	this->ButtonTable = new QButtonGroup;
	this->sourceImage = vtkSmartPointer<vtkImageData>::New();//VTK image pointer
	this->m_MaskVectorImage = DiffusionCalculatorVectorImageType::New();
	
	m_SourceImageCurrentSlice = 0;
	m_QuantitativeImageCurrentSlice = 0;		

	CreateQtPartControl(this);

	this->m_DicomHelper = nullptr;
	this->m_MaskThreshold = 3;
	this->m_ComputedBValue = 2000;

}

DiffusionCore::~DiffusionCore()
{
	//delete m_Controls;
	//this->m_Controls = NULL;

	this->sourceImage->Delete();
	this->sourceImage = NULL;
	this->m_MaskVectorImage->Delete();
	this->m_MaskVectorImage = ITK_NULLPTR;
}

void DiffusionCore::CreateQtPartControl(QWidget *parent)
{
	if (!m_Controls)
	{				
		this->m_Controls = new Ui::DiffusionModule;
		this->m_Controls->setupUi(parent);
		
		viewFrame = new QGroupBox(tr(""));
		displayLayout = new DisplayPort;
		viewFrame->setLayout(displayLayout);	
		this->m_Controls->horizontalLayout_7->addWidget(viewFrame);

		m_Controls->bSlider->setMaximum(10000); //maximum supported b value;
		m_Controls->bSlider->setMinimum(500);
		m_Controls->bSlider->setDecimals(0);
		m_Controls->bSlider->setSingleStep(100);
		m_Controls->bSlider->setTickInterval(100);
		m_Controls->bSlider->setValue(2000);
		m_Controls->bSlider->setTracking(false);
		m_Controls->ThreshSlider->setMaximum(100); //maximum threshhold value;
		m_Controls->ThreshSlider->setDecimals(0);
		m_Controls->ThreshSlider->setSingleStep(1);
		m_Controls->ThreshSlider->setTickInterval(1);
		m_Controls->ThreshSlider->setValue(10);
		m_Controls->ThreshSlider->setTracking(false);
		//connect Buttons and handle visibility

		this->m_Controls->ADCTool->setDisabled(true);
		this->m_Controls->DTITool->setDisabled(true);

		//this->m_Controls->adcToggle->setDisabled(true);
		//this->m_Controls->eadcToggle->setDisabled(true);
		//this->m_Controls->cdwiToggle->setDisabled(true);
		//this->m_Controls->faToggle->setDisabled(true);
		//this->m_Controls->colorFAToggle->setDisabled(true);
		//this->m_Controls->ivimToggle->setDisabled(true);


		ButtonTable->setExclusive(false); //non-exclusive button group. 
		ButtonTable->addButton(m_Controls->adcToggle, 301);
		ButtonTable->addButton(m_Controls->eadcToggle, 302);
		ButtonTable->addButton(m_Controls->cdwiToggle, 303);
		ButtonTable->addButton(m_Controls->faToggle, 304);
		ButtonTable->addButton(m_Controls->colorFAToggle, 305);
		ButtonTable->addButton(m_Controls->ivimToggle, 306);

		//Connect Toggles
		connect(m_Controls->adcToggle, SIGNAL(toggled(bool)), this, SLOT(onCalcADC(bool)));
		connect(m_Controls->eadcToggle, SIGNAL(toggled(bool)), this, SLOT(onCalcEADC(bool)));
		connect(m_Controls->cdwiToggle, SIGNAL(toggled(bool)), this, SLOT(onCalcCDWI(bool)));
		connect(m_Controls->faToggle, SIGNAL(toggled(bool)), this, SLOT(onCalcFA(bool)));
		connect(m_Controls->colorFAToggle, SIGNAL(toggled(bool)), this, SLOT(onCalcColorFA(bool)));
		connect(m_Controls->ivimToggle, SIGNAL(toggled(bool)), this, SLOT(onCalcIVIM(bool)));

		m_Controls->pushButton->setCheckable(true);
		connect(m_Controls->pushButton, SIGNAL(toggled(bool)), this, SLOT(onTestButton(bool)));

		//Connect Sliders
		connect(m_Controls->ThreshSlider, SIGNAL(valueChanged(double)), this, SLOT(onThreshSlide(double)));
		connect(m_Controls->bSlider, SIGNAL(valueChanged(double)), this, SLOT(onBSlide(double)));
		
		//Connect Pointer Buttons
		//connect(m_Controls->pointer1, SIGNAL(toggled(bool)), this, SLOT(onRoiPointer(bool)));
		connect(m_Controls->pointer1, SIGNAL(pressed()), this, SLOT(addROI()));
		connect(m_Controls->pointer2, SIGNAL(toggled(bool)), this, SLOT(onCursorPickValue(bool)));

		m_Controls->cDWI->hide();
		m_Controls->Thresh->hide();		
	}
}

void DiffusionCore::onTestButton(bool _istoggled)
{
	vtkSmartPointer <vtkImageData> calculatedAdc;
	const QString imageName(m_Controls->pushButton->text());

	float scale(0.0), slope(0.0);
	if (_istoggled)
	{
		calculatedAdc = vtkSmartPointer <vtkImageData>::New();
		
		this->AdcCalculator(calculatedAdc, scale, slope);
	}
	else
	{
		calculatedAdc = NULL;		
	}
	emit SignalTestButtonFired(_istoggled, calculatedAdc, imageName, scale, slope);

	std::cout << "test button fired" << std::endl;
}


void DiffusionCore::OnImageFilesLoaded(const QStringList& fileLists)
{	
	//enable all the buttons 
	this->m_Controls->ADCTool->setEnabled(true);
	this->m_Controls->DTITool->setEnabled(true);

	//load data
	vtkStringArray* loadingFiles = vtkStringArray::New();
	loadingFiles->SetNumberOfValues(fileLists.size());
	for (int i = 0; i < fileLists.size(); i++)
	{
		loadingFiles->SetValue(i, fileLists.at(i).toStdString().c_str());
	}
	this->m_DicomHelper = new DicomHelper(loadingFiles);
	
	//tmp Test here to check if thresh slider is hided.
	this->m_Controls->Thresh->setVisible(false);

	if (!m_DicomHelper->tensorComputationPossible)
	{
		this->m_Controls->faToggle->setDisabled(true);
		this->m_Controls->colorFAToggle->setDisabled(true);
		//this->m_Controls->dtiNameTag->setText("Data does not contain multiple direction, view Only");
		if (m_DicomHelper->numberOfBValue < 2)
		{
			QMessageBox::StandardButton reply;
			reply = QMessageBox::information(this, tr("QMessageBox::information()"), tr("Must Select Diffusion Weighted Image Data"));
			//neutralize all DWI related functions
			this->m_Controls->adcToggle->setDisabled(true);
			this->m_Controls->eadcToggle->setDisabled(true);
			this->m_Controls->cdwiToggle->setDisabled(true);
			this->m_Controls->ivimToggle->setDisabled(true);
		}
	}
	if (m_DicomHelper->numberOfBValue < 4)
	{
		this->m_Controls->ivimToggle->setDisabled(true);
	}

	if (m_DicomHelper->numberOfBValue < 2)
		//for none diffusion images
		this->sourceImage = this->m_DicomHelper->DicomReader->GetOutput();
	else
	{
		//for Diffusion images.
		this->SortingSourceImage();
		this->UpdateMaskVectorImage();
	}
	
	const QString orgLabel("ORIGINAL"); //TO_DO: using serie name/protocol name instead
		
	QWidget *orgItem (this->displayLayout->getWindow(orgLabel));
	QVTKWidget *vtkWindow;
	if (orgItem != NULL)
	{
		std::cout << "Original Image existed " << std::endl;		
		vtkWindow = static_cast <QVTKWidget*>(orgItem);
		
		//for (int i = 301; i < 307; i++)
		//{
		//	ButtonTable->button(i)->blockSignals(true);
		//	ButtonTable->button(i)->setChecked(false);
		//	ButtonTable->button(i)->blockSignals(false);
		//	ButtonTable->button(i)->setEnabled(true);
		//}

		m_SourceImageCurrentSlice = 0;
		m_QuantitativeImageCurrentSlice = 0;
		this->m_DicomHelper = nullptr;
		this->m_MaskThreshold = 3;
		this->m_ComputedBValue = 2000;
	}
	else{
		vtkWindow = new QVTKWidget;
		this->displayLayout->insertWindow(vtkWindow, orgLabel);
	}
	emit SignalDicomLoaded(true);

	//std::cout << "srcimage viewer" << std::endl;
	DisplayDicomInfo(this->sourceImage);
	SourceImageViewer2D(this->sourceImage, vtkWindow);
}

void DiffusionCore::SortingSourceImage()
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
	this->sourceImage->AllocateScalars(VTK_UNSIGNED_SHORT,this->m_DicomHelper->numberOfComponents);

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
				sourcePtr[0] = dicomPtr[ bValueOrderIndex[0] * numOfGradDir ];

				//sorting based on the b value order
				int sourceCmpIndex = 1;
				for (int cmp = 1; cmp < this->m_DicomHelper->DicomReader->GetNumberOfScalarComponents(); cmp++)
				{
					//get the b value index for current component
					int bValueIndex = (cmp - 1) / numOfGradDir + 1;
					//get the grad direction indec for current component
					int gradDirIndex = cmp - 1 - numOfGradDir*(bValueIndex-1);
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

void DiffusionCore::UpdateMaskVectorImage()
{
	if (!this->sourceImage)
	{
		std::cout << "sourceImage NULL" << std::endl;
		return;
	}

	// currently implementation, for GE data; need a decent way!!!
	//typedef T SourceImagePixelType;
	//typedef itk::Image < SourceImagePixelType, 3> SourceImageType;
	typedef itk::VectorContainer< SourceImagePixelType, DiffusionCalculatorImageType::Pointer > ImageContainerType;
	typedef itk::VTKImageToImageFilter <SourceImageType>	VtkToItkConverterType;
	typedef itk::CastImageFilter< SourceImageType, DiffusionCalculatorImageType >	CastFilterType;
	typedef itk::ShiftScaleImageFilter<DiffusionCalculatorImageType, DiffusionCalculatorImageType>	ShiftScaleType;


	ImageContainerType::Pointer imageContainer = ImageContainerType::New();
	//if (m_DicomHelper->tensorComputationPossible && (m_DicomHelper->IsoImageLabel > -1))
	//{		
	//	imageContainer->Reserve(this->m_DicomHelper->numberOfComponents -1);// only works for DTI b value 2, otherwise wrong
	//}
	//else
	//{
	imageContainer->Reserve(this->m_DicomHelper->numberOfComponents);
	//}
	//std::cout << "m_DicomHelper numof components" << this->m_DicomHelper->numberOfComponents << std::endl;

	vtkSmartPointer <vtkExtractVOI> ExtractVOI = vtkSmartPointer <vtkExtractVOI>::New();
	ExtractVOI->SetInputData(this->sourceImage);
	ExtractVOI->SetVOI(0, this->m_DicomHelper->imageDimensions[0] - 1, 0, this->m_DicomHelper->imageDimensions[1] - 1, m_SourceImageCurrentSlice, m_SourceImageCurrentSlice);
	ExtractVOI->Update();
	//std::cout << "---------------------------- VOI is correct ? ---------------------" << std::endl;
	//this->DisplayDicomInfo(ExtractVOI->GetOutput());
	/*std::cout << "---------------------------- End of VOI ---------------------" << std::endl;*/

	//PR: mask image doesn't change for slices other than the first one whlie sliding the maskthreshold slider
	//fix extent range here, default z extent is m_SourceImageCurrentSlice to m_SourceImageCurrentSlice;
	//Update image extent, otherwise maskFilter won't work for slices other than the first one (because seed starts at origin)
	//Update the origin as well
	vtkSmartPointer <vtkImageChangeInformation> changeInfo = vtkSmartPointer <vtkImageChangeInformation>::New();
	changeInfo->SetInputData(ExtractVOI->GetOutput());
	changeInfo->SetOutputOrigin(0, 0, 0);
	changeInfo->SetExtentTranslation(0, 0, -m_SourceImageCurrentSlice);
	changeInfo->Update();

	//std::cout << "---------------------------- translateExtent is correct ? ---------------------" << std::endl;
	//this->DisplayDicomInfo(changeInfo->GetOutput());
	//std::cout << "---------------------------- End of translateExtent ---------------------" << std::endl;
	//std::cout << " update vector image DTI numberOfComponents " << m_DicomHelper->numberOfComponents << std::endl;

	int DTIindex = 0;
	for (int i = 0; i < this->m_DicomHelper->numberOfComponents; i++)
	{				
		//Handle each scalar component indivisually
		vtkSmartPointer <vtkImageExtractComponents> scalarComponent = vtkSmartPointer <vtkImageExtractComponents>::New();
		scalarComponent->SetInputData(changeInfo->GetOutput());
		scalarComponent->SetComponents(i);
		scalarComponent->Update();//Crutial, otherwise abort after running

		//VTK to ITK Image Data
		VtkToItkConverterType::Pointer vtkToItkImageFilter = VtkToItkConverterType::New();
		vtkToItkImageFilter->SetInput(scalarComponent->GetOutput());
		vtkToItkImageFilter->Update();

		//unsigned short image to float image
		CastFilterType::Pointer castFilter = CastFilterType::New();
		castFilter->SetInput(vtkToItkImageFilter->GetOutput());
		castFilter->Update();

		//Shift and scale signal back to FP value
		//Take some time to finish the computation
		ShiftScaleType::Pointer shiftScale = ShiftScaleType::New();
		shiftScale->SetInput(castFilter->GetOutput());
		shiftScale->SetShift(-m_DicomHelper->scaleIntercept);
		shiftScale->SetScale(1.0 / m_DicomHelper->scaleSlope);
		shiftScale->Update();

		//removing the isotropic direction for DTI has been implemented in the sorting source image.
		//std::cout << " Update vector image DWI" << std::endl;
		imageContainer->InsertElement(i, dynamic_cast <DiffusionCalculatorImageType*> (shiftScale->GetOutput()));
	}

	//std::cout << " Update vector image DTI00  imageContainer size " << imageContainer->Size() << std::endl;
	//Get vector Image
	typedef itk::ComposeImageFilter<DiffusionCalculatorImageType>		ImageToVectorImageType;
	ImageToVectorImageType::Pointer imageToVectorImageFilter = ImageToVectorImageType::New();

	for (int i = 0; i < imageContainer->Size(); i++)
	{
		imageToVectorImageFilter->SetInput(i, imageContainer->GetElement(i));
	}

	imageToVectorImageFilter->Update();
	//std::cout << "vectorImage: vectorLength = " << imageToVectorImageFilter->GetOutput()->GetVectorLength() << std::endl;

	//Mask image filter
	typedef itk::MaskVectorImageFilter <DiffusionCalculatorPixelType> MaskFilterType;
	MaskFilterType::Pointer maskFilter = MaskFilterType::New();
	maskFilter->SetInput(imageToVectorImageFilter->GetOutput());//input is a Image Pointer!!!
	maskFilter->SetMaskThreshold(m_MaskThreshold);//Get from UI or user-interaction
	maskFilter->Update();//output is a Image Pointer!!!

	//std::cout << "maskFilter: vectorLength = " << maskFilter->GetOutput()->GetVectorLength() << std::endl;

	//itk version of DeepCopy	
	this->m_MaskVectorImage->SetSpacing(maskFilter->GetOutput()->GetSpacing());
	this->m_MaskVectorImage->SetOrigin(maskFilter->GetOutput()->GetOrigin());
	this->m_MaskVectorImage->SetDirection(maskFilter->GetOutput()->GetDirection());
	this->m_MaskVectorImage->SetRegions(maskFilter->GetOutput()->GetLargestPossibleRegion());
	this->m_MaskVectorImage->SetVectorLength(maskFilter->GetOutput()->GetVectorLength());
	//std::cout << " m_MaskVectorImage length 00 0000000= " << this->m_MaskVectorImage->GetVectorLength() << std::endl;
	this->m_MaskVectorImage->Allocate();
	//std::cout << " m_MaskVectorImage length 00 = " << this->m_MaskVectorImage->GetVectorLength() << std::endl;


	typedef itk::ImageRegionConstIterator <DiffusionCalculatorVectorImageType> ConstMaskFilterIteratorType;
	typedef itk::ImageRegionIterator <DiffusionCalculatorVectorImageType> MaskVectorImageIteratorType;

	ConstMaskFilterIteratorType maskFilterIterator(maskFilter->GetOutput(), maskFilter->GetOutput()->GetLargestPossibleRegion());
	MaskVectorImageIteratorType maskVectorImageIterator(this->m_MaskVectorImage, this->m_MaskVectorImage->GetLargestPossibleRegion());

	maskFilterIterator.GoToBegin();
	maskVectorImageIterator.GoToBegin();
	while (!maskFilterIterator.IsAtEnd())
	{
		maskVectorImageIterator.Set(maskFilterIterator.Get());
		++maskFilterIterator;
		++maskVectorImageIterator;
	}
	//std::cout << " m_MaskVectorImage length 11 = " << this->m_MaskVectorImage->GetVectorLength() << std::endl;
}

void DiffusionCore::onCalcADC(bool _istoggled) //SLOT of adcToggle
{
	//Get the image name from the text of the corresponding button.
	const QString imageName(this->m_Controls->adcToggle->text());

	//qDebug() << "[DiffusionCalculation] " << imageName.c_str() << " is clicked: ";
	if (_istoggled)
	{
		//Set Threshhold bar visible
		this->m_Controls->Thresh->setVisible(_istoggled);
		
		QWidget *wdwItem(this->displayLayout->getWindow(imageName));
		QVTKWidget* vtkWindow;
		if (wdwItem != NULL)
		{			
			//std::cout << imageName.c_str() << " window exists"<<std::endl;
			vtkWindow = static_cast <QVTKWidget*> (wdwItem);
		}else{
			vtkWindow = new QVTKWidget;
			//std::cout << imageName.c_str() << " window not exists, creating window";
			this->displayLayout->insertWindow(vtkWindow, imageName);
			//std::cout << "... window created" << std::endl;
		}		
		vtkSmartPointer <vtkImageData> calculatedAdc = vtkSmartPointer <vtkImageData>::New();
		float scale(0.0), slope(0.0);
		this->AdcCalculator(calculatedAdc, scale, slope);
		if (m_ScalingParameter.contains(imageName + '_s'))
		{
			m_ScalingParameter.remove(imageName + '_s');
			m_ScalingParameter.remove(imageName + '_k');
		}
		m_ScalingParameter.insert(imageName + '_s', scale);
		m_ScalingParameter.insert(imageName + '_k', slope);
		QuantitativeImageViewer2D(calculatedAdc, vtkWindow, imageName.toStdString());
	}
	else
	{
		this->displayLayout->removeWindow(imageName);
	}
	
}

void DiffusionCore::AdcCalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope)
{	
	if (!this->m_MaskVectorImage) return;	
	//std::cout << "------------- AdcMapFilter begin runing ----------- " << std::endl;
	// Adc Map filter
	typedef itk::AdcMapFilter <DiffusionCalculatorPixelType, DiffusionCalculatorPixelType> AdcMapFilterType;
	AdcMapFilterType::Pointer adcMap = AdcMapFilterType::New();
	adcMap->SetInput(this->m_MaskVectorImage);
	adcMap->SetBValueList(this->m_DicomHelper->BvalueList);
	adcMap->Update();
	//std::cout << "------------- AdcMapFilter end runing ----------- " << std::endl;
	//std::cout << "adcVectorImage: vectorLength = " << adcMap->GetOutput()->GetVectorLength() << std::endl;

	//vector image to scalar image or imageContainer
	typedef itk::VectorIndexSelectionCastImageFilter <DiffusionCalculatorVectorImageType, DiffusionCalculatorImageType> VectorImageToImageType;
	VectorImageToImageType::Pointer vectorImageToImageFilter = VectorImageToImageType::New();
	//set input for different data: DWI or DTI
	if (m_DicomHelper->tensorComputationPossible)
	{
		//DTI claculation
		cout << "DTI is calculating.." << endl;
		typedef itk::GetDiffusionImageFilter<DiffusionCalculatorPixelType, DiffusionCalculatorPixelType> GetDiffusionImageType;
		GetDiffusionImageType::Pointer GetDiffusionImages = GetDiffusionImageType::New();
		GetDiffusionImages->SetInput(adcMap->GetOutput());
		GetDiffusionImages->SetHMatrix(m_DicomHelper->finalH);
		GetDiffusionImages->SetSlice2PatMatrix(m_DicomHelper->slice2PatMatrix);
		GetDiffusionImages->SetBValueList(m_DicomHelper->BvalueList);
		GetDiffusionImages->Update();

		vectorImageToImageFilter->SetIndex(0);
		vectorImageToImageFilter->SetInput(GetDiffusionImages->GetOutput1());
	}
	else
	{
		//cout << "number of components" << adcMap->GetOutput()->GetNumberOfComponentsPerPixel() << endl;
		int index = adcMap->GetOutput()->GetNumberOfComponentsPerPixel() - 1;
		vectorImageToImageFilter->SetIndex(index); //1
 		vectorImageToImageFilter->SetInput(adcMap->GetOutput());

	}
	vectorImageToImageFilter->Update();


	//Data Clipping
	typedef itk::DisplayOptimizer < DiffusionCalculatorImageType, DiffusionCalculatorImageType> DisplayOptimizerType;
	DisplayOptimizerType::Pointer displayOptimizer = DisplayOptimizerType::New();
	displayOptimizer->SetInput(vectorImageToImageFilter->GetOutput());
	displayOptimizer->SetCoveragePercent(0.98);//Default is 0.99
	displayOptimizer->Update();

	//Rescale signal intensity to display
	typedef itk::RescaleIntensityImageFilter < DiffusionCalculatorImageType, SourceImageType> RescaleIntensityImageType;
	RescaleIntensityImageType::Pointer rescaleFilter = RescaleIntensityImageType::New();
	rescaleFilter->SetInput(displayOptimizer->GetOutput());
	rescaleFilter->SetOutputMaximum(4095);
	rescaleFilter->SetOutputMinimum(0);
	rescaleFilter->Update();

	scale = 1 / rescaleFilter->GetScale();
	slope = -1 * rescaleFilter->GetShift() / rescaleFilter->GetScale();
	//std::cout << "rescaleFilter: inputMaximum = " << m_ScalingParameter[2 * ADC] << std::endl;
	//std::cout << "rescaleFilter: inputMinimum = " << m_ScalingParameter[2 * ADC + 1] << std::endl;

	////Data Clipping
	//typedef itk::DisplayOptimizer < DiffusionCalculatorImageType, SourceImageType> DisplayOptimizerType;
	//DisplayOptimizerType::Pointer displayOptimizer = DisplayOptimizerType::New();
	//displayOptimizer->SetInput(rescaleFilter->GetOutput());
	//displayOptimizer->SetCoveragePercent(0.98);//Default is 0.99
	//displayOptimizer->Update();

	///////////////////////////////////////////
	//ITK to VTK
	typedef itk::ImageToVTKImageFilter <SourceImageType> itkToVtkConverter;
	itkToVtkConverter::Pointer convItkToVtk = itkToVtkConverter::New();
	convItkToVtk->SetInput(rescaleFilter->GetOutput());
	convItkToVtk->Update();

	imageData->DeepCopy(convItkToVtk->GetOutput());

	cout << "ADC calculator: " << imageData->GetScalarComponentAsFloat(60, 60, 0, 0) << endl;
}

void DiffusionCore::onCalcEADC(bool _istoggled) //SLOT of eadcToggle
{
	//Get the image name from the text of the corresponding button.
	const QString imageName(this->m_Controls->eadcToggle->text());

	if (_istoggled)
	{
		this->m_Controls->Thresh->setVisible(_istoggled);

		QWidget *wdwItem(this->displayLayout->getWindow(imageName));
		QVTKWidget* vtkWindow;
		if (wdwItem != NULL)
		{
			//std::cout << imageName.c_str() << " window exists"<<std::endl;
			vtkWindow = static_cast <QVTKWidget*> (wdwItem);
		}
		else{
			vtkWindow = new QVTKWidget;
			//std::cout << imageName.c_str() << " window not exists, creating window";
			this->displayLayout->insertWindow(vtkWindow, imageName);
			//std::cout << "... window created" << std::endl;
		}
		vtkSmartPointer <vtkImageData> calculatedEAdc = vtkSmartPointer <vtkImageData>::New();
		float scale(0.0), slope(0.0);
		this->EAdcCalculator(calculatedEAdc, scale, slope);
		if (m_ScalingParameter.contains(imageName + '_s'))
		{
			m_ScalingParameter.remove(imageName + '_s');
			m_ScalingParameter.remove(imageName + '_k');
		}
		m_ScalingParameter.insert(imageName + '_s', scale);
		m_ScalingParameter.insert(imageName + '_k', slope);
		QuantitativeImageViewer2D(calculatedEAdc, vtkWindow, imageName.toStdString());

	}
	else
	{
		this->displayLayout->removeWindow(imageName);
	}
}

void DiffusionCore::EAdcCalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope)
{
	if (!this->m_MaskVectorImage) return;
	//std::cout << "------------- AdcMapFilter begin runing ----------- " << std::endl;
	// Adc Map filter
	typedef itk::AdcMapFilter <DiffusionCalculatorPixelType, DiffusionCalculatorPixelType> AdcMapFilterType;
	AdcMapFilterType::Pointer adcMap = AdcMapFilterType::New();
	adcMap->SetInput(this->m_MaskVectorImage);
	adcMap->SetBValueList(this->m_DicomHelper->BvalueList);
	adcMap->Update();
	//std::cout << "------------- AdcMapFilter end runing ----------- " << std::endl;
	//std::cout << "adcVectorImage: vectorLength = " << adcMap->GetOutput()->GetVectorLength() << std::endl;


	//vector image to scalar image or imageContainer
	typedef itk::VectorIndexSelectionCastImageFilter <DiffusionCalculatorVectorImageType, DiffusionCalculatorImageType> VectorImageToImageType;
	VectorImageToImageType::Pointer vectorImageToImageFilter = VectorImageToImageType::New();
	//set input for different data: DWI or DTI
	if (m_DicomHelper->tensorComputationPossible)
	{
		//DTI claculation
		cout << "DTI is calculating.." << endl;
		typedef itk::GetDiffusionImageFilter<DiffusionCalculatorPixelType, DiffusionCalculatorPixelType> GetDiffusionImageType;
		GetDiffusionImageType::Pointer GetDiffusionImages = GetDiffusionImageType::New();
		GetDiffusionImages->SetInput(adcMap->GetOutput());
		GetDiffusionImages->SetHMatrix(m_DicomHelper->finalH);
		GetDiffusionImages->SetSlice2PatMatrix(m_DicomHelper->slice2PatMatrix);
		GetDiffusionImages->SetBValueList(m_DicomHelper->BvalueList);
		GetDiffusionImages->Update();

		vectorImageToImageFilter->SetIndex(1);
		vectorImageToImageFilter->SetInput(GetDiffusionImages->GetOutput1());
	}
	else
	{

		typedef itk::ComputedEadcFilter <DiffusionCalculatorPixelType, DiffusionCalculatorPixelType> ComputedEadcFilterType;
		ComputedEadcFilterType::Pointer computedEadc = ComputedEadcFilterType::New();
		computedEadc->SetInput(adcMap->GetOutput());
		computedEadc->SetNumOfDiffDirections(this->m_DicomHelper->numberOfGradDirection);
		computedEadc->SetEadcBValue(this->m_DicomHelper->BvalueList.at(this->m_DicomHelper->BvalueList.size() - 1));//Get from UI input
		computedEadc->Update();

		vectorImageToImageFilter->SetIndex(0);
		vectorImageToImageFilter->SetInput(computedEadc->GetOutput());

	}
	vectorImageToImageFilter->Update();

	//Rescale signal intensity to display
	typedef itk::RescaleIntensityImageFilter < DiffusionCalculatorImageType, DiffusionCalculatorImageType> RescaleIntensityImageType;
	RescaleIntensityImageType::Pointer rescaleFilter = RescaleIntensityImageType::New();
	rescaleFilter->SetInput(vectorImageToImageFilter->GetOutput());
	rescaleFilter->SetOutputMaximum(4095.0);
	rescaleFilter->SetOutputMinimum(0.0);
	rescaleFilter->Update();
	//std::cout << "rescaleFilter: inputMaximum = " << rescaleFilter->GetInputMaximum() << std::endl;
	//std::cout << "rescaleFilter: inputMinimum = " << rescaleFilter->GetInputMinimum() << std::endl;
	scale = 1 / rescaleFilter->GetScale();
	slope = -1 * rescaleFilter->GetShift() / rescaleFilter->GetScale();
	//Data Clipping
	typedef itk::DisplayOptimizer < DiffusionCalculatorImageType, SourceImageType> DisplayOptimizerType;
	DisplayOptimizerType::Pointer displayOptimizer = DisplayOptimizerType::New();
	displayOptimizer->SetInput(rescaleFilter->GetOutput());
	displayOptimizer->SetCoveragePercent(0.98);//Default is 0.99
	displayOptimizer->Update();

	//std::cout << "rescaleFilter: inputMaximum = " << rescaleFilter->GetInputMaximum() << std::endl;
	//std::cout << "rescaleFilter: inputMinimum = " << rescaleFilter->GetInputMinimum() << std::endl;

	///////////////////////////////////////////
	//ITK to VTK
	typedef itk::ImageToVTKImageFilter <SourceImageType> itkToVtkConverter;
	itkToVtkConverter::Pointer convItkToVtk = itkToVtkConverter::New();
	convItkToVtk->SetInput(displayOptimizer->GetOutput());
	convItkToVtk->Update();

	imageData->DeepCopy(convItkToVtk->GetOutput());
}

void DiffusionCore::onCalcCDWI(bool _istoggled)  //SLOT of cdwiToggle
{
	//Get the image name from the text of the corresponding button.
	const QString imageName(this->m_Controls->cdwiToggle->text());

	if (_istoggled)
	{
		//Set Threshhold bar visible
		this->m_Controls->Thresh->setVisible(_istoggled);
		this->m_Controls->cDWI->setVisible(_istoggled);		

		QWidget *wdwItem(this->displayLayout->getWindow(imageName));
		QVTKWidget* vtkWindow;
		if (wdwItem != NULL)
		{
			//std::cout << imageName.c_str() << " window exists"<<std::endl;
			vtkWindow = static_cast <QVTKWidget*> (wdwItem);
		}
		else{
			vtkWindow = new QVTKWidget;
			//std::cout << imageName.c_str() << " window not exists, creating window";
			this->displayLayout->insertWindow(vtkWindow, imageName);
			//std::cout << "... window created" << std::endl;
		}

		vtkSmartPointer <vtkImageData> computedDwi = vtkSmartPointer <vtkImageData>::New();
		float scale(0.0), slope(0.0);
		this->CDWICalculator(computedDwi, scale, slope);
		if (m_ScalingParameter.contains(imageName + '_s'))
		{
			m_ScalingParameter.remove(imageName + '_s');
			m_ScalingParameter.remove(imageName + '_k');
		}
		m_ScalingParameter.insert(imageName + '_s', scale);
		m_ScalingParameter.insert(imageName + '_k', slope);

		QuantitativeImageViewer2D(computedDwi, vtkWindow, imageName.toStdString());
	}
	else
	{
		this->m_Controls->cDWI->setVisible(_istoggled);		
		this->displayLayout->removeWindow(imageName);
	}
}

void DiffusionCore::CDWICalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope)
{
	if (!this->m_MaskVectorImage) return;
	//std::cout << "------------- AdcMapFilter begin runing ----------- " << std::endl;
	// Adc Map filter
	typedef itk::AdcMapFilter <DiffusionCalculatorPixelType, DiffusionCalculatorPixelType> AdcMapFilterType;
	AdcMapFilterType::Pointer adcMap = AdcMapFilterType::New();
	adcMap->SetInput(this->m_MaskVectorImage);
	adcMap->SetBValueList(this->m_DicomHelper->BvalueList);
	adcMap->Update();
	//std::cout << "------------- AdcMapFilter begin runing ----------- " << std::endl;
	//std::cout << "adcVectorImage: vectorLength = " << adcMap->GetOutput()->GetVectorLength() << std::endl;

	//cDwi filter
	typedef itk::ComputedDwiFilter <DiffusionCalculatorPixelType, DiffusionCalculatorPixelType> ComputedDwiFilterType;
	ComputedDwiFilterType::Pointer computedDwi = ComputedDwiFilterType::New();
	computedDwi->SetInput(adcMap->GetOutput());
	computedDwi->SetNumOfDiffDirections(this->m_DicomHelper->numberOfGradDirection);
	computedDwi->SetComputedBValue(m_ComputedBValue);//Get from UI input
	computedDwi->Update();
	//std::cout << "cDWi vectorImage: vectorLength = " << computedDwi->GetOutput()->GetVectorLength() << std::endl;

	//cout << "number of components" << computedDwi->GetOutput()->GetNumberOfComponentsPerPixel() << endl;
	//vector image to scalar image or imageContainer
	typedef itk::VectorIndexSelectionCastImageFilter <DiffusionCalculatorVectorImageType, DiffusionCalculatorImageType> VectorImageToImageType;
	VectorImageToImageType::Pointer vectorImageToImageFilter = VectorImageToImageType::New();
	int index = computedDwi->GetOutput()->GetNumberOfComponentsPerPixel() - 1;
	vectorImageToImageFilter->SetIndex(index); //0
	vectorImageToImageFilter->SetInput(computedDwi->GetOutput());
	vectorImageToImageFilter->Update();

	//Rescale signal intensity to display
	typedef itk::RescaleIntensityImageFilter < DiffusionCalculatorImageType, DiffusionCalculatorImageType> RescaleIntensityImageType;
	RescaleIntensityImageType::Pointer rescaleFilter = RescaleIntensityImageType::New();
	rescaleFilter->SetInput(vectorImageToImageFilter->GetOutput());
	rescaleFilter->SetOutputMaximum(255.0);
	rescaleFilter->SetOutputMinimum(0.0);
	rescaleFilter->Update();
	//std::cout << "rescaleFilter: inputMaximum = " << rescaleFilter->GetInputMaximum() << std::endl;
	//std::cout << "rescaleFilter: inputMinimum = " << rescaleFilter->GetInputMinimum() << std::endl;
	scale = 1 / rescaleFilter->GetScale();
	slope = -1 * rescaleFilter->GetShift() / rescaleFilter->GetScale();

	//Data Clipping
	typedef itk::DisplayOptimizer < DiffusionCalculatorImageType, SourceImageType> DisplayOptimizerType;
	DisplayOptimizerType::Pointer displayOptimizer = DisplayOptimizerType::New();
	displayOptimizer->SetInput(rescaleFilter->GetOutput());
	displayOptimizer->SetCoveragePercent(0.98);//Default is 0.99
	displayOptimizer->Update();

	///////////////////////////////////////////
	//ITK to VTK for visualization
	typedef itk::ImageToVTKImageFilter < SourceImageType > itkToVtkConverter;
	itkToVtkConverter::Pointer convItkToVtk = itkToVtkConverter::New();
	convItkToVtk->SetInput(displayOptimizer->GetOutput());
	convItkToVtk->Update();

	imageData->DeepCopy(convItkToVtk->GetOutput());
}

void DiffusionCore::onThreshSlide(double maskThreshold) //SLOT of cdwiToggle
{
	m_MaskThreshold = maskThreshold/5;
	DiffusionCore::ShareWindowEvent();
}

void DiffusionCore::onBSlide(double computedBValue) //SLOT of cdwiToggle
{
	const QString imageName(this->m_Controls->cdwiToggle->text());

	m_ComputedBValue = computedBValue;
	//clear up existing widget
	QVTKWidget *cDwiWidget = static_cast <QVTKWidget*> (this->displayLayout->getWindow(imageName));
	
	if (cDwiWidget != NULL)
	{
		vtkSmartPointer <vtkImageData> computedDwi = vtkSmartPointer <vtkImageData>::New();
		float scale(0.0), slope(0.0);
		this->CDWICalculator(computedDwi, scale, slope);
		if (m_ScalingParameter.contains(imageName + '_s'))
		{
			m_ScalingParameter.remove(imageName + '_s');
			m_ScalingParameter.remove(imageName + '_k');
		}
		m_ScalingParameter.insert(imageName + '_s', scale);
		m_ScalingParameter.insert(imageName + '_k', slope);
		QuantitativeImageViewer2D(computedDwi, cDwiWidget, imageName.toStdString());
	}
}

void DiffusionCore::onCalcFA(bool _istoggled)  //SLOT of faToggle
{
	//Get the image name from the text of the corresponding button.
	const QString imageName(this->m_Controls->faToggle->text());

	if (_istoggled)
	{
		//Set Threshhold bar visible
		this->m_Controls->Thresh->setVisible(_istoggled);

		QWidget *wdwItem(this->displayLayout->getWindow(imageName));
		QVTKWidget* vtkWindow;
		if (wdwItem != NULL)
		{
			//std::cout << imageName.c_str() << " window exists"<<std::endl;
			vtkWindow = static_cast <QVTKWidget*> (wdwItem);
		}
		else{
			vtkWindow = new QVTKWidget;
			//std::cout << imageName.c_str() << " window not exists, creating window";
			this->displayLayout->insertWindow(vtkWindow, imageName);
			//std::cout << "... window created" << std::endl;
		}

		vtkSmartPointer <vtkImageData> calculatedFA = vtkSmartPointer <vtkImageData>::New();
		float scale(0.0), slope(0.0);
		this->FaCalculator(calculatedFA,scale,slope);
		if (m_ScalingParameter.contains(imageName + '_s'))
		{
			m_ScalingParameter.remove(imageName + '_s');
			m_ScalingParameter.remove(imageName + '_k');
		}
		m_ScalingParameter.insert(imageName + '_s', scale);
		m_ScalingParameter.insert(imageName + '_k', slope);
		QuantitativeImageViewer2D(calculatedFA, vtkWindow, imageName.toStdString());

	}
	else
	{
		//Hide Threshhold bar
		//this->m_Controls->Thresh->setVisible(_istoggled);

		this->displayLayout->removeWindow(imageName);
		ShareWindowEvent();
	}
}

void DiffusionCore::FaCalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope)
{
	//this->UpdateMaskVectorImage();
	if (!this->m_MaskVectorImage) return;

	typedef itk::AdcMapFilter <DiffusionCalculatorPixelType, DiffusionCalculatorPixelType> AdcMapFilterType;
	AdcMapFilterType::Pointer adcMap = AdcMapFilterType::New();
	adcMap->SetInput(this->m_MaskVectorImage);
	adcMap->SetBValueList(this->m_DicomHelper->BvalueList);
	adcMap->Update();

	//DTI claculation
	cout << "DTI is calculating.." << endl;
	typedef itk::GetDiffusionImageFilter<DiffusionCalculatorPixelType, DiffusionCalculatorPixelType> GetDiffusionImageType;
	GetDiffusionImageType::Pointer GetDiffusionImages = GetDiffusionImageType::New();
	GetDiffusionImages->SetInput(adcMap->GetOutput());
	GetDiffusionImages->SetHMatrix(m_DicomHelper->finalH);
	GetDiffusionImages->SetSlice2PatMatrix(m_DicomHelper->slice2PatMatrix);
	GetDiffusionImages->SetBValueList(m_DicomHelper->BvalueList);
	GetDiffusionImages->Update();

	//FA
	typedef itk::VectorIndexSelectionCastImageFilter <DiffusionCalculatorVectorImageType, DiffusionCalculatorImageType> VectorImageToImageType;
	VectorImageToImageType::Pointer vectorImageToImageFilter = VectorImageToImageType::New();
	vectorImageToImageFilter->SetIndex(2);
	vectorImageToImageFilter->SetInput(GetDiffusionImages->GetOutput1());
	vectorImageToImageFilter->Update();

	//Rescale signal intensity to display
	typedef itk::RescaleIntensityImageFilter < DiffusionCalculatorImageType, DiffusionCalculatorImageType> RescaleIntensityImageType;
	RescaleIntensityImageType::Pointer rescaleFilter = RescaleIntensityImageType::New();
	rescaleFilter->SetInput(vectorImageToImageFilter->GetOutput());
	rescaleFilter->SetOutputMaximum(4095.0);
	rescaleFilter->SetOutputMinimum(0.0);
	rescaleFilter->Update();

	scale = 1 / rescaleFilter->GetScale();
	slope = -1 * rescaleFilter->GetShift() / rescaleFilter->GetScale();
	//std::cout << "rescaleFilter: inputMaximum = " << rescaleFilter->GetInputMaximum() << std::endl;
	//std::cout << "rescaleFilter: inputMinimum = " << rescaleFilter->GetInputMinimum() << std::endl;

	//Data Clipping
	typedef itk::DisplayOptimizer < DiffusionCalculatorImageType, SourceImageType> DisplayOptimizerType;
	DisplayOptimizerType::Pointer displayOptimizer = DisplayOptimizerType::New();
	displayOptimizer->SetInput(rescaleFilter->GetOutput());
	displayOptimizer->SetCoveragePercent(0.9);//Default is 0.99
	displayOptimizer->Update();

	///////////////////////////////////////////
	//ITK to VTK
	typedef itk::ImageToVTKImageFilter <SourceImageType> itkToVtkConverter;
	itkToVtkConverter::Pointer convItkToVtk = itkToVtkConverter::New();
	convItkToVtk->SetInput(displayOptimizer->GetOutput());
	convItkToVtk->Update();

	imageData->DeepCopy(convItkToVtk->GetOutput());
}

void DiffusionCore::onCalcColorFA(bool _istoggled)
{
	const QString imageName(this->m_Controls->colorFAToggle->text());
	if (_istoggled)
	{
		//Set Threshhold bar visible
		this->m_Controls->Thresh->setVisible(_istoggled);

		QWidget *wdwItem(this->displayLayout->getWindow(imageName));
		QVTKWidget* vtkWindow;
		if (wdwItem != NULL)
		{
			//std::cout << imageName.c_str() << " window exists"<<std::endl;
			vtkWindow = static_cast <QVTKWidget*> (wdwItem);
		}
		else{
			vtkWindow = new QVTKWidget;
			//std::cout << imageName.c_str() << " window not exists, creating window";
			this->displayLayout->insertWindow(vtkWindow, imageName);
			//std::cout << "... window created" << std::endl;
		}

		vtkSmartPointer <vtkImageData> calculatedColorFA = vtkSmartPointer <vtkImageData>::New();
		this->ColorFACalculator(calculatedColorFA);
		QuantitativeImageViewer2D(calculatedColorFA, vtkWindow, imageName.toStdString());
	}
	else
	{
		this->displayLayout->removeWindow(imageName);
	}
}

void DiffusionCore::ColorFACalculator(vtkSmartPointer <vtkImageData> imageData)
{
	//this->UpdateMaskVectorImage();
	if (!this->m_MaskVectorImage) return;

	typedef itk::AdcMapFilter <DiffusionCalculatorPixelType, DiffusionCalculatorPixelType> AdcMapFilterType;
	AdcMapFilterType::Pointer adcMap = AdcMapFilterType::New();
	adcMap->SetInput(this->m_MaskVectorImage);
	adcMap->SetBValueList(this->m_DicomHelper->BvalueList);
	adcMap->Update();

	//DTI claculation
	cout << "DTI is calculating.." << endl;
	typedef itk::GetDiffusionImageFilter<DiffusionCalculatorPixelType, DiffusionCalculatorPixelType> GetDiffusionImageType;
	GetDiffusionImageType::Pointer GetDiffusionImages = GetDiffusionImageType::New();
	GetDiffusionImages->SetInput(adcMap->GetOutput());
	GetDiffusionImages->SetHMatrix(m_DicomHelper->finalH);
	GetDiffusionImages->SetSlice2PatMatrix(m_DicomHelper->slice2PatMatrix);
	GetDiffusionImages->SetBValueList(m_DicomHelper->BvalueList);
	GetDiffusionImages->Update();

	///////////////////////////////////////////
	//ITK to VTK
	typedef itk::ImageToVTKImageFilter <itk::Image<itk::RGBPixel< unsigned char >, 3>> itkToVtkConverter;
	itkToVtkConverter::Pointer convItkToVtk = itkToVtkConverter::New();
	convItkToVtk->SetInput(GetDiffusionImages->GetOutput2());
	convItkToVtk->Update();

	//vtkSmartPointer <itkImageExtractComponents> scalarComponent = vtkSmartPointer <vtkImageExtractComponents>::New();
	//scalarComponent->SetInputData(GetDiffusionImages->GetOutput1());
	//scalarComponent->SetComponents(3,4,5);
	//scalarComponent->Update();//Crutial, otherwise abort after running

	imageData->DeepCopy(convItkToVtk->GetOutput());
	cout << "color FA is finished calculating" << endl;
}

void DiffusionCore::onCalcIVIM(bool _istoggled)
{
	const QString imageName(this->m_Controls->ivimToggle->text());
	const QString imageName_0(imageName + "_F");
	const QString imageName_1(imageName + "_Dstar");
	const QString imageName_2(imageName + "_D");

	if (_istoggled)
	{
		if (m_DicomHelper->BvalueList.size() < 4)
		{
			QMessageBox::StandardButton reply = QMessageBox::information(this, tr("QMessageBox::information()"), tr("Diffusion b valules less than 4, failed to calculate IVIM"));
			this->m_Controls->ivimToggle->blockSignals(true);
			this->m_Controls->ivimToggle->setChecked(false);
			this->m_Controls->ivimToggle->blockSignals(false);
			return;
		}

		if (m_DicomHelper->numberOfGradDirection > 1) // replace with extract the last direction data in future
		{
			QMessageBox::StandardButton reply = QMessageBox::information(this, tr("QMessageBox::information()"), tr("Diffusion Directions must be 1, failed to calculate IVIM"));
			this->m_Controls->ivimToggle->blockSignals(true);
			this->m_Controls->ivimToggle->setChecked(false);
			this->m_Controls->ivimToggle->blockSignals(false);
			return;
		}

		if (m_DicomHelper->BvalueList.at(m_DicomHelper->BvalueList.size() - 1) < 200) // replace with extract the last direction data in future
		{
			QMessageBox::StandardButton reply = QMessageBox::information(this, tr("QMessageBox::information()"), tr("Max b value must be larger than 200 s/mm2, failed to calculate IVIM"));
			this->m_Controls->ivimToggle->blockSignals(true);
			this->m_Controls->ivimToggle->setChecked(false);
			this->m_Controls->ivimToggle->blockSignals(false);
			return;
		}

		for (int i = 301; i < 307; i++)
		{
			ButtonTable->button(i)->blockSignals(true);
			ButtonTable->button(i)->setChecked(false);
			ButtonTable->button(i)->blockSignals(false);
			ButtonTable->button(i)->setEnabled(false);
		}
			
		QWidget *wdwItem_0(this->displayLayout->getWindow(imageName_0));
		QWidget *wdwItem_1(this->displayLayout->getWindow(imageName_1));
		QWidget *wdwItem_2(this->displayLayout->getWindow(imageName_2));

		QVTKWidget* vtkWindow_0;
		QVTKWidget* vtkWindow_1;
		QVTKWidget* vtkWindow_2;

		if (wdwItem_0 != NULL)
		{
			//std::cout << imageName.c_str() << " window exists"<<std::endl;
			vtkWindow_0 = static_cast <QVTKWidget*> (wdwItem_0);
			vtkWindow_1 = static_cast <QVTKWidget*> (wdwItem_1);
			vtkWindow_2 = static_cast <QVTKWidget*> (wdwItem_2);
		}
		else{
			vtkWindow_0 = new QVTKWidget;
			vtkWindow_1 = new QVTKWidget;
			vtkWindow_2 = new QVTKWidget;

			//std::cout << imageName.c_str() << " window not exists, creating window";
			this->displayLayout->insertWindow(vtkWindow_0, imageName_0);
			this->displayLayout->insertWindow(vtkWindow_1, imageName_1);
			this->displayLayout->insertWindow(vtkWindow_2, imageName_2);
			//std::cout << "... window created" << std::endl;
		}

		vtkSmartPointer <vtkImageData> computedIVIM = vtkSmartPointer <vtkImageData>::New();
		this->IVIMCalculator(computedIVIM);
		if (!computedIVIM)
		{
			return;
		}

		IVIMImageViewer(computedIVIM, vtkWindow_0, 0); //IVIM_F = 0
		IVIMImageViewer(computedIVIM, vtkWindow_1, 1);  //IVIM_Dstar = 1
		IVIMImageViewer(computedIVIM, vtkWindow_2, 2);  //IVIM_D = 2
	}
	else
	{ 
		this->displayLayout->removeWindow(imageName_0);
		this->displayLayout->removeWindow(imageName_1);
		this->displayLayout->removeWindow(imageName_2);
	}
}

void DiffusionCore::IVIMCalculator(vtkSmartPointer <vtkImageData> imageData)
{
	if (!this->m_MaskVectorImage) return;

	//IVIM calculator
	std::cout << "------------- IVIM begin runing ----------- " << std::endl;
	typedef itk::DwiIVIMFilter2 <DiffusionCalculatorPixelType, DiffusionCalculatorPixelType> IVIMFilterType;
	IVIMFilterType::Pointer dwiIVIM = IVIMFilterType::New();
	dwiIVIM->SetInput(this->m_MaskVectorImage);
	dwiIVIM->SetBValueList(this->m_DicomHelper->BvalueList);
	dwiIVIM->SetNumOfIterations(50);
	dwiIVIM->Update();
	std::cout << "------------- IVIM end runing ----------- " << std::endl;
	std::cout << "IVIM: vectorLength = " << dwiIVIM->GetOutput()->GetVectorLength() << std::endl;

	//Component 0---------------------------------------
	//vector image to scalar image or imageContainer
	typedef itk::VectorIndexSelectionCastImageFilter <DiffusionCalculatorVectorImageType, DiffusionCalculatorImageType> VectorImageToImageType;
	VectorImageToImageType::Pointer vectorImageToImageFilter = VectorImageToImageType::New();
	vectorImageToImageFilter->SetIndex(0);
	vectorImageToImageFilter->SetInput(dwiIVIM->GetOutput());
	vectorImageToImageFilter->Update();

	//Data Clipping
	typedef itk::DisplayOptimizer < DiffusionCalculatorImageType, DiffusionCalculatorImageType> DisplayOptimizerType;
	DisplayOptimizerType::Pointer displayOptimizer = DisplayOptimizerType::New();
	displayOptimizer->SetInput(vectorImageToImageFilter->GetOutput());
	displayOptimizer->SetCoveragePercent(0.98);//Default is 0.99
	displayOptimizer->Update();

	//Rescale signal intensity to display
	//It doesn't take vector image type, so handle each component seperately
	typedef itk::RescaleIntensityImageFilter < DiffusionCalculatorImageType, DiffusionCalculatorImageType> RescaleIntensityImageType;
	RescaleIntensityImageType::Pointer rescaleFilter = RescaleIntensityImageType::New();
	rescaleFilter->SetInput(displayOptimizer->GetOutput());
	rescaleFilter->SetOutputMaximum(255.0);
	rescaleFilter->SetOutputMinimum(0.0);
	rescaleFilter->Update();

	///////////////////////////////////////////
	//ITK to VTK for visualization
	typedef itk::ImageToVTKImageFilter < DiffusionCalculatorImageType> itkToVtkConverter;
	itkToVtkConverter::Pointer convItkToVtk = itkToVtkConverter::New();
	convItkToVtk->SetInput(rescaleFilter->GetOutput());// changed to rescale filter
	convItkToVtk->Update();
	//}

	vtkSmartPointer <vtkImageAppendComponents> appendComponents = vtkSmartPointer <vtkImageAppendComponents>::New();
	appendComponents->SetInputData(convItkToVtk->GetOutput());

	//Component 1---------------------------------------
	//vector image to scalar image or imageContainer
	//typedef itk::VectorIndexSelectionCastImageFilter <DiffusionCalculatorVectorImageType, DiffusionCalculatorImageType> VectorImageToImageType;
	VectorImageToImageType::Pointer vectorImageToImageFilter1 = VectorImageToImageType::New();
	vectorImageToImageFilter1->SetIndex(1);
	vectorImageToImageFilter1->SetInput(dwiIVIM->GetOutput());
	vectorImageToImageFilter1->Update();

	//Data Clipping
	//typedef itk::DisplayOptimizer < DiffusionCalculatorImageType, DiffusionCalculatorImageType> DisplayOptimizerType;
	DisplayOptimizerType::Pointer displayOptimizer1 = DisplayOptimizerType::New();
	displayOptimizer1->SetInput(vectorImageToImageFilter1->GetOutput());
	displayOptimizer1->SetCoveragePercent(0.98);//Default is 0.99
	displayOptimizer1->Update();

	//Rescale signal intensity to display
	//It doesn't take vector image type, so handle each component seperately
	//typedef itk::RescaleIntensityImageFilter < DiffusionCalculatorImageType, DiffusionCalculatorImageType> RescaleIntensityImageType;
	RescaleIntensityImageType::Pointer rescaleFilter1 = RescaleIntensityImageType::New();
	rescaleFilter1->SetInput(displayOptimizer1->GetOutput());
	rescaleFilter1->SetOutputMaximum(255.0);
	rescaleFilter1->SetOutputMinimum(0.0);
	rescaleFilter1->Update();

	///////////////////////////////////////////
	//ITK to VTK for visualization
	//typedef itk::ImageToVTKImageFilter < DiffusionCalculatorImageType> itkToVtkConverter;
	itkToVtkConverter::Pointer convItkToVtk1 = itkToVtkConverter::New();
	convItkToVtk1->SetInput(rescaleFilter1->GetOutput());// changed to rescale filter
	convItkToVtk1->Update();
	
	appendComponents->AddInputData(convItkToVtk1->GetOutput());

	//Component 2---------------------------------------
	//vector image to scalar image or imageContainer
	//typedef itk::VectorIndexSelectionCastImageFilter <DiffusionCalculatorVectorImageType, DiffusionCalculatorImageType> VectorImageToImageType;
	VectorImageToImageType::Pointer vectorImageToImageFilter2 = VectorImageToImageType::New();
	vectorImageToImageFilter2->SetIndex(2);
	vectorImageToImageFilter2->SetInput(dwiIVIM->GetOutput());
	vectorImageToImageFilter2->Update();

	//Data Clipping
	//typedef itk::DisplayOptimizer < DiffusionCalculatorImageType, DiffusionCalculatorImageType> DisplayOptimizerType;
	DisplayOptimizerType::Pointer displayOptimizer2 = DisplayOptimizerType::New();
	displayOptimizer2->SetInput(vectorImageToImageFilter2->GetOutput());
	displayOptimizer2->SetCoveragePercent(0.98);//Default is 0.99
	displayOptimizer2->Update();

	//Rescale signal intensity to display
	//It doesn't take vector image type, so handle each component seperately
	//typedef itk::RescaleIntensityImageFilter < DiffusionCalculatorImageType, DiffusionCalculatorImageType> RescaleIntensityImageType;
	RescaleIntensityImageType::Pointer rescaleFilter2 = RescaleIntensityImageType::New();
	rescaleFilter2->SetInput(displayOptimizer2->GetOutput());
	rescaleFilter2->SetOutputMaximum(255.0);
	rescaleFilter2->SetOutputMinimum(0.0);
	rescaleFilter2->Update();

	///////////////////////////////////////////
	//ITK to VTK for visualization
	//typedef itk::ImageToVTKImageFilter < DiffusionCalculatorImageType> itkToVtkConverter;
	itkToVtkConverter::Pointer convItkToVtk2 = itkToVtkConverter::New();
	convItkToVtk2->SetInput(rescaleFilter2->GetOutput());// changed to rescale filter
	convItkToVtk2->Update();

	appendComponents->AddInputData(convItkToVtk2->GetOutput());
	appendComponents->Update();
	std::cout << "IVIM calculator imageData components before deepcopy = " << std::endl;
	imageData->DeepCopy(appendComponents->GetOutput());
	std::cout << "IVIM calculator imageData components = " << imageData->GetNumberOfScalarComponents() << std::endl;
}

void DiffusionCore::ShareWindowEvent()
{	
	this->UpdateMaskVectorImage();
	if (!this->m_MaskVectorImage) return;

	for (int i = 301; i < 307; i++)
	{
		if (ButtonTable->button(i)->isChecked())
		{
			ButtonTable->button(i)->blockSignals(true);
			ButtonTable->button(i)->setChecked(false);
			ButtonTable->button(i)->blockSignals(false);
			ButtonTable->button(i)->setChecked(true);
		}
	}
}

void DiffusionCore::onCursorPickValue(bool _istoggled)
{
	if (_istoggled)
	{
		QHash < const QString, QWidget * > currentWindows = displayLayout->getAllWindow();
		QHashIterator<const QString, QWidget * > wdwIter(currentWindows);
		while (wdwIter.hasNext()) {
			wdwIter.next();
			QVTKWidget *thisWindow = static_cast <QVTKWidget*> (wdwIter.value());
			thisWindow->setAutomaticImageCacheEnabled(true);
			Connections = vtkEventQtSlotConnect::New();
			QString imageLabel = wdwIter.key();
			Connections->Connect(thisWindow->GetRenderWindow()->GetInteractor(), vtkCommand::MouseMoveEvent,
				this, SLOT(onDisplayPickValue(vtkObject*, unsigned long, void*, void*, vtkCommand*)), &imageLabel);
		}
	}
	else
	{
		ShareWindowEvent();
	}

}

void DiffusionCore::onDisplayPickValue(vtkObject* obj, unsigned long, void* client_data, void*, vtkCommand*)
{
	//vtkRenderWindow* _renderWindow = static_cast<vtkRenderWindow*>(renderWindow);
	// get interactor
	vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::SafeDownCast(obj);
	QString* _imageLabel = static_cast<QString*>(client_data);

	//vtkRenderWindowInteractor *rwi = _renderWindow->GetInteractor();
	vtkRenderer* renderer = rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
	vtkImageActor* _Actor = static_cast<myVtkInteractorStyleImage*>(rwi->GetInteractorStyle())->GetImageActor();
		/*(vtkImageActor*)rwi->GetRenderWindow()->GetRenderers()
		->GetFirstRenderer()->GetViewProps()->GetItemAsObject(3);*/
		

	_Actor->InterpolateOff();
	vtkCornerAnnotation* _Annotation = (vtkCornerAnnotation*)rwi->GetRenderWindow()->GetRenderers()
		->GetFirstRenderer()->GetViewProps()->GetItemAsObject(2);
	vtkImageData* _image = _Actor->GetMapper()->GetInput();
		//static_cast<myVtkInteractorStyleImage*>(rwi->GetInteractorStyle())->GetInputImage();

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

	if (!validPick)
	{
		_Annotation->SetText(1, "Off Image");
		rwi->Render();
		return;
	}

	// Get the world coordinates of the pick
	double pos[3];
	rwi->GetPicker()->GetPickPosition(pos);
	int image_coordinate[3];

	double spacing[3];
	_image->GetSpacing(spacing);
	
    // vtkImageViewer2::SLICE_ORIENTATION_XY
	image_coordinate[0] = vtkMath::Round(pos[0] / spacing[0]);
	image_coordinate[1] = vtkMath::Round(pos[1] / spacing[1]);
	image_coordinate[2] = 0; // it's 2D image, the third dimension was set to zero.

	std::string message = "Location: ( ";
	message += vtkVariant(image_coordinate[0]).ToString();
	message += ", ";
	message += vtkVariant(image_coordinate[1]).ToString();
	//message += ", ";
	//message += vtkVariant(image_coordinate[2]).ToString();
	message += "]\nValue: ";

	float tuple_float = _image->GetScalarComponentAsFloat(image_coordinate[0], image_coordinate[1], image_coordinate[2], 0);

	tuple_float = tuple_float * m_ScalingParameter.value(*_imageLabel + "_s") + m_ScalingParameter.value(*_imageLabel + "_k");
	tuple_float = int(tuple_float * 100);
	tuple_float = tuple_float / 100;
	message += vtkVariant(tuple_float).ToString();

	if (*_imageLabel == "ADC")
	{
		tuple_float *= 1000;
		message += vtkVariant(tuple_float).ToString();
		message += "*10^(-3)mm2/s";
	}
	else{
		message += vtkVariant(tuple_float).ToString();
	}

	//switch (*_imageLabel)
	//{
	//case "ADC":
	//	tuple_float = tuple_float * m_ScalingParameter[2*ADC] + m_ScalingParameter[2 * ADC+1];

	//	tuple_float *= 1000;  // for 10^-3 display
	//	// mentain the two bits after the decimal point
	//	tuple_float = int(tuple_float * 100);
	//	tuple_float = tuple_float / 100;

	//	message += vtkVariant(tuple_float).ToString();
	//	message += "*10^(-3)mm2/s";
	//	break;
	//case FA:
	//	tuple_float = tuple_float * m_ScalingParameter[2*FA] + m_ScalingParameter[2*FA+1];
	//	//no need scientific notation
	//	tuple_float = int(tuple_float * 100);
	//	tuple_float = tuple_float / 100;
	//	message += vtkVariant(tuple_float).ToString();
	//	break;
	//default:
	//	message = " ";
	//}

	_Annotation->SetText(1, message.c_str());
	renderer->AddActor(_Annotation);
	rwi->Render();
}

void DiffusionCore::addROI() //bool _istoggled
{
	QHash < const QString, QWidget * > currentWindows = displayLayout->getAllWindow();	
	QHashIterator<const QString, QWidget * > wdwIter(currentWindows);
	while (wdwIter.hasNext()) {		
		wdwIter.next();
		if (wdwIter.key() != "ORIGINAL")
		{
			QVTKWidget *thisWindow = static_cast <QVTKWidget*> (wdwIter.value());
			float scalingPara[2];
			scalingPara[0] = m_ScalingParameter.value(wdwIter.key() + '_s');
			scalingPara[1] = m_ScalingParameter.value(wdwIter.key() + '_k');
			qDebug() << wdwIter.key() << ": slope = " << scalingPara[0] << "intercept =" << scalingPara[1];
			myVtkInteractorStyleImage* style = static_cast<myVtkInteractorStyleImage*>(thisWindow->GetRenderWindow()->GetInteractor()->GetInteractorStyle());
			cout << "it's OK" << endl;
			style->GetRoiInteraction()->SetScalingPara(scalingPara);
			style->GetRoiInteraction()->AddWidgetItem();
		}
	}
}

void DiffusionCore::DisplayDicomInfo(vtkSmartPointer <vtkImageData> imageData)
{

	//cout << "fileNames: " << reader->GetFileNames()<< endl;
	//qDebug() << "number of COMPONENTS: " << imageData->GetNumberOfScalarComponents() << endl;

	const int dataDim = imageData->GetDataDimension();
	//int cells = reader->GetOutput()->GetNumberOfCells();
	//int points = reader->GetOutput()->GetNumberOfPoints();
	//cout << "points: " << points << endl;
	//cout << "cells: " << cells << endl;
	int dims[3];
	double origins[3];
	double spacing[3];
	int extent[6];
	//	double center[3];
	double range[2];
	imageData->GetDimensions(dims);
	qDebug() << "image dims: " << dims[0] << "x" << dims[1] << "x" << dims[2] << endl;
	imageData->GetOrigin(origins);
	qDebug() << "image origins: " << origins[0] << " " << origins[1] << " " << origins[2] << endl;
	imageData->GetSpacing(spacing);
	qDebug() << "image spacing: " << spacing[0] << "x" << spacing[1] << "x" << spacing[2] << endl;
	imageData->GetExtent(extent);
	qDebug() << "extent: " << extent[0] << "x" << extent[1] << "x" << extent[2] << "x" << extent[3] << "x" << extent[4] << "x" << extent[5] << endl;
	imageData->GetScalarRange(range);//1. cannot be type of float here, it's a bug of vtk?  2. error while calculating quantitative output images: pipeline should be updated before calling this method!!!
	qDebug() << "range: " << range[0] << "x" << range[1] << endl;
	//std::cout << " imageData: GetScalarType() = " << imageData->GetScalarType() << std::endl;
	//std::cout << " scaleSlope = " << this->m_DicomHelper->scaleSlope << std::endl;
	//std::cout << "scaleIntercept = " << this->m_DicomHelper->scaleIntercept << std::endl;

	//std::cout << "diffusion related parameters---" << std::endl;
	//for (int i = 0; i < this->m_DicomHelper->numberOfComponents; i = i + this->m_DicomHelper->numberOfGradDirection)
	//{
	//	int j = 0;
	//	std::cout << "bValueList " << j << ": " << this->m_DicomHelper->BvalueList.at(i / this->m_DicomHelper->numberOfGradDirection) << std::endl;
	//	j++;
	//}
	//std::cout << "numberOfComponents = " << this->m_DicomHelper->numberOfComponents << std::endl;
	//std::cout << "numberOfGradDirection = " << this->m_DicomHelper->numberOfGradDirection << std::endl;
	//std::cout << "numberOfBValue = " << this->m_DicomHelper->numberOfBValue << std::endl;

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
	m_Controls->infoBrowser->setText(imageInfo);
}

void DiffusionCore::SourceImageViewer2D(vtkSmartPointer <vtkImageData> imageData, QVTKWidget *qvtkWidget)
{
	//double *imageDataRange = new double[2];
	//imageDataRange = imageData->GetScalarRange();//Replace with to be displayed

	vtkSmartPointer<vtkImageViewer2> imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
	imageViewer->SetInputData(imageData);
	imageViewer->SetSliceOrientationToXY();
	//imageViewer->SetColorWindow(imageDataRange[1] - imageDataRange[0]);
	//imageViewer->SetColorLevel(0.5* (imageDataRange[1] + imageDataRange[0]));

	vtkSmartPointer<vtkTextProperty> sliceTextProp = vtkSmartPointer<vtkTextProperty>::New();
	sliceTextProp->SetFontFamilyToCourier();
	sliceTextProp->SetFontSize(18);
	sliceTextProp->SetVerticalJustificationToBottom();
	sliceTextProp->SetJustificationToLeft();

	vtkSmartPointer<vtkTextMapper> sliceTextMapper = vtkSmartPointer<vtkTextMapper>::New();
	std::string msg = StatusMessage::Format(imageViewer->GetSliceMin(), imageViewer->GetSliceMax());
	sliceTextMapper->SetInput(msg.c_str());
	sliceTextMapper->SetTextProperty(sliceTextProp);

	vtkSmartPointer<vtkActor2D> sliceTextActor = vtkSmartPointer<vtkActor2D>::New();
	sliceTextActor->SetMapper(sliceTextMapper);
	sliceTextActor->SetPosition(15, 10);

	imageViewer->GetRenderer()->AddActor2D(sliceTextActor);

	//For Source imageviewer: use renderwindowInteractor. 
	//so that imageviewer pointer would not be released. Otherwise move slice forward/backward will report error, because imageviewer has been released.
	
	//vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	
	vtkSmartPointer<QVTKInteractor> renderWindowInteractor = vtkSmartPointer<QVTKInteractor>::New();
	
    vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle = vtkSmartPointer<myVtkInteractorStyleImage>::New();
	//vtkSmartPointer<vtkTestCallbackCommand> testCallbackCommand = vtkSmartPointer<vtkTestCallbackCommand>::New();

	myInteractorStyle->SetImageViewer(imageViewer);
	myInteractorStyle->SetStatusMapper(sliceTextMapper);
	myInteractorStyle->GetCurrentSliceNumber(m_SourceImageCurrentSlice);

	myInteractorStyle->GetRoiInteraction()->SetQTextBrowser(this->m_Controls->statisBrowser);
	myInteractorStyle->GetRoiInteraction()->SetInteractor(renderWindowInteractor);
	myInteractorStyle->GetRoiInteraction()->SetImageActor(imageViewer->GetImageActor());

	//imageViewer->SetupInteractor(renderWindowInteractor);
	
	renderWindowInteractor->SetInteractorStyle(myInteractorStyle);
	renderWindowInteractor->AddObserver(vtkCommand::MouseWheelForwardEvent, this, &DiffusionCore::ShareWindowEvent);
	renderWindowInteractor->AddObserver(vtkCommand::MouseWheelBackwardEvent, this, &DiffusionCore::ShareWindowEvent);
	//imageViewer->GetRenderWindow()->SetSize(qvtkWidget->width(), qvtkWidget->height());
	//imageViewer->GetRenderer()->SetBackground(0.2, 0.3, 0.4);
	qvtkWidget->SetRenderWindow(imageViewer->GetRenderWindow());
	
	//qvtkWidget->GetRenderWindow()->vtkRenderWindow::SetSize(qvtkWidget->width(), qvtkWidget->height());
	//std::cout << "RenderWindow SIZE = " << *(qvtkWidget->GetRenderWindow()->GetSize()) << std::endl;
	//std::cout << "ImageViewer SIZE = " << *(imageViewer->GetSize()) << std::endl;
	//std::cout << "QVTKWIDEGT SIZE = " << qvtkWidget->width() << "-" << qvtkWidget->height() << std::endl;
	//qvtkWidget->GetRenderWindow()->vtkRenderWindow::SetSize(800, 800);
	//qvtkWidget->GetRenderWindow()->vtkRenderWindow::SetPosition(qvtkWidget->x(), qvtkWidget->y());
	
	qvtkWidget->GetRenderWindow()->SetInteractor(renderWindowInteractor);//crutial to let qvtkWidget share the same interactor with imageViewer

	imageViewer->GetRenderer()->ResetCamera(); //Reset camera and then render is better
	vtkSmartPointer<vtkCamera> camera = imageViewer->GetRenderer()->GetActiveCamera();
	this->SetImageFillWindow(camera, imageData, qvtkWidget->width(), qvtkWidget->height());
	//imageViewer->GetRenderer()->SetActiveCamera(camera);
	//imageViewer->GetRenderer()->SetBackground(0.2, 0.3, 0.4);
	qvtkWidget->GetRenderWindow()->Render();
	renderWindowInteractor->Initialize();
	qvtkWidget->show();
	//renderWindowInteractor->Start();
}

void DiffusionCore::QuantitativeImageViewer2D(vtkSmartPointer <vtkImageData> imageData, QVTKWidget *qvtkWidget, std::string imageLabel)
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

	//vtkSmartPointer<vtkTextMapper> sliceTextMapper = vtkSmartPointer<vtkTextMapper>::New();
	//std::string msg = StatusMessage::Format(imageViewer->GetSliceMin(), imageViewer->GetSliceMax());
	//sliceTextMapper->SetInput(msg.c_str());
	//sliceTextMapper->SetTextProperty(sliceTextProp);

	//vtkSmartPointer<vtkActor2D> sliceTextActor = vtkSmartPointer<vtkActor2D>::New();
	//sliceTextActor->SetMapper(sliceTextMapper);
	//sliceTextActor->SetPosition(15, 10);

	vtkSmartPointer<vtkTextMapper> imageLabelMapper = vtkSmartPointer<vtkTextMapper>::New();
	//std::string msg = StatusMessage::Format(imageViewer->GetSliceMin(), imageViewer->GetSliceMax());
	imageLabelMapper->SetInput(imageLabel.c_str());
	imageLabelMapper->SetTextProperty(sliceTextProp);

	vtkSmartPointer<vtkActor2D>  imageLabelActor = vtkSmartPointer<vtkActor2D>::New();
	imageLabelActor->SetMapper(imageLabelMapper);
	imageLabelActor->SetPosition(0, 10);

	imageViewer->GetRenderer()->AddActor2D(imageLabelActor);
	
	// Annotate the image with window/level and mouse over pixel
	// information
	vtkSmartPointer<vtkCornerAnnotation> cornerAnnotation =
		vtkSmartPointer<vtkCornerAnnotation>::New();
	cornerAnnotation->SetLinearFontScaleFactor(2);
	cornerAnnotation->SetNonlinearFontScaleFactor(1);
	cornerAnnotation->SetMaximumFontSize(20);
	cornerAnnotation->SetText(3, "<window>\n<level>");
	cornerAnnotation->GetTextProperty()->SetColor(0.0, 1.0, 0.0);
	imageViewer->GetRenderer()->AddActor2D(cornerAnnotation);

	//vtkSmartPointer<vtkImageActor> imageActor = vtkSmartPointer<vtkImageActor>::New();
	//imageActor->SetInputData(imageData);
	//imageViewer->GetRenderer()->AddViewProp(imageActor);
	
	//vtkImageActor* imageActor = static_cast<vtkImageActor*>(imageViewer->GetRenderer()->GetActors()->GetItemAsObject(0));
	//vtkImageData* imageData_0 = imageActor->GetInput();
	//cout << "image actor value: " << imageData_0->GetScalarComponentAsFloat(60, 60, 0, 0) << endl;
	//vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();


	//vtkSmartPointer<vtkRoiInteractor> roiInteraction =
	//	vtkSmartPointer<vtkRoiInteractor>::New();
	//roiInteraction->SetQTextBrowser(this->m_Controls->statisBrowser);
	//roiInteraction->SetInteractor(thisWindow->GetRenderWindow()->GetInteractor());
	//roiInteraction->SetScalingPara(scalingPara);
	//Use QVTKInteractor rather than vtkRenderWindowInteractor!!! So that interactor start and end events are handled by qApp->exec() and qApp->Exit();
	vtkSmartPointer<QVTKInteractor> renderWindowInteractor = vtkSmartPointer<QVTKInteractor>::New();
	vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle = vtkSmartPointer<myVtkInteractorStyleImage>::New();

	myInteractorStyle->SetImageViewer(imageViewer);
	myInteractorStyle->GetRoiInteraction()->SetQTextBrowser(this->m_Controls->statisBrowser);
	myInteractorStyle->GetRoiInteraction()->SetInteractor(renderWindowInteractor);
	myInteractorStyle->GetRoiInteraction()->SetImageActor(imageViewer->GetImageActor());
	//myInteractorStyle->SetStatusMapper(sliceTextMapper);
	//myInteractorStyle->GetCurrentSliceNumber(m_QuantitativeImageCurrentSlice);
	
	renderWindowInteractor->SetInteractorStyle(myInteractorStyle);
	//choose the highlight widget 
	///////renderWindowInteractor->AddObserver(vtkCommand::LeftButtonPressEvent, this, &DiffusionCore::HighlightWidget);

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
	//imageViewer->GetRenderer()->SetActiveCamera(camera);
	//imageViewer->GetRenderer()->SetBackground(0.2, 0.3, 0.4);
	qvtkWidget->GetRenderWindow()->Render();
	renderWindowInteractor->Initialize();
	qvtkWidget->show();//qvtkWidget->show() changes window size!!!!
	//std::cout << "QVTKWIDEGT SIZE after show= " << qvtkWidget->width() << "-" << qvtkWidget->height() << std::endl;
}

void DiffusionCore::IVIMImageViewer(vtkSmartPointer <vtkImageData> imageData, QVTKWidget *qvtkWidget, int imageIdx)
{
	vtkSmartPointer <vtkLookupTable> lookupTable = vtkSmartPointer <vtkLookupTable>::New();
	lookupTable->SetNumberOfTableValues(256);//try below color range
	lookupTable->SetTableRange(0.1, 255.1);//try below color range
	lookupTable->SetHueRange(0.66667,0.0);//rainbow color map: from blue to red
	lookupTable->UseBelowRangeColorOn();
	lookupTable->SetBelowRangeColor(0.0,0.0,0.0,1.0);
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

void DiffusionCore::SetImageFillWindow(vtkSmartPointer <vtkCamera> & camera, vtkSmartPointer <vtkImageData> imageData, double width, double height){

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

	double parallelScale = imageRatio > screenRatio ? yFov : screenRatio/imageRatio*yFov;

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