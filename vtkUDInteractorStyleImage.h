#pragma once

#ifndef vtkUDInteractorStyleImage_h
#define vtkUDInteractorStyleImage_h

#include <vtkInteractorStyleImage.h>
#include <vtkRoiInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTextMapper.h>

class vtkImageData;
class vtkImageProperty;
class vtkImageActor;
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkRenderer;
class vtkImageViewer2;
class vtkContourWidget;

#include <sstream>

class StatusMessage {
public:
	static std::string Format(int slice, int maxSlice) {
		std::stringstream tmp;
		tmp << "Slice Number  " << slice + 1 << "/" << maxSlice + 1;
		return tmp.str();
	}
};

class myVtkInteractorStyleImage : public vtkInteractorStyleImage
{
public:
	static myVtkInteractorStyleImage* New();
	vtkTypeMacro(myVtkInteractorStyleImage, vtkInteractorStyleImage);

protected:
	myVtkInteractorStyleImage();
	~myVtkInteractorStyleImage() {}
	vtkSmartPointer<vtkImageViewer2> _ImageViewer;
	vtkSmartPointer<vtkTextMapper> _StatusMapper;
	int _Slice;
	int _MinSlice;
	int _MaxSlice;

	int *_CurrentSlice;//Change from outside
	int *_CurrentWidget;

	int _Component;
	int _MaxComponent;

	vtkImageActor* _imageActor;
	vtkImageData* _OriginalInputImageData;
	vtkRenderer* _currentRender;
	vtkContourWidget* contourWidget[5];
	vtkRoiInteractor* roiInteraction;

public:
	void SetImageViewer(vtkImageViewer2* imageViewer);
	vtkImageData* GetInputImage()
	{
		return _OriginalInputImageData;
	}
	vtkImageActor* GetImageActor()
	{
		return _imageActor;
	}

	void SetStatusMapper(vtkTextMapper* statusMapper) {
		_StatusMapper = statusMapper;
	}

	void GetCurrentSliceNumber(int & CurrentSlice)
	{
		_CurrentSlice = &CurrentSlice;
	}

	vtkRoiInteractor* GetRoiInteraction()
	{
		return this->roiInteraction;
	}

	void SetDefaultWindowLevel(int currentSlice, int currentComponent);

protected:
	void MoveSliceForward();

	void MoveSliceBackward();

	void MoveSliceComponentForward();

	void MoveSliceComponentBackward();

	virtual void OnKeyDown();

	virtual void OnMouseWheelForward();

	virtual void OnMouseWheelBackward();

	virtual void OnRightButtonDown();
	//Get keybord input, only support Reset window level for now
	virtual void OnChar();

	virtual void WindowLevel();
};

#endif