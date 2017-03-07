/*===================================================================

DWI Core UI

===================================================================*/

#ifndef PerfusionCore_h
#define PerfusionCore_h

#include <QWidget>
#include <qdebug.h>
#include <itkVectorImage.h>
#include <itkImage.h>

#include <vtkSmartPointer.h>
namespace Ui{
	class PerfusionModule;
}
class QVTKWidget;
class QButtonGroup;
class DicomHelper;
class vtkImageData;
class QGroupBox;
class QButtonGroup;

typedef unsigned short SourceImagePixelType;
typedef float PerfusionCalculatorPixelType;
typedef itk::Image < SourceImagePixelType, 3> SourceImageType;
typedef itk::Image <PerfusionCalculatorPixelType, 3> PerfusionCalculatorImageType;
typedef itk::VectorImage <PerfusionCalculatorPixelType, 3> PerfusionCalculatorVectorImageType;

class PerfusionCore :public QWidget
{
	// this is needed for all Qt objects that should have a Qt meta-object
	// (everything that derives from QObject and wants to have signal/slots)
	Q_OBJECT

public:
	/**
	* \brief PerfusionCore(QWidget *parent) constructor.
	*
	* \param parent is a pointer to the parent widget
	*/
	PerfusionCore(QWidget *parent);

	/**
	* \brief PerfusionCore destructor.
	*/
	virtual ~PerfusionCore();

	/**
	* \brief CreateQtPartControl(QWidget *parent) sets the view objects from ui_Perfusion.h.
	*
	* \param parent is a pointer to the parent widget
	*/
	virtual void CreateQtPartControl(QWidget *parent);

	/**
	* \brief Initializes the widget. This method has to be called before widget can start.
	*/
	void Initialize();

signals:
	void SignalTestButtonFired(bool _istoggled, vtkSmartPointer <vtkImageData>, QString, float, float);

	protected slots :
	///// @brief
	///// In this slot, create/update SourceImage
	///// 
	void onSetSourceImage(DicomHelper*, int);
	///// @brief
	///// In this slot, call perfusion calc and render the image
	///// 
	void onCalcRelativeEnhance(bool _istoggled);
	void onCalcMaxRelativeEnhance(bool _istoggled);
	void onCalcMaxEnhance(bool _istoggled);
	void onCalcWashOutRate(bool _istoggled);
	void onCalcWashInRate(bool _istoggled);
	void onCalcAreaUnderCurve(bool _istoggled);
	void onCalcT0(bool _istoggled);
	void onCalcTimetoPeak(bool _istoggled);
	void onCalcBrevityOfEnhance(bool _istoggled);

	void onRecalcAll(int inputSlice);

	///// @brief
	///// In this slot, update the adc image with filtering using input threshhold
	///// 
	void onThreshSlide(double threshhold);

protected:
	void PerfusionCalculator(vtkSmartPointer <vtkImageData> imageData, float& scale, float& slope, int index);

protected:
	Ui::PerfusionModule* m_Controls;
	double m_MaskThreshold;
	DicomHelper* m_DicomHelper;
	QButtonGroup* ButtonTable; // A QButtonGroup to store all algorithm Buttons. 

	int m_CurrentSlice;

	PerfusionCalculatorVectorImageType::Pointer m_MaskVectorImage;//USE VectorImageType::Pointer
	void UpdateMaskVectorImage(DicomHelper* dicomData, PerfusionCalculatorVectorImageType::Pointer _MaskVectorImage);
};

#endif //

