/*===================================================================

DWI Core UI

===================================================================*/

#ifndef DiffusionCore_h
#define DiffusionCore_h

// Headers in this module
// #include <DicomHelper.h>

//include QT

//#include <QLabel>
//#include <QProgressDialog>
//#include <QString>
//#include <QStringList>
//#include <QVariant>

// include VTK
//#include <vtkSmartPointer.h
//#include <vtkImageData.h>
//#include <vtkRenderWindowInteractor.h>

#include <QWidget>
#include <qdebug.h>

//#include "QVTKWidget.h"

#include <itkVectorImage.h>
#include <itkImage.h>

#include <vtkSmartPointer.h>
#include <vtkObject.h>
//#include "vtkRendererCollection.h"
//#include <vtkActor2DCollection.h>
//#include <itkVectorContainer.h>
//#include <itkComposeImageFilter.h>
//#include <itkShiftScaleImageFilter.h>
//#include <itkCastImageFilter.h>

typedef unsigned short SourceImagePixelType;
typedef float DiffusionCalculatorPixelType; 
typedef itk::Image < SourceImagePixelType, 3> SourceImageType;
typedef itk::Image <DiffusionCalculatorPixelType, 3> DiffusionCalculatorImageType;
typedef itk::VectorImage <DiffusionCalculatorPixelType, 3> DiffusionCalculatorVectorImageType;
//jiangli end

class ctkFileDialog;

namespace Ui{
	class DiffusionModule;
}
class DicomHelper;
class QVTKWidget;
class vtkCamera;
class vtkEventQtSlotConnect;
class vtkImageData;
class vtkRenderWindowInteractor;
class DisplayPort;
class QGroupBox;
class QButtonGroup;
class QProgressDialog;
/**
* \brief DiffusionCore is a QWidget providing functionality for diffusion weighted image calculation.
*
* \sa 
* \ingroup 
*/
#define MAXWINDOWNUMBER 5 //difine the busiest viewing window layout as 5 by 5
class DiffusionCore : public QWidget
{
	// this is needed for all Qt objects that should have a Qt meta-object
	// (everything that derives from QObject and wants to have signal/slots)
	Q_OBJECT

public:

	//static const std::string Widget_ID;

	/**
	* \brief DiffusionCore(QWidget *parent) constructor.
	*
	* \param parent is a pointer to the parent widget
	*/
	DiffusionCore(QWidget *parent);

	/**
	* \brief DiffusionCore destructor.
	*/
	virtual ~DiffusionCore();

	/**
	* \brief CreateQtPartControl(QWidget *parent) sets the view objects from ui_DiffusionModule.h.
	*
	* \param parent is a pointer to the parent widget
	*/
	virtual void CreateQtPartControl(QWidget *parent);

	/**
	* \brief Initializes the widget. This method has to be called before widget can start.
	*/
	void Initialize();

	/**
	* \brief A QHash to store calculated 3D image.
	*/
	//QHash< const QString, vtkSmartPointer<vtkImageData> >  image3Dstorage;
signals:

	///// @brief emitted when dicomdata is imported.
	void SignalImageLoaded(bool);

	void SignalTestButtonFired(bool _istoggled, vtkSmartPointer <vtkImageData>, QString, float , float );

	void signal3DImage(vtkSmartPointer <vtkImageData>, QString);
	void signalSaveDcmComplete(bool);
	public slots:

	/////// @brief 
	/////// This slot retrieve source image.
	/////// 
	//void OnImageFilesLoaded(const QStringList&);

	///// @brief 
	///// This slot recalculate all displaying image.
	///// 
	void onRecalcAll(int);

	///// @brief
	///// In this slot, 3d calculation of input buttonID. 
	///// 
	void onCalc3D(QString directory);

	protected slots:

	///// @brief
	///// In this slot, create/update SourceImage
	///// 
	void onSetSourceImage(DicomHelper*, int );
	
	///// @brief
	///// In this slot, call adc calculation and render the image
	///// 
	void onCalcADC(bool toggle);

	///// @brief
	///// In this slot, call cdwi calculation and render the image
	///// 
	void onCalcCDWI(bool toggle);

	///// @brief
	///// In this slot, call eADC calculation and render the image
	///// 
	void onCalcEADC(bool toggle);

	///// @brief
	/////In this slot, call FA calculation and render the image
	///// 
	void onCalcFA(bool toggle);

	///// @brief
	///// In this slot, call colorFA calculation and render the image
	/////
	void onCalcColorFA(bool toggle);

	///// @brief
	///// In this slot, call IVIM calculation and render the image
	///// 
	void onCalcIVIM(bool toggle);

	///// @brief
	///// In this slot, update the cdwi image with input bvalue
	///// 
	void onBSlide(double bvalue);

	///// @brief
	///// In this slot, update the adc image with filtering using input threshhold
	///// 
	void onThreshSlide(double threshhold);

	///// @brief
	///// In this slot, it is a portal to processing's for specific images. 
	///// 
	void onSelectImage(const QString);


protected:

	void UpdateMaskVectorImage(DicomHelper* inputDcmData, int, DiffusionCalculatorVectorImageType::Pointer outputImage);//This should be moved to DicomHelper class.
	void AdcCalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope);
	void FaCalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope);
	void ColorFACalculator(vtkSmartPointer <vtkImageData> imageData);
	void EAdcCalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope);
	void CDWICalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope);
	void IVIMCalculator(vtkSmartPointer <vtkImageData> IVIM_F, vtkSmartPointer <vtkImageData> IVIM_DStar, vtkSmartPointer <vtkImageData> IVIM_D, double* scaleSlope, double* scaleIntercept);
	void ComputeCurrentSourceImage(int currentSlice, vtkSmartPointer <vtkImageData> SourceImageData);
	//void IVIMCalculator2(std::vector <vtkSmartPointer <vtkImageData>> vtkImageDataVector);
	//void DiffusionCore::IVIMImageViewer2(std::vector <vtkSmartPointer <vtkImageData>> vtkImageDataVector, QVTKWidget *qvtkWidget);
	
protected:
	
	Ui::DiffusionModule* m_Controls;

	DicomHelper* m_DicomHelper;	

	DiffusionCalculatorVectorImageType::Pointer m_MaskVectorImage;//USE VectorImageType::Pointer
	int m_CurrentSlice;

	QHash<int, DiffusionCalculatorVectorImageType::Pointer> m_vectorImage;

	vtkEventQtSlotConnect* Connections;

	int sourceScalarType = 0;

	double m_MaskThreshold;
	double m_ComputedBValue;

	QButtonGroup* ButtonTable; // A QButtonGroup to store all algorithm Buttons;
	QProgressDialog* progressDialog;
	QList<int> Diff_ActiveWdw; // This should be synchronising to mainwindow activeWDW;
};


#endif //

