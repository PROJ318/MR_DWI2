/*=========================================================================

Program:   MR Diffusion toolkit
Module:    vtkRoiInteractor.cxx

=========================================================================*/
#include "vtkRoiInteractor.h"

////VTK related included files
#include <vtkUDInteractorStyleImage.h> //customized render window interactor style

#include <vtkImageActor.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageTracerWidget.h>
#include <vtkContourWidget.h>
#include <vtkOrientedGlyphContourRepresentation.h>
#include <vtkImageActorPointPlacer.h>
#include <vtkProperty.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include "vtkObjectFactory.h"
#include <vtkPolyData.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkImageAccumulate.h>
#include <vtkBoundedPlanePointPlacer.h>
#include <vtkPlane.h>

//for debug
#include <vtkRendererCollection.h>
#include <vtkActor2DCollection.h>
#include <vtkPropPicker.h>
#include <vtkAssemblyPath.h>
#include <vtkPointData.h>

//QT related included files
#include <qobject.h>


#include <vtkRenderWindow.h>

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

		contourWidget = contour;
	
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

		QString RoiInfo(QObject::tr("Area : %1").arg(Area));
		RoiInfo.append("\n");
		RoiInfo.append(QObject::tr("Mean : %1").arg(Mean));
		RoiInfo.append("\n");
		RoiInfo.append(QObject::tr("Max : %1").arg(Max));
		RoiInfo.append("\n");
		RoiInfo.append(QObject::tr("Min : %1").arg(Min));
		RoiInfo.append("\n");
		RoiInfo.append(QObject::tr("std : %1").arg(std));
		RoiInfo.append("\n");
		browser->setText(RoiInfo);

		//contour->GetCurrentRenderer()->AddViewProp(rep);
		
		vtkPropCollection* actors = vtkPropCollection::New();
		glyContour->GetActors(actors);
		vtkActor* actor = vtkActor::SafeDownCast(actors->GetItemAsObject(0));
		//actor->SetVisibility(0);
		cout << "number of actors:" << actors->GetNumberOfItems() << endl;

	}

	vtkTracerCallback() : contourWidget(0),scalingValue(0), shiftValue(0), browser(0){};
	QTextBrowser *browser;
	float scalingValue;
	float shiftValue;
	vtkContourWidget* contourWidget;
};

//--------
vtkStandardNewMacro(vtkRoiInteractor);
vtkRoiInteractor::vtkRoiInteractor()
{
	this->QtextBrowser = NULL;
	//default value---none scaling
	this->scalingPara[0] = 1;
	this->scalingPara[1] = 0;

	this->contourWidgetCollection = vtkCollection::New();
	this->imageActor = vtkImageActor::New();
}

vtkRoiInteractor::~vtkRoiInteractor()
{
	this->contourWidgetCollection->Delete();
	this->QtextBrowser->destroyed();
	this->imageActor->Delete();
}

//----------------------------------------------------------------------------
void vtkRoiInteractor::AddWidgetItem()
{
	//Callback for the statistic calculating
	vtkSmartPointer<vtkTracerCallback> traceCallback =
		vtkTracerCallback::New();
	traceCallback->browser = this->QtextBrowser;
	traceCallback->scalingValue = this->scalingPara[0];
	traceCallback->shiftValue = this->scalingPara[1];


	vtkSmartPointer<vtkContourWidget> newContourWidget = vtkSmartPointer<vtkContourWidget>::New();
	newContourWidget->SetInteractor(this->Interactor);
	newContourWidget->FollowCursorOn();
	vtkOrientedGlyphContourRepresentation* rep = vtkOrientedGlyphContourRepresentation::New();
	rep->GetLinesProperty()->SetColor(1, 1, 0);
	rep->GetLinesProperty()->SetLineWidth(1.5);
	vtkImageActorPointPlacer* placer = vtkImageActorPointPlacer::New();
	placer->SetImageActor(this->imageActor);
	//double* bounds = this->imageActor->GetBounds();
	//cout << "Bounds:" << bounds[0] << endl;
	//cout << "Bounds:" << bounds[1] << endl;
	//cout << "Bounds:" << bounds[2] << endl;
	//cout << "Bounds:" << bounds[3] << endl;
	//cout << "Bounds:" << bounds[4] << endl;
	//cout << "Bounds:" << bounds[5] << endl;
	//vtkBoundedPlanePointPlacer* placer = vtkBoundedPlanePointPlacer::New();
	//placer->RemoveAllBoundingPlanes();
	//placer->SetProjectionNormalToZAxis();
	//placer->SetProjectionPosition(this->imageActor->GetCenter()[2]);
	//
	//vtkPlane* plane1 = vtkPlane::New();
	//plane1->SetOrigin(bounds[0], bounds[2], bounds[4] + 0.1 );
	//plane1->SetNormal(0.0, 0.0, 1.0);
	//placer->AddBoundingPlane(plane1);

	//vtkPlane* plane2 = vtkPlane::New();
	//plane2->SetOrigin(bounds[1], bounds[3], bounds[5] - 0.1 );
	//plane2->SetNormal(0.0, 0.0, -1.0);
	//placer->AddBoundingPlane(plane2);
	//rep->SetPointPlacer(placer);
	newContourWidget->SetRepresentation(rep);
	newContourWidget->ContinuousDrawOn();
	newContourWidget->On();
	newContourWidget->AddObserver(vtkCommand::EndInteractionEvent, traceCallback);

	cout << "contour Widget" << endl;
	this->contourWidgetCollection->AddItem(newContourWidget);
}

//----------------------------------------------------------------------------
void vtkRoiInteractor::RemoveWidgetIterm(vtkContourWidget* contour)
{
	//contour->SetEnabled(0);
	vtkContourWidget *self = reinterpret_cast<vtkContourWidget*>(contour);
	self->Initialize(NULL);

	this->contourWidgetCollection->RemoveItem(contour);
}

//----------------------------------------------------------------------------
void vtkRoiInteractor::SetQTextBrowser(QTextBrowser* browser)
{
	if(this->QtextBrowser != browser)
		this->QtextBrowser = browser;
}

//-----------------------------------------------------------------------------
void vtkRoiInteractor::SetImageActor(vtkImageActor* actor)
{
	if (this->imageActor != actor)
		this->imageActor = actor;
}
