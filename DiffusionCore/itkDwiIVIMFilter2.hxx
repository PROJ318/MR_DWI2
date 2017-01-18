#ifndef _itkDwiIVIMFilter2_hxx_
#define _itkDwiIVIMFilter2_hxx_

#include "itkDwiIVIMFilter2.h"
#include "itkComposeImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkAdcMapFilter.h"

namespace itk
{
	template< typename TInputPixelType, typename TOutputPixelType >
	DwiIVIMFilter2< TInputPixelType, TOutputPixelType >
	::DwiIVIMFilter2()
	{
		m_NumOfIterations = 80;
		m_GradientTolerence = 1e-5;
		m_ValueTolerence = 1e-5;
		m_EpsilonFunction = 1e-6;
		m_InputImage = ITK_NULLPTR;
		//m_TempImage = ITK_NULLPTR;
		m_OutputImage = ITK_NULLPTR;
	}

	/*template< typename TInputPixelType, typename TOutputPixelType >
	void
	DwiIVIMFilter2< TInputPixelType, TOutputPixelType >
	::BeforeThreadedGenerateData()
	{
		m_InputImage = this->GetInput();
		m_OutputImage = this->GetOutput();
		m_OutputImage->SetVectorLength(3);//IVIM: Only f, D and D* are required to output 
		m_OutputImage->Allocate();//Crutial.

		//typename TempImageType::RegionType region = this->GetInput()->GetLargestPossibleRegion();
		//m_TempImage->SetSpacing(this->GetInput()->GetSpacing());
		//m_TempImage->SetOrigin(this->GetInput()->GetOrigin());
		//m_TempImage->SetDirection(this->GetInput()->GetDirection());
		//m_TempImage->SetRegions(region);
		//m_TempImage->SetVectorLength(3);
		//m_TempImage->Allocate();
		//m_TempImage->DeepCopy(m_OutputImage);

	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
	DwiIVIMFilter2< TInputPixelType, TOutputPixelType >
	::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId)
	{
		if (m_BValueList.size() < 4) 
		{   
			std::cout << "Diffusion b valules less than 4, failed to calculate IVIM" << std::endl; 
			throw ExceptionObject(__FILE__, __LINE__, "Diffusion b valules less than 4, failed to calculate IVIM");
		}

		if ((m_InputImage->GetVectorLength() - 1)/ (m_BValueList.size() - 1) > 1)// replace with extract the last direction data in future
		{
			std::cout << "Diffusion Directions must be 1, failed to calculate IVIM" << std::endl;
			throw ExceptionObject(__FILE__, __LINE__, "Diffusion Directions must be 1, failed to calculate IVIM");
		}
		
		if (m_BValueList.at(m_BValueList.size() - 1) < 200)
		{
			std::cout << "Max b value must be larger than 200 s/mm2, failed to calculate IVIM" << std::endl;
			throw ExceptionObject(__FILE__, __LINE__, "Max b value must be larger than 200 s/mm2, failed to calculate IVIM");
		}

		//Now Input data is in the order of b0, b1, b2, b3...
		//Extract B0 image as predicted IVIM S0
		//Extract B0, Max B value for predictedD calculation
		typedef itk::ComposeImageFilter<itk::Image<TOutputPixelType, 3>>		ImageToVectorImageType;
		ImageToVectorImageType::Pointer imageToVectorImageFilter = ImageToVectorImageType::New();
		for (int i = 0; i < 2; i++)
		{			
			typedef itk::VectorIndexSelectionCastImageFilter <InputImageType, itk::Image<TOutputPixelType, 3>> VectorImageToImageType;
			VectorImageToImageType::Pointer vectorImageToImageFilter = VectorImageToImageType::New();
			vectorImageToImageFilter->SetIndex(i*(m_BValueList.size() - 1));
			vectorImageToImageFilter->SetInput(m_InputImage);
			vectorImageToImageFilter->Update();

			imageToVectorImageFilter->SetInput(i, vectorImageToImageFilter->GetOutput());
		}

		imageToVectorImageFilter->Update();//Extracted images
		std::vector<TOutputPixelType> tempBValueList;
		tempBValueList.push_back(m_BValueList.at(0));
		tempBValueList.push_back(m_BValueList.at(m_BValueList.size() - 1));

		typedef itk::AdcMapFilter <TOutputPixelType, TOutputPixelType> AdcMapFilterType;
		AdcMapFilterType::Pointer adcMap = AdcMapFilterType::New();
		adcMap->SetInput(imageToVectorImageFilter->GetOutput());
		adcMap->SetBValueList(tempBValueList);
		adcMap->Update();

		ConstInputIteratorType inputIt(m_InputImage, outputRegionForThread);
		ConstTempIteratorType extractedImageIt(imageToVectorImageFilter->GetOutput(), outputRegionForThread);
		ConstTempIteratorType adcMapIt(adcMap->GetOutput(), outputRegionForThread);
		OutputIteratorType outputIt(m_OutputImage, outputRegionForThread);

		VariableLengthVectorType IVIMResultVector;
		IVIMResultVector.SetSize(3);//IVIM: Only f, D and D* are required to output

		CostType::Pointer cost = CostType::New();
		CostType::ParametersType p(cost->GetNumberOfParameters());
		p[0] = 0.3;//predicted f
		p[1] = 0.079;//predicted DStar
		cost->SetBValueList(m_BValueList);

		OptimizerType::Pointer optimizer = OptimizerType::New();
		optimizer->SetNumberOfIterations(m_NumOfIterations);
		optimizer->SetValueTolerance(m_ValueTolerence);
		optimizer->SetGradientTolerance(m_GradientTolerence);
		optimizer->SetEpsilonFunction(m_EpsilonFunction);
		optimizer->UseCostFunctionGradientOff();
		optimizer->SetInitialPosition(p);

		inputIt.GoToBegin();
		extractedImageIt.GoToBegin();
		adcMapIt.GoToBegin();
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
				cost->SetPixelS0(extractedImageIt.Get()[0]);//predicted IVIM S0
				cost->SetPixelD(adcMapIt.Get()[1]);//predicted IVIM D
				optimizer->SetCostFunction(cost);
				optimizer->StartOptimization();


				// We estimated 2 parameters (f and DStar), D is calculated via ADC mapfilter.

				IVIMResultVector[0] = static_cast<TOutputPixelType>(DwiIVIMCutOff(0.0, 1.0, optimizer->GetCurrentPosition()[0]));
				IVIMResultVector[1] = static_cast<TOutputPixelType>(DwiIVIMCutOff(0.0, 1.0, optimizer->GetCurrentPosition()[1]));
				IVIMResultVector[2] = static_cast<TOutputPixelType>(adcMapIt.Get()[1]);				
			}
			outputIt.Set(IVIMResultVector);
			++outputIt;
			++inputIt;
			++extractedImageIt;
			++adcMapIt;
		}
	}*/

	//Multi-threadedGenerateData version goes wrong for 3D data, use below single thread generate data to solve this issue
	template< typename TInputPixelType, typename TOutputPixelType >
	void
	DwiIVIMFilter2< TInputPixelType, TOutputPixelType >
	::GenerateData(){
		m_InputImage = this->GetInput();
		m_OutputImage = this->GetOutput();
		m_OutputImage->SetSpacing(m_InputImage->GetSpacing());
		m_OutputImage->SetOrigin(m_InputImage->GetOrigin());
		m_OutputImage->SetDirection(m_InputImage->GetDirection());
		m_OutputImage->SetRegions(m_InputImage->GetLargestPossibleRegion());
		m_OutputImage->SetVectorLength(3);
		m_OutputImage->Allocate();


		if (m_BValueList.size() < 4)
		{
			std::cout << "Diffusion b valules less than 4, failed to calculate IVIM" << std::endl;
			throw ExceptionObject(__FILE__, __LINE__, "Diffusion b valules less than 4, failed to calculate IVIM");
		}

		if ((m_InputImage->GetVectorLength() - 1) / (m_BValueList.size() - 1) > 1)// replace with extract the last direction data in future
		{
			std::cout << "Diffusion Directions must be 1, failed to calculate IVIM" << std::endl;
			throw ExceptionObject(__FILE__, __LINE__, "Diffusion Directions must be 1, failed to calculate IVIM");
		}

		if (m_BValueList.at(m_BValueList.size() - 1) < 200)
		{
			std::cout << "Max b value must be larger than 200 s/mm2, failed to calculate IVIM" << std::endl;
			throw ExceptionObject(__FILE__, __LINE__, "Max b value must be larger than 200 s/mm2, failed to calculate IVIM");
		}

		//Now Input data is in the order of b0, b1, b2, b3...
		//Extract B0 image as predicted IVIM S0
		//Extract B0, Max B value for predictedD calculation
		typedef itk::ComposeImageFilter<itk::Image<TOutputPixelType, 3>>		ImageToVectorImageType;
		ImageToVectorImageType::Pointer imageToVectorImageFilter = ImageToVectorImageType::New();
		for (int i = 0; i < 2; i++)
		{
			typedef itk::VectorIndexSelectionCastImageFilter <InputImageType, itk::Image<TOutputPixelType, 3>> VectorImageToImageType;
			VectorImageToImageType::Pointer vectorImageToImageFilter = VectorImageToImageType::New();
			vectorImageToImageFilter->SetIndex(i*(m_BValueList.size() - 1));
			vectorImageToImageFilter->SetInput(m_InputImage);
			vectorImageToImageFilter->Update();

			imageToVectorImageFilter->SetInput(i, vectorImageToImageFilter->GetOutput());
		}

		imageToVectorImageFilter->Update();//Extracted images
		std::vector<TOutputPixelType> tempBValueList;
		tempBValueList.push_back(m_BValueList.at(0));
		tempBValueList.push_back(m_BValueList.at(m_BValueList.size() - 1));

		typedef itk::AdcMapFilter <TOutputPixelType, TOutputPixelType> AdcMapFilterType;
		AdcMapFilterType::Pointer adcMap = AdcMapFilterType::New();
		adcMap->SetInput(imageToVectorImageFilter->GetOutput());
		adcMap->SetBValueList(tempBValueList);
		adcMap->Update();

		ConstInputIteratorType inputIt(m_InputImage, m_InputImage->GetLargestPossibleRegion());
		ConstTempIteratorType extractedImageIt(imageToVectorImageFilter->GetOutput(), m_InputImage->GetLargestPossibleRegion());
		ConstTempIteratorType adcMapIt(adcMap->GetOutput(), m_InputImage->GetLargestPossibleRegion());
		OutputIteratorType outputIt(m_OutputImage, m_InputImage->GetLargestPossibleRegion());

		VariableLengthVectorType IVIMResultVector;
		IVIMResultVector.SetSize(3);//IVIM: Only f, D and D* are required to output

		CostType::Pointer cost = CostType::New();
		CostType::ParametersType p(cost->GetNumberOfParameters());
		p[0] = 0.3;//predicted f
		p[1] = 0.079;//predicted DStar
		cost->SetBValueList(m_BValueList);

		OptimizerType::Pointer optimizer = OptimizerType::New();
		optimizer->SetNumberOfIterations(m_NumOfIterations);
		optimizer->SetValueTolerance(m_ValueTolerence);
		optimizer->SetGradientTolerance(m_GradientTolerence);
		optimizer->SetEpsilonFunction(m_EpsilonFunction);
		optimizer->UseCostFunctionGradientOff();
		optimizer->SetInitialPosition(p);

		inputIt.GoToBegin();
		extractedImageIt.GoToBegin();
		adcMapIt.GoToBegin();
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
				cost->SetPixelS0(extractedImageIt.Get()[0]);//predicted IVIM S0
				cost->SetPixelD(adcMapIt.Get()[1]);//predicted IVIM D
				optimizer->SetCostFunction(cost);
				optimizer->StartOptimization();

				//std::cout << "Position: " << optimizer->GetCurrentPosition() << std::endl;
				// We estimated 2 parameters (f and DStar), D is calculated via ADC mapfilter.
				IVIMResultVector[0] = static_cast<TOutputPixelType>(DwiIVIMCutOff(0.0, 1.0, optimizer->GetCurrentPosition()[0]));
				IVIMResultVector[1] = static_cast<TOutputPixelType>(DwiIVIMCutOff(0.0, 1.0, optimizer->GetCurrentPosition()[1]));
				IVIMResultVector[2] = static_cast<TOutputPixelType>(adcMapIt.Get()[1]);
			}
			outputIt.Set(IVIMResultVector);
			++outputIt;
			++inputIt;
			++extractedImageIt;
			++adcMapIt;
		}
	}

	template< typename TInputPixelType, typename TOutputPixelType >
	void
	DwiIVIMFilter2< TInputPixelType, TOutputPixelType >
	::PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os, indent);
		//os << indent << "NumOfComponents: " << m_NumOfComponents << std::endl;
	}

} // end namespace itk

#endif
