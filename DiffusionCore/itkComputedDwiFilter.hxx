#ifndef _itkComputedDwiFilter_hxx_
#define _itkComputedDwiFilter_hxx_

#include "itkComputedDwiFilter.h"

namespace itk
{
	template< typename TInputPixelType, typename TOutputPixelType >
	ComputedDwiFilter< TInputPixelType, TOutputPixelType >
	::ComputedDwiFilter()
	{
		m_NumOfDiffDirections = 1;
		m_InputImage = ITK_NULLPTR;
		m_OutputImage = ITK_NULLPTR;
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
	ComputedDwiFilter< TInputPixelType, TOutputPixelType >
	::BeforeThreadedGenerateData()
	{
		m_InputImage = this->GetInput();
		m_OutputImage = this->GetOutput();
		m_OutputImage->SetVectorLength(m_NumOfDiffDirections);
		m_OutputImage->Allocate();//Crutial.
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
	ComputedDwiFilter< TInputPixelType, TOutputPixelType >
	::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId)
	{
		ConstInputIteratorType inputIt(m_InputImage, outputRegionForThread);
		OutputIteratorType outputIt(m_OutputImage, outputRegionForThread);

		/*std::cout << "output Largest Region Index: " << m_OutputImage->GetLargestPossibleRegion().GetIndex() << std::endl;
		std::cout << "output Largest Region size: " << m_OutputImage->GetLargestPossibleRegion().GetSize() << std::endl;
		std::cout <<  "NumOfComponents: " << m_NumOfComponents << std::endl;
		std::cout << "NumOfDiffBValues: " << m_NumOfDiffBValues << std::endl;
		std::cout << "NumOfDiffDirections: " << m_NumOfDiffDirections << std::endl;*/

		VariableLengthVectorType outputResultVector;
		outputResultVector.SetSize(m_NumOfDiffDirections);

		inputIt.GoToBegin();
		outputIt.GoToBegin();
		while (!inputIt.IsAtEnd())
		{
			for (int direction = 0; direction < m_NumOfDiffDirections; direction++)
			{				
				outputResultVector[direction] = exp(-inputIt.Get()[2 * direction + 1] * m_ComputedBValue + inputIt.Get()[2 * direction]);
			}
			outputIt.Set(outputResultVector);
			++inputIt;
			++outputIt;
		}
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
	ComputedDwiFilter< TInputPixelType, TOutputPixelType >
	::PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os, indent);
	}
} // end namespace itk
#endif

////////////////////cDwiTest and debug
// 
/*typedef itk::VectorImage<float, SrcImgDimension> VectorImageType;
typedef itk::ImageRegionConstIterator <VectorImageType> ConstSrcImageIteratorType;
typedef itk::ImageRegionIterator <VectorImageType> CDwiIteratorType;
typedef itk::VariableLengthVector <float> VariableLengthVectorType;

VectorImageType::Pointer inputVectorImage = adcMap->GetOutput();
VectorImageType::Pointer cDwiImage = VectorImageType::New();
cDwiImage->SetRegions(inputVectorImage->GetLargestPossibleRegion());
cDwiImage->CopyInformation(inputVectorImage);
cDwiImage->SetVectorLength(numOfDiffDirections);
cDwiImage->Allocate();

ConstSrcImageIteratorType inputImageIt(inputVectorImage, inputVectorImage->GetLargestPossibleRegion());
CDwiIteratorType cDwiIt(cDwiImage, cDwiImage->GetLargestPossibleRegion());

VariableLengthVectorType cDwiResultVector;
cDwiResultVector.SetSize(numOfDiffDirections);

inputImageIt.GoToBegin();
cDwiIt.GoToBegin();
while (!inputImageIt.IsAtEnd())
{
	for (int direction = 0; direction < numOfDiffDirections; direction++)
	{
		inputImageIt.Get();
		cDwiResultVector[direction] = -inputImageIt.Get()[2 * direction + 1] * computedBValue + inputImageIt.Get()[2 * direction];
	}
	cDwiIt.Set(cDwiResultVector);
	++inputImageIt;
	++cDwiIt;
}*/