#ifndef DicomHelper_h
#define DicomHelper_h

#include <vtkDICOMDirectory.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMReader.h>
#include <vtkDICOMMetaData.h>

#include <itkVector.h>
#include <itkVectorContainer.h>

#include <vnl/vnl_matrix.h>
#include <vnl/algo/vnl_matrix_inverse.h>

#include <iostream>
#include <vector>

#define PI 3.141592653589793
#define RAD  (PI/180.0)




class DicomHelper
{
private:
	// default philips
	vtkDICOMTag ScaleSlop= vtkDICOMTag(0x2005, 0x100E);
	vtkDICOMTag ScaleInterpcept = vtkDICOMTag(0x2005, 0x100D);

	vtkDICOMTag DiffusionBValues = vtkDICOMTag(0x2001, 0x1003);
	//vtkDICOMTag DiffusionBValues = vtkDICOMTag(0x0018, 0x9087);
	vtkDICOMTag DiffusionDirectionRL = vtkDICOMTag(0x2005, 0x10b0); //x direction
	vtkDICOMTag DiffusionDirectionFH = vtkDICOMTag(0x2005, 0x10b2); //y direction
	vtkDICOMTag DiffusionDirectionAP = vtkDICOMTag(0x2005, 0x10b1);//z direction

	vtkDICOMTag ImagingOrientation = vtkDICOMTag(0x2001, 0x100B); //trans cor sag
	vtkDICOMTag ImagingDirectionRL = vtkDICOMTag(0x2005, 0x1002);//RL
	vtkDICOMTag ImagingDirectionFH = vtkDICOMTag(0x2005, 0x1001);//FH
	vtkDICOMTag ImagingDirectionAP = vtkDICOMTag(0x2005, 0x1000);//AP

public:
	DicomHelper(vtkStringArray* Files);

	void Dicomread();
	vtkSmartPointer<vtkDICOMReader> DicomReader;
	~DicomHelper() {};

	float scaleSlope;
	float scaleIntercept;
	int numberOfGradDirection;
	int numberOfBValue;
	int numberOfComponents;
	int IsoImageLabel = -1; // -1 means no isotropic image
	bool tensorComputationPossible = false;

	std::vector <float> BvalueList;
	std::vector <float> directionLabel;
	vnl_matrix<float> finalH;
	int imageDimensions[3];
	char* manuFacturer;

	const char *image_orientation;
	itk::Vector<double, 3> ang;
	vnl_matrix<double> slice2PatMatrix;

	typedef itk::Vector<float, 3> GradientDirectionType;
	typedef itk::VectorContainer< unsigned int, GradientDirectionType >
		GradientDirectionContainerType;
	GradientDirectionContainerType::Pointer gradientDirection;

private:
	vtkStringArray *FileNamesForDiffusionSeries;
	bool isEnhanced;
	//int GetDiffusionDataset(char *DirectoryName);

	void CalculateFinalHMatrix();
	void GetSliceToPatMatrix();
	void UpdateGradDirectionNumber();
	void RemoveIsoImage();
	void DicomTagForGE();
	void DicomInfo();
	vtkDICOMValue GetAttributeValue(vtkDICOMMetaData* metaData, vtkDICOMTag tagValue,
		int fileIndex, int frameIndex);

};

#endif