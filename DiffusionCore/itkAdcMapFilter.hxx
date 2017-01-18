#ifndef _itkAdcMapFilter_hxx_
#define _itkAdcMapFilter_hxx_

#include "itkAdcMapFilter.h"
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_matrix_inverse.h"
#include "vnl/vnl_vector.h"

namespace itk
{
	template< typename TInputPixelType, typename TOutputPixelType >
	AdcMapFilter< TInputPixelType, TOutputPixelType >
		::AdcMapFilter()
	{
		m_NumOfDiffDirections = 1;
		m_NumOfDiffBValues = 2;
		m_InputImage = ITK_NULLPTR;
		m_OutputImage = ITK_NULLPTR;

		SetNumberOfThreads(1);
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
		AdcMapFilter< TInputPixelType, TOutputPixelType >
		::BeforeThreadedGenerateData()
	{
		//m_NumOfDiffDirections = ; //use set
		m_NumOfDiffBValues = m_BValueList.size();
		m_NumOfComponents = this->GetInput()->GetVectorLength();
		if (m_NumOfDiffBValues > 1)
		{
			m_NumOfDiffDirections = (m_NumOfComponents - 1) / (m_NumOfDiffBValues - 1);
		}
		m_InputImage = this->GetInput();
		m_OutputImage = this->GetOutput();
		m_OutputImage->SetVectorLength(2 * m_NumOfDiffDirections);
		m_OutputImage->Allocate();//Crutial.
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
		AdcMapFilter< TInputPixelType, TOutputPixelType >
		::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId)
	{
		ConstInputIteratorType inputIt(m_InputImage, outputRegionForThread);
		OutputIteratorType outputIt(m_OutputImage, outputRegionForThread);

		VariableLengthVectorType adcResultVector;
		adcResultVector.SetSize(2 * m_NumOfDiffDirections);//[computedB0i, ADCi], i = 1,2,...numOfDiffDirections.	

		vnl_matrix<float> x(m_NumOfDiffBValues, 2);
		for (int row = 0; row < m_NumOfDiffBValues; row++)
		{
			x(row, 0) = 1;
			x(row, 1) = m_BValueList.at(row);
		}

		vnl_matrix <float> leftSide = vnl_matrix_inverse <float>(x.transpose()*x) * x.transpose();
		vnl_vector<float> rightSide(m_NumOfDiffBValues);//need to extract bvalue index if numOfDirections > 1
		vnl_vector <float> adcVectorPerDirection(2);

		inputIt.GoToBegin();
		outputIt.GoToBegin();
		while (!inputIt.IsAtEnd())
		{
			// skip mask value and zero value
			bool skip = true;
			for (unsigned int i = 0; i < m_NumOfComponents; i++)
			{
				if (inputIt.Get()[i] != 0)
				{
					skip = false;
					break;
				}
			}

			if (skip)
			{
				adcResultVector.Fill(0);
			}
			else
			{
				//handle b0 image seperately: crutial!
				rightSide[0] = inputIt.Get()[0];
				rightSide[0] = rightSide[0] > 1 ? rightSide[0] : 1;//values smaller than 1 will be negative after log, replace with 1
				rightSide[0] = std::log(rightSide[0]);

				for (int direction = 0; direction < m_NumOfDiffDirections; direction++)
				{
					for (int bValue = 1; bValue < m_NumOfDiffBValues; bValue++)
					{
						rightSide[bValue] = inputIt.Get()[(bValue - 1)*m_NumOfDiffDirections + direction + 1];
						rightSide[bValue] = rightSide[bValue] > 1 ? rightSide[bValue] : 1;//values smaller than 1 will be negative after log, replace with 1
						rightSide[bValue] = std::log(rightSide[bValue]);
					}
					adcVectorPerDirection = leftSide*rightSide;

					adcVectorPerDirection[1] = fmax(0.0, -adcVectorPerDirection[1]);
					adcResultVector[2 * direction] = adcVectorPerDirection[0];
					adcResultVector[2 * direction + 1] = adcVectorPerDirection[1];
				}
			}
			outputIt.Set(adcResultVector);
			++outputIt;
			++inputIt;
		}
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
		AdcMapFilter< TInputPixelType, TOutputPixelType >
		::PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os, indent);
		//os << indent << "NumOfComponents: " << m_NumOfComponents << std::endl;
	}

} // end namespace itk

#endif

/*///////////////////////AdcMapFilter Debug version
//We're Assuming input data order: b0, b11, b12...b13.... bij....bn1, bn2, ...bnm (n = bvalues, m = directions)
//Sort Image required

typedef itk::VectorImage<float, SrcImgDimension> VectorImageType;
typedef itk::ImageRegionConstIterator <VectorImageType> ConstSrcImageIteratorType;
typedef itk::ImageRegionIterator <VectorImageType> AdcIteratorType;
typedef itk::VariableLengthVector <float> VariableLengthVectorType;

VectorImageType::Pointer inputVectorImage = imageToVectorImageFilter->GetOutput();
VectorImageType::Pointer adcImage = VectorImageType::New();
adcImage->SetRegions(inputVectorImage->GetLargestPossibleRegion());
//std::cout << "Requested Region Index: " << adcImage->GetRequestedRegion().GetIndex() << std::endl;
//std::cout << "Reqeusted Region size: " << adcImage->GetRequestedRegion().GetSize() << std::endl;
//std::cout << "Largest Region Index: " << adcImage->GetLargestPossibleRegion().GetIndex() << std::endl;
//std::cout << "Largest Region size: " << adcImage->GetLargestPossibleRegion().GetSize() << std::endl;
adcImage->SetSpacing(inputVectorImage->GetSpacing());
adcImage->SetOrigin(inputVectorImage->GetOrigin());
adcImage->SetDirection(inputVectorImage->GetDirection());
//adcImage->CopyInformation(inputVectorImage);
adcImage->SetVectorLength(2*numOfDiffDirections);
adcImage->Allocate();

ConstSrcImageIteratorType inputImageIt(inputVectorImage, inputVectorImage->GetLargestPossibleRegion());
AdcIteratorType adcIt(adcImage, adcImage->GetLargestPossibleRegion());

VariableLengthVectorType adcResultVector;
adcResultVector.SetSize(2*numOfDiffDirections);//[computedB0i, ADCi], i = 1,2,...numOfDiffDirections.

inputImageIt.GoToBegin();
adcIt.GoToBegin();

vnl_matrix<float> x(numOfDiffBValues, 2);
for (int row = 0; row < numOfDiffBValues; row++)
{
x(row, 0) = 1;
x(row, 1) = bValueList.at(row);
}

vnl_matrix <float> leftSide = vnl_matrix_inverse <float>(x.transpose()*x) * x.transpose();
vnl_vector<float> rightSide(numOfDiffBValues);//need to extract bvalue index if numOfDirections > 1
//rightSide.fill(0);
vnl_vector <float> adcVectorPerDirection(2);

while (!inputImageIt.IsAtEnd())
{
//handle b0 image seperately: crutial!
rightSide[0] = inputImageIt.Get()[0];
rightSide[0] > 1 ? rightSide[0] : 1;
rightSide[0] = std::log(rightSide[0]);
for (int direction = 0; direction < numOfDiffDirections; direction++)
{
for (int bValue = 1; bValue < numOfDiffBValues; bValue++)
{
rightSide[bValue] = inputImageIt.Get()[(bValue-1)*numOfDiffDirections + direction + 1];
rightSide[bValue] > 1 ? rightSide[bValue] : 1;//values smaller than 1 will be negative after log, replace with 1
rightSide[bValue] = std::log(rightSide[bValue]);
}
adcVectorPerDirection = leftSide*rightSide;

if (isnan(adcVectorPerDirection[1]) || isinf(adcVectorPerDirection[1]))
{
adcVectorPerDirection[0] = 0;
adcVectorPerDirection[1] = 0;
}
adcVectorPerDirection[1] = fmax(0.0, -adcVectorPerDirection[1]);
adcResultVector[2 * direction] = adcVectorPerDirection[0];
adcResultVector[2 * direction + 1] = adcVectorPerDirection[1];
}

//cDwiPixel = -adcResultPixel[1] * computedBValue + adcResultPixel[0];
//cout << "adc ResultPixel: " << adcResultPixel << endl;

adcIt.Set(adcResultVector);
//cDwiImageIt.Set(cDwiPixel);
++adcIt;
//++cDwiImageIt;//do not foget!!!
++inputImageIt;
}
//std::cout << "adcImage components:" << adcImage->GetVectorLength() << std::endl;
//std::cout << "After adcImage rightSide:" << rightSide << std::endl;*/