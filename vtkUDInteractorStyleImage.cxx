//#pragma once
//
//#ifndef vtkUDInteractorStyleImage_h
//#define vtkUDInteractorStyleImage_h
#include <vtkUDInteractorStyleImage.h>

#include <vtkImageViewer2.h>

#include <vtkInteractorStyleImage.h>
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkExtractVOI.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkCommand.h>
#include <vtkContourWidget.h>
#include <vtkContourRepresentation.h>

#include <vtkRoiInteractor.h>

myVtkInteractorStyleImage::myVtkInteractorStyleImage()
{
	roiInteraction = vtkRoiInteractor::New();
	_ImageViewer = vtkSmartPointer<vtkImageViewer2>::New();
	_StatusMapper = vtkSmartPointer<vtkTextMapper>::New();
	_Component = 0;
}
void myVtkInteractorStyleImage::SetImageViewer(vtkImageViewer2* imageViewer) {
	_ImageViewer = imageViewer;
	_MinSlice = imageViewer->GetSliceMin();
	_MaxSlice = imageViewer->GetSliceMax();
	_Slice = _MinSlice;
	std::cout << "_Slice = , _MaxSlice" << _Slice << " " << _MaxSlice << std::endl;
	_OriginalInputImageData = imageViewer->GetInput();
	_imageActor = imageViewer->GetImageActor();
	_currentRender = imageViewer->GetRenderer();
	_MaxComponent = imageViewer->GetInput()->GetNumberOfScalarComponents() - 1;


	SetDefaultWindowLevel(_Slice, _Component);
	//cout << "Slicer: Min = " << _MinSlice << ", Max = " << _MaxSlice << std::endl;
}

void myVtkInteractorStyleImage::SetDefaultWindowLevel(int currentSlice, int currentComponent)
{
	int dims[3];
	_ImageViewer->GetInput()->GetDimensions(dims);
	double colorWindow, colorLevel;
	//3D data set in Source image window
	if (dims[2] > 1)
	{
		//Extract slice
		vtkSmartPointer <vtkExtractVOI> ExtractVOI = vtkSmartPointer <vtkExtractVOI>::New();
		ExtractVOI->SetInputData(_OriginalInputImageData);
		ExtractVOI->SetVOI(0, dims[0] - 1, 0, dims[1] - 1, currentSlice, currentSlice);
		ExtractVOI->Update();

		//Extract component
		vtkSmartPointer <vtkImageExtractComponents> scalarComponent = vtkSmartPointer <vtkImageExtractComponents>::New();
		scalarComponent->SetInputData(ExtractVOI->GetOutput());
		scalarComponent->SetComponents(currentComponent);
		scalarComponent->Update();

		vtkSmartPointer <vtkImageChangeInformation> changeInfo = vtkSmartPointer <vtkImageChangeInformation>::New();
		changeInfo->SetInputData(scalarComponent->GetOutput());
		changeInfo->SetOutputOrigin(0, 0, 0);
		changeInfo->SetExtentTranslation(0, 0, -currentSlice);
		changeInfo->Update();
		//std::cout << "before window leve range = " << std::endl;
		double *range = static_cast<double *>(changeInfo->GetOutput()->GetScalarRange());
		//std::cout << "window leve range = " << range[0] << " " << range[1] << std::endl;
		colorWindow = range[1] - range[0];
		colorLevel = 0.5* (range[1] + range[0]);
	}
	//2D dataset in quantitative image window
	else if (dims[2] == 1)
	{
		double *imageDataRange = new double[2];
		imageDataRange = _ImageViewer->GetInput()->GetScalarRange();//Replace with to be displayed

		if (_MaxComponent == 2)
		{
			//color map 
			colorWindow = 255.0;
			colorLevel = 127.5;

		}
		else
		{
			double *imageDataRange = new double[2];
			imageDataRange = _ImageViewer->GetInput()->GetScalarRange();
			colorWindow = imageDataRange[1] - imageDataRange[0];
			colorLevel = 0.5* (imageDataRange[1] + imageDataRange[0]);
		}

	}

	_ImageViewer->SetColorWindow(colorWindow);
	_ImageViewer->SetColorLevel(colorLevel);
}

void myVtkInteractorStyleImage::MoveSliceForward()
{
	if (_Slice < _MaxSlice)
	{
		_Slice += 1;
	
	//else
	//{
	//	_Slice = _Slice - _MaxSlice;
	//}

	_ImageViewer->SetSlice(_Slice);
	cout << "MoveSliceForward::Slice = " << _Slice << std::endl;

	if (_StatusMapper != NULL)
	{
		std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
		_StatusMapper->SetInput(msg.c_str());
		cout << "msg slice" << msg.c_str() << endl;
	}


	if (_CurrentSlice != NULL)
		*_CurrentSlice = _Slice;

	SetDefaultWindowLevel(_Slice, _Component);
	_ImageViewer->Render();
	}

//	_ImageViewer->Render();

}

void myVtkInteractorStyleImage::MoveSliceBackward()
{
	if (_Slice > _MinSlice)
	{
		_Slice -= 1;
	
	//else
	//{
	//	_Slice += _MaxSlice;
	//}
	cout << "MoveSliceBackward::Slice = " << _Slice << std::endl;
	_ImageViewer->SetSlice(_Slice);

	if (_StatusMapper != NULL)
	{
		std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
		_StatusMapper->SetInput(msg.c_str());
	}

	if (_CurrentSlice != NULL)
		*_CurrentSlice = _Slice;
	//_ImageViewer->GetRenderer()->ResetCamera();

	SetDefaultWindowLevel(_Slice, _Component);
	_ImageViewer->Render();
	}
}

void myVtkInteractorStyleImage::MoveSliceComponentForward()
{
	if (_Component < _MaxComponent)
	{
		_Component++;

		SetDefaultWindowLevel(_Slice, _Component);

		cout << "MoveSliceComponentForward: Component = " << _Component << std::endl;

		vtkSmartPointer <vtkImageExtractComponents> scalarComponent = vtkSmartPointer <vtkImageExtractComponents>::New();
		scalarComponent->SetInputData(_OriginalInputImageData);
		scalarComponent->SetComponents(_Component);
		scalarComponent->Update();
		_ImageViewer->SetInputData(scalarComponent->GetOutput());
		_ImageViewer->SetSlice(_Slice);



		//_ImageViewer->Render();

		//Needed to set input back???
		//_ImageViewer->SetInputData(_OriginalInputImageData);
	}
}

void myVtkInteractorStyleImage::MoveSliceComponentBackward()
{
	if (_Component > 0)
	{
		_Component--;

		SetDefaultWindowLevel(_Slice, _Component);

		cout << "MoveSliceComponentBackward: Component = " << _Component << std::endl;
		vtkSmartPointer <vtkImageExtractComponents> scalarComponent = vtkSmartPointer <vtkImageExtractComponents>::New();
		scalarComponent->SetInputData(_OriginalInputImageData);
		scalarComponent->SetComponents(_Component);
		scalarComponent->Update();
		_ImageViewer->SetInputData(scalarComponent->GetOutput());
		_ImageViewer->SetSlice(_Slice);



		_ImageViewer->Render();

		//Needed to set input back???
		//_ImageViewer->SetInputData(_OriginalInputImageData);
	}
}

void myVtkInteractorStyleImage::OnKeyDown() {
	std::string key = this->GetInteractor()->GetKeySym();
	if (key.compare("Up") == 0) {
		//cout << "Up arrow key was pressed." << endl;
		MoveSliceForward();
	}
	else if (key.compare("Down") == 0) {
		//cout << "Down arrow key was pressed." << endl;
		MoveSliceBackward();
	}
	else if (key.compare("Left") == 0){
		MoveSliceComponentBackward();
	}
	else if (key.compare("Right") == 0){
		MoveSliceComponentForward();
	}

	// forward event
	vtkInteractorStyleImage::OnKeyDown();
}

void myVtkInteractorStyleImage::OnMouseWheelForward() {
	if (_CurrentSlice != NULL)
		MoveSliceForward();
	// don't forward events, otherwise the image will be zoomed 
	// in case another interactorstyle is used (e.g. trackballstyle, ...)
	//vtkInteractorStyleImage::OnMouseWheelForward();
}

void myVtkInteractorStyleImage::OnMouseWheelBackward() {
	//std::cout << "Scrolled mouse wheel backward." << std::endl;
	if (_CurrentSlice != NULL) {
		MoveSliceBackward();
	}
	// don't forward events, otherwise the image will be zoomed 
	// in case another interactorstyle is used (e.g. trackballstyle, ...)
	//vtkInteractorStyleImage::OnMouseWheelBackward();
}

//Right button click for deleting the choosed ROI, add by Wenxing
void myVtkInteractorStyleImage::OnRightButtonDown()
{
	int* clickPos = this->GetInteractor()->GetEventPosition();
	int numberOfWidgets = this->roiInteraction->contourWidgetCollection->GetNumberOfItems();
	//cout << "number of nodes:" << numberOfWidgets << endl;

	if (numberOfWidgets > 0)
	{
		for (int i = 0; i < numberOfWidgets; i++)
		{
			vtkContourWidget* contour = vtkContourWidget::SafeDownCast(this->roiInteraction->contourWidgetCollection->GetItemAsObject(i));
			vtkContourRepresentation* rep = contour->GetContourRepresentation();

			if (rep->SetActiveNodeToDisplayPosition(clickPos)) //rep->SetActiveNodeToDisplayPosition(clickPos)
			{
				this->roiInteraction->RemoveWidgetIterm(contour);
				return;
			}
			//}
		}
	}
	// forward event
	vtkInteractorStyleImage::OnRightButtonDown();

}

//Get keybord input, only support Reset window level for now
void myVtkInteractorStyleImage::OnChar()
{
	switch (this->GetInteractor()->GetKeyCode())
	{
	case 'r':
	case 'R':
	{
		//PR fix: press reset window on quantitative window results crash but ont on source image window
		//Because Source image use vtkRenderWindowInteractor type, Quantitative image uses QVTKInteractor type!!!
		//Plus, _ImageViewer->GetInput()->GetScalarRange() doesn't work for source image, why?
		if (_OriginalInputImageData)
		{
			//std::cout << "reset window 11" << std::endl;
			//std::cout << "image data quantitative viewer " << _ImageViewer->GetInput()->GetNumberOfScalarComponents() << std::endl;
			//std::cout << "image data quantitative viewer range 1 " << _OriginalInputImageData->GetScalarRange()[1] << std::endl;
			double *range = static_cast<double *>(_OriginalInputImageData->GetScalarRange());
			range[1] = (range[1] - range[0] > 255 ? 255 + range[0] : range[1]);
			// *range is from 0 to rescaleFilter->GetOutputMaximum().
			//Howerver, windows will automaticcaly change the display range to [0,255], thus if we reset color window to rescaleFilter->GetOutputMaximum(), it won't be optimal
			vtkImageProperty *property = this->CurrentImageProperty;
			property->SetColorWindow(range[1] - range[0]);
			property->SetColorLevel(0.5 * (range[1] + range[0]));
			this->GetInteractor()->GetRenderWindow()->Render();
		}
		break;
	}
	default:
		//this->Superclass::OnChar();
		break;
	}
}

void myVtkInteractorStyleImage::WindowLevel()
{
	vtkRenderWindowInteractor *rwi = this->Interactor;

	this->WindowLevelCurrentPosition[0] = rwi->GetEventPosition()[0];
	this->WindowLevelCurrentPosition[1] = rwi->GetEventPosition()[1];

	if (this->HandleObservers &&
		this->HasObserver(vtkCommand::WindowLevelEvent))
	{
		this->InvokeEvent(vtkCommand::WindowLevelEvent, this);
	}
	else if (this->CurrentImageProperty)
	{
		int *size = this->CurrentRenderer->GetSize();

		double window = this->CurrentImageProperty->GetColorWindow();
		//double window = this->WindowLevelInitial[0];//WindwoLevelInitial doesn't work properly, replace it
		double level = this->CurrentImageProperty->GetColorLevel();
		// Compute normalized delta
		double dx = (this->WindowLevelCurrentPosition[0] -
			this->WindowLevelStartPosition[0]) * 0.00005;//jiangli modify 
		double dy = (this->WindowLevelStartPosition[1] -
			this->WindowLevelCurrentPosition[1]) * 0.00005; // / size[1];//jiangli modify

		// Scale by current values
		if (fabs(window) > 0.01)
		{
			dx = dx * window;
		}
		else
		{
			dx = dx * (window < 0 ? -0.01 : 0.01);
		}
		if (fabs(level) > 0.01)
		{
			dy = dy * level;
		}
		else
		{
			dy = dy * (level < 0 ? -0.01 : 0.01);
		}

		// Abs so that direction does not flip

		if (window < 0.0)
		{
			dx = -1 * dx;
		}
		if (level < 0.0)
		{
			dy = -1 * dy;
		}

		// Compute new window level

		double newWindow = dx + window;
		double newLevel = level - dy;

		if (newWindow < 0.01)
		{
			newWindow = 0.01;
		}
		newLevel = newLevel < 0.01 ? 0.01 : newLevel;

		this->CurrentImageProperty->SetColorWindow(newWindow);
		this->CurrentImageProperty->SetColorLevel(newLevel);
		this->Interactor->Render();
	}
};

vtkStandardNewMacro(myVtkInteractorStyleImage);
//#endif