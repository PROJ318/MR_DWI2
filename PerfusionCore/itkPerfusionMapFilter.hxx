#ifndef _itkPerfusionMapFilter_hxx_
#define _itkPerfusionMapFilter_hxx_

#include "itkPerfusionMapFilter.h"
#include <perfusionAlgorithm.h>
#include <iostream>

namespace itk
{
	template< typename TInputPixelType, typename TOutputPixelType >
	PerfusionMapFilter< TInputPixelType, TOutputPixelType >
		::PerfusionMapFilter()
	{
		m_NumOfDynamics = 1;
		m_InputImage = ITK_NULLPTR;
		m_OutputImage = ITK_NULLPTR;

		SetNumberOfThreads(1);
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
		PerfusionMapFilter< TInputPixelType, TOutputPixelType >
		::BeforeThreadedGenerateData()
	{
		//m_NumOfComponents = this->GetInput()->GetVectorLength();
		m_NumOfDynamics = this->GetInput()->GetVectorLength();
		m_InputImage = this->GetInput();
		m_OutputImage = this->GetOutput();
		/*the following maps can be calculated:
		Relative enhance; max relative enhance, time to Peak, Wash out Rate, 
		Area under the curve, max enhance, T0, Wash in Rate, Brevity of enhance*/
		m_OutputImage->SetVectorLength(9); 
		m_OutputImage->Allocate();//Crutial.
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
		PerfusionMapFilter< TInputPixelType, TOutputPixelType >
		::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId)
	{
		ConstInputIteratorType inputIt(m_InputImage, outputRegionForThread);
		OutputIteratorType outputIt(m_OutputImage, outputRegionForThread);

		VariableLengthVectorType perfusionResultVector;
		perfusionResultVector.SetSize(9);

		perfusionAlgorithm perfusionCalc = perfusionAlgorithm(m_NumOfDynamics);
	 
		cout << "number of components: " << m_NumOfDynamics << endl;
		std::vector<float> sourceImage(m_NumOfDynamics);
		inputIt.GoToBegin();
		outputIt.GoToBegin();
		while (!inputIt.IsAtEnd())
		{
			for (int i = 0; i < m_NumOfDynamics - 1; i++)
			{
				//cout << "input value" << inputIt.Get()[i] << endl;
				sourceImage[i] = inputIt.Get()[i];
			}

			perfusionCalc.SetDynamicPixels(sourceImage);
			perfusionCalc.SetDynamicTime(m_imageTimer);
			perfusionCalc.GenerateMaps();

			perfusionResultVector = perfusionCalc.perfusionMaps;
			outputIt.Set(perfusionResultVector);
			++outputIt;
			++inputIt;
		}
	}

	//template< typename TInputPixelType, typename TOutputPixelType >
	//void
	//	PerfusionMapFilter< TInputPixelType, TOutputPixelType >
	//	::CalculateT0S0(int* toIndex, float* S0)
	//{

	//}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
		PerfusionMapFilter< TInputPixelType, TOutputPixelType >
		::PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os, indent);
		//os << indent << "NumOfComponents: " << m_NumOfComponents << std::endl;
	}

} // end namespace itk

#endif