#ifndef _itkDwiIVIMFilter_hxx_
#define _itkDwiIVIMFilter_hxx_

#include "itkDwiIVIMFilter.h"

namespace itk
{
	template< typename TInputPixelType, typename TOutputPixelType >
	DwiIVIMFilter< TInputPixelType, TOutputPixelType >
	::DwiIVIMFilter()
	{
		m_NumOfIterations = 80;
		m_GradientTolerence = 1e-5;
		m_ValueTolerence = 1e-5;
		m_EpsilonFunction = 1e-6;
		m_InputImage = ITK_NULLPTR;
		m_OutputImage = ITK_NULLPTR;
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
	DwiIVIMFilter< TInputPixelType, TOutputPixelType >
	::BeforeThreadedGenerateData()
	{
		m_InputImage = this->GetInput();
		//m_OutputImage->SetVectorLength(2 * m_NumOfDiffDirections);
		m_OutputImage = this->GetOutput();
		m_OutputImage->SetVectorLength(3);//IVIM: Only f, D and D* are required to output 
		m_OutputImage->Allocate();//Crutial.
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
	DwiIVIMFilter< TInputPixelType, TOutputPixelType >
	::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId)
	{
		if (m_BValueList.size() < 4) 
		{   
			std::cout << "Diffusion b valules less than 4, failed to calculate IVIM" << std::endl;
			throw ExceptionObject(__FILE__, __LINE__, "Diffusion b valules less than 4, failed to calculate IVIM");
		}
		
		ConstInputIteratorType inputIt(m_InputImage, outputRegionForThread);
		OutputIteratorType outputIt(m_OutputImage, outputRegionForThread);

		VariableLengthVectorType IVIMResultVector;
		IVIMResultVector.SetSize(3);//IVIM: Only f, D and D* are required to output

		CostType::Pointer cost = CostType::New();
		CostType::ParametersType p(cost->GetNumberOfParameters());
		p[0] = 1000;
		p[1] = 0.3;
		p[2] = 0.001;
		p[3] = 0.079;
		cost->SetBValueList(m_BValueList);

		OptimizerType::Pointer optimizer = OptimizerType::New();
		optimizer->SetNumberOfIterations(m_NumOfIterations);
		optimizer->SetValueTolerance(m_ValueTolerence);
		optimizer->SetGradientTolerance(m_GradientTolerence);
		optimizer->SetEpsilonFunction(m_EpsilonFunction);
		optimizer->UseCostFunctionGradientOff();
		optimizer->SetInitialPosition(p);

		inputIt.GoToBegin();
		outputIt.GoToBegin();
		while (!inputIt.IsAtEnd())
		{
			// skip mask value and zero value
			bool skip = true;
			for (unsigned int i = 0; i < m_InputImage->GetVectorLength(); i++)
			{
				if (inputIt.Get()[i] != 0)
				{
					skip = false;
					break;
				}
			}

			if (skip)
			{
				IVIMResultVector.Fill(0);
			}
			else
			{
				cost->SetPixelArray(inputIt.Get());
				optimizer->SetCostFunction(cost);
				optimizer->StartOptimization();

				//std::cout << "Position: " << optimizer->GetCurrentPosition() << std::endl;
				// We estimated 4 parameters, but discarded the first S0 estimation result
				for (int j = 0; j < 3; j++)
				{
					IVIMResultVector[j] = optimizer->GetCurrentPosition()[j + 1];
				}
			}
			outputIt.Set(IVIMResultVector);
			++outputIt;
			++inputIt;
		}
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
	DwiIVIMFilter< TInputPixelType, TOutputPixelType >
	::PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os, indent);
		//os << indent << "NumOfComponents: " << m_NumOfComponents << std::endl;
	}

} // end namespace itk

#endif
