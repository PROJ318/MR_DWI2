#include <DicomHelper.h>

//#include <vtkDICOMMetaData.h>
#include <vtkDICOMValue.h>
#include <vtkMedicalImageProperties.h>
#include <vtkDICOMApplyRescale.h>
#include <vtkImageData.h>
#include <vtkStringArray.h>
#include <vtkIntArray.h>
//#include "qdebug.h"


DicomHelper::DicomHelper(vtkStringArray* Files)
{
	numberOfGradDirection = 1;
	numberOfBValue = 1;
	scaleSlope = 1;
	scaleIntercept = 0;
	isEnhanced = 1;
	ang.Fill(0.0);

	//if (Files->GetSize() == 1) isEnhanced = 1;
	DicomReader = vtkSmartPointer<vtkDICOMReader>::New();
	DicomReader->SetFileNames(Files);
	//cout << "file:" << Files->GetPointer(0)->c_str() << endl;
	//DicomReader->SetFileName(Files->GetPointer(0)->c_str());
	//Resacle will be done out of the reader.
	DicomReader->AutoRescaleOff(); 
	DicomReader->Update();

	numberOfComponents = DicomReader->GetNumberOfScalarComponents();
	DicomReader->GetOutput()->GetDimensions(imageDimensions);

	vtkMedicalImageProperties* properties = DicomReader->GetMedicalImageProperties();
	manuFacturer = properties->GetManufacturer();
	if ((manuFacturer[0] == 'G') || (manuFacturer[0] == 'g'))
	{
		DicomTagForGE();
	}

	DicomInfo();
};

void DicomHelper::DicomTagForGE()
{
	DiffusionBValues = vtkDICOMTag(0x0043, 0x1039);//    vtkDICOMTag(0x0043, 0x1039)

    //DICOM_PE_DIRECT           vtkDICOMTag(0x0018, 0x1312)// COL or ROW?
    //DICOM_DIFF_DIRECT_1       vtkDICOMTag(0x0019, 0x10bb)//if COL, i; if RWO, j
    //DICOM_DIFF_DIRECT_2       vtkDICOMTag(0x0019, 0x10bc)//if COL, j; if RWO, i
    //DICOM_DIFF_DIRECT_3       vtkDICOMTag(0x0019, 0x10bd)// slice; k
};

void DicomHelper::DicomInfo()
{
	DicomReader->UpdateInformation();
	vtkDICOMMetaData* meta = vtkDICOMMetaData::New();
	meta = DicomReader->GetMetaData();

	vtkIntArray* fileIndexArray = vtkIntArray::New();
	fileIndexArray = DicomReader->GetFileIndexArray();

	vtkIntArray* frameIndexArray = vtkIntArray::New();
	frameIndexArray = DicomReader->GetFrameIndexArray();
	//cout << "number of components:" << frameIndexArray->GetNumberOfComponents() << endl;

	//GetAttributeValue(0, 0, dicomTag) is applied for Enhanced DICOM
	if (meta->GetAttributeValue(0, 0, ScaleSlop).IsValid())
	{
		scaleSlope = meta->GetAttributeValue(0, 0, ScaleSlop).AsFloat();
	}

	if (meta->GetAttributeValue(0, 0, ScaleInterpcept).IsValid())
	{
		scaleIntercept = meta->GetAttributeValue(0, 0, ScaleInterpcept).AsFloat();
	}

	//if (meta->HasAttribute(DiffusionBValues))
	if (meta->GetAttributeValue(0, 0, DiffusionBValues).IsValid())
	{
		char* pEnd;
		int bval_basic = 1000000000;

		int former_bValue = -1;
		//replace with below to fix possible multiple b value multiple direction error
		for (int i = 0; i < numberOfComponents; i++) //i = i + numberOfGradDirection
		{
			int index = fileIndexArray->GetComponent(0, i);
			int framIndex = frameIndexArray->GetComponent(0, i);
			int b_value;

			if ((manuFacturer[0] == 'P') || (manuFacturer[0] == 'p'))
				//Philips Vendor;
				b_value = meta->GetAttributeValue(index, framIndex, DiffusionBValues).AsInt();
			if ((manuFacturer[0] == 'G') || (manuFacturer[0] == 'g'))
			{
				//GE Vendor;
				std::string slopInt4 = meta->GetAttributeValue(index, DiffusionBValues).AsString();
				std::string bval_string = slopInt4.substr(0, slopInt4.length() - 6);
				b_value = static_cast<unsigned int>(strtod(bval_string.c_str(), &pEnd));
				//in case some GE bvalue is: 1000000050 which is b = 50;
				if (b_value > bval_basic) b_value = b_value - bval_basic;
			}

			if (b_value != former_bValue)
			{
				BvalueList.push_back((float)b_value);
				former_bValue = b_value;
			}
		}
		numberOfBValue = BvalueList.size();
		numberOfGradDirection = numberOfBValue > 1 ? (numberOfComponents - 1) / (numberOfBValue - 1) : (numberOfComponents - 1);
		cout << "number of B value:" << numberOfBValue << endl;

		if (numberOfGradDirection > 6)  tensorComputationPossible = true;
	}


	if (tensorComputationPossible)
	{

		if (meta->GetAttributeValue(0, 0, ImagingOrientation).IsValid())
		{
			//std::string image_orientation
			image_orientation = meta->GetAttributeValue(0, 0, ImagingOrientation).GetCharData();
			if (!image_orientation) image_orientation = "TRANSVERSAL";
			cout << "image orientation " << *image_orientation << endl;

		}

		if (meta->GetAttributeValue(0, 0, ImagingDirectionRL).IsValid()
			&& meta->GetAttributeValue(0, 0, ImagingDirectionFH).IsValid()
			&& meta->GetAttributeValue(0, 0, ImagingDirectionAP).IsValid())
		{
			ang[0] = meta->GetAttributeValue(0, 0, ImagingDirectionRL).AsDouble();
			ang[1] = meta->GetAttributeValue(0, 0, ImagingDirectionAP).AsDouble();
			ang[2] = meta->GetAttributeValue(0, 0, ImagingDirectionFH).AsDouble();
			cout << "angle: " << ang[0] << " " << ang[1] << endl;
 		}
		GetSliceToPatMatrix();

		if (meta->GetAttributeValue(0, 0, DiffusionDirectionAP).IsValid()
			&& meta->GetAttributeValue(0, 0, DiffusionDirectionFH).IsValid()
			&& meta->GetAttributeValue(0, 0, DiffusionDirectionRL).IsValid()
			)
		{
			gradientDirection = GradientDirectionContainerType::New();
			gradientDirection->Reserve(numberOfGradDirection);

			GradientDirectionContainerType::Iterator directionIterator = gradientDirection->Begin();
			for (int cmp = 1; cmp <= numberOfGradDirection; cmp++)
			{
				// b0 is the first component, gradientdirection stars from the second component.
				int index = fileIndexArray->GetComponent(0, cmp);
				int framIndex = frameIndexArray->GetComponent(0, cmp);
				float AP_direction = meta->GetAttributeValue(index, framIndex, DiffusionDirectionAP).AsFloat();
				float FH_direction = meta->GetAttributeValue(index, framIndex, DiffusionDirectionFH).AsFloat();
				float RL_direction = meta->GetAttributeValue(index, framIndex, DiffusionDirectionRL).AsFloat();
				GradientDirectionType direction;
				direction[0] = RL_direction;
				direction[1] = AP_direction;
				direction[2] = FH_direction;
				cout << "diffusion direction: " << direction[0] << "  " << direction[1] << endl;
				float sum = abs(direction[0]) + abs(direction[1]) + abs(direction[2]);
				if (sum < exp(-100))
				{
					IsoImageLabel = cmp; //the index in the component vector
				}
				else
				{
					directionIterator->Value() = direction;
					++directionIterator;
				}
			}

		}
		if (IsoImageLabel > -1)
		{
			numberOfGradDirection = numberOfGradDirection - 1;
			numberOfComponents -= 1;
		}
		CalculateFinalHMatrix();
	}
};

void DicomHelper::CalculateFinalHMatrix()
{
	finalH.set_size(numberOfGradDirection, 6);
	GradientDirectionContainerType::Iterator directionIterator = gradientDirection->Begin();
	for (int row = 0; row < numberOfGradDirection; row++)
	{
		GradientDirectionType direction;
		direction = directionIterator->Value();
		vnl_vector<double> newDirection(3);
		vnl_matrix<double> slice2PatInverse = vnl_matrix_inverse<double>(slice2PatMatrix);
		
		for (int i = 0; i < 3; i++)
		{
			double temp;
			temp = direction[0] * slice2PatInverse(i, 0) + direction[1] * slice2PatInverse(i, 1) + direction[2] * slice2PatInverse(i, 2);
			if (abs(temp) < exp(-8))
				newDirection(i) = 0.0;
			else
				newDirection(i) = temp;
			
		}
		finalH(row, 0) = newDirection[0] * newDirection[0];
		finalH(row, 1) = newDirection[1] * newDirection[1];
		finalH(row, 2) = newDirection[2] * newDirection[2];
		finalH(row, 3) = 2 * newDirection[0] * newDirection[1];
		finalH(row, 4) = 2 * newDirection[0] * newDirection[2];
		finalH(row, 5) = 2 * newDirection[1] * newDirection[2];
		directionIterator++;

		//cout << "newdirection: " << newDirection << endl;
	}
};

void DicomHelper::GetSliceToPatMatrix()
{
	vnl_matrix<double> slice_apat(3,3);
	if (strcmp(image_orientation, "TRANSVERSAL"))
	{
		slice_apat(0, 0) = 0.0; slice_apat(0, 1) = -1.0; slice_apat(0, 2) = 0.0;
		slice_apat(1, 0) = -1.0; slice_apat(1, 1) = 0.0; slice_apat(1, 2) = 0.0;
		slice_apat(2, 0) = 0.0; slice_apat(2, 1) =  0.0; slice_apat(2, 2) = 1.0;
		//cout << " transeveral " << endl;
	}
	else if (strcmp(image_orientation, "CORONAL"))
	{
		slice_apat(0, 0) = 0.0; slice_apat(0, 1) = 0.0; slice_apat(0, 2) = 1.0;
		slice_apat(1, 0) = -1.0; slice_apat(1, 1) = 0.0; slice_apat(1, 2) = 0.0;
		slice_apat(2, 0) = 0.0; slice_apat(2, 1) = 1.0; slice_apat(2, 2) = 0.0;
	}
	else if (strcmp(image_orientation, "SAGITTAL"))
	{
		slice_apat(0, 0) = 0.0; slice_apat(0, 1) = 0.0; slice_apat(0, 2) = 1.0;
		slice_apat(1, 0) = 0.0; slice_apat(1, 1) = -1.0; slice_apat(1, 2) = 0.0;
		slice_apat(2, 0) = -1.0; slice_apat(2, 1) = 0.0; slice_apat(2, 2) = 0.0;
	}

	vnl_matrix<double> apat_pat(3, 3);
	double	sx;  double	sy;  double	sz;
	double	cx;  double	cy;  double	cz;

	//if (right_handed)
	//{
	//	SGMAT_invert_vec(ang, &ang);
	//}

	sx = sin(-ang[0] * RAD); sy = sin(-ang[1] * RAD); sz = sin(-ang[2] * RAD);
	cx = cos(-ang[0] * RAD); cy = cos(-ang[1] * RAD); cz = cos(-ang[2] * RAD);
	
	apat_pat(0,0) = cy * cz;
	apat_pat(0,1) = -sz * cx + sx * sy * cz;
	apat_pat(0,2) = sx * sz + sy * cx * cz;

	apat_pat(1, 0) = sz * cy;
	apat_pat(1, 1) = cx * cz + sx * sy * sz;
	apat_pat(1, 2) = -sx * cz + sy * sz * cx;

	apat_pat(2, 0) = -sy;
	apat_pat(2, 1) = sx * cy;
	apat_pat(2, 2) = cx * cy;
	//cout << "apat to pat matrix " << endl;
	//cout << apat_pat << endl;

	slice2PatMatrix.set_size(3, 3);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3;j++)
		{
			slice2PatMatrix(i, j) = apat_pat(0, j)*slice_apat(i, 0) + apat_pat(1, j)*slice_apat(i, 1) + apat_pat(2, j)*slice_apat(i, 2);
		}
		
	}
	vnl_matrix<double> multiplier(3, 3);
	multiplier(0, 0) = 0.0; multiplier(0, 1) = -1.0; multiplier(0, 2) = 0.0;
	multiplier(1, 0) = -1.0; multiplier(1, 1) = 0.0; multiplier(1, 2) = 0.0;
	multiplier(2, 0) = 0.0; multiplier(2, 1) = 0.0; multiplier(2, 2) = 1.0;

	slice2PatMatrix = slice2PatMatrix.transpose()*multiplier;
};

vtkDICOMValue DicomHelper::GetAttributeValue(vtkDICOMMetaData* metaData, vtkDICOMTag tagValue,
	int fileIndex, int frameIndex)
{
	cout << "is enhanced: " << isEnhanced << endl;
	if (isEnhanced)
		return metaData->GetAttributeValue(fileIndex, frameIndex, tagValue);
	else
		return metaData->GetAttributeValue(fileIndex, tagValue);

}


