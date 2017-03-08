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
	//roiInteraction = vtkRoiInteractor::New();
	_ImageViewer = vtkSmartPointer<vtkImageViewer2>::New();
	_StatusMapper = vtkSmartPointer<vtkTextMapper>::New();
	_OriginalInputImageData = vtkSmartPointer<vtkImageData>::New();
	_CurrentSlice = NULL;
	_MaxSlice = 0;
	_CurrentComponent = NULL;
	_MaxComponent = 0;
	_ColorImage = false;
}

void myVtkInteractorStyleImage::SetImageViewer(vtkImageViewer2* imageViewer)
{
	_ImageViewer = imageViewer;
	//_MinSlice = imageViewer->GetSliceMin();
	//_MaxSlice = imageViewer->GetSliceMax();
	_Slice = imageViewer->GetSliceMin();
	//imageViewer->GetSlice();
	//_ImageViewer->SetSlice(_Slice);

	//_ImageViewer->OffScreenRenderingOn();	
	//_ImageViewer->Render();
	//_ImageViewer->OffScreenRenderingOff();

	//std::cout << "_Slice = , _MaxSlice = " << _Slice << " " << _MaxSlice << std::endl;
	_OriginalInputImageData = imageViewer->GetInput();
	_imageActor = imageViewer->GetImageActor();
	_currentRender = imageViewer->GetRenderer();
	_MaxComponent = imageViewer->GetInput()->GetNumberOfScalarComponents() - 1;
	//We can also use imageViewer->GetRenderWindow()->GetWindowName();
	_ColorImage = imageViewer->GetInput()->GetScalarType() == 3 ? true : false;//3 is unsigned char, it is used exclusively for Color image data
	this->SetDefaultWindowLevel();

	//cout << "Slicer: Min = " << _MinSlice << ", Max = " << _MaxSlice << std::endl;
}

void myVtkInteractorStyleImage::SetStatusMessageInfo(vtkTextMapper* statusMapper, int & currentSlice, int maxSlice, int & currentComponent, std::vector <std::string> componentInfo)
{
	_StatusMapper = statusMapper;
	_CurrentSlice = &currentSlice;
	_CurrentComponent = &currentComponent;
	_MaxSlice = maxSlice;
	_ComponentInfo = componentInfo;
}

void myVtkInteractorStyleImage::SetDefaultWindowLevel()
{
	if (_ColorImage) return;
	double colorWindow, colorLevel;

	double *imageDataRange = new double[2];
	imageDataRange = _ImageViewer->GetInput()->GetScalarRange();
	colorWindow = imageDataRange[1] - imageDataRange[0];
	colorLevel = 0.5* (imageDataRange[1] + imageDataRange[0]);

	_ImageViewer->SetColorWindow(colorWindow);
	_ImageViewer->SetColorLevel(colorLevel);
	//Delete pointer here would result in software error
	//delete[] imageDataRange;
}

void myVtkInteractorStyleImage::MoveSliceForward()
{

	//if (_Slice < _MaxSlice)
	//{
	//	//if (_Slice == 0)
	//	//{
	//	//	_ImageViewer->SetSlice(_Slice);
	//	//	_ImageViewer->Render();
	//	//}

	//	_Slice += 1;
	//	//_ImageViewer->SetSlice(_Slice);
	//	//_ImageViewer->GetRenderWindow()->OffScreenRenderingOn();
	//	//if (_Slice == 0)
	////else
	////{
	////	_Slice = _Slice - _MaxSlice;
	////}

	//cout << "MoveSliceForward::Slice = " << _Slice << std::endl;

	//if (_StatusMapper != NULL)
	//{
	//	std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
	//	_StatusMapper->SetInput(msg.c_str());
	//	cout << "msg slice" << msg.c_str() << endl;
	//}


	//if (_CurrentSlice != NULL)
	//	*_CurrentSlice = _Slice;

	//SetDefaultWindowLevel(_Slice, _Component);
	//_ImageViewer->GetRenderWindow()->Render();

	//////////
	////PR fix here
	//_ImageViewer->GetRenderer()->ResetCamera();
	//vtkSmartPointer<vtkCamera> camera = _ImageViewer->GetRenderer()->GetActiveCamera();
	////cout << "before imagefill focal point = " << camera->GetFocalPoint()[2] << std::endl;
	////cout << "before imagefill position = " << camera->GetPosition()[2] << std::endl;
	//double windowWidth = _ImageViewer->GetRenderWindow()->GetSize()[0];
	//double windowHeight = _ImageViewer->GetRenderWindow()->GetSize()[1];
	//this->ImageAutoFillWindow(camera, _OriginalInputImageData, windowWidth, windowHeight);
	//_ImageViewer->GetRenderer()->SetActiveCamera(camera);
	//_ImageViewer->GetRenderer()->ResetCamera();
	//cout << "Before setslice parallelscale = " << camera->GetParallelScale() << std::endl;
	//cout << "Before setslice focal point 0= " << camera->GetFocalPoint()[0] << std::endl;
	//cout << "Before setslice focal point 1= " << camera->GetFocalPoint()[1] << std::endl;
	//cout << "Before setslice focal point 2= " << camera->GetFocalPoint()[2] << std::endl;
	//cout << "Before setslice position = " << camera->GetPosition()[2] << std::endl;
	//_ImageViewer->SetSlice(_Slice);
	//cout << "after setslice parallelscale = " << camera->GetParallelScale() << std::endl;
	//cout << "aefore setslice focal point 0= " << camera->GetFocalPoint()[0] << std::endl;
	//cout << "aefore setslice focal point 1= " << camera->GetFocalPoint()[1] << std::endl;
	//cout << "aefore setslice focal point 2= " << camera->GetFocalPoint()[2] << std::endl;
	//cout << "after setslice position = " << camera->GetPosition()[2] << std::endl;
	//_ImageViewer->GetRenderWindow()->OffScreenRenderingOff();
	//_ImageViewer->GetRenderWindow()->Render();
	//_ImageViewer->Render();
	//////////
	//End of PR fix
	//}
}

void myVtkInteractorStyleImage::MoveSliceBackward()
{
	//if (_Slice > _MinSlice)
	//{
	//	_Slice -= 1;
	//
	////else
	////{
	////	_Slice += _MaxSlice;
	////}
	//cout << "MoveSliceBackward::Slice = " << _Slice << std::endl;
	////_ImageViewer->SetSlice(_Slice);

	//if (_StatusMapper != NULL)
	//{
	//	std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
	//	_StatusMapper->SetInput(msg.c_str());
	//}

	//if (_CurrentSlice != NULL)
	//	*_CurrentSlice = _Slice;
	////_ImageViewer->GetRenderer()->ResetCamera();

	//SetDefaultWindowLevel(_Slice, _Component);
	//_ImageViewer->GetRenderWindow()->Render();
	////_ImageViewer->Render();
	//}
}

void myVtkInteractorStyleImage::MoveSliceComponentForward()
{
	if (_ColorImage || *_CurrentComponent >= _MaxComponent) return;

	(*_CurrentComponent)++;

	cout << "MoveSliceComponentForward: Component = " << *_CurrentComponent << std::endl;

	//update Status Message
	std::string statusMsg = myStatusMessage::Format(*_CurrentSlice, _MaxSlice, *_CurrentComponent, _ComponentInfo);
	_StatusMapper->SetInput(statusMsg.c_str());

	//update image
	vtkSmartPointer <vtkImageExtractComponents> scalarComponent = vtkSmartPointer <vtkImageExtractComponents>::New();
	scalarComponent->SetInputData(_OriginalInputImageData);
	scalarComponent->SetComponents(*_CurrentComponent);
	scalarComponent->Update();
	_ImageViewer->SetInputData(scalarComponent->GetOutput());
	SetDefaultWindowLevel();
	//_ImageViewer->SetSlice(_Slice);
	_ImageViewer->GetRenderWindow()->Render();

}

void myVtkInteractorStyleImage::MoveSliceComponentBackward()
{
	if (_ColorImage || *_CurrentComponent == 0) return;
	//if (_Component > 0)
	//{
		(*_CurrentComponent)--;
		//_OriginalInputImageData->GetNumberOfScalarComponents();
		cout << "MoveSliceComponentBackward: Current Component = " << *_CurrentComponent << " Number of Components = " << _OriginalInputImageData->GetNumberOfScalarComponents() << std::endl;
		
		//update Status Message
		std::string statusMsg = myStatusMessage::Format(*_CurrentSlice, _MaxSlice, *_CurrentComponent, _ComponentInfo);
		_StatusMapper->SetInput(statusMsg.c_str());

		//update image		
		vtkSmartPointer <vtkImageExtractComponents> scalarComponent = vtkSmartPointer <vtkImageExtractComponents>::New();
		scalarComponent->SetInputData(_OriginalInputImageData);
		scalarComponent->SetComponents(*_CurrentComponent);
		scalarComponent->Update();

		_ImageViewer->SetInputData(scalarComponent->GetOutput());
		SetDefaultWindowLevel();
		_ImageViewer->GetRenderWindow()->Render();
	//}
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
	// don't forward events, otherwise the image will be zoomed 
	// in case another interactorstyle is used (e.g. trackballstyle, ...)
	//vtkInteractorStyleImage::OnMouseWheelForward();
}

void myVtkInteractorStyleImage::OnMouseWheelBackward() {
	//std::cout << "Scrolled mouse wheel backward." << std::endl;
	// don't forward events, otherwise the image will be zoomed 
	// in case another interactorstyle is used (e.g. trackballstyle, ...)
	//vtkInteractorStyleImage::OnMouseWheelBackward();
}

//	}//Right button click for deleting the choosed ROI, add by Wenxing
//void myVtkInteractorStyleImage::OnRightButtonDown()
//{
//	int* clickPos = this->GetInteractor()->GetEventPosition();
//	int numberOfWidgets = this->roiInteraction->contourWidgetCollection->GetNumberOfItems();
//	//cout << "number of nodes:" << numberOfWidgets << endl;
//
//	if (numberOfWidgets > 0)
//	{
//		for (int i = 0; i < numberOfWidgets; i++)
//		{
//			vtkContourWidget* contour = vtkContourWidget::SafeDownCast(this->roiInteraction->contourWidgetCollection->GetItemAsObject(i));
//			vtkContourRepresentation* rep = contour->GetContourRepresentation();
//
//			if (rep->SetActiveNodeToDisplayPosition(clickPos)) //rep->SetActiveNodeToDisplayPosition(clickPos)
//			{
//				this->roiInteraction->RemoveWidgetIterm(contour);
//				return;
//			}
//			//}
//		}
//	// forward event
//	vtkInteractorStyleImage::OnRightButtonDown();
//
//}

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
	//std::cout << "-----------------------window Level event within window-----------------" << std::endl;
	vtkRenderWindowInteractor *rwi = this->Interactor;

	this->WindowLevelCurrentPosition[0] = rwi->GetEventPosition()[0];
	this->WindowLevelCurrentPosition[1] = rwi->GetEventPosition()[1];

	//if (this->HandleObservers &&
	//	this->HasObserver(vtkCommand::WindowLevelEvent))
	//{
	//	this->InvokeEvent(vtkCommand::WindowLevelEvent, this);
	//}
	//else 
		if (this->CurrentImageProperty)
	{
		int *size = this->CurrentRenderer->GetSize();

		double window = this->CurrentImageProperty->GetColorWindow();
		//double window = this->WindowLevelInitial[0];//WindwoLevelInitial doesn't work as expected, replace it
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

		//double newWindow = dx + window;
		//double newLevel = level - dy;
		double newWindowLevel[2]; 
		newWindowLevel[0] = dx + window;
		newWindowLevel[1] = level - dy;

		if (newWindowLevel[0] < 0.01)
		{
			newWindowLevel[0] = 0.01;
		}
		newWindowLevel[1] = newWindowLevel[1] < 0.01 ? 0.01 : newWindowLevel[1];

		this->CurrentImageProperty->SetColorWindow(newWindowLevel[0]);
		this->CurrentImageProperty->SetColorLevel(newWindowLevel[1]);
		this->Interactor->Render();
		this->GetImageViewer2()->GetColorWindow();
		this->GetImageViewer2()->GetColorLevel();

		//double imageViewer2WindowLevel
		//std::cout << "new window & level " << newWindowLevel[0] << " x " << newWindowLevel[1] << std::endl;
		//std::cout << "-----------------------invoke interactor windowlevel event-----------------" << std::endl;
		//this->Interactor->InvokeEvent(vtkCommand::WindowLevelEvent, newWindowLevel);
	}
};

vtkStandardNewMacro(myVtkInteractorStyleImage);
//#endif