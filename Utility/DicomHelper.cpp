#include <DicomHelper.h>
#include <algorithm>
//#include <vtkDICOMMetaData.h>
#include <vtkDICOMValue.h>
#include <vtkMedicalImageProperties.h>
#include <vtkDICOMApplyRescale.h>
#include <vtkImageData.h>
#include <vtkStringArray.h>
#include <vtkIntArray.h>
//#include "qdebug.h"

bool cmp(std::pair<float, int> p1, std::pair<float, int> p2)
{
	if (p1.first < p2.first) return 1;
	return 0;

}

DicomHelper::DicomHelper(vtkStringArray* Files)
{
	// dicomreader (input: filenames, TimeAsVectorOn for dynamic data)
	DicomReader = vtkSmartPointer<vtkDICOMReader>::New();
	DicomReader->SetFileNames(Files);
	DicomReader->TimeAsVectorOn();
	DicomReader->AutoRescaleOff();
	DicomReader->Update();
	//if (Files->GetSize() == 1) isEnhanced = 1;

	// Dicom meta data: includs the dicom info
	DicomReader->UpdateInformation();
	vtkDICOMMetaData* DicomMetaData = vtkDICOMMetaData::New();
	DicomMetaData = DicomReader->GetMetaData();

	imageMedicalProperties = vtkSmartPointer<vtkMedicalImageProperties>::New();
	imageMedicalProperties = DicomReader->GetMedicalImageProperties();
	manuFacturer = imageMedicalProperties->GetManufacturer();
	if ((manuFacturer[0] == 'G') || (manuFacturer[0] == 'g'))
	{
		//change the dicom tag value to GE standars; default is Philips'
		this->DicomTagForGE();
	}

	// Get the data type: DIFFUSSION or DYNAMIC? (only for these two kinds data now)
	imageDataType = "NONE";
	std::string diffusionChar = DicomMetaData->GetAttributeValue(IsDiffusionSeries).AsString();
	std::string dynamicChar = DicomMetaData->GetAttributeValue(IsDynamicSeries).AsString();
	numberOfDynamic = DicomMetaData->GetAttributeValue(NrOfDynamics).AsInt();
	if (diffusionChar[0] == 'Y' || diffusionChar[0] == 'y') 
		imageDataType = "DIFFUSION";
	if ((dynamicChar[0] == 'Y'||dynamicChar[0] == 'y') 
		&& numberOfDynamic > 5)
		imageDataType = "PERFUSION";
	cout << "[DICOM DATA TYPE]: " << imageDataType.c_str() << endl;

	//general parametes;
	scaleSlope = 1;
	scaleIntercept = 0;
	/*for diffusion, the number of components is the diffusion encoding numbers: includs b value and direction;
	  for perfusion, the number of components is the number of dynamics.
	*/
	numberOfComponents = DicomReader->GetNumberOfScalarComponents(); 
	DicomReader->GetOutput()->GetDimensions(imageDimensions);
	isEnhanced = 1;


	//Diffusion parameters;
	numberOfGradDirection = 1;
	numberOfBValue = 1;
	ang.Fill(0.0);
	//perfusion parameters;

	//the image data;
	imageData = vtkSmartPointer<vtkImageData>::New();
	imageData = this->DicomReader->GetOutput();

	// get the related parameters;
	this->DicomInfo(DicomMetaData);

	//if (imageDataType == "DIFFUSION" && numberOfBValue > 2)
	//{
	//	//only for Diffusion images with number of b values larger than 2(includes b = 0)
	//	this->SortingSourceImage(imageData);
	//}
};

void DicomHelper::DicomTagForGE()
{
	DiffusionBValues = vtkDICOMTag(0x0043, 0x1039);//    vtkDICOMTag(0x0043, 0x1039)

    //DICOM_PE_DIRECT           vtkDICOMTag(0x0018, 0x1312)// COL or ROW?
    //DICOM_DIFF_DIRECT_1       vtkDICOMTag(0x0019, 0x10bb)//if COL, i; if RWO, j
    //DICOM_DIFF_DIRECT_2       vtkDICOMTag(0x0019, 0x10bc)//if COL, j; if RWO, i
    //DICOM_DIFF_DIRECT_3       vtkDICOMTag(0x0019, 0x10bd)// slice; k
};

void DicomHelper::DicomInfo(vtkDICOMMetaData* metaData)
{
	vtkIntArray* fileIndexArray = vtkIntArray::New();
	fileIndexArray = DicomReader->GetFileIndexArray();
	vtkIntArray* frameIndexArray = vtkIntArray::New();
	frameIndexArray = DicomReader->GetFrameIndexArray();
	if (metaData->GetAttributeValue(0, 0, ScaleSlop).IsValid())
	{
		scaleSlope = metaData->GetAttributeValue(0, 0, ScaleSlop).AsFloat();
		cout << "scale slope: " << scaleSlope << endl;
	}
	if (metaData->GetAttributeValue(0, 0, ScaleInterpcept).IsValid())
	{
		scaleIntercept = metaData->GetAttributeValue(0, 0, ScaleInterpcept).AsFloat();
		cout << "scale intercept: " << scaleIntercept << endl;
	}

	//Get diffusion paramters;
	if (imageDataType == "DIFFUSION")
		DiffusionInfo(metaData, fileIndexArray, frameIndexArray);

	if (imageDataType == "PERFUSION")
		PerfusionInfo(metaData, fileIndexArray, frameIndexArray);
};

void DicomHelper::DiffusionInfo(vtkDICOMMetaData* metaData, 
	vtkIntArray* fileIndexArray, vtkIntArray* frameIndexArray)
{
	// b values
	if (metaData->GetAttributeValue(0, 0, DiffusionBValues).IsValid())
	{
		char* pEnd;
		int bval_basic = 1000000000;
		cout << "here 3" << endl;
		int former_bValue = -1;
		//replace with below to fix possible multiple b value multiple direction error
		for (int i = 0; i < numberOfComponents; i++) //i = i + numberOfGradDirection
		{
			int index = fileIndexArray->GetComponent(0, i);
			int framIndex = frameIndexArray->GetComponent(0, i);
			int b_value;

			if ((manuFacturer[0] == 'P') || (manuFacturer[0] == 'p'))
				//Philips Vendor;
				b_value = metaData->GetAttributeValue(index, framIndex, DiffusionBValues).AsInt();
			cout << "b_value:" << b_value << endl;
			if ((manuFacturer[0] == 'G') || (manuFacturer[0] == 'g'))
			{
				//GE Vendor;
				std::string slopInt4 = metaData->GetAttributeValue(index, DiffusionBValues).AsString();
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
		if (numberOfGradDirection > 6)  tensorComputationPossible = true;
	}

	//if tensor calculation need, just DTI now, further for DKI
	if (tensorComputationPossible)
	{

		if (metaData->GetAttributeValue(0, 0, ImagingOrientation).IsValid())
		{
			//std::string image_orientation
			image_orientation = metaData->GetAttributeValue(0, 0, ImagingOrientation).GetCharData();
			if (!image_orientation) image_orientation = "TRANSVERSAL";
			cout << "image orientation " << *image_orientation << endl;

		}

		if (metaData->GetAttributeValue(0, 0, ImagingDirectionRL).IsValid()
			&& metaData->GetAttributeValue(0, 0, ImagingDirectionFH).IsValid()
			&& metaData->GetAttributeValue(0, 0, ImagingDirectionAP).IsValid())
		{
			ang[0] = metaData->GetAttributeValue(0, 0, ImagingDirectionRL).AsDouble();
			ang[1] = metaData->GetAttributeValue(0, 0, ImagingDirectionAP).AsDouble();
			ang[2] = metaData->GetAttributeValue(0, 0, ImagingDirectionFH).AsDouble();
			cout << "angle: " << ang[0] << " " << ang[1] << endl;
		}
		GetSliceToPatMatrix();

		if (metaData->GetAttributeValue(0, 0, DiffusionDirectionAP).IsValid()
			&& metaData->GetAttributeValue(0, 0, DiffusionDirectionFH).IsValid()
			&& metaData->GetAttributeValue(0, 0, DiffusionDirectionRL).IsValid()
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
				float AP_direction = metaData->GetAttributeValue(index, framIndex, DiffusionDirectionAP).AsFloat();
				float FH_direction = metaData->GetAttributeValue(index, framIndex, DiffusionDirectionFH).AsFloat();
				float RL_direction = metaData->GetAttributeValue(index, framIndex, DiffusionDirectionRL).AsFloat();
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

	int numOfGradDir = this->numberOfGradDirection;
	if (this->IsoImageLabel > -1) numOfGradDir += 1;
	this->SortingSourceImage(imageData, this->BvalueList, numOfGradDir);


};

void DicomHelper::PerfusionInfo(vtkDICOMMetaData* metaData, 
	vtkIntArray* fileIndexArray, vtkIntArray* frameIndexArray)
{
	for (int i = 0; i < numberOfComponents; i++) //i = i + numberOfGradDirection
	{
		int index = fileIndexArray->GetComponent(0, i);
		int framIndex = frameIndexArray->GetComponent(0, i);
		if (metaData->GetAttributeValue(index, framIndex, DynamicTime).IsValid())
		{
			float time = metaData->GetAttributeValue(index, framIndex, DynamicTime).AsFloat();
			dynamicTime.push_back(time);
			cout << "time: " << time << endl;
		}

	}

	this->SortingSourceImage(imageData, this->dynamicTime, 1);//the third parameter usded for diffusion gradient direction number
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

void DicomHelper::SortingSourceImage(vtkImageData* sourceData, std::vector<float> basedVector, int secondOrderNumber)
{
	//sorting b value list from small to larger using vector Pair sort.
	//which can get the sorted index array. it can be used for the 
	//source image sorting.
	std::vector< std::pair<float, int> > vectorPair;
	int length = basedVector.size();
	std::vector<int> paraOrderIndex(length);

	for (int i = 0; i < length; i++)
	{
		paraOrderIndex[i] = i;
		vectorPair.push_back(std::make_pair(basedVector[i], paraOrderIndex[i]));
	}
	std::stable_sort(vectorPair.begin(), vectorPair.end(), cmp);

	for (int i = 0; i < length; i++)
	{
		cout << "value:" << vectorPair[i].first << "index:" << vectorPair[i].second << endl;
		basedVector[i] = vectorPair[i].first;
		paraOrderIndex[i] = vectorPair[i].second;
	}

	// Sorting the output data based on the b Value order
	int* dims = this->imageDimensions;
	for (int z = 0; z<dims[2]; z++)
	{
		for (int y = 0; y<dims[1]; y++)
		{
			for (int x = 0; x<dims[0]; x++)
			{

				unsigned short *dicomPtr = static_cast<unsigned short *>(this->DicomReader->GetOutput()->GetScalarPointer(x, y, z));
				unsigned short *sourcePtr = static_cast<unsigned short *>(sourceData->GetScalarPointer(x, y, z));
				sourcePtr[0] = dicomPtr[paraOrderIndex[0] * secondOrderNumber];

				//sorting based on the b value order
				int sourceCmpIndex = 1;
				for (int cmp = 1; cmp < numberOfComponents; cmp++)
				{
					//get the b value index for current component
					int paraIndex = (cmp - 1) / secondOrderNumber + 1;
					//get the grad direction indec for current component
					int secondOrderIndex = cmp - 1 - secondOrderNumber*(paraIndex - 1);
					//calculate the corrsponding component index in the dicom output data
					int dicomCmpIndex;
					if (paraOrderIndex[paraIndex] < paraOrderIndex[0])
						dicomCmpIndex = (paraOrderIndex[paraIndex]) * secondOrderNumber + secondOrderIndex;
					else
						dicomCmpIndex = (paraOrderIndex[paraIndex] - 1) * secondOrderNumber + secondOrderIndex + 1;

					// remove the Isotropic direction for DTI
					if (this->IsoImageLabel != dicomCmpIndex)
						sourcePtr[sourceCmpIndex++] = dicomPtr[dicomCmpIndex];
				}
			}
		}
	}
}


