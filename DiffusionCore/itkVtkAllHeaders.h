#ifndef vtkcDWI_h
#define vtkcDWI_h

////////////////////////////////
//VTK headers: redundant~~
// some standard vtk headers
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
// headers needed for this example
#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkInteractorStyleImage.h>
#include <vtkActor2D.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>

#include <vtkImageData.h>
#include <vtkImageMapper.h>
#include <vtkImageChangeInformation.h>
#include <vtkExtractVOI.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkImageActor.h>
#include <vtkOutlineFilter.h>
#include <vtkImageSliceMapper.h>
#include <vtkMatrix4x4.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkWarpScalar.h>
#include <vtkImagePlaneWidget.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkJPEGReader.h>
#include <vtkCommand.h>
#include <vtkSliderWidget.h>
#include <vtkWidgetEvent.h>
#include <vtkCallbackCommand.h>
#include <vtkWidgetEventTranslator.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkProperty.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageShiftScale.h>
#include <vtkImageChangeInformation.h>
#include <vtkCamera.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencilToImage.h>
#include <vtkImageAccumulate.h>
#include <vtkImageTracerWidget.h>
////jiangli test reset window
//#include "vtkInformation.h"
//#include "vtkAlgorithm.h"
//#include "vtkStreamingDemandDrivenPipeline.h"
//#include "vtkImageMapper3D.h"
//#include "vtkImageMapToWindowLevelColors.h"
////jiangli test reset window end

//DicomReader not DicomImageReader
#include "vtkDICOMDirectory.h"
#include "vtkDICOMItem.h"
#include "vtkDICOMMetaData.h"
#include "vtkDICOMDictionary.h"
#include "vtkDICOMReader.h"
#include "vtkMedicalImageProperties.h"
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkIntArray.h>
#include <vtkImageProperty.h>
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_matrix_inverse.h"
#include "vnl/vnl_vector.h"

////////////////////////////////////////
//ITK headers
#include "itkImageToImageFilter.h"
#include "itkArray.h"
#include "itkSmartPointer.h"
#include "itkImage.h"
#include "itkImageToVTKImageFilter.h"
#include "itkVTKImageToImageFilter.h"
#include "itkVectorImage.h"
#include "itkVectorContainer.h"
#include "itkVariableLengthVector.h"
#include "itkComposeImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageToHistogramFilter.h"
#include <itkAdcMapFilter.h>
#include <itkComputedDwiFilter.h>
//#include <itkDwiIVIMFilter.h>
#include <itkDwiIVIMFilter2.h>
#include <itkMaskVectorImageFilter.h>
#include <itkDisplayOptimizer.h>
#include <itkComputedEadcFilter.h>

#include "itkMatrix.h"
#include "itkArray.h"
//#include "itkNumericTraits.h"
#include "itkCastImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkRescaleIntensityImageFilter.h"
//#include "itkImageDuplicator.h"
#include "itkShiftScaleImageFilter.h"
//ITK segmentation
#include "itkGradientMagnitudeImageFilter.h"
#include "itkWatershedImageFilter.h"
#include "itkConnectedThresholdImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryMorphologicalOpeningImageFilter.h"
#include "itkBinaryBallStructuringElement.h"

////////////////////////////////////////
// Glue headers
// needed to easily convert from int to std::string
#include <sstream>
#include <string> //difference between string and string.h?
#include <stdlib.h>
#include <iostream>
#include <vector>





// Define own interaction style


class vtkTestCallbackCommand : public vtkCommand
{
public:

	static vtkTestCallbackCommand *New()
	{
		return new vtkTestCallbackCommand;
	}

	virtual void Execute(vtkObject *caller, unsigned long eventId, void*)
	{
		vtkSmartPointer <vtkRenderWindowInteractor> iren = static_cast <vtkRenderWindowInteractor*> (caller);

		std::string key = iren->GetKeySym();
		if (key.compare("Left") == 0)
		{
			std::cout << "TestCallbackFunc Left " << std::endl;
		}
		else if (key.compare("Right") == 0)
		{
			std::cout << "TestCallbackFunc Right " << std::endl;
		}
		else if (key.compare("Down") == 0)
		{
			std::cout << "TestCallbackFunc Down" << std::endl;
		}

		//int valueWindow = static_cast<int>(static_cast<vtkSliderRepresentation *>(sliderWidgetWindow->GetRepresentation())->GetValue());
		//this->_ImageViewer->SetColorWindow(valueWindow);
	}

	vtkTestCallbackCommand()
	{

	}
	//vtkImageViewer2* _ImageViewer;
};

#endif
