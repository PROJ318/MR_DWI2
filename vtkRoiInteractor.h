/*=========================================================================

Program:   MR Diffusion toolkit
Module:    vtkRoiInteractor.cxx

=========================================================================*/
// .NAME vtkRoiInteractor - ROI measurement related interaction for images
// .SECTION Description
//

#ifndef vtkRoiInteractor_h
#define vtkRoiInteractor_h

//included files

#include <vtkSmartPointer.h>
#include <vtkInteractorObserver.h>
#include <QStandardItemModel>
#include <QStandardItem>
//#include <qtextbrowser.h>

//included classes
class vtkImageActor;
class vtkImageTracerWidget;
class vtkContourWidget;
class vtkCollection;
class vtkPolyData;

//class vtkRoiInteractor : public vtkInteractorObserver
//{
//public:
//	// Description:
//	// Instantiate the object.
//	static vtkRoiInteractor *New();
//
//	vtkTypeMacro(vtkRoiInteractor, vtkInteractorObserver);
//	//void PrintSelf(ostream& os, vtkIndent indent);
//
//	void SetQTextBrowser(QTextBrowser* browser);
//	void SetImageActor(vtkImageActor* actor);
//	void SetScalingPara(float scaling[])
//	{
//		scalingPara[0] = scaling[0];
//		scalingPara[1] = scaling[1];
//	};
//
//	void AddWidgetItem();
//	void RemoveWidgetIterm(vtkContourWidget* contour);
//
//	vtkCollection* contourWidgetCollection;
//
//protected:
//	vtkRoiInteractor();
//	~vtkRoiInteractor();
//
//	QTextBrowser* QtextBrowser;
//	vtkImageActor* imageActor;
//	float scalingPara[2];
//};

class vtkRoiInteractor //: public vtkInteractorObserver
{
public:
	// Description:
	// Instantiate the object.
	static vtkRoiInteractor *New();
	//vtkTypeMacro(vtkRoiInteractor, vtkInteractorObserver);
	//void PrintSelf(ostream& os, vtkIndent indent);
	//void SetQTextBrowser(QTextBrowser* browser);
	//void SetImageActor(vtkImageActor* actor);
	//void SetScalingPara(float scaling[])
	//{
	//	scalingPara[0] = scaling[0];
	//	scalingPara[1] = scaling[1];
	//};
	//void AddWidgetItem();
	void RemoveWidgetIterm(vtkContourWidget* contour);

	//vtkCollection* contourWidgetCollection;	

public:
	vtkRoiInteractor();
	void initialize(vtkSmartPointer<vtkRenderWindowInteractor>, QStandardItem*, float*, const QString, vtkCollection*);
	void usePolydata(vtkSmartPointer<vtkRenderWindowInteractor> , vtkPolyData* );
	~vtkRoiInteractor();

	QString imageName;
	//QList<QStandardItem *> roiInfoRow;
	//QTextBrowser* QtextBrowser;	
	//vtkImageActor* imageActor;
	//float scalingPara[2];

protected:
	vtkSmartPointer<vtkContourWidget> newContourWidget;
	vtkSmartPointer<vtkRenderWindowInteractor> interactor;
	void OnRightButtonDown();

};


#endif