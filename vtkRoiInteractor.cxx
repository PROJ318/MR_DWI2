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
#include <vtkImageExtractComponents.h>
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
		//std::cout << "After close There are " << glyContour->GetNumberOfNodes() << " nodes and " << glyContour->GetNumberOfPaths() << " path" << std::endl;		
		//double* coordinate;
		//coordinate = contourActor->GetPoint(5);
		//cout << "point 5 " << coordinate[0] <<"," <<coordinate[1] << "," << coordinate[2] << endl;


		//Statistics:
		//1. it can be done at callback. vtkcommand::endofinteraction has been sent.
		//2. it has three modes: roimode = 0:, init. 1: append: roi of same name is drawn on a different slice. 
		//   2: edit, roi of same name is drawn on the same slice. 3: remove, delete data. 

		if ((*RoiHash)[sliceNum].contains(parentItem->text())){ RoiMode = 2; }

		if (imageName == QString("Source"))
		{
			if (RoiMode == 0 || RoiMode == 1) //init ROI
			{
				//QHash<QString, vtkContourRepresentation *> newChildHash;
				//newChildHash.insert(parentItem->text(), rep);
				(*RoiHash)[sliceNum].insert(parentItem->text(), rep);
			//}
			//else if (RoiMode == 1) //append ROI
			//{
			//	(*RoiHash)[parentItem->text()].insert(sliceNum, rep);
			}
			else if (RoiMode == 2) //edit ROI
			{
				(*RoiHash)[sliceNum][parentItem->text()] = rep;
			}
			else
			{
				qDebug() << "wrong roi mode";
			}
		}

		qDebug() << "<<<<<< ENTERING CALLBACK OF " << parentItem->parent()->text() << "with MODE: " << RoiMode ;

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
		
		//
		//3. Edit ROI tree model 
		//
		int numComp = o_image->GetNumberOfScalarComponents();
		vtkSmartPointer <vtkImageExtractComponents> scalarComponent = vtkSmartPointer <vtkImageExtractComponents>::New();
		vtkSmartPointer<vtkImageAccumulate> imageAccumulate = vtkSmartPointer<vtkImageAccumulate>::New();

		for (int thisComp = 0; thisComp < numComp; thisComp++)
		{			
			
			scalarComponent->SetInputData(o_image);
			scalarComponent->SetComponents(thisComp);
			scalarComponent->Update();						
			
			imageAccumulate->SetStencilData(polyDataToImageStencil->GetOutput());
			imageAccumulate->SetInputData(scalarComponent->GetOutput());
			imageAccumulate->Update();


			float thisArea = imageAccumulate->GetVoxelCount()*o_image->GetSpacing()[0] * o_image->GetSpacing()[1] * o_image->GetSpacing()[2];

			//*****temporary fix block: fix the error of area calc:		
			
			//if (parentItem->hasChildren()){
			//	for (int i = 0; i < parentItem->child(0)->rowCount(); i++)
			//	{
			//		if (sliceNum == parentItem->child(0)->child(i)->text().toInt())
			//		{
			//			thisArea = parentItem->child(0)->child(i, 1)->text().toFloat();
			//		}
			//	}
			//}

			//*******temporary fix block ends

			float thisMean = *imageAccumulate->GetMean()*scalingValue + shiftValue;
			float thisstd = *imageAccumulate->GetStandardDeviation()*scalingValue + shiftValue;
			float thisMin = *imageAccumulate->GetMin()*scalingValue + shiftValue;
			float thisMax = *imageAccumulate->GetMax()*scalingValue + shiftValue;

			qDebug() << "[Component " << thisComp + 1 <<" of "<<numComp<< "] "<< "[slice " << sliceNum << "] " 
				<< thisArea << thisMean << thisstd << thisMin << thisMax;

			QList<QStandardItem *> slcRow;
			slcRow << new QStandardItem(QString("%1").arg(sliceNum));
			slcRow << new QStandardItem(QString("%1").arg(thisArea));
			slcRow << new QStandardItem(QString("%1 (%2)").arg(thisMean).arg(thisstd));
			slcRow << new QStandardItem(QString("%1 ~ %2").arg(thisMin).arg(thisMax));

			//qDebug() << "handling window "<<imageName<<"parent node is" << parentItem->text();
			//qDebug() << imageAccumulate->GetVoxelCount() << " * " << o_image->GetSpacing()[0] << "-" << o_image->GetSpacing()[1] << "-" << o_image->GetSpacing()[2];
			//qDebug() << " of " << o_image->GetDimensions()[0] << "-" << o_image->GetDimensions()[1]<<"-" << o_image->GetDimensions()[2];
			if (RoiMode == 0) //init ROI
			{
				

				QStandardItem* imageRoot = parentItem->parent();
				qDebug() << "Init a new ROI " << parentItem->index().row() << " on " << imageRoot->text() << "Slice " << sliceNum;

				//QList<QStandardItem *> imgRow;
				//imgRow << new QStandardItem(imageName);
				//imgRow << new QStandardItem(QString("%1").arg(thisArea));
				//imgRow << new QStandardItem(QString("%1 (%2)").arg(thisMean).arg(thisstd));
				//imgRow << new QStandardItem(QString("%1 ~ %2").arg(thisMin).arg(thisMax));
				//parentItem->appendRow(imgRow);
				if (numComp > 1)
				{
					imageRoot->child(parentItem->index().row(), 1)->setText(QString("%1").arg(0));
					imageRoot->child(parentItem->index().row(), 2)->setText(QString("%1 (%2)").arg(0.0).arg(0.0));
					imageRoot->child(parentItem->index().row(), 3)->setText(QString("%1 ~ %2").arg(0.0).arg(0.0));

					QList<QStandardItem *> compRow;
					compRow << new QStandardItem(QString("IMAGE_%1").arg(thisComp));
					compRow << new QStandardItem(QString("%1").arg(thisArea));
					compRow << new QStandardItem(QString("%1 (%2)").arg(thisMean).arg(thisstd));
					compRow << new QStandardItem(QString("%1 ~ %2").arg(thisMin).arg(thisMax));
					parentItem->appendRow(compRow);
					compRow.first()->appendRow(slcRow);
				}
				else{
					imageRoot->child(parentItem->index().row(), 1)->setText(QString("%1").arg(thisArea));
					imageRoot->child(parentItem->index().row(), 2)->setText(QString("%1 (%2)").arg(thisMean).arg(thisstd));
					imageRoot->child(parentItem->index().row(), 3)->setText(QString("%1 ~ %2").arg(thisMin).arg(thisMax));
					parentItem->appendRow(slcRow);
				}				
			}
			else
			{
				//parentItem->sortChildren(0);
				////1. locate Component Branch 
				//int ImageRowInd(-10);
				//for (int i = 0; i < parentItem->rowCount(); i++)
				//{
				//	if (imageName == parentItem->child(i)->text())
				//	{
				//		qDebug() << "found ROW of " << imageName << "at row:" << i;
				//		ImageRowInd = i;
				//	}
				//}

				if (numComp > 1) //if this image has multiple component;
				{
					//2. locate ImageName Branch			
					if (RoiMode == 1)//append ROI: add a slice row
					{
						qDebug() << "ROImode = append";
						parentItem->child(thisComp)->appendRow(slcRow);
					}
					else if (RoiMode == 2) //edit ROI:: edit existing slic Row
					{
						qDebug() << "ROImode = edit";
						for (int i = 0; i < parentItem->child(thisComp)->rowCount(); i++)
						{
							if (sliceNum == parentItem->child(thisComp)->child(i)->text().toInt())
							{
								parentItem->child(thisComp)->child(i, 1)->setText(QString("%1").arg(thisArea));
								parentItem->child(thisComp)->child(i, 2)->setText(QString("%1 (%2)").arg(thisMean).arg(thisstd));
								parentItem->child(thisComp)->child(i, 3)->setText(QString("%1 ~ %2").arg(thisMin).arg(thisMax));
							}
						}

					}
					else
					{
						qDebug() << "wrong roi mode";
					}
					//3. update ImageName Branch
					parentItem->child(thisComp)->sortChildren(0);
					updateImageRow(parentItem->child(thisComp));
				}
				else //if this image has only 1 component
				{
					//2. locate ImageName Branch			
					if (RoiMode == 1)//append ROI: add a slice row
					{
						qDebug() << "ROImode = append";
						parentItem->appendRow(slcRow);
					}
					else if (RoiMode == 2) //edit ROI:: edit existing slic Row
					{
						qDebug() << "ROImode = edit";
						for (int i = 0; i < parentItem->rowCount(); i++)
						{
							if (sliceNum == parentItem->child(i)->text().toInt())
							{
								parentItem->child(i, 1)->setText(QString("%1").arg(thisArea));
								parentItem->child(i, 2)->setText(QString("%1 (%2)").arg(thisMean).arg(thisstd));
								parentItem->child(i, 3)->setText(QString("%1 ~ %2").arg(thisMin).arg(thisMax));
							}
						}

					}
					else
					{
						qDebug() << "wrong roi mode";
					}
					//3. update ImageName Branch
					parentItem->sortChildren(0);
					updateImageRow(parentItem);

					//qDebug() << "edit/append roimode, though no such image exists,creating new";
					////1. Create ImageName Branch first
					//QList<QStandardItem *> imgRow;
					//imgRow << new QStandardItem(imageName);
					//imgRow << new QStandardItem(QString("%1").arg(thisArea));
					//imgRow << new QStandardItem(QString("%1 (%2)").arg(thisMean).arg(thisstd));
					//imgRow << new QStandardItem(QString("%1 ~ %2").arg(thisMin).arg(thisMax));
					//parentItem->appendRow(imgRow);

					////2. Append slice Branch
					//imgRow.first()->appendRow(slcRow);
				}
			}
		}
		qDebug() << ">>>>>> "<< imageName <<" CALLBACK FINISHED";	
	}

	vtkTracerCallback() : scalingValue(0), shiftValue(0), RoiMode(-1), sliceNum(-1){};
	void initialize(float _scalingValue, float _shiftValue, QStandardItem* _parentItem,
		QHash<int, QHash<QString, vtkContourRepresentation*> >* _RoiHash, int _RoiMode, int _sliceNum)
	{
		scalingValue = _scalingValue; shiftValue = _shiftValue; parentItem = _parentItem;
		imageName = parentItem->parent()->text(); RoiHash = _RoiHash; RoiMode = _RoiMode; sliceNum = _sliceNum;
		//cout << *rowHead << endl;
	};

	void updateImageRow(QStandardItem* branchroot)
	{
		//QStandardItem* branchroot = parentItem->child(ImageRowInd);
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
		QStandardItem* parentRoot = branchroot->parent();
		parentRoot->child(branchroot->index().row(), 1)->setText(QString("%1").arg(AreaSum));
		parentRoot->child(branchroot->index().row(), 2)->setText(QString("%1 (%2)").arg(MeanSum).arg(StdSum));
		parentRoot->child(branchroot->index().row(), 3)->setText(QString("%1 ~ %2").arg(MinSum).arg(MaxSum));
	};
protected:
	float scalingValue;
	float shiftValue;

	QStandardItem* parentItem;
	QString imageName;
	QHash<int, QHash<QString, vtkContourRepresentation*> >* RoiHash;
	int RoiMode;
	int sliceNum;

};

class EditTracerCallback :public vtkCommand
{
public:
	static EditTracerCallback *New()
	{
		return new EditTracerCallback;
	}

	void Execute(vtkObject *caller, unsigned long, void*) VTK_OVERRIDE
	{
		qDebug() << "<<<<<< ENTERING EDIT CALLBACK OF " << parentItem->parent()->text();

		vtkContourWidget *contour = reinterpret_cast<vtkContourWidget*>(caller);
		vtkContourRepresentation *rep = vtkContourRepresentation::SafeDownCast(contour->GetRepresentation());
		vtkOrientedGlyphContourRepresentation *glyContour =
			vtkOrientedGlyphContourRepresentation::SafeDownCast(rep);

		//contourWidget = contour;

		vtkSmartPointer<myVtkInteractorStyleImage> style =
			vtkSmartPointer<myVtkInteractorStyleImage>::New();
		style = static_cast<myVtkInteractorStyleImage*>(contour->GetInteractor()->GetInteractorStyle());
		vtkImageData* o_image = style->GetInputImage();



		(*RoiHash)[sliceNum][parentItem->text()] = rep;

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

		//
		//3. Edit ROI tree model 
		//
		int numComp = o_image->GetNumberOfScalarComponents();
		vtkSmartPointer <vtkImageExtractComponents> scalarComponent = vtkSmartPointer <vtkImageExtractComponents>::New();
		vtkSmartPointer<vtkImageAccumulate> imageAccumulate = vtkSmartPointer<vtkImageAccumulate>::New();

		for (int thisComp = 0; thisComp < numComp; thisComp++)
		{

			scalarComponent->SetInputData(o_image);
			scalarComponent->SetComponents(thisComp);
			scalarComponent->Update();

			imageAccumulate->SetStencilData(polyDataToImageStencil->GetOutput());
			imageAccumulate->SetInputData(scalarComponent->GetOutput());
			imageAccumulate->Update();


			float thisArea = imageAccumulate->GetVoxelCount()*o_image->GetSpacing()[0] * o_image->GetSpacing()[1] * o_image->GetSpacing()[2];

			//*****temporary fix block: fix the error of area calc:		

			//if (parentItem->hasChildren()){
			//	for (int i = 0; i < parentItem->child(0)->rowCount(); i++)
			//	{
			//		if (sliceNum == parentItem->child(0)->child(i)->text().toInt())
			//		{
			//			thisArea = parentItem->child(0)->child(i, 1)->text().toFloat();
			//		}
			//	}
			//}

			//*******temporary fix block ends

			float thisMean = *imageAccumulate->GetMean()*scalingValue + shiftValue;
			float thisstd = *imageAccumulate->GetStandardDeviation()*scalingValue + shiftValue;
			float thisMin = *imageAccumulate->GetMin()*scalingValue + shiftValue;
			float thisMax = *imageAccumulate->GetMax()*scalingValue + shiftValue;

			qDebug() << "[Component " << thisComp + 1 << " of " << numComp << "] " << "[slice " << sliceNum << "] "
				<< thisArea << thisMean << thisstd << thisMin << thisMax;

			QList<QStandardItem *> slcRow;
			slcRow << new QStandardItem(QString("%1").arg(sliceNum));
			slcRow << new QStandardItem(QString("%1").arg(thisArea));
			slcRow << new QStandardItem(QString("%1 (%2)").arg(thisMean).arg(thisstd));
			slcRow << new QStandardItem(QString("%1 ~ %2").arg(thisMin).arg(thisMax));

			//qDebug() << "handling window "<<imageName<<"parent node is" << parentItem->text();
			//qDebug() << imageAccumulate->GetVoxelCount() << " * " << o_image->GetSpacing()[0] << "-" << o_image->GetSpacing()[1] << "-" << o_image->GetSpacing()[2];
			//qDebug() << " of " << o_image->GetDimensions()[0] << "-" << o_image->GetDimensions()[1]<<"-" << o_image->GetDimensions()[2];
				//parentItem->sortChildren(0);
				////1. locate Component Branch 
				//int ImageRowInd(-10);
				//for (int i = 0; i < parentItem->rowCount(); i++)
				//{
				//	if (imageName == parentItem->child(i)->text())
				//	{
				//		qDebug() << "found ROW of " << imageName << "at row:" << i;
				//		ImageRowInd = i;
				//	}
				//}

				if (numComp > 1) //if this image has multiple component;
				{
			
					for (int i = 0; i < parentItem->child(thisComp)->rowCount(); i++)
					{
						if (sliceNum == parentItem->child(thisComp)->child(i)->text().toInt())
						{
							parentItem->child(thisComp)->child(i, 1)->setText(QString("%1").arg(thisArea));
							parentItem->child(thisComp)->child(i, 2)->setText(QString("%1 (%2)").arg(thisMean).arg(thisstd));
							parentItem->child(thisComp)->child(i, 3)->setText(QString("%1 ~ %2").arg(thisMin).arg(thisMax));
						}
					}

					//3. update ImageName Branch
					parentItem->child(thisComp)->sortChildren(0);
					updateImageRow(parentItem->child(thisComp));
				}
				else //if this image has only 1 component
				{
					//2. locate ImageName Branch			

						for (int i = 0; i < parentItem->rowCount(); i++)
						{
							if (sliceNum == parentItem->child(i)->text().toInt())
							{
								parentItem->child(i, 1)->setText(QString("%1").arg(thisArea));
								parentItem->child(i, 2)->setText(QString("%1 (%2)").arg(thisMean).arg(thisstd));
								parentItem->child(i, 3)->setText(QString("%1 ~ %2").arg(thisMin).arg(thisMax));
							}
						}

					//3. update ImageName Branch
					parentItem->sortChildren(0);
					updateImageRow(parentItem);

					//qDebug() << "edit/append roimode, though no such image exists,creating new";
					////1. Create ImageName Branch first
					//QList<QStandardItem *> imgRow;
					//imgRow << new QStandardItem(imageName);
					//imgRow << new QStandardItem(QString("%1").arg(thisArea));
					//imgRow << new QStandardItem(QString("%1 (%2)").arg(thisMean).arg(thisstd));
					//imgRow << new QStandardItem(QString("%1 ~ %2").arg(thisMin).arg(thisMax));
					//parentItem->appendRow(imgRow);

					////2. Append slice Branch
					//imgRow.first()->appendRow(slcRow);
				}
			
		}
		qDebug() << ">>>>>> " << imageName << " EDIT CALLBACK FINISHED";

	}


	//QList<QStandardItem *>* infoModel;

	EditTracerCallback() : scalingValue(0), shiftValue(0), sliceNum(-1){};
	void initialize(float _scalingValue, float _shiftValue, QStandardItem* _parentItem,
		QHash<int, QHash<QString, vtkContourRepresentation*> >* _RoiHash, int _sliceNum)
	{
		scalingValue = _scalingValue; shiftValue = _shiftValue; parentItem = _parentItem;
		imageName = parentItem->parent()->text(); RoiHash = _RoiHash; sliceNum = _sliceNum;
		//cout << *rowHead << endl;
	};

	void updateImageRow(QStandardItem* branchroot)
	{
		//QStandardItem* branchroot = parentItem->child(ImageRowInd);
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
		QStandardItem* parentRoot = branchroot->parent();
		parentRoot->child(branchroot->index().row(), 1)->setText(QString("%1").arg(AreaSum));
		parentRoot->child(branchroot->index().row(), 2)->setText(QString("%1 (%2)").arg(MeanSum).arg(StdSum));
		parentRoot->child(branchroot->index().row(), 3)->setText(QString("%1 ~ %2").arg(MinSum).arg(MaxSum));
	};
protected:
	float scalingValue;
	float shiftValue;
	QStandardItem* parentItem;
	QString imageName;
	QHash<int, QHash<QString, vtkContourRepresentation*> >* RoiHash;
	int sliceNum;
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
	QStandardItem * parentItem, float* scalingPara, QHash<int, QHash<QString, vtkContourRepresentation*> >* RoiHash, int RoiMode, int sliceNum)
{
	//default value---none scaling
	imageName = parentItem->parent()->text();

	qDebug() << "Initializing new vtk ROI " << parentItem->text() << " at " << imageName << endl;;

	vtkSmartPointer< vtkTracerCallback> traceCallback = vtkTracerCallback::New();
	traceCallback->initialize(scalingPara[0], scalingPara[1], parentItem, RoiHash, RoiMode, sliceNum);
	//traceCallback->scalingValue = this->scalingPara[0];
	//traceCallback->shiftValue = this->scalingPara[1];

	//traceCallback->SetClientData();
	interactor = iInt;
	newContourWidget = vtkSmartPointer<vtkContourWidget>::New();
	newContourWidget->SetInteractor(interactor);
	newContourWidget->FollowCursorOn();

	vtkOrientedGlyphContourRepresentation* rep = vtkOrientedGlyphContourRepresentation::New();
	rep->GetLinesProperty()->SetLineWidth(1.5);

	if (parentItem->text() == QString("ROI1"))
	{
		qDebug() << "set color of ROI1";
		rep->GetLinesProperty()->SetColor(0, 1, 0);
	}
	else if (parentItem->text() == QString("ROI2")){
		rep->GetLinesProperty()->SetColor(1, 1, 0);
	}
	else if (parentItem->text() == QString("ROI3")){
		rep->GetLinesProperty()->SetColor(1, 0, 0);
	}

	std::cout << "There are " << rep->GetNumberOfNodes() << " nodes and " << rep->GetNumberOfPaths() << " path" << std::endl;

	vtkImageActor* imageActor = static_cast<myVtkInteractorStyleImage*>(iInt->GetInteractorStyle())->GetImageActor();

	if (!imageActor)
	{ 
		qDebug() << "Image Actor cannot be retreived";
		return; 
	}

	vtkImageActorPointPlacer* placer = vtkImageActorPointPlacer::New();
	placer->SetImageActor(imageActor);

	rep->SetPointPlacer(placer);
	//vtkSmartPointer<vtkCallbackCommand> RoicloseCallback =
	//	vtkSmartPointer<vtkCallbackCommand>::New();
	//// Allow the observer to access the sphereSource
	//RoicloseCallback->SetClientData(&roiInfoRow);
	//RoicloseCallback->SetCallback(RoicloseCallbackFunction);


	newContourWidget->SetRepresentation(rep);
	newContourWidget->ContinuousDrawOff();
	newContourWidget->On();
	newContourWidget->AddObserver(vtkCommand::EndInteractionEvent, traceCallback);
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
	QStandardItem * parentItem, float* scalingPara, QHash<int, QHash<QString, vtkContourRepresentation*> >* RoiHash, int sliceNum)
{
	imageName = parentItem->parent()->text();

	//m_representation = contourRep;
	qDebug() << "retriving and editing ROIs at window " << this->imageName;
	vtkSmartPointer< EditTracerCallback> edittraceCallback = EditTracerCallback::New();
	edittraceCallback->initialize(scalingPara[0], scalingPara[1], parentItem, RoiHash, sliceNum);

	interactor = iInt;

	newContourWidget = vtkSmartPointer<vtkContourWidget>::New();
	newContourWidget->SetInteractor(interactor);
	newContourWidget->FollowCursorOn();

	vtkOrientedGlyphContourRepresentation* rep = vtkOrientedGlyphContourRepresentation::New();

	if (parentItem->text() == QString("ROI1"))
	{
		qDebug() << "set color of ROI1";
		rep->GetLinesProperty()->SetColor(0, 1, 0);
	}
	else if (parentItem->text() == QString("ROI2")){
		rep->GetLinesProperty()->SetColor(1, 1, 0);
	}
	else if (parentItem->text() == QString("ROI3")){
		rep->GetLinesProperty()->SetColor(1, 0, 0);
	}

	rep->GetLinesProperty()->SetLineWidth(1.5);
	std::cout << "There are " << rep->GetNumberOfNodes() << " nodes and " << rep->GetNumberOfPaths() << " path" << std::endl;
	vtkImageActor* imageActor = static_cast<myVtkInteractorStyleImage*>(iInt->GetInteractorStyle())->GetImageActor();
	vtkImageActorPointPlacer* placer = vtkImageActorPointPlacer::New();
	placer->SetImageActor(imageActor);
	rep->SetPointPlacer(placer);

	newContourWidget->SetRepresentation(rep);
	newContourWidget->On();

	vtkSmartPointer<vtkPolyData> path = vtkSmartPointer<vtkPolyData>::New();
	contourRep->GetNodePolyData(path);

	newContourWidget->ContinuousDrawOff();
	newContourWidget->Initialize(path);

	vtkContourWidget* thiscontour = newContourWidget;
	qDebug() << "ROI widget created at " << this->imageName << "Address is " << thiscontour <<" Countour Rep is "<<contourRep;
	qDebug() << "Rep in ROI widget is " << thiscontour->GetRepresentation();
	newContourWidget->AddObserver(vtkCommand::EndInteractionEvent, edittraceCallback);
}
vtkStandardNewMacro(vtkRoiInteractor);