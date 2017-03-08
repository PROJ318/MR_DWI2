#include "PerfusionCore.h"
#include "ui_PerfusionCore.h"

//vtk
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkExtractVOI.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageGaussianSmooth.h>
#include <QVTKWidget.h>
#include <QVTKInteractor.h>

#include <itkCastImageFilter.h>
#include <itkComposeImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkVTKImageToImageFilter.h>
#include <itkVectorImageToImageAdaptor.h>

//customize
#include <DicomHelper.h>
#include <../DiffusionCore/itkMaskVectorImageFilter.h>
#include <itkPerfusionMapFilter.h>

	
// Qt
#include <QCheckBox>
#include <QMessageBox>
#include <QDialog>
#include <QMouseEvent>
#include <QGroupBox>
//#include <qbuttongroup.h>

#define DEFAULTTHRESH 10
PerfusionCore::PerfusionCore(QWidget *parent)
	:QWidget(parent)
	//, dicomHelp(nullptr)
{
	this->m_Controls = nullptr;
	this->ButtonTable = new QButtonGroup;
	this->m_MaskVectorImage = PerfusionCalculatorVectorImageType::New();
	m_MaskThreshold = DEFAULTTHRESH / 5;

	CreateQtPartControl(this);
}

PerfusionCore::~PerfusionCore()
{

}

void PerfusionCore::onSetSourceImage(DicomHelper* dicomData, int inputSlice)
{

	m_DicomHelper = dicomData;
	std::cout << m_DicomHelper->imageDataType;

	if (m_DicomHelper->imageDataType.compare("PERFUSION") != 0)
	{
		cout << "not PERFUSION" << endl;
		return;
	}
	else
	{
		for (int i = 401; i < 409; i++)
		{
			if (ButtonTable->button(i)->isChecked())
			{
				ButtonTable->button(i)->setChecked(false);
			}
		}

		cout << "image TYPE" << m_DicomHelper->imageDataType.c_str() << endl;
		this->m_Controls->PerfusionTool->setEnabled(true);
		this->m_Controls->PerfusionTool2->setEnabled(true);
		this->m_Controls->PerfusionTool3->setEnabled(true);
		//m_CurrentSlice = inputSlice;
		//cout << "current slice" << m_CurrentSlice << endl;
		//UpdateMaskVectorImage(m_DicomHelper, this->m_MaskVectorImage);
		//calculate the 
		onRecalcAll(inputSlice); //recalculated all calculated images 
	}
}

void PerfusionCore::onCalcRelativeEnhance(bool _istoggled)
{
	cout << "calculating perfusion parameters." << endl;
	vtkSmartPointer <vtkImageData> calculatedT1P;
	const QString imageName(this->m_Controls->REToggle->text());

	float scale(0.0), slope(0.0);

	if (_istoggled)
	{
		this->m_Controls->Thresh->setVisible(_istoggled);
		calculatedT1P = vtkSmartPointer <vtkImageData>::New();
		this->PerfusionCalculator(calculatedT1P, scale, slope, 0);
	}
	else
	{
		calculatedT1P = NULL;
	}
	cout << "perfusion imags are calculated" << endl;
	emit SignalTestButtonFired(_istoggled, calculatedT1P, imageName, scale, slope);
}

void PerfusionCore::onCalcMaxRelativeEnhance(bool _istoggled)
{
	cout << "calculating perfusion parameters." << endl;
	vtkSmartPointer <vtkImageData> calculatedT1P;
	const QString imageName(this->m_Controls->MREToggle->text());

	float scale(0.0), slope(0.0);

	if (_istoggled)
	{
		this->m_Controls->Thresh->setVisible(_istoggled);
		calculatedT1P = vtkSmartPointer <vtkImageData>::New();
		this->PerfusionCalculator(calculatedT1P, scale, slope, 1);
	}
	else
	{
		calculatedT1P = NULL;
	}
	cout << "perfusion imags are calculated" << endl;
	emit SignalTestButtonFired(_istoggled, calculatedT1P, imageName, scale, slope);
}

void PerfusionCore::onCalcMaxEnhance(bool _istoggled)
{
	cout << "calculating perfusion parameters." << endl;
	vtkSmartPointer <vtkImageData> calculatedT1P;
	const QString imageName(this->m_Controls->METoggle->text());

	float scale(0.0), slope(0.0);

	if (_istoggled)
	{
		this->m_Controls->Thresh->setVisible(_istoggled);
		calculatedT1P = vtkSmartPointer <vtkImageData>::New();
		this->PerfusionCalculator(calculatedT1P, scale, slope, 2);
	}
	else
	{
		calculatedT1P = NULL;
	}
	cout << "perfusion imags are calculated" << endl;
	emit SignalTestButtonFired(_istoggled, calculatedT1P, imageName, scale, slope);
}

void PerfusionCore::onCalcWashOutRate(bool _istoggled)
{
	cout << "calculating perfusion parameters." << endl;
	vtkSmartPointer <vtkImageData> calculatedT1P;
	const QString imageName(this->m_Controls->WORToggle->text());

	float scale(0.0), slope(0.0);

	if (_istoggled)
	{
		this->m_Controls->Thresh->setVisible(_istoggled);
		calculatedT1P = vtkSmartPointer <vtkImageData>::New();
		this->PerfusionCalculator(calculatedT1P, scale, slope, 3);
	}
	else
	{
		calculatedT1P = NULL;
	}
	cout << "perfusion imags are calculated" << endl;
	emit SignalTestButtonFired(_istoggled, calculatedT1P, imageName, scale, slope);
}

void PerfusionCore::onCalcWashInRate(bool _istoggled)
{
	cout << "calculating perfusion parameters." << endl;
	vtkSmartPointer <vtkImageData> calculatedT1P;
	const QString imageName(this->m_Controls->WIRToggle->text());

	float scale(0.0), slope(0.0);

	if (_istoggled)
	{
		this->m_Controls->Thresh->setVisible(_istoggled);
		calculatedT1P = vtkSmartPointer <vtkImageData>::New();
		this->PerfusionCalculator(calculatedT1P, scale, slope, 4);
	}
	else
	{
		calculatedT1P = NULL;
	}
	cout << "perfusion imags are calculated" << endl;
	emit SignalTestButtonFired(_istoggled, calculatedT1P, imageName, scale, slope);
}

void PerfusionCore::onCalcAreaUnderCurve(bool _istoggled)
{
	cout << "calculating perfusion parameters." << endl;
	vtkSmartPointer <vtkImageData> calculatedT1P;
	const QString imageName(this->m_Controls->AreaToggle->text());

	float scale(0.0), slope(0.0);

	if (_istoggled)
	{
		this->m_Controls->Thresh->setVisible(_istoggled);
		calculatedT1P = vtkSmartPointer <vtkImageData>::New();
		this->PerfusionCalculator(calculatedT1P, scale, slope, 5);
	}
	else
	{
		calculatedT1P = NULL;
	}
	cout << "perfusion imags are calculated" << endl;
	emit SignalTestButtonFired(_istoggled, calculatedT1P, imageName, scale, slope);
}

void PerfusionCore::onCalcT0(bool _istoggled)
{
	cout << "calculating perfusion parameters." << endl;
	vtkSmartPointer <vtkImageData> calculatedT1P;
	const QString imageName(this->m_Controls->T0Toggle->text());

	float scale(0.0), slope(0.0);

	if (_istoggled)
	{
		this->m_Controls->Thresh->setVisible(_istoggled);
		calculatedT1P = vtkSmartPointer <vtkImageData>::New();
		this->PerfusionCalculator(calculatedT1P, scale, slope, 6);
	}
	else
	{
		calculatedT1P = NULL;
	}
	cout << "perfusion imags are calculated" << endl;
	emit SignalTestButtonFired(_istoggled, calculatedT1P, imageName, scale, slope);
}

void PerfusionCore::onCalcTimetoPeak(bool _istoggled)
{
	cout << "calculating perfusion parameters." << endl;
	vtkSmartPointer <vtkImageData> calculatedT1P;
	const QString imageName(this->m_Controls->TTPToggle->text());

	float scale(0.0), slope(0.0);

	if (_istoggled)
	{
		this->m_Controls->Thresh->setVisible(_istoggled);
		calculatedT1P = vtkSmartPointer <vtkImageData>::New();
		this->PerfusionCalculator(calculatedT1P, scale, slope, 7);
	}
	else
	{
		calculatedT1P = NULL;
	}
	cout << "perfusion imags are calculated" << endl;
	emit SignalTestButtonFired(_istoggled, calculatedT1P, imageName, scale, slope);
}

void PerfusionCore::onCalcBrevityOfEnhance(bool _istoggled)
{
	cout << "calculating perfusion parameters." << endl;
	vtkSmartPointer <vtkImageData> calculatedT1P;
	const QString imageName(this->m_Controls->BOEToggle->text());

	float scale(0.0), slope(0.0);

	if (_istoggled)
	{
		this->m_Controls->Thresh->setVisible(_istoggled);
		calculatedT1P = vtkSmartPointer <vtkImageData>::New();
		this->PerfusionCalculator(calculatedT1P, scale, slope, 8);
	}
	else
	{
		calculatedT1P = NULL;
	}
	cout << "perfusion imags are calculated" << endl;
	emit SignalTestButtonFired(_istoggled, calculatedT1P, imageName, scale, slope);
}

void PerfusionCore::CreateQtPartControl(QWidget *parent)
{
	if (!m_Controls)
	{
		this->m_Controls = new Ui::PerfusionModule;
		this->m_Controls->setupUi(parent);

		m_Controls->ThreshSlider->setMaximum(100); //maximum threshhold value;
		m_Controls->ThreshSlider->setDecimals(0);
		m_Controls->ThreshSlider->setSingleStep(1);
		m_Controls->ThreshSlider->setTickInterval(1);
		m_Controls->ThreshSlider->setValue(DEFAULTTHRESH);
		m_Controls->ThreshSlider->setTracking(false);
		//connect Buttons and handle visibility

		this->m_Controls->PerfusionTool->setDisabled(true);
		this->m_Controls->PerfusionTool2->setDisabled(true);
		this->m_Controls->PerfusionTool3->setDisabled(true);

		ButtonTable->setExclusive(false); //non-exclusive button group. 
		ButtonTable->addButton(m_Controls->REToggle,   401);
		ButtonTable->addButton(m_Controls->MREToggle,  402);
		ButtonTable->addButton(m_Controls->METoggle,   403);
		ButtonTable->addButton(m_Controls->WORToggle,  404);
		ButtonTable->addButton(m_Controls->WIRToggle,  405);
		ButtonTable->addButton(m_Controls->AreaToggle, 406);
		ButtonTable->addButton(m_Controls->T0Toggle,   407);
		ButtonTable->addButton(m_Controls->TTPToggle,  408);
		ButtonTable->addButton(m_Controls->BOEToggle,  409);

		////Connect Toggles
		connect(m_Controls->REToggle,   SIGNAL(toggled(bool)), this, SLOT(onCalcRelativeEnhance(bool)));
		connect(m_Controls->MREToggle,  SIGNAL(toggled(bool)), this, SLOT(onCalcMaxRelativeEnhance(bool)));
		connect(m_Controls->METoggle,   SIGNAL(toggled(bool)), this, SLOT(onCalcMaxEnhance(bool)));
		connect(m_Controls->WORToggle,  SIGNAL(toggled(bool)), this, SLOT(onCalcWashOutRate(bool)));
		connect(m_Controls->WIRToggle,  SIGNAL(toggled(bool)), this, SLOT(onCalcWashInRate(bool)));
		connect(m_Controls->AreaToggle, SIGNAL(toggled(bool)), this, SLOT(onCalcAreaUnderCurve(bool)));
		connect(m_Controls->T0Toggle,   SIGNAL(toggled(bool)), this, SLOT(onCalcT0(bool)));
		connect(m_Controls->TTPToggle,  SIGNAL(toggled(bool)), this, SLOT(onCalcTimetoPeak(bool)));
		connect(m_Controls->BOEToggle,  SIGNAL(toggled(bool)), this, SLOT(onCalcBrevityOfEnhance(bool)));

		////Connect Sliders
		connect(m_Controls->ThreshSlider, SIGNAL(valueChanged(double)), this, SLOT(onThreshSlide(double)));
		//connect(m_Controls->bSlider, SIGNAL(valueChanged(double)), this, SLOT(onBSlide(double)));	
		m_Controls->Thresh->hide();
	}
}

void PerfusionCore::PerfusionCalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope, int index)
{
	if (!this->m_MaskVectorImage) return;
	std::cout << "------------- perfusionMapFilter begin runing ----------- " << std::endl;
	cout << "number of components: " << this->m_MaskVectorImage->GetNumberOfComponentsPerPixel() << endl;
	//// Adc Map filter
	//typedef itk::PerfusionMapFilter <PerfusionCalculatorPixelType, PerfusionCalculatorPixelType> PerfusionMapFilterType;
	//PerfusionMapFilterType::Pointer perfusionMap = PerfusionMapFilterType::New();
	//perfusionMap->SetInput(this->m_MaskVectorImage);
	//perfusionMap->setimageTimer(this->m_DicomHelper->dynamicTime);
	//perfusionMap->Update();

	typedef itk::ComposeImageFilter<SourceImageType, itk::Image<itk::Vector<PerfusionCalculatorPixelType, 8>,3>>  CasterType;
	CasterType::Pointer Caster = CasterType::New();

	std::cout << " m_MaskVectorImage length 00 = " << this->m_MaskVectorImage->GetVectorLength() << std::endl;
	//for (int i = 0; i < 8; i++)
	//{ 
		typedef itk::VectorIndexSelectionCastImageFilter <PerfusionCalculatorVectorImageType, PerfusionCalculatorImageType> VectorImageToImageType;
		VectorImageToImageType::Pointer vectorImageToImageFilter = VectorImageToImageType::New();
		vectorImageToImageFilter->SetInput(this->m_MaskVectorImage);
		vectorImageToImageFilter->SetIndex(index);
		vectorImageToImageFilter->Update();
		std::cout << "------------- rescaleFilter begin runing ----------- " << std::endl;

		//Rescale signal intensity to display
		typedef itk::RescaleIntensityImageFilter < PerfusionCalculatorImageType, SourceImageType> RescaleIntensityImageType;
		RescaleIntensityImageType::Pointer rescaleFilter = RescaleIntensityImageType::New();
		rescaleFilter->SetInput(vectorImageToImageFilter->GetOutput());
		rescaleFilter->SetOutputMaximum(4095);
		rescaleFilter->SetOutputMinimum(0);
		rescaleFilter->Update();

		scale = 1 / rescaleFilter->GetScale();
		slope = -1 * rescaleFilter->GetShift() / rescaleFilter->GetScale();
		std::cout << "rescaleFilter: inputMaximum = " << scale << std::endl;
		//std::cout << "rescaleFilter: inputMinimum = " << m_ScalingParameter[2 * ADC + 1] << std::endl;
		//imageToVectorImageFilter->SetInput(i, rescaleFilter->GetOutput());



		//Caster->SetInput(i, rescaleFilter->GetOutput());
		//if (i == 0)
		//{
		//	imageAppend->SetInputData(convItkToVtk->GetOutput());
		//}
		//Data Clipping
		//typedef itk::DisplayOptimizer < DiffusionCalculatorImageType, SourceImageType> DisplayOptimizerType;
		//DisplayOptimizerType::Pointer displayOptimizer = DisplayOptimizerType::New();
		//displayOptimizer->SetInput(rescaleFilter->GetOutput());
		//displayOptimizer->SetCoveragePercent(0.98);//Default is 0.99
		//displayOptimizer->Update();
	//}
	//Caster->Update();
	//imageToVectorImageFilter->Update();
	///////////////////////////////////////////
	//ITK to VTK
		typedef itk::ImageToVTKImageFilter <SourceImageType> itkToVtkConverter;
	itkToVtkConverter::Pointer convItkToVtk = itkToVtkConverter::New();
	convItkToVtk->SetInput(rescaleFilter->GetOutput());
	convItkToVtk->Update();

	imageData->DeepCopy(convItkToVtk->GetOutput());
	cout << "ADC calculator: " << imageData->GetNumberOfScalarComponents() << endl;
}

void PerfusionCore::UpdateMaskVectorImage(DicomHelper* dicomData, PerfusionCalculatorVectorImageType::Pointer _MaskVectorImage)
{

	// currently implementation, for GE data; need a decent way!!!
	//typedef T SourceImagePixelType;
	//typedef itk::Image < SourceImagePixelType, 3> SourceImageType;
	cout << "[UpdateMaskVectorImage] Slicing Source Image at " << m_CurrentSlice << endl;

	typedef itk::VectorContainer< SourceImagePixelType, PerfusionCalculatorImageType::Pointer > ImageContainerType;
	typedef itk::VTKImageToImageFilter <SourceImageType>	VtkToItkConverterType;
	typedef itk::CastImageFilter< SourceImageType, PerfusionCalculatorImageType >	CastFilterType;
	typedef itk::ShiftScaleImageFilter<PerfusionCalculatorImageType, PerfusionCalculatorImageType>	ShiftScaleType;


	ImageContainerType::Pointer imageContainer = ImageContainerType::New();
	//if (m_DicomHelper->tensorComputationPossible && (m_DicomHelper->IsoImageLabel > -1))
	//{		
	//	imageContainer->Reserve(this->m_DicomHelper->numberOfComponents -1);// only works for DTI b value 2, otherwise wrong
	//}
	//else
	//{
	imageContainer->Reserve(this->m_DicomHelper->numberOfComponents);
	//}
	std::cout << "m_DicomHelper numof components" << this->m_DicomHelper->numberOfComponents << std::endl;

	vtkSmartPointer <vtkExtractVOI> ExtractVOI = vtkSmartPointer <vtkExtractVOI>::New();
	ExtractVOI->SetInputData(dicomData->imageData);
	ExtractVOI->SetVOI(0, this->m_DicomHelper->imageDimensions[0] - 1, 0, this->m_DicomHelper->imageDimensions[1] - 1, m_CurrentSlice, m_CurrentSlice);
	ExtractVOI->Update();
	std::cout << "---------------------------- VOI is correct ? ---------------------" << std::endl;
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

	std::cout << "---------------------------- translateExtent is correct ? ---------------------" << std::endl;
	//this->DisplayDicomInfo(changeInfo->GetOutput());
	//std::cout << "---------------------------- End of translateExtent ---------------------" << std::endl;
	//std::cout << " update vector image DTI numberOfComponents " << m_DicomHelper->numberOfComponents << std::endl;
	for (int i = 0; i < this->m_DicomHelper->numberOfComponents; i++)
	{
		//Handle each scalar component indivisually
		vtkSmartPointer <vtkImageExtractComponents> scalarComponent = vtkSmartPointer <vtkImageExtractComponents>::New();
		scalarComponent->SetInputData(changeInfo->GetOutput());
		scalarComponent->SetComponents(i);
		scalarComponent->Update();//Crutial, otherwise abort after running

		////double* spacing = imageData->GetSpacing();
		//vtkSmartPointer<vtkImageGaussianSmooth> gaussianFilter =
		//	vtkSmartPointer<vtkImageGaussianSmooth>::New();
		//gaussianFilter->SetDimensionality(2);
		//gaussianFilter->SetInputData(scalarComponent->GetOutput());
		//gaussianFilter->SetStandardDeviations(1.0, 1.0);
		////gaussianFilter->SetRadiusFactors(imageDimensions[0], imageDimensions[1], 1);
		//gaussianFilter->Update();

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

		//typedef itk::MedianImageFilter<DiffusionCalculatorImageType, DiffusionCalculatorImageType > FilterType;
		//FilterType::Pointer medianFilter = FilterType::New();
		//FilterType::InputSizeType radius;
		//radius.Fill(1);
		//medianFilter->SetRadius(radius);
		//medianFilter->SetInput(shiftScale->GetOutput());
		//medianFilter->Update();


		//removing the isotropic direction for DTI has been implemented in the sorting source image.
		//std::cout << " Update vector image DWI" << std::endl;
		imageContainer->InsertElement(i, dynamic_cast <PerfusionCalculatorImageType*> (shiftScale->GetOutput()));
	}

	//std::cout << " Update vector image DTI00  imageContainer size " << imageContainer->Size() << std::endl;
	//Get vector Image
	typedef itk::ComposeImageFilter<PerfusionCalculatorImageType>		ImageToVectorImageType;
	ImageToVectorImageType::Pointer imageToVectorImageFilter = ImageToVectorImageType::New();

	for (int i = 0; i < imageContainer->Size(); i++)
	{
		imageToVectorImageFilter->SetInput(i, imageContainer->GetElement(i));
	}

	imageToVectorImageFilter->Update();
	std::cout << "vectorImage: vectorLength = " << imageToVectorImageFilter->GetOutput()->GetVectorLength() << std::endl;

	//Mask image filter
	typedef itk::MaskVectorImageFilter <PerfusionCalculatorPixelType> MaskFilterType;
	MaskFilterType::Pointer maskFilter = MaskFilterType::New();
	maskFilter->SetInput(imageToVectorImageFilter->GetOutput());//input is a Image Pointer!!!
	maskFilter->SetMaskThreshold(m_MaskThreshold);//Get from UI or user-interaction
	maskFilter->Update();//output is a Image Pointer!!!

	// Adc Map filter
	typedef itk::PerfusionMapFilter <PerfusionCalculatorPixelType, PerfusionCalculatorPixelType> PerfusionMapFilterType;
	PerfusionMapFilterType::Pointer perfusionMap = PerfusionMapFilterType::New();
	perfusionMap->SetInput(maskFilter->GetOutput());
	perfusionMap->setimageTimer(this->m_DicomHelper->dynamicTime);
	perfusionMap->Update();

	std::cout << "maskFilter: vectorLength = " << maskFilter->GetOutput()->GetVectorLength() << std::endl;

	//itk version of DeepCopy	
	_MaskVectorImage->SetSpacing(perfusionMap->GetOutput()->GetSpacing());
	_MaskVectorImage->SetOrigin(perfusionMap->GetOutput()->GetOrigin());
	_MaskVectorImage->SetDirection(perfusionMap->GetOutput()->GetDirection());
	_MaskVectorImage->SetRegions(perfusionMap->GetOutput()->GetLargestPossibleRegion());
	_MaskVectorImage->SetVectorLength(perfusionMap->GetOutput()->GetVectorLength());
	//std::cout << " m_MaskVectorImage length 00 0000000= " << this->m_MaskVectorImage->GetVectorLength() << std::endl;
	_MaskVectorImage->Allocate();
	std::cout << " m_MaskVectorImage length 00 = " << this->m_MaskVectorImage->GetVectorLength() << std::endl;


	typedef itk::ImageRegionConstIterator <PerfusionCalculatorVectorImageType> ConstMaskFilterIteratorType;
	typedef itk::ImageRegionIterator <PerfusionCalculatorVectorImageType> MaskVectorImageIteratorType;

	ConstMaskFilterIteratorType maskFilterIterator(perfusionMap->GetOutput(), perfusionMap->GetOutput()->GetLargestPossibleRegion());
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

void PerfusionCore::onThreshSlide(double maskThreshold) //SLOT of cdwiToggle
{
	m_MaskThreshold = maskThreshold / 5; //slider value is 5 fold of real value. 
	PerfusionCore::onRecalcAll(m_CurrentSlice);
}

void PerfusionCore::ComputeCurrentSourceImage(int currentSlice, vtkSmartPointer <vtkImageData> SourceImageData)
{
	if (!m_DicomHelper->GetDicomReader()->GetOutput()) return;

	vtkSmartPointer <vtkExtractVOI> ExtractVOI = vtkSmartPointer <vtkExtractVOI>::New();
	ExtractVOI->SetInputData(m_DicomHelper->GetDicomReader()->GetOutput());
	ExtractVOI->SetVOI(0, this->m_DicomHelper->imageDimensions[0] - 1, 0, this->m_DicomHelper->imageDimensions[1] - 1, currentSlice, currentSlice);
	ExtractVOI->Update();

	//Maybe we can make use of the extent info here rather than set it back to 0 via changeInformationFiter
	vtkSmartPointer <vtkImageChangeInformation> changeInfo = vtkSmartPointer <vtkImageChangeInformation>::New();
	changeInfo->SetInputData(ExtractVOI->GetOutput());
	changeInfo->SetOutputOrigin(0, 0, 0);
	changeInfo->SetExtentTranslation(0, 0, -currentSlice);
	changeInfo->Update();

	SourceImageData->DeepCopy(changeInfo->GetOutput());
}

void PerfusionCore::onRecalcAll(int inputSlice)
{
	if (m_DicomHelper->imageDataType == "PERFUSION")
	{
		if (inputSlice >= 0)
		{
			//std::cout << "[onRecalcAll] updating slice" << endl;
			m_CurrentSlice = inputSlice;
			vtkSmartPointer<vtkImageData> SourceImageSlice = vtkSmartPointer<vtkImageData>::New();
			ComputeCurrentSourceImage(inputSlice, SourceImageSlice);
			UpdateMaskVectorImage(m_DicomHelper, this->m_MaskVectorImage);
			emit SignalTestButtonFired(true, SourceImageSlice, QString("Source"), 1, 0);
			cout << "end update" << endl;
			//m_vectorImage.insert(inputSlice, m_MaskVectorImage);
			if (!this->m_MaskVectorImage) return;

			//Retrigger All Checked Buttons. 
			for (int i = 401; i <= 409; i++)
			{
				bool isChecked = ButtonTable->button(i)->isChecked();
				cout << "is Checked: " << isChecked << endl;
				if (ButtonTable->button(i)->isChecked())
				{
					cout << "press button again" << endl;
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
}