#ifndef _itkComputedEadcFilter_hxx_
#define _itkComputedEadcFilter_hxx_

#include "itkComputedEadcFilter.h"

namespace itk
{
	template< typename TInputPixelType, typename TOutputPixelType >
	ComputedEadcFilter< TInputPixelType, TOutputPixelType >
	::ComputedEadcFilter()
	{
		m_NumOfDiffDirections = 1;
		m_InputImage = ITK_NULLPTR;
		m_OutputImage = ITK_NULLPTR;
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
	ComputedEadcFilter< TInputPixelType, TOutputPixelType >
	::BeforeThreadedGenerateData()
	{
		m_InputImage = this->GetInput();
		m_OutputImage = this->GetOutput();
		m_OutputImage->SetVectorLength(m_NumOfDiffDirections);
		m_OutputImage->Allocate();//Crutial.
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
	ComputedEadcFilter< TInputPixelType, TOutputPixelType >
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
			// skip zero value
			bool skip = true;
			for (unsigned int i = 0; i < 2*m_NumOfDiffDirections; i++)
			{
				if (inputIt.Get()[i] != 0)
				{
					skip = false;
					break;
				}
			}

			if (skip)
			{
				outputResultVector.Fill(0);
			}
			else
			{
				for (int direction = 0; direction < m_NumOfDiffDirections; direction++)
				{
					outputResultVector[direction] = exp(-inputIt.Get()[2 * direction + 1] * m_EadcBValue);
				}
			}
			outputIt.Set(outputResultVector);
			++inputIt;
			++outputIt;
		}
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
	ComputedEadcFilter< TInputPixelType, TOutputPixelType >
	::PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os, indent);
	}
} // end namespace itk
#endif
