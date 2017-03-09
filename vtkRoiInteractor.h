/*=========================================================================

Program:   MR Diffusion toolkit
Module:    vtkRoiInteractor.cxx

=========================================================================*/
// .NAME vtkRoiInteractor - ROI measurement related interaction for images
// .SECTION Description
//

#ifndef vtkRoiInteractor_h
#define vtkRoiInteractor_h

//included qt
#include <QWidget>
#include <qdebug.h>
#include <QStandardItemModel>
#include <QStandardItem>

//include vtk
#include <vtkSmartPointer.h>
#include <vtkInteractorObserver.h>

//#include <qtextbrowser.h>

//included classes
class vtkImageActor;
class vtkImageTracerWidget;
class vtkContourWidget;
class vtkCollection;
class vtkPolyData;
class vtkContourRepresentation;

class vtkRoiInteractor
{

public:

	// Description:
	// Instantiate the object.
	static vtkRoiInteractor *New();
	// vtkTypeMacro(vtkRoiInteractor, vtkInteractorObserver);
	// void PrintSelf(ostream& os, vtkIndent indent);
	// void SetQTextBrowser(QTextBrowser* browser);
	// void SetImageActor(vtkImageActor* actor);
	// void SetScalingPara(float scaling[])
	// {
	//	 scalingPara[0] = scaling[0];
	//	 scalingPara[1] = scaling[1];
	// };
	// void AddWidgetItem();
	// void RemoveWidgetIterm(vtkContourWidget* contour);
	//vtkCollection* contourWidgetCollection;	
    //signals:
    //void sigSendRoiStatis(float*, QString, int);
public:
	vtkRoiInteractor();
	~vtkRoiInteractor();
	void initialize(vtkSmartPointer<vtkRenderWindowInteractor>, QStandardItem*, float*, QHash<int, QHash<QString, vtkContourRepresentation*> >*, int, int);
	void usePolydata(vtkSmartPointer<vtkRenderWindowInteractor> iInt, vtkPolyData* plydata);
	void useContourRep(vtkSmartPointer<vtkRenderWindowInteractor> iInt, vtkContourRepresentation* contourRep,
		QStandardItem * parentItem, float* scalingPara, QHash<int, QHash<QString, vtkContourRepresentation*> >* RoiHash, int sliceNum);


	QString imageName;
	
	//QList<QStandardItem *> roiInfoRow;
	//QTextBrowser* QtextBrowser;	
	//vtkImageActor* imageActor;
	//float scalingPara[2];
	//vtkCollection* contourCollection;

protected:
	vtkSmartPointer<vtkContourWidget> newContourWidget;
	vtkSmartPointer<vtkRenderWindowInteractor> interactor;

};


#endif