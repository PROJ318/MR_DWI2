/*=========================================================================
*
*
*=========================================================================*/

#include "cDWI.h"

void DicomRead(vtkSmartPointer <vtkDICOMReader> & reader, const char *directoryName)
{

	vtkSmartPointer<vtkDICOMDirectory> dDir = vtkSmartPointer<vtkDICOMDirectory>::New();
	dDir->SetDirectoryName(directoryName);
	dDir->SetScanDepth(1);
	dDir->Update();

	int numPatients = dDir->GetNumberOfPatients();
	cout << "number of patients: " << numPatients << endl;
	for (int patient = 0; patient < numPatients; patient++)
	{
		vtkIntArray *numStudies = dDir->GetStudiesForPatient(patient);
		vtkIdType studyId = numStudies->GetMaxId() + 1;
		cout << "number of studies: " << studyId << endl;
		for (vtkIdType studyIndex = 0; studyIndex < studyId; studyIndex++)
		{
			int study = numStudies->GetValue(studyIndex);
			int firstSeries = dDir->GetFirstSeriesForStudy(study);
			int lastSeries = dDir->GetLastSeriesForStudy(study);
			cout << "number of series: " << lastSeries - firstSeries + 1 << endl;
			for (int series = firstSeries; series <= lastSeries; series++)
			{
				vtkStringArray *fileNamesForLastSeries = dDir->GetFileNamesForSeries(series);
				reader->SetFileNames(fileNamesForLastSeries);
			}
		}
	}
}

void DisplayDicomInfo(vtkSmartPointer <vtkImageData> imageData)
{

	//cout << "fileNames: " << reader->GetFileNames()<< endl;
	cout << "number of COMPONENTS: " << imageData->GetNumberOfScalarComponents() << endl;
	const int dataDim = imageData->GetDataDimension();
	//int cells = reader->GetOutput()->GetNumberOfCells();
	//int points = reader->GetOutput()->GetNumberOfPoints();
	//cout << "points: " << points << endl;
	//cout << "cells: " << cells << endl;
	int dims[3];
	double origins[3];
	double spacing[3];
	int extent[6];
	double center[3];
	double range[2];
	imageData->GetDimensions(dims);
	cout << "image dims: " << dims[0] << "x" << dims[1] << "x" << dims[2] << endl;
	imageData->GetOrigin(origins);
	cout << "image origins: " << origins[0] << " " << origins[1] << " " << origins[2] << endl;
	imageData->GetSpacing(spacing);
	cout << "image spacing: " << spacing[0] << "x" << spacing[1] << "x" << spacing[2] << endl;
	imageData->GetExtent(extent);
	cout << "extent: " << extent[0] << "x" << extent[1] << "x" << extent[2] << endl;
	imageData->GetScalarRange(range);
	cout << "range: " << range[0] << "x" << range[1] << endl;
}

void SetShiftScale(vtkSmartPointer <vtkDICOMReader> & reader, float *scaleSlope, float *scaleIntercept)
{

	reader->UpdateInformation();

	vtkSmartPointer<vtkDICOMMetaData> meta = vtkSmartPointer<vtkDICOMMetaData>::New();
	meta = reader->GetMetaData();
	
	if (meta->HasAttribute(vtkDICOMTag(0x2005, 0x100E)))
	{
		*scaleSlope = meta->GetAttributeValue(vtkDICOMTag(0x2005, 0x100E)).AsFloat();
		//std::cout << "scaleSlope = " << *scaleSlope << std::endl;
	}

	if (meta->HasAttribute(vtkDICOMTag(0x2005, 0x100d)))
	{
		*scaleIntercept = meta->GetAttributeValue(vtkDICOMTag(0x2005, 0x100D)).AsFloat();
		//std::cout << "scaleIntercept = " << *scaleIntercept << std::endl;
	}

	//if (meta->HasAttribute(vtkDICOMTag(0x0018, 0x9087)))
	//{
	//	float diffBValues = meta->GetAttributeValue(1,vtkDICOMTag(0x0018, 0x9087)).AsFloat();
	//	std::cout << "diffBValues = " << diffBValues << std::endl;
	//}

	if (meta->HasAttribute(DC::SeriesDescription))// attribute not found in Dicom Header!!!
	{
		//std::string str = meta->GetAttributeValue(vtkDICOMTag(0x0008, 0x103E)).AsString();	
		//std::cout << "SeriesDescription: " << str << std::endl; // Success ??
	}	
}

void GetBValues(vtkSmartPointer <vtkDICOMReader> & reader, unsigned int & numOfComponents, std::vector <float> & bValueList, unsigned int & numOfDiffDirections)//, float bValues[], int *bValuesSize)
{
	reader->UpdateInformation();
	vtkSmartPointer<vtkDICOMMetaData> meta = vtkSmartPointer<vtkDICOMMetaData>::New();
	meta = reader->GetMetaData();

	//int n = reader->GetOutput()->GetNumberOfScalarComponents();
	numOfComponents = reader->GetOutput()->GetNumberOfScalarComponents();
	std::cout << "GetBValues() COMPONENTS: " << numOfComponents << std::endl;
	int numOfBValues = 2;//Diffusion data must have more than 1 b values	
	if (meta->HasAttribute(vtkDICOMTag(0x2005, 0x1414)))//Tag: Number of Diffusion B Values
	{
		numOfBValues = meta->GetAttributeValue(vtkDICOMTag(0x2005, 0x1414)).AsInt();
		std::cout << "numOfBValues = " << numOfBValues << std::endl;
		if (numOfBValues == 1)
		{
			numOfBValues = 2;
		}
		numOfDiffDirections = (numOfComponents - 1) / (numOfBValues - 1);
		std::cout << "GetBValues() numOfDiffDirections = " << numOfDiffDirections << std::endl;
	}

	for (vtkDICOMDataElementIterator iter = meta->Begin(); iter != meta->End(); ++iter)
	{
		vtkDICOMTag tag = iter->GetTag();
		if (tag == vtkDICOMTag(0x2001, 0x1003) && iter->IsPerInstance())
		{
			for (int i = 0; i < numOfComponents; i = i + numOfDiffDirections)
			{
				bValueList.push_back(meta->GetAttributeValue(i, vtkDICOMTag(0x2001, 0x1003)).AsFloat());
				std::cout << "bValueList " << i << ": " << bValueList.at(i/numOfDiffDirections) << std::endl;
			}
		}
	}
}


int main(int argc, char* argv[])
{
	//Test functions work or not
	const char *testFolder =// "C:/DicomData/BrainPA3/DiffusionOnly";
							"C:/DicomData/MultipleBValue"; 
							//"C:/DicomData/DTI";

	vtkSmartPointer <vtkDICOMReader> reader = vtkSmartPointer <vtkDICOMReader>::New();
	DicomRead(reader, testFolder);
	reader->Update();

	DisplayDicomInfo( reader->GetOutput() );

	float myScaleSlope = 1.0, myScaleIntercept = 0.0;
	SetShiftScale(reader, &myScaleSlope, &myScaleIntercept);//worked now~~~
		
	//////////////////////////////////////////////
	//Extract image info:
	const unsigned int SrcImgDimension = 3;//Image dimensions: 3 is reasonable for all inputs
	//const float smallestPixelValue = 1;//threshold for ADC calculation
	//unsigned int computedBValue = 2000;
	std::vector<float> bValueList;
	unsigned int numOfDiffDirections = 1, numOfComponents = 1;
	GetBValues(reader, numOfComponents, bValueList, numOfDiffDirections);
	int numOfDiffBValues = bValueList.size();

	////////////////////////////////////
	//preprocessing: from VTK to ITK vector images
	typedef itk::Image < unsigned short, SrcImgDimension> SrcImageType;
	typedef itk::Image < float, SrcImgDimension> FloatImageType;
	typedef itk::VectorContainer< unsigned int, FloatImageType::Pointer > ImageContainerType;

	typedef itk::VTKImageToImageFilter <SrcImageType>	VtkToItkConverterType;
	typedef itk::CastImageFilter< SrcImageType, FloatImageType >	CastFilterType;
	typedef itk::ShiftScaleImageFilter<FloatImageType, FloatImageType>	ShiftScaleType;


	ImageContainerType::Pointer imageContainer = ImageContainerType::New();
	imageContainer->Reserve(numOfComponents);

	for (int i = 0; i < numOfComponents; i++)
	{
		//Handle each scalar component indivisually
		vtkSmartPointer <vtkImageExtractComponents> scalarComponent = vtkSmartPointer <vtkImageExtractComponents>::New();
		scalarComponent->SetInputConnection(reader->GetOutputPort());
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
		shiftScale->SetShift(-myScaleIntercept);
		shiftScale->SetScale(1.0/myScaleSlope);
		shiftScale->Update();

		//Save vector image & image container
		imageContainer->InsertElement(i, dynamic_cast <FloatImageType*> (shiftScale->GetOutput()));
	}

	//Get vector Image
	typedef itk::ComposeImageFilter<FloatImageType>		ImageToVectorImageType;
	ImageToVectorImageType::Pointer imageToVectorImageFilter = ImageToVectorImageType::New();
	
	for (int i = 0; i < numOfComponents; i++)
	{	
		imageToVectorImageFilter->SetInput(i, imageContainer->GetElement(i));
	}

	imageToVectorImageFilter->Update();
	std::cout << "vectorImage: vectorLength = " << imageToVectorImageFilter->GetOutput()->GetVectorLength() <<std::endl;

	//////////////////////////
	// Sort Data order
	//B0 image should be first!!!
	//data order: b0, b11, b12...b13.... bij....bn1, bn2, ...bnm (n = bvalues, m = directions)


	std::cout << "------------- AdcMapFilter begin runing ----------- " << std::endl;
	// Adc Map filter
	typedef itk::AdcMapFilter <float,float> AdcMapFilterType;
	AdcMapFilterType::Pointer adcMap = AdcMapFilterType::New();
	adcMap->SetInput(imageToVectorImageFilter->GetOutput());
	adcMap->SetBValueList(bValueList);
	adcMap->Update();
	std::cout << "------------- AdcMapFilter end runing ----------- " << std::endl;
	std::cout << "adcvectorImage: vectorLength = " << adcMap->GetOutput()->GetVectorLength() << std::endl;

	
	//cDwi filter
	typedef itk::ComputedDwiFilter <float, float> ComputedDwiFilterType;
	ComputedDwiFilterType::Pointer computedDwi = ComputedDwiFilterType::New();
	computedDwi->SetInput(adcMap->GetOutput());
	computedDwi->SetNumOfDiffDirections (numOfDiffDirections);
	computedDwi->SetComputedBValue(2000);//Get from UI input
	computedDwi->Update();
	std::cout << "cDWi vectorImage: vectorLength = " << computedDwi->GetOutput()->GetVectorLength() << std::endl;

	////////////////////cDwiTest and debug
	// 
	/*unsigned int computedBValue = 2000;
	typedef itk::VectorImage<float, SrcImgDimension> VectorImageType;
	typedef itk::ImageRegionConstIterator <VectorImageType> ConstSrcImageIteratorType;
	typedef itk::ImageRegionIterator <VectorImageType> CDwiIteratorType;
	typedef itk::VariableLengthVector <float> VariableLengthVectorType;

	VectorImageType::Pointer inputVectorImage = adcMap->GetOutput();
	VectorImageType::Pointer cDwiImage = VectorImageType::New();
	cDwiImage->SetRegions(inputVectorImage->GetLargestPossibleRegion());
	cDwiImage->CopyInformation(inputVectorImage);
	cDwiImage->SetVectorLength(numOfDiffDirections);
	cDwiImage->Allocate();
		
	ConstSrcImageIteratorType inputImageIt(inputVectorImage, inputVectorImage->GetLargestPossibleRegion());
	CDwiIteratorType cDwiIt(cDwiImage, cDwiImage->GetLargestPossibleRegion());

	VariableLengthVectorType cDwiResultVector;
	cDwiResultVector.SetSize(numOfDiffDirections);

	inputImageIt.GoToBegin();
	cDwiIt.GoToBegin();
	while (!inputImageIt.IsAtEnd())
	{
		for (int direction = 0; direction < numOfDiffDirections; direction++)
		{
			inputImageIt.Get();
			cDwiResultVector[direction] = -inputImageIt.Get()[2 * direction + 1] * computedBValue + inputImageIt.Get()[2 * direction];
		}
		cDwiIt.Set(cDwiResultVector);
		++inputImageIt;
		++cDwiIt;
	}*/


	//IVIM calculator
	std::cout << "------------- IVIM begin runing ----------- " << std::endl;
	typedef itk::DwiIVIMFilter <float, float> IVIMFilterType;
	IVIMFilterType::Pointer dwiIVIM = IVIMFilterType::New();
	dwiIVIM->SetInput(imageToVectorImageFilter->GetOutput());
	dwiIVIM->SetBValueList(bValueList);
	dwiIVIM->SetNumOfIterations(30);
	dwiIVIM->Update();
	std::cout << "------------- IVIM end runing ----------- " << std::endl;
	std::cout << "IVIM: vectorLength = " << dwiIVIM->GetOutput()->GetVectorLength() << std::endl;


	//vector image to scalar image or imageContainer
	typedef itk::VectorIndexSelectionCastImageFilter <itk::VectorImage<float, 3>, FloatImageType> VectorImageToImageType;
	VectorImageToImageType::Pointer vectorImageToImageFilter = VectorImageToImageType::New();
	vectorImageToImageFilter->SetIndex(2);
	vectorImageToImageFilter->SetInput(dwiIVIM->GetOutput());
	vectorImageToImageFilter->Update();

	///////////////////////////////////////////
	//ITK to VTK for visualization
	typedef itk::ImageToVTKImageFilter <FloatImageType> itkToVtkConverter;
	itkToVtkConverter::Pointer convItkToVtk = itkToVtkConverter::New();
	convItkToVtk->SetInput(vectorImageToImageFilter->GetOutput());
	convItkToVtk->Update();
	

	////check if output result is correct
	std::cout << "----------------------------------------------------" << endl;
	std::cout << "after conveter to VTK image data:" << endl;
	std::cout << "----------------------------------------------------" << endl;
	DisplayDicomInfo(convItkToVtk->GetOutput());
	
	
	//////////////////////////////////////////
	//Extract components
	//not needed anymore
	vtkSmartPointer <vtkImageExtractComponents> component = vtkSmartPointer <vtkImageExtractComponents>::New();
	component->SetInputData (convItkToVtk->GetOutput());
	//component->SetComponents(0, 1);
	component->SetComponents(0);
	component->Update();
	double originalRange[2];
	component->GetOutput()->GetScalarRange(originalRange);
	cout << "orignal range: " << originalRange[0] << "x" << originalRange[1] << endl;
	
	///////////////////////////////////////////
	//rescale image intensity to [0, 4096]	
	vtkSmartPointer <vtkImageShiftScale> rescaleImage = vtkSmartPointer <vtkImageShiftScale>::New();
	rescaleImage->SetInputData(convItkToVtk->GetOutput());
	rescaleImage->SetShift(-originalRange[0]);
	rescaleImage->SetScale(4096/(originalRange[1] - originalRange[0]));
	rescaleImage->ClampOverflowOn();
	rescaleImage->SetOutputScalarTypeToUnsignedShort();
	rescaleImage->Update();//crutial
	
	
	/////////////////////////////////////
	//Visualize data
	vtkSmartPointer<vtkImageViewer2> imageviewer =
	vtkSmartPointer<vtkImageViewer2>::New();	
	//imageviewer->SetInputData(component->GetOutput());
	imageviewer->SetInputData(rescaleImage->GetOutput());
	
	//visualize different plane of data!!!
	imageviewer->SetSliceOrientationToXY();
	imageviewer->GetSliceOrientation();
	//imageviewer->SetColorWindow(255);
	//imageviewer->SetColorLevel(127);
	//cout << "imageviewer slice orientation: " << imageviewer->GetSliceOrientation() << endl;
	// slice status message
	vtkSmartPointer<vtkTextProperty> slicetextprop = vtkSmartPointer<vtkTextProperty>::New();
	slicetextprop->SetFontFamilyToCourier();
	slicetextprop->SetFontSize(20);
	slicetextprop->SetVerticalJustificationToBottom();
	slicetextprop->SetJustificationToLeft();
	
	vtkSmartPointer<vtkTextMapper> slicetextmapper = vtkSmartPointer<vtkTextMapper>::New();
	std::string msg = StatusMessage::Format(imageviewer->GetSliceMin(), imageviewer->GetSliceMax());
	slicetextmapper->SetInput(msg.c_str());
	slicetextmapper->SetTextProperty(slicetextprop);
	
	vtkSmartPointer<vtkActor2D> sliceTextActor = vtkSmartPointer<vtkActor2D>::New();
	sliceTextActor->SetMapper(slicetextmapper);
	sliceTextActor->SetPosition(15, 10);
	
	// usage hint message
	vtkSmartPointer<vtkTextProperty> usagetextprop = vtkSmartPointer<vtkTextProperty>::New();
	usagetextprop->SetFontFamilyToCourier();
	usagetextprop->SetFontSize(14);
	usagetextprop->SetVerticalJustificationToTop();
	usagetextprop->SetJustificationToLeft();
	
	vtkSmartPointer<vtkTextMapper> usagetextmapper = vtkSmartPointer<vtkTextMapper>::New();
	usagetextmapper->SetInput("- slice with mouse wheel\n  or up/down-key\n- zoom with pressed right\n  mouse button while dragging");
	usagetextmapper->SetTextProperty(usagetextprop);
	
	vtkSmartPointer<vtkActor2D> usagetextactor = vtkSmartPointer<vtkActor2D>::New();
	usagetextactor->SetMapper(usagetextmapper);
	usagetextactor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
	usagetextactor->GetPositionCoordinate()->SetValue(0.05, 0.95);
	
	// create an interactor with our own style (inherit from vtkinteractorstyleimage)
	// in order to catch mousewheel and key events
	vtkSmartPointer<vtkRenderWindowInteractor> renderwindowinteractor =
	vtkSmartPointer<vtkRenderWindowInteractor>::New();
	
	vtkSmartPointer<myVtkInteractorStyleImage> myinteractorstyle =
	vtkSmartPointer<myVtkInteractorStyleImage>::New();
	
	// make imageviewer2 and slicetextmapper visible to our interactorstyle
	// to enable slice status message updates when scrolling through the slices
	myinteractorstyle->SetImageViewer(imageviewer);
	myinteractorstyle->SetStatusMapper(slicetextmapper);
	//myinteractorstyle->SetInteractionMode(0);
	
	imageviewer->SetupInteractor(renderwindowinteractor);
	// make the interactor use our own interactorstyle
	// cause setupinteractor() is defining it's own default interatorstyle
	// this must be called after setupinteractor()
	renderwindowinteractor->SetInteractorStyle(myinteractorstyle);
	// add slice status message and usage hint message to the renderer
	imageviewer->GetRenderer()->AddActor2D(sliceTextActor);
	///imageviewer->getrenderer()->addactor2d(usagetextactor);
	//imageviewer->
	// initialize rendering and interaction
	imageviewer->GetRenderWindow()->SetSize(800, 800);
	//imageviewer->getrenderer()->setbackground(0.2, 0.3, 0.4);
	
	
	// Here we describe the representation of the widget.
	vtkSmartPointer<vtkSliderRepresentation2D> sliderRepWindow = vtkSmartPointer<vtkSliderRepresentation2D>::New();
	sliderRepWindow->SetMinimumValue(0);
	sliderRepWindow->SetMaximumValue(4000);//2000
	sliderRepWindow->SetValue(4000);// imageviewer->GetColorWindow());
	sliderRepWindow->SetTitleText("Window");
	sliderRepWindow->SetEndCapLength(0.02);
	sliderRepWindow->SetEndCapWidth(0.01);
	sliderRepWindow->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
	sliderRepWindow->GetPoint1Coordinate()->SetValue(.75, .9);
	sliderRepWindow->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
	sliderRepWindow->GetPoint2Coordinate()->SetValue(.95, .9);
	// Create the callback and pass it the sphereSource to be controlled
	vtkSmartPointer<vtkSliderWindowCallback> callbackWindow = vtkSmartPointer<vtkSliderWindowCallback>::New();
	callbackWindow->_ImageViewer = imageviewer;
	// The widget is the controller for the interction.
	vtkSmartPointer<vtkSliderWidget> sliderWidgetWindow = vtkSmartPointer<vtkSliderWidget>::New();
	sliderWidgetWindow->SetInteractor(renderwindowinteractor);
	sliderWidgetWindow->SetRepresentation(sliderRepWindow);
	sliderWidgetWindow->SetAnimationModeToAnimate();
	sliderWidgetWindow->EnabledOn();
	
	// Here we describe the representation of the widget.
	vtkSmartPointer<vtkSliderRepresentation2D> sliderRepLevel = vtkSmartPointer<vtkSliderRepresentation2D>::New();
	sliderRepLevel->SetMinimumValue(0);
	sliderRepLevel->SetMaximumValue(4000);//2000
	sliderRepLevel->SetValue(2000);
	sliderRepLevel->SetTitleText("Level");
	sliderRepLevel->SetEndCapLength(0.02);
	sliderRepLevel->SetEndCapWidth(0.01);
	sliderRepLevel->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
	sliderRepLevel->GetPoint1Coordinate()->SetValue(.75, .8);
	sliderRepLevel->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
	sliderRepLevel->GetPoint2Coordinate()->SetValue(.95, .8);
	// Create the callback and pass it the sphereSource to be controlled
	vtkSmartPointer<vtkSliderLevelCallback> callbackLevel = vtkSmartPointer<vtkSliderLevelCallback>::New();
	callbackLevel->_ImageViewer = imageviewer;
	// The widget is the controller for the interction.
	vtkSmartPointer<vtkSliderWidget> sliderWidgetLevel = vtkSmartPointer<vtkSliderWidget>::New();
	sliderWidgetLevel->SetInteractor(renderwindowinteractor);
	sliderWidgetLevel->SetRepresentation(sliderRepLevel);
	sliderWidgetLevel->SetAnimationModeToAnimate();
	sliderWidgetLevel->EnabledOn();
	
	
	// Observe the interaction events of the widget. If the computation
	// in the callback is time consuming, observe the
	// EndInteractionEvent instead.
	sliderWidgetWindow->AddObserver(vtkCommand::InteractionEvent, callbackWindow);
	sliderWidgetLevel->AddObserver(vtkCommand::InteractionEvent, callbackLevel);
	
	imageviewer->Render();
	imageviewer->GetRenderer()->ResetCamera();
	imageviewer->Render();
	renderwindowinteractor->Start();

	return EXIT_SUCCESS;
}