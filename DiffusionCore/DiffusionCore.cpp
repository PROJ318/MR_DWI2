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

#include <itkAdcMapFilter.h>
#include <itkComputedDwiFilter.h>
#include <itkComputedEadcFilter.h>
#include <itkMaskVectorImageFilter.h>
#include <itkDisplayOptimizer.h>
#include <itkDwiIVIMFilter2.h>
#include <itkGetDiffusionImageFilter.h>
#include <itkTensor.h>


#include <DicomHelper.h>

#include <vtkOrientedGlyphContourRepresentation.h>
#include <vtkImageActorPointPlacer.h>

//TRY SetNumberOfThreads(1) to solve the multi-thread ranmdom results
//Notify Wenxing
#define DEFAULTTHRESH 10
#define DEFAULTB 2000

DiffusionCore::DiffusionCore(QWidget *parent)
	:QWidget(parent)
	//, dicomHelp(nullptr)
{	
	this->m_Controls = nullptr;	
	this->ButtonTable = new QButtonGroup;
	this->m_MaskVectorImage = DiffusionCalculatorVectorImageType::New();
	m_MaskThreshold = DEFAULTTHRESH/5;

	CreateQtPartControl(this);
}

DiffusionCore::~DiffusionCore()
{
	//delete m_Controls;
	//this->m_Controls = NULL;
	this->m_MaskVectorImage->Delete();
	this->m_MaskVectorImage = ITK_NULLPTR;
}

void DiffusionCore::CreateQtPartControl(QWidget *parent)
{
	if (!m_Controls)
	{				
		this->m_Controls = new Ui::DiffusionModule;
		this->m_Controls->setupUi(parent);		

		m_Controls->bSlider->setMaximum(10000); //maximum supported b value;
		m_Controls->bSlider->setMinimum(500);
		m_Controls->bSlider->setDecimals(0);
		m_Controls->bSlider->setSingleStep(100);
		m_Controls->bSlider->setTickInterval(100);
		m_Controls->bSlider->setValue(DEFAULTB);
		m_Controls->bSlider->setTracking(false);
		m_Controls->ThreshSlider->setMaximum(100); //maximum threshhold value;
		m_Controls->ThreshSlider->setDecimals(0);
		m_Controls->ThreshSlider->setSingleStep(1);
		m_Controls->ThreshSlider->setTickInterval(1);
		m_Controls->ThreshSlider->setValue(DEFAULTTHRESH);
		m_Controls->ThreshSlider->setTracking(false);
		//connect Buttons and handle visibility

		this->m_Controls->ADCTool->setDisabled(true);
		this->m_Controls->DTITool->setDisabled(true);
		this->m_Controls->ivimToggle->setDisabled(true);
		

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


		//Connect Sliders
		connect(m_Controls->ThreshSlider, SIGNAL(valueChanged(double)), this, SLOT(onThreshSlide(double)));
		connect(m_Controls->bSlider, SIGNAL(valueChanged(double)), this, SLOT(onBSlide(double)));
		
		m_Controls->cDWI->hide();
		m_Controls->Thresh->hide();		
	}
}

void DiffusionCore::onSetSourceImage(DicomHelper* dicomData, int inputSlice)
{
	
	m_DicomHelper = dicomData;
	
	//m_CurrentSlice = inputSlice;
	
	//
	//Enable/Disable Buttons.
	//

	//this->m_Controls->Thresh->setVisible(false);

	if (m_DicomHelper->tensorComputationPossible)
	{
		std::cout << "[SetSourceImage] DTI OK ";
		this->m_Controls->DTITool->setEnabled(true);
	}else{
		//this->ui->dtiNameTag->setText("Data does not contain multiple direction, view Only");
		if (m_DicomHelper->numberOfBValue >= 2)
		{
			std::cout << "[SetSourceImage] DTI NOT OK, DWI OK" << std::endl;
			this->m_Controls->ADCTool->setEnabled(true);
			if (m_DicomHelper->numberOfBValue >= 4)
			{
				this->m_Controls->ivimToggle->setEnabled(true);
			}
		}
	}

	onRecalcAll(inputSlice); //recalculated all calculated images 
}

//void DiffusionCore::onTestButton(bool _istoggled)
//{
//	std::cout << "test button is clicked" << std::endl;
//
//	vtkSmartPointer <vtkImageData> calculatedAdc;
//	const QString imageName(m_Controls->pushButton->text());
//
//	float scale(0.0), slope(0.0);
//	if (_istoggled)
//	{
//		calculatedAdc = vtkSmartPointer <vtkImageData>::New();
//		
//		this->AdcCalculator(calculatedAdc, scale, slope);
//	}
//	else
//	{
//		calculatedAdc = NULL;		
//	}
//
//	std::cout << "test button fired" << std::endl;
//
//	emit SignalTestButtonFired(_istoggled, calculatedAdc, imageName, scale, slope);
//}

void DiffusionCore::onCalcADC(bool _istoggled) //SLOT of adcToggle
{
	vtkSmartPointer <vtkImageData> calculatedAdc;
	const QString imageName(this->m_Controls->adcToggle->text());
	float scale(0.0), slope(0.0);

	if (_istoggled)
	{
		this->m_Controls->Thresh->setVisible(_istoggled);
		calculatedAdc = vtkSmartPointer <vtkImageData>::New();
		this->AdcCalculator(calculatedAdc, scale, slope);
	}
	else
	{
		calculatedAdc = NULL;
	}
	emit SignalTestButtonFired(_istoggled, calculatedAdc, imageName, scale, slope);

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
	vtkSmartPointer <vtkImageData> calculatedEAdc;
	const QString imageName(this->m_Controls->eadcToggle->text());
	float scale(0.0), slope(0.0);

	if (_istoggled)
	{
		this->m_Controls->Thresh->setVisible(_istoggled);

		calculatedEAdc = vtkSmartPointer <vtkImageData>::New();

		this->EAdcCalculator(calculatedEAdc, scale, slope);

	}
	else
	{
		calculatedEAdc = NULL;
	}

	emit SignalTestButtonFired(_istoggled, calculatedEAdc, imageName, scale, slope);
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
	vtkSmartPointer <vtkImageData> computedDwi;
	const QString imageName(this->m_Controls->cdwiToggle->text());
	float scale(0.0), slope(0.0);

	if (_istoggled)
	{
		//Set Threshhold bar visible
		this->m_Controls->Thresh->setVisible(_istoggled);
		this->m_Controls->cDWI->setVisible(_istoggled);	

		computedDwi = vtkSmartPointer <vtkImageData>::New();

		this->CDWICalculator(computedDwi, scale, slope);

	}
	else
	{
		computedDwi = NULL;
	}

	emit SignalTestButtonFired(_istoggled, computedDwi, imageName, scale, slope);
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
	m_MaskThreshold = maskThreshold/5; //slider value is 5 fold of real value. 
	DiffusionCore::onRecalcAll(m_CurrentSlice);
}

void DiffusionCore::onBSlide(double computedBValue) //SLOT of cdwiToggle
{
	const QString imageName(this->m_Controls->cdwiToggle->text());
	m_ComputedBValue = computedBValue;
	float scale(0.0), slope(0.0);
	vtkSmartPointer <vtkImageData> computedDwi = vtkSmartPointer <vtkImageData>::New();	
	this->CDWICalculator(computedDwi, scale, slope);
	emit SignalTestButtonFired(this->m_Controls->cdwiToggle->isChecked(), computedDwi, imageName, scale, slope);
}

void DiffusionCore::onCalcFA(bool _istoggled)  //SLOT of faToggle
{
	vtkSmartPointer <vtkImageData> calculatedFA;
	const QString imageName(this->m_Controls->faToggle->text());
	float scale(0.0), slope(0.0);

	if (_istoggled)
	{	
		this->m_Controls->Thresh->setVisible(_istoggled);
		calculatedFA = vtkSmartPointer <vtkImageData>::New();
		this->FaCalculator(calculatedFA,scale,slope);
	}
	else
	{
		calculatedFA = NULL;
	}

	emit SignalTestButtonFired(_istoggled, calculatedFA, imageName, scale, slope);
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
	vtkSmartPointer <vtkImageData> calculatedColorFA;
	const QString imageName(this->m_Controls->colorFAToggle->text());
	float scale(0.0), slope(0.0);

	if (_istoggled)
	{
		//Set Threshhold bar visible
		this->m_Controls->Thresh->setVisible(_istoggled);
		calculatedColorFA = vtkSmartPointer <vtkImageData>::New();
		this->ColorFACalculator(calculatedColorFA);
	}
	else
	{
		calculatedColorFA = NULL;
	}
	emit SignalTestButtonFired(_istoggled, calculatedColorFA, imageName, scale, slope);
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
	vtkSmartPointer <vtkImageData> computedIVIM;

	const QString imageName(this->m_Controls->ivimToggle->text());
	const QString imageName_0(imageName + "_F");
	const QString imageName_1(imageName + "_Dstar");
	const QString imageName_2(imageName + "_D");
	float scale(0.0), slope(0.0);

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
			
		computedIVIM = vtkSmartPointer <vtkImageData>::New();
		this->IVIMCalculator(computedIVIM);

	}
	else
	{ 
		computedIVIM = NULL;
	}
	emit SignalTestButtonFired(_istoggled, computedIVIM, imageName_0, scale, slope);
	emit SignalTestButtonFired(_istoggled, computedIVIM, imageName_1, scale, slope);
	emit SignalTestButtonFired(_istoggled, computedIVIM, imageName_2, scale, slope);
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

void DiffusionCore::onRecalcAll(int inputSlice)
{		
	if (inputSlice >= 0)
	{
		//std::cout << "[onRecalcAll] updating slice" << endl;
		m_CurrentSlice = inputSlice;
		UpdateMaskVectorImage(m_DicomHelper, this->m_MaskVectorImage);
		//m_vectorImage.insert(inputSlice, m_MaskVectorImage);
		if (!this->m_MaskVectorImage) return;

		//Retrigger All Checked Buttons. 
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
	else
	{
		std::cout << "[onRecalcAll] ERROR input slice is negative" << endl;
	}
}

void DiffusionCore::UpdateMaskVectorImage(DicomHelper* dicomData, DiffusionCalculatorVectorImageType::Pointer _MaskVectorImage)
{

	// currently implementation, for GE data; need a decent way!!!
	//typedef T SourceImagePixelType;
	//typedef itk::Image < SourceImagePixelType, 3> SourceImageType;
	cout << "[UpdateMaskVectorImage] Slicing Source Image at " <<m_CurrentSlice<< endl;

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
	ExtractVOI->SetInputData(dicomData->DicomReader->GetOutput());
	ExtractVOI->SetVOI(0, this->m_DicomHelper->imageDimensions[0] - 1, 0, this->m_DicomHelper->imageDimensions[1] - 1, m_CurrentSlice, m_CurrentSlice);
	ExtractVOI->Update();
	//std::cout << "---------------------------- VOI is correct ? ---------------------" << std::endl;
	//std::cout << "Current Slice is " << m_CurrentSlice << std::endl;
	//this->DisplayDicomInfo(ExtractVOI->GetOutput());
	//std::cout << "---------------------------- End of VOI ---------------------" << std::endl;

	//PR: mask image doesn't change for slices other than the first one whlie sliding the maskthreshold slider
	//fix extent range here, default z extent is inputSlice to inputSlice;
	//Update image extent, otherwise maskFilter won't work for slices other than the first one (because seed starts at origin)
	//Update the origin as well
	vtkSmartPointer <vtkImageChangeInformation> changeInfo = vtkSmartPointer <vtkImageChangeInformation>::New();
	changeInfo->SetInputData(ExtractVOI->GetOutput());
	changeInfo->SetOutputOrigin(0, 0, 0);
	changeInfo->SetExtentTranslation(0, 0, -m_CurrentSlice);
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
	_MaskVectorImage->SetSpacing(maskFilter->GetOutput()->GetSpacing());
	_MaskVectorImage->SetOrigin(maskFilter->GetOutput()->GetOrigin());
	_MaskVectorImage->SetDirection(maskFilter->GetOutput()->GetDirection());
	_MaskVectorImage->SetRegions(maskFilter->GetOutput()->GetLargestPossibleRegion());
	_MaskVectorImage->SetVectorLength(maskFilter->GetOutput()->GetVectorLength());
	//std::cout << " m_MaskVectorImage length 00 0000000= " << this->m_MaskVectorImage->GetVectorLength() << std::endl;
	_MaskVectorImage->Allocate();
	//std::cout << " m_MaskVectorImage length 00 = " << this->m_MaskVectorImage->GetVectorLength() << std::endl;


	typedef itk::ImageRegionConstIterator <DiffusionCalculatorVectorImageType> ConstMaskFilterIteratorType;
	typedef itk::ImageRegionIterator <DiffusionCalculatorVectorImageType> MaskVectorImageIteratorType;

	ConstMaskFilterIteratorType maskFilterIterator(maskFilter->GetOutput(), maskFilter->GetOutput()->GetLargestPossibleRegion());
	MaskVectorImageIteratorType maskVectorImageIterator(_MaskVectorImage, _MaskVectorImage->GetLargestPossibleRegion());

	maskFilterIterator.GoToBegin();
	maskVectorImageIterator.GoToBegin();
	while (!maskFilterIterator.IsAtEnd())
	{
		maskVectorImageIterator.Set(maskFilterIterator.Get());
		++maskFilterIterator;
		++maskVectorImageIterator;
	}
	
}

