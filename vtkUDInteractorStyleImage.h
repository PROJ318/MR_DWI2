#pragma once

#ifndef vtkUDInteractorStyleImage_h
#define vtkUDInteractorStyleImage_h

#include <vtkInteractorStyleImage.h>
#include <vtkRoiInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTextMapper.h>

#include <vector>

class vtkImageData;
class vtkImageProperty;
class vtkImageActor;
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkRenderer;
class vtkImageViewer2;
class vtkContourWidget;


#include <sstream>

//class StatusMessage {
//public:
//	static std::string Format(int slice, int maxSlice) {
//		std::stringstream tmp;
//		tmp << "Slice Number  " << slice + 1 << "/" << maxSlice + 1;
//		return tmp.str();
//	}
//};

class myStatusMessage {
public:
	static myStatusMessage *New()
	{
		myStatusMessage *statusMessage = new myStatusMessage;
		//msg->m_CurrentSlice = NULL;
		//msg->m_MaxSlice = 0;
		//msg->m_MinSlice = 0;
		//msg->m_ParentWindow = NULL;

		return statusMessage;
	}

	static std::string Format(int slice, int maxSlice, int component, std::vector <std::string> componentInfo)
	{
		std::stringstream msg;
		msg << "Slice Number: " << slice + 1 << "/" << maxSlice + 1 << "\n" 
			<< componentInfo.at(component);
		return msg.str();
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

	int *_CurrentSlice;
	int *_CurrentComponent;//Change from outside
	int *_CurrentWidget;

	//int _Component;
	int _MaxComponent;
	bool _ColorImage;
	std::vector <std::string> _ComponentInfo;

	vtkImageActor* _imageActor;
	vtkSmartPointer<vtkImageData> _OriginalInputImageData;
	vtkRenderer* _currentRender;
	
	//vtkContourWidget* contourWidget[5];
	//vtkRoiInteractor* roiInteraction;

public:
	void SetImageViewer(vtkImageViewer2* imageViewer);
	void SetStatusMessageInfo(vtkTextMapper* statusMapper, int & currentSlice, int maxSlice, int & currentComponent, std::vector <std::string> _ComponentInfo);
	//void InitialSlice(int minSlice, int maxSlice)
	//{
	//	_MinSlice = minSlice;
	//	_MaxSlice = maxSlice;
	//}

	vtkImageData* GetInputImage()
	{
		return _OriginalInputImageData;
	}

	vtkSmartPointer<vtkImageViewer2> GetImageViewer2()
	{
		return _ImageViewer;
	}

	vtkImageActor* GetImageActor()
	{
		return _imageActor;
	}

	//void SetCurrentComponent(int & currentComponent)
	//{
	//	_CurrentComponent = &currentComponent;
	//}

	//vtkRoiInteractor* GetRoiInteraction()
	//{
	//	return this->roiInteraction;
	//}

	void SetDefaultWindowLevel();
protected:
	void MoveSliceForward();

	void MoveSliceBackward();

	void MoveSliceComponentForward();

	void MoveSliceComponentBackward();

	virtual void OnKeyDown();

	virtual void OnMouseWheelForward();

	virtual void OnMouseWheelBackward();

	//virtual void OnRightButtonDown();
	//Get keybord input, only support Reset window level for now
	virtual void OnChar();

	virtual void WindowLevel();

	
};

#endif