/*=========================================================================

Program:   MR Diffusion toolkit
Module:    vtkRoiInteractor.cxx

=========================================================================*/
#include "vtkRoiInteractor.h"

////VTK related included files
#include <vtkUDInteractorStyleImage.h> //customized render window interactor style

#include <vtkImageActor.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkContourWidget.h>
#include <vtkOrientedGlyphContourRepresentation.h>
#include <vtkImageActorPointPlacer.h>
#include <vtkProperty.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include "vtkObjectFactory.h"
#include <vtkPolyData.h>
#include <vtkPolyDataToImageStencil.h>

#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkImageAccumulate.h>
#include <vtkBoundedPlanePointPlacer.h>
#include <vtkPlane.h>
#include <vtkRenderWindow.h>

//for debug
#include <vtkRendererCollection.h>
#include <vtkActor2DCollection.h>
#include <vtkPropPicker.h>
#include <vtkAssemblyPath.h>
#include <vtkPointData.h>

//QT related included files
#include <qobject.h>
#include <qdebug.h>




class vtkTracerCallback :public vtkCommand
{
public:
	static vtkTracerCallback *New()
	{
		return new vtkTracerCallback;
	}

	void Execute(vtkObject *caller, unsigned long, void*) VTK_OVERRIDE
	{
		//vtkImageTracerWidget *_tracer = reinterpret_cast<vtkImageTracerWidget*>(caller);
		//if (!_tracer) { return; }
		//if (!_tracer->IsClosed())
		//{
		//	QString warnInfo(QObject::tr("ROI contour is not closed. Please Draw Again")); // no tr
		//	browser->setText(warnInfo);
		//	return;
		//}

		vtkContourWidget *contour = reinterpret_cast<vtkContourWidget*>(caller); 
		vtkContourRepresentation *rep = vtkContourRepresentation::SafeDownCast(contour->GetRepresentation());
		vtkOrientedGlyphContourRepresentation *glyContour =
			vtkOrientedGlyphContourRepresentation::SafeDownCast(rep);

		//contourWidget = contour;
	
		vtkSmartPointer<myVtkInteractorStyleImage> style = 
			vtkSmartPointer<myVtkInteractorStyleImage>::New();
		style = static_cast<myVtkInteractorStyleImage*>(contour->GetInteractor()->GetInteractorStyle());
		vtkImageData* o_image = style->GetInputImage();

		vtkSmartPointer<vtkPolyData> path =
			vtkSmartPointer<vtkPolyData>::New();
		//_tracer->GetPath(path);
		path = rep->GetContourRepresentationAsPolyData();
		std::cout << "There are " << path->GetNumberOfPoints() << " points in the path." << std::endl;
		double* coordinate;
		coordinate = path->GetPoint(5);
		cout << "point 5 " << coordinate[0] <<"," <<coordinate[1] << "," << coordinate[2] << endl;
		 
		vtkSmartPointer<vtkPolyDataToImageStencil> polyDataToImageStencil =
			vtkSmartPointer<vtkPolyDataToImageStencil>::New();
		polyDataToImageStencil->SetTolerance(0);
		polyDataToImageStencil->SetInputData(path); // if version < 5 setinputConnection
		polyDataToImageStencil->SetOutputOrigin(o_image->GetOrigin());
		polyDataToImageStencil->SetOutputSpacing(o_image->GetSpacing());
		polyDataToImageStencil->SetOutputWholeExtent(o_image->GetExtent());
		polyDataToImageStencil->Update();

		vtkSmartPointer<vtkImageAccumulate> imageAccumulate =
			vtkSmartPointer<vtkImageAccumulate>::New();
		imageAccumulate->SetStencilData(polyDataToImageStencil->GetOutput());
		imageAccumulate->SetInputData(o_image);
		//imageAccumulate->SetInputData(_image);
		imageAccumulate->Update();

		float Area = imageAccumulate->GetVoxelCount()*o_image->GetSpacing()[0] * o_image->GetSpacing()[1];
		float Mean = *imageAccumulate->GetMean()*scalingValue + shiftValue;
		float Max = *imageAccumulate->GetMax()*scalingValue + shiftValue;
		float Min = *imageAccumulate->GetMin()*scalingValue + shiftValue;
		float std = *imageAccumulate->GetStandardDeviation()*scalingValue + shiftValue;

		//QString RoiInfo(QObject::tr("Area : %1").arg(Area));
		//RoiInfo.append("\n");
		//RoiInfo.append(QObject::tr("Mean : %1").arg(Mean));
		//RoiInfo.append("\n");
		//RoiInfo.append(QObject::tr("Max : %1").arg(Max));
		//RoiInfo.append("\n");
		//RoiInfo.append(QObject::tr("Min : %1").arg(Min));
		//RoiInfo.append("\n");
		//RoiInfo.append(QObject::tr("std : %1").arg(std));
		//RoiInfo.append("\n");
		//browser->setText(RoiInfo);
		
		cout << parentItem->rowCount() << endl;
		for (int row = 0; row < parentItem->rowCount(); ++row)
		{
			QStandardItem* child = parentItem->child(row);
			if (child->text() == roiName)
			{
				parentItem->child(row, 1)->setText(QString("%1").arg(Area));
				parentItem->child(row, 2)->setText(QString("%1").arg(Mean));
				parentItem->child(row, 3)->setText(QString("%1").arg(std));
				parentItem->child(row, 4)->setText(QString("%1").arg(Max));
				parentItem->child(row, 5)->setText(QString("%1").arg(Min));
			}

		}

		//parentItem->child(0, 2)->setText(QString("%1").arg(Mean));
		//parentItem->child(0, 3)->setText(QString("%1").arg(std));
		//parentItem->child(0, 4)->setText(QString("%1").arg(Max));
		//parentItem->child(0, 5)->setText(QString("%1").arg(Min));
		//infoRow << new QStandardItem(QString("%1").arg(Mean));
		//infoRow << new QStandardItem(QString("%1").arg(std));
		//infoRow << new QStandardItem(QString("%1").arg(Max));
		//infoRow << new QStandardItem(QString("%1").arg(Min));
		//parentItem->appendRow(infoRow);
		//RoiInfo[0] = Area; RoiInfo[1] = Mean; RoiInfo[2] = std; RoiInfo[3] = Max; RoiInfo[4] = Min;
		
		//contour->GetCurrentRenderer()->AddViewProp(rep);
		//QStandardItem* areaItem = infoModel->at(0);
		//cout << "In CallbackFunc: Area is " << RoiInfo[0] << endl;

		vtkPropCollection* actors = vtkPropCollection::New();
		glyContour->GetActors(actors);
		vtkActor* actor = vtkActor::SafeDownCast(actors->GetItemAsObject(0));
		//actor->SetVisibility(0);
		cout << "number of actors:" << actors->GetNumberOfItems() << endl;		

	}

	vtkTracerCallback() : scalingValue(0), shiftValue(0){};
	//QList<QStandardItem*>::iterator rowHead;
	QStandardItem* parentItem;
	QString roiName;
	//QList<QStandardItem *>* infoModel;

	void initialize(float _scalingValue, float _shiftValue, const QString _text, QStandardItem* _parentItem)
	{ 
		scalingValue = _scalingValue; shiftValue = _shiftValue; parentItem = _parentItem;
		roiName = _text;
		//cout << *rowHead << endl;
	};
protected:
	float scalingValue;
	float shiftValue;
	//vtkContourWidget* contourWidget;
};


//static void RoicloseCallbackFunction(vtkObject* caller, long unsigned int eventId,
//	void* clientData, void* callData);
//--------
vtkRoiInteractor::vtkRoiInteractor()
{
    //this->QtextBrowser = NULL;
	//this->contourWidgetCollection = vtkCollection::New();
	//this->imageActor = vtkImageActor::New();
}

vtkRoiInteractor::~vtkRoiInteractor()
{
	//this->contourWidgetCollection->Delete();
	//this->QtextBrowser->destroyed();
	//this->imageActor->Delete();
}

//----------------------------------------------------------------------------
void vtkRoiInteractor::initialize(vtkSmartPointer<vtkRenderWindowInteractor> iInt, QStandardItem * parentItem, float* scalingPara, const QString text, vtkCollection* ThisRoiCllct)
{
	//default value---none scaling
	imageName = text;
	qDebug() << "Initializing new vtk ROI at " << imageName << endl;;
	vtkSmartPointer< vtkTracerCallback> traceCallback = vtkTracerCallback::New();
	traceCallback->initialize(scalingPara[0], scalingPara[1], text, parentItem);

	//cout << *RowHead << endl;
	//traceCallback->scalingValue = this->scalingPara[0];
	//traceCallback->shiftValue = this->scalingPara[1];

	//traceCallback->SetClientData();
	interactor = iInt;
	vtkSmartPointer<vtkContourWidget> newContourWidget = vtkSmartPointer<vtkContourWidget>::New();
	newContourWidget->SetInteractor(interactor);
	newContourWidget->FollowCursorOn();

	vtkOrientedGlyphContourRepresentation* rep = vtkOrientedGlyphContourRepresentation::New();
	rep->GetLinesProperty()->SetColor(1, 1, 0);
	rep->GetLinesProperty()->SetLineWidth(1.5);

	vtkImageActor* imageActor = static_cast<myVtkInteractorStyleImage*>(iInt->GetInteractorStyle())->GetImageActor();

	vtkImageActorPointPlacer* placer = vtkImageActorPointPlacer::New();
	placer->SetImageActor(imageActor);

	rep->SetPointPlacer(placer);
	//vtkSmartPointer<vtkCallbackCommand> RoicloseCallback =
	//	vtkSmartPointer<vtkCallbackCommand>::New();
	//// Allow the observer to access the sphereSource
	//RoicloseCallback->SetClientData(&roiInfoRow);
	//RoicloseCallback->SetCallback(RoicloseCallbackFunction);

	newContourWidget->SetRepresentation(rep);
	newContourWidget->ContinuousDrawOn();
	newContourWidget->On();
	newContourWidget->AddObserver(vtkCommand::EndInteractionEvent, traceCallback);

	iInt->AddObserver(vtkCommand::EndInteractionEvent,this, &vtkRoiInteractor::OnRightButtonDown);
	//std::cout << "in initialize: Area is " << traceCallback->RoiInfo[0] << endl;
	//roiInfoRow += traceCallback->infoModel;		
	/*QStandardItem* areaItem = roiInfoRow.at(0);
	cout << "in initialize: Area is " << areaItem->text().toStdString() << endl;
	cout << "contour Widget" << endl;*/

	this->contourWidgetCollection = ThisRoiCllct;
	this->contourWidgetCollection->AddItem(newContourWidget);
}

//----------------------------------------------------------------------------
void vtkRoiInteractor::RemoveWidgetIterm(vtkContourWidget* contour)
{
	//contour->SetEnabled(0);
	//vtkContourWidget *self = reinterpret_cast<vtkContourWidget*>(contour);
	//self->Initialize(NULL);

	this->contourWidgetCollection->RemoveItem(contour);
	contour->Delete();
	qDebug() << "a contourWidget in " << imageName << "is deleted" << endl;
}

void vtkRoiInteractor::OnRightButtonDown()
{
	int* clickPos = interactor->GetEventPosition();
	int numberOfWidgets = this->contourWidgetCollection->GetNumberOfItems();
	qDebug() << numberOfWidgets << " ROIs are in :" << imageName << endl;

	if (numberOfWidgets > 0)
	{
		for (int i = 0; i < numberOfWidgets; i++)
		{
			vtkContourWidget* contour = vtkContourWidget::SafeDownCast(this->contourWidgetCollection->GetItemAsObject(i));
			vtkContourRepresentation* rep = contour->GetContourRepresentation();

			if (rep->SetActiveNodeToDisplayPosition(clickPos)) //rep->SetActiveNodeToDisplayPosition(clickPos)
			{
				this->RemoveWidgetIterm(contour);				
				return;
			}
			//}
		}
	}
	// forward event
	//vtkInteractorStyleImage::OnRightButtonDown();

}
vtkStandardNewMacro(vtkRoiInteractor);