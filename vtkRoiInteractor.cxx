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


		//rep->GetNodePolyData(path);
		//contourActor = rep->GetContourRepresentationAsPolyData();
		//std::cout << "After close There are " << path->GetNumberOfPoints() << " points in the path." << std::endl;		
		std::cout << "After close There are " << glyContour->GetNumberOfNodes() << " nodes and " << glyContour->GetNumberOfPaths() << " path" << std::endl;
		//double* coordinate;
		//coordinate = contourActor->GetPoint(5);
		//cout << "point 5 " << coordinate[0] <<"," <<coordinate[1] << "," << coordinate[2] << endl;


		//Statistics:
		//1. it can be done at callback. vtkcommand::endofinteraction has been sent.
		//2. it has three modes: roimode = 0:, init. 1: append: roi of same name is drawn on a different slice. 
		//   2: edit, roi of same name is drawn on the same slice. 3: remove, delete data. 

		if ((*RoiHash)[parentItem->text()].contains(sliceNum)){ RoiMode = 2; }

		if (imageName == QString("Source"))
		{
			if (RoiMode == 0) //init ROI
			{
				QHash<int, vtkContourRepresentation *> newChildHash;
				newChildHash.insert(sliceNum, rep);
				(*RoiHash).insert(parentItem->text(), newChildHash);
			}
			else if (RoiMode == 1) //append ROI
			{
				(*RoiHash)[parentItem->text()].insert(sliceNum, rep);
			}
			else if (RoiMode == 2) //edit ROI
			{
				(*RoiHash)[parentItem->text()][sliceNum] = rep;
			}
			else
			{
				qDebug() << "wrong roi mode";
			}
		}

		vtkSmartPointer<vtkPolyData> path =
			vtkSmartPointer<vtkPolyData>::New();
		//_tracer->GetPath(path);
		path = rep->GetContourRepresentationAsPolyData();

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

		float thisArea = imageAccumulate->GetVoxelCount()*o_image->GetSpacing()[0] * o_image->GetSpacing()[1] * o_image->GetSpacing()[2];
		
		//*****temporary fix block: fix the error of area calc:
		thisArea = (imageName == QString("Source") )? thisArea / 2 : thisArea;
		if (parentItem->hasChildren()){
			for (int i = 0; i < parentItem->child(0)->rowCount(); i++)
			{
				if (sliceNum == parentItem->child(0)->child(i)->text().toInt())
				{
					thisArea = parentItem->child(0)->child(i, 1)->text().toFloat();					
				}
			}
		}
		//*******temporary fix block ends

		float thisMean = *imageAccumulate->GetMean()*scalingValue + shiftValue;
		float thisstd = *imageAccumulate->GetStandardDeviation()*scalingValue + shiftValue;
		float thisMin = *imageAccumulate->GetMin()*scalingValue + shiftValue;
		float thisMax = *imageAccumulate->GetMax()*scalingValue + shiftValue;

		qDebug() << "slice info: " << thisArea << thisMean << thisstd << thisMin << thisMax;

		QList<QStandardItem *> slcRow;
		slcRow << new QStandardItem(QString("%1").arg(sliceNum));
		slcRow << new QStandardItem(QString("%1").arg(thisArea));
		slcRow << new QStandardItem(QString("%1 (%2)").arg(thisMean).arg(thisstd));
		slcRow << new QStandardItem(QString("%1 ~ %2").arg(thisMin).arg(thisMax));

		//qDebug() << "handling window "<<imageName<<"parent node is" << parentItem->text();
		qDebug() << imageAccumulate->GetVoxelCount() << " * " << o_image->GetSpacing()[0] << "-" << o_image->GetSpacing()[1] << "-" << o_image->GetSpacing()[2];
		//qDebug() << " of " << o_image->GetDimensions()[0] << "-" << o_image->GetDimensions()[1]<<"-" << o_image->GetDimensions()[2];
		if (RoiMode == 0) //init ROI
		{
			qDebug() << "ROImode = init";

			//1. Create ImageName Branch first
			QList<QStandardItem *> imgRow;
			imgRow << new QStandardItem(imageName);
			imgRow << new QStandardItem(QString("%1").arg(thisArea));
			imgRow << new QStandardItem(QString("%1 (%2)").arg(thisMean).arg(thisstd));
			imgRow << new QStandardItem(QString("%1 ~ %2").arg(thisMin).arg(thisMax));
			parentItem->appendRow(imgRow);

			//2. Append slice Branch
			imgRow.first()->appendRow(slcRow);
		}
		else
		{

			//1. locate ImageName Branch 
			int ImageRowInd(-10);
			for (int i = 0; i < parentItem->rowCount(); i++)
			{
				if (imageName == parentItem->child(i)->text())
				{
					qDebug() << "found ROW of " << imageName << "at row:" << i;
					ImageRowInd = i;
				}
			}

			if (ImageRowInd >= 0) //if such image name exists
			{
				//2. locate ImageName Branch			
				if (RoiMode == 1)//append ROI: add a slice row
				{
					qDebug() << "ROImode = append";
					parentItem->child(ImageRowInd)->appendRow(slcRow);
				}
				else if (RoiMode == 2) //edit ROI:: edit existing slic Row
				{
					qDebug() << "ROImode = edit";
					for (int i = 0; i < parentItem->child(ImageRowInd)->rowCount(); i++)
					{
						if (sliceNum == parentItem->child(ImageRowInd)->child(i)->text().toInt())
						{
							parentItem->child(ImageRowInd)->child(i, 1)->setText(QString("%1").arg(thisArea));
							parentItem->child(ImageRowInd)->child(i, 2)->setText(QString("%1 (%2)").arg(thisMean).arg(thisstd));
							parentItem->child(ImageRowInd)->child(i, 3)->setText(QString("%1 ~ %2").arg(thisMin).arg(thisMax));
						}
					}

				}
				else
				{
					qDebug() << "wrong roi mode";
				}
				//3. update ImageName Branch
				parentItem->child(ImageRowInd)->sortChildren(0);
				updateImageRow(ImageRowInd);
			}
			else //if no such image name exists, this window must be added new. considerit as init widget
			{
				qDebug() << "edit/append roimode, though no such image exists,creating new";
				//1. Create ImageName Branch first
				QList<QStandardItem *> imgRow;
				imgRow << new QStandardItem(imageName);
				imgRow << new QStandardItem(QString("%1").arg(thisArea));
				imgRow << new QStandardItem(QString("%1 (%2)").arg(thisMean).arg(thisstd));
				imgRow << new QStandardItem(QString("%1 ~ %2").arg(thisMin).arg(thisMax));
				parentItem->appendRow(imgRow);

				//2. Append slice Branch
				imgRow.first()->appendRow(slcRow);
			}
		}

		qDebug() << imageName <<" callback finished";	

	}


	//QList<QStandardItem*>::iterator rowHead;
	QStandardItem* parentItem;
	QString imageName;
	QHash<QString, QHash<int, vtkContourRepresentation*> >* RoiHash;
	int RoiMode;
	int sliceNum;
	//QList<QStandardItem *>* infoModel;

	vtkTracerCallback() : scalingValue(0), shiftValue(0), RoiMode(-1), sliceNum(-1){};
	void initialize(float _scalingValue, float _shiftValue, const QString _text, QStandardItem* _parentItem,
		QHash<QString, QHash<int, vtkContourRepresentation*> >* _RoiHash, int _RoiMode, int _sliceNum)
	{
		scalingValue = _scalingValue; shiftValue = _shiftValue; parentItem = _parentItem;
		imageName = _text; RoiHash = _RoiHash; RoiMode = _RoiMode; sliceNum = _sliceNum;
		//cout << *rowHead << endl;
	};

	void updateImageRow(int ImageRowInd)
	{
		QStandardItem* branchroot = parentItem->child(ImageRowInd);
		//qDebug() << "update image row: " << branchroot->rowCount();
		float AreaSum(0.0), MeanSum(0.0), StdSum(0.0), MinSum(500000000000.0), MaxSum(0.0);
		for (int i = 0; i < branchroot->rowCount(); i++)
		{
			QString areaTxt = branchroot->child(i, 1)->text();
			QString valueTxt = branchroot->child(i, 2)->text();
			QString rangeTxt = branchroot->child(i, 3)->text();
			//qDebug() << i << "_" << areaTxt << "_" << valueTxt << "_" << rangeTxt;
			float sArea = areaTxt.toFloat();
			//qDebug() << sArea;
			AreaSum += sArea;
			float rowMean = valueTxt.section(" (", 0, 0).toFloat(); // not +=
			//qDebug() << rowMean;
			MeanSum = (MeanSum*(i)+rowMean) / (i + 1);
			float rowStd = valueTxt.section("(", 1, 1).section(")", 0, 0).toFloat(); //not +=
			//qDebug() << rowStd;
			StdSum = std::max(rowStd, StdSum); //to do, this equation is wrong!
			float sMin = rangeTxt.section(" ~ ", 0, 0).toFloat();
			//qDebug() << sMin;
			MinSum = std::min(sMin, MinSum);
			float sMax = rangeTxt.section(" ~ ", 1, 1).toFloat();
			//qDebug() << sMax;
			MaxSum = std::max(sMax, MaxSum);
		}
		qDebug() << "roi info: " << AreaSum << MeanSum << StdSum << MinSum << MaxSum;
		parentItem->child(ImageRowInd, 1)->setText(QString("%1").arg(AreaSum));
		parentItem->child(ImageRowInd, 2)->setText(QString("%1 (%2)").arg(MeanSum).arg(StdSum));
		parentItem->child(ImageRowInd, 3)->setText(QString("%1 ~ %2").arg(MinSum).arg(MaxSum));
	};
protected:
	float scalingValue;
	float shiftValue;
};



static void traceRemovalCallback(vtkObject* caller, long unsigned int eventId,
	void* clientData, void* callData)
{
	vtkContourWidget* contour = reinterpret_cast<vtkContourWidget*>(caller);
	vtkCollection* RoiCllct = reinterpret_cast<vtkCollection*>(clientData);

	contour->Delete();
	vtkContourRepresentation* rep = contour->GetContourRepresentation();

	int numberOfWidgets = RoiCllct->GetNumberOfItems();
	qDebug() << numberOfWidgets << " ROI reps are stored" << endl;
	RoiCllct->RemoveItem(rep);

}
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
void vtkRoiInteractor::initialize(vtkSmartPointer<vtkRenderWindowInteractor> iInt,
	QStandardItem * parentItem, float* scalingPara, const QString text, QHash<QString, QHash<int, vtkContourRepresentation*> >* RoiHash, int RoiMode, int sliceNum)
{
	//default value---none scaling
	imageName = text;

	qDebug() << "Initializing new vtk ROI at " << imageName << endl;;

	vtkSmartPointer< vtkTracerCallback> traceCallback = vtkTracerCallback::New();
	traceCallback->initialize(scalingPara[0], scalingPara[1], text, parentItem, RoiHash, RoiMode, sliceNum);
	//traceCallback->scalingValue = this->scalingPara[0];
	//traceCallback->shiftValue = this->scalingPara[1];

	//traceCallback->SetClientData();
	interactor = iInt;
	newContourWidget = vtkSmartPointer<vtkContourWidget>::New();
	newContourWidget->SetInteractor(interactor);
	newContourWidget->FollowCursorOn();

	vtkOrientedGlyphContourRepresentation* rep = vtkOrientedGlyphContourRepresentation::New();
	rep->GetLinesProperty()->SetColor(1, 1, 0);
	rep->GetLinesProperty()->SetLineWidth(1.5);

	std::cout << "There are " << rep->GetNumberOfNodes() << " nodes and " << rep->GetNumberOfPaths() << " path" << std::endl;

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

	//vtkSmartPointer<vtkCallbackCommand> deleteCallback =
	//	vtkSmartPointer<vtkCallbackCommand>::New();
	//deleteCallback->SetClientData(ThisRoiCllct);
	//deleteCallback->SetCallback(traceRemovalCallback);

	//newContourWidget->AddObserver(vtkCommand::RightButtonReleaseEvent, deleteCallback);

	//roiInfoRow += traceCallback->infoModel;		
	/*QStandardItem* areaItem = roiInfoRow.at(0);
	cout << "in initialize: Area is " << areaItem->text().toStdString() << endl;
	cout << "contour Widget" << endl;*/

	//this->contourWidgetCollection = ThisRoiCllct;
	//this->contourWidgetCollection->AddItem(newContourWidget);
}

//----------------------------------------------------------------------------
void vtkRoiInteractor::usePolydata(vtkSmartPointer<vtkRenderWindowInteractor> iInt, vtkPolyData* plydata)
{
	interactor = iInt;
	newContourWidget = vtkSmartPointer<vtkContourWidget>::New();
	newContourWidget->SetInteractor(interactor);
	//newContourWidget->FollowCursorOn();

	vtkOrientedGlyphContourRepresentation* rep = vtkOrientedGlyphContourRepresentation::New();
	rep->GetLinesProperty()->SetColor(1, 1, 0);
	rep->GetLinesProperty()->SetLineWidth(1.5);
	vtkImageActor* imageActor = static_cast<myVtkInteractorStyleImage*>(iInt->GetInteractorStyle())->GetImageActor();
	vtkImageActorPointPlacer* placer = vtkImageActorPointPlacer::New();
	placer->SetImageActor(imageActor);
	rep->SetPointPlacer(placer);
	newContourWidget->ContinuousDrawOff();
	newContourWidget->SetRepresentation(rep);
	newContourWidget->On();
	newContourWidget->Initialize(plydata);
}

void vtkRoiInteractor::useContourRep(vtkSmartPointer<vtkRenderWindowInteractor> iInt, vtkContourRepresentation* contourRep,
	QStandardItem * parentItem, float* scalingPara, const QString text, QHash<QString, QHash<int, vtkContourRepresentation*> >* RoiHash, int sliceNum)
{
	imageName = text;
	qDebug() << "retriving and editing ROIs at window " << text;
	vtkSmartPointer< vtkTracerCallback> traceCallback = vtkTracerCallback::New();
	traceCallback->initialize(scalingPara[0], scalingPara[1], text, parentItem, RoiHash, 2, sliceNum);

	newContourWidget = vtkSmartPointer<vtkContourWidget>::New();
	newContourWidget->SetInteractor(iInt);
	newContourWidget->ContinuousDrawOff();
	newContourWidget->SetRepresentation(contourRep);
	newContourWidget->On();
	//newContourWidget->CloseLoop();
	vtkSmartPointer<vtkPolyData> path =
		vtkSmartPointer<vtkPolyData>::New();
	//_tracer->GetPath(path);
	//path = rep->GetContourRepresentationAsPolyData();
	contourRep->GetNodePolyData(path);
	newContourWidget->Initialize(path);

	newContourWidget->AddObserver(vtkCommand::EndInteractionEvent, traceCallback);
}
vtkStandardNewMacro(vtkRoiInteractor);