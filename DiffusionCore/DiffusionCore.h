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
protected:
	enum imageType
	{
		ORIGINAL = 0,
		ADC = 1,
		CDWI = 2,
		EADC = 3,
		FA = 4,
		CFA = 5,
		IVIM_F = 6,
		IVIM_D = 7,
		IVIM_Dstar = 8,
		NOIMAGE = 100
	};

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

	DisplayPort* displayLayout;

signals:

	///// @brief emitted when dicomdata is imported.
	void SignalDicomLoaded(bool);

	void SignalTestButtonFired(bool _istoggled, vtkSmartPointer <vtkImageData>, QString, float , float );

	public slots:

	///// @brief 
	///// In this slot,  render input dicom files.
	///// 
	void OnImageFilesLoaded(const QStringList&);

	protected slots:

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
	///// In this slot, change the interactor of qvtkwindows to ROI drawing.
	///// 
	void addROI();//bool toggle


	///// @brief
	///// In this slot, change the interactor of qvtkwindows to ROI drawing.
	///// 
	void onTestButton(bool _istoggled);

	///// @brief
	///// In this slot, change the interactor of qvtkwindows to pixel picker.
	///// 
	void onCursorPickValue(bool _istoggled);

	///// @brief
	///// In this slot, change the interactor of qvtkwindows to pixel picker.
	///// 
	void onDisplayPickValue(vtkObject* obj, unsigned long,
		void* client_data, void*, 
		vtkCommand * command);

protected:



	void DisplayDicomInfo(vtkSmartPointer <vtkImageData> imageData);
	void SourceImageViewer2D(vtkSmartPointer <vtkImageData>, QVTKWidget *qvtkWidget);
	void QuantitativeImageViewer2D(vtkSmartPointer <vtkImageData>, QVTKWidget *qvtkWidget, std::string imageLabel);
	void SetImageFillWindow(vtkSmartPointer <vtkCamera> &camera, vtkSmartPointer <vtkImageData> imageData, double width, double height);
	void TestCallbackFunc(vtkObject *caller, long unsigned int eventId, void *clientData, void* callData);
	void ShareWindowEvent();

	void SortingSourceImage();
	void UpdateMaskVectorImage();
	void AdcCalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope);
	void FaCalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope);
	void ColorFACalculator(vtkSmartPointer <vtkImageData> imageData);
	void EAdcCalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope);
	void CDWICalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope);
	void IVIMCalculator(vtkSmartPointer <vtkImageData> imageData);
	void IVIMImageViewer(vtkSmartPointer <vtkImageData>, QVTKWidget *qvtkWidget, int imageIdx);
	//void IVIMCalculator2(std::vector <vtkSmartPointer <vtkImageData>> vtkImageDataVector);
	//void DiffusionCore::IVIMImageViewer2(std::vector <vtkSmartPointer <vtkImageData>> vtkImageDataVector, QVTKWidget *qvtkWidget);
	
protected:
	
	Ui::DiffusionModule* m_Controls;
	DicomHelper *m_DicomHelper;//initialization? 

	vtkSmartPointer < vtkImageData > sourceImage;
	DiffusionCalculatorVectorImageType::Pointer m_MaskVectorImage;//USE VectorImageType::Pointer

	vtkEventQtSlotConnect* Connections;

	int sourceScalarType = 0;
	int m_SourceImageCurrentSlice;
	int m_QuantitativeImageCurrentSlice;
	QHash < const QString, float >  m_ScalingParameter;

	double m_MaskThreshold;
	double m_ComputedBValue;

	QGroupBox* viewFrame;


	//std::vector< std::vector<int> > layoutTable; // Table used for tracing window content
	QButtonGroup* ButtonTable; // A QButtonGroup to store all algorithm Buttons. 

};


#endif //

